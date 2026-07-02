#include "Shader.h"
#include "MissUtility.h"
#include <cassert>       // assert関数を使うために必要
#include <d3dcompiler.h> // D3DCompileFromFile関数を使うために必要
#include <dxcapi.h>
#pragma comment(lib, "d3dcompiler.lib") // D3DCompileFromFile関数を使うために必要
#pragma comment(lib, "dxcompiler.lib")  // DxcCreateInstance関数を使うために必要

// シェーダーファイルを読み込み、コンパイルする
void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	std::string mdShaderModel = ConvertString(shaderModel);

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", mdShaderModel.c_str(),                   // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &shaderBlob, &errorBlob);
	// エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}
	// 生成したshaderを取っておく
	blob_ = shaderBlob;
}

// シェーダーファイルを読み込み、コンパイルする
// 外部コンパイラ版　シェーダーモデル　6.0以上を使う場合はこちらを使う
void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) {
	static IDxcUtils* dxcUtils = nullptr;
	static IDxcCompiler3* dxcCompiler = nullptr;
	static IDxcIncludeHandler* includeHandler = nullptr;

	HRESULT hr;

	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr)); // dxcUtilsの生成に失敗した場合、止める
	}

	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr)); // dxcCompilerの生成に失敗した場合、止める
	}

	if (includeHandler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
		assert(SUCCEEDED(hr)); // includeHandlerの生成に失敗した場合、止める
	}

	// 1. hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr)); // ファイルの読み込みに失敗した場合、止める

	// 読み込んだファイルの内容をDxcBufferに設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2. コンパイルする
	//    コンパイルに必要なコンパイルオプションの準備
	LPCWSTR arguments[] = {
	    L"-E",
	    L"main", // エントリーポイントの指定。基本的にmain以外にしない
	    L"-T",
	    shaderModel.c_str(), // ShaderProfileの指定。例) vs_6_0, ps_6_0, cs_6_0
	    L"-Zi",
	    L"-Qembed_debug", // デバッグ用の情報を埋め込む
	    L"-Od",           // 最適化を外す
	    L"-Zpr",          // メモリレイアウトを行優先に指定
	    L"-I",
	    L"Resources/shaders", // hlsliファイルを探すパスを入れておく

	};

	//  実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
	    &shaderSourceBuffer,            // 読み込んだファイル
	    arguments,                      // コンパイルオプション
	    _countof(arguments),            // コンパイルオプションの数
	    includeHandler,                 // incloudeが含まれた諸々
	    IID_PPV_ARGS(&shaderResult)     // コンパイル結果
	);

	//  コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	// 3. 警告・エラーが出ていないかを確認する
	IDxcBlobUtf8* shaderError = nullptr;
	IDxcBlobWide* nameBlob = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) 
	{
		OutputDebugStringA(shaderError->GetStringPointer());
		assert(false);
	}

	// 4. コンパイル結果を受け取る
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
	assert(SUCCEEDED(hr));

	//もう使わないリソースを開放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用のバイナリを取っておく
	dxcBlob_ = shaderBlob;

}

// コンパイル済みのデータを返す　＊未コンパイルの場合はnullptrとなる
ID3DBlob* Shader::GetBlob() { return blob_; }


IDxcBlob* Shader::GetDxcBlob() { return dxcBlob_; }
// コンストラクタ
Shader::Shader() {
	// blob_はnullptrで初期化される
}

// デストラクタ
Shader::~Shader() {
	// blob_がnullptrでなければ解放する
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
	if (dxcBlob_ != nullptr) {
		dxcBlob_->Release();
		dxcBlob_ = nullptr;
	}
}