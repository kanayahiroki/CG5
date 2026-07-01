#include "Shader.h"
#include <d3dcompiler.h> // D3DCompileFromFile関数を使うために必要
#include <cassert>      // assert関数を使うために必要


// シェーダーファイルを読み込み、コンパイルする
void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダーモデル指定
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
	// コンパイル済みのデータを返す　＊未コンパイルの場合はnullptrとなる
	ID3DBlob* Shader::GetBlob(){ return blob_; }

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
    }