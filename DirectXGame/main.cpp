#include "KamataEngine.h"
#include "Shader.h"
#include <Windows.h>
#include <cassert>
//#include <d3dcompiler.h>
#include "RootSignature.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace KamataEngine;

// 関数プロトタイプ宣言
//ID3D10Blob* CompileShader(const std::wstring& filePath, const std::string& shaderModel);

// PipelineStateObjectの生成
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

// ★ここに追記します！ (RenderTextureResourceの生成)
ID3D12Resource* CreateRenderTextureResource(
	ID3D12Device* device, uint32_t width, uint32_t height, 
	DXGI_FORMAT format, const FLOAT* clearColor);
// DepthStencilTextureResourceの生成
ID3D12Resource* CreateDepthStencilTextureResource(
	ID3D12Device* device, int32_t width, int32_t height);

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) 
{
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; // Vector2なので R32G32
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState ------------------------ 今回は不透明
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	// RasterizerState -------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};	
	// 裏面(反時計回り)をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする(ワイヤーフレームなら D3D12_FILL_MODE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	// PSO(PipelineStateObject)の生成 -------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayput
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;   // RasterizerState


	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトボロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(今は気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// PSOを作成
	pipelineState.Create(graphicsPipelineStateDesc);

}

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor) {
	// 1. 生成するRenderTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                             // RenderTextureの幅
	resourceDesc.Height = UINT(height);                           // RenderTextureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数
	resourceDesc.DepthOrArraySize = 1;                            // 奥行 or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // Textureの次元数。普段使っているのは 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使う通知

	// 2. 利用するHeapの設定 (3/4)
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 3. ClearValueの用意 (3/4)
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4. RenderTextureResourceの生成 (4/4)
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                            // Heapの設定
	    D3D12_HEAP_FLAG_NONE,                       // Heapの特殊な設定
	    &resourceDesc,                              // Resourceの設定
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // Pixel Shader でアクセスできるようにする
	    &clearValue,                                // Clear最適値
	    IID_PPV_ARGS(&resource)                     // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}

// DepthStencilTextureの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
	// 1. 生成するDepthStencilTextureの Descの設定 (2/4)
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                   // Textureの幅
	resourceDesc.Height = height;                                 // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数 DepthStencilなので 1つでいい
	resourceDesc.DepthOrArraySize = 1;                            // Textureの配列数 DepthStencilは 1つでいい
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;                  // DepthStencilとして利用可能なフォーマット ※KamataEngineと合わせる
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 2. 利用するHeapの設定 (3/4)
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;      // 1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // Zバッファ形式、resourceと合わせる

	// 3. Resourceの生成 (4/4)
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                  // Heapの設定
	    D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定
	    &resourceDesc,                    // Resourceの設定
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
	    &depthClearValue,                 // Clear最適値
	    IID_PPV_ARGS(&resource)           // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	KamataEngine::Initialize(L"LE3D_08_カナヤ_ヒロキ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width: {}, height: {}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignature作成　-------------------------------------
	// 構造体にデータを用意する
	RootSignature rs;
	rs.Create();

	//// InputLayout
	// D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	// inputElementDescs[0].SemanticName = "POSITION";
	// inputElementDescs[0].SemanticIndex = 0;
	// inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	// inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	// D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// inputLayoutDesc.pInputElementDescs = inputElementDescs;
	// inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//// BlendState ------------------------ 今回は不透明
	// D3D12_BLEND_DESC blendDesc{};
	//// 全ての色要素を書き込む
	// blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//// RasterizerState -------------------
	// D3D12_RASTERIZER_DESC rasterizerDesc{};
	//// 裏面(反時計回り)をカリングする
	// rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//// 塗りつぶしモードをソリッドにする(ワイヤーフレームなら D3D12_FILL_MODE_WIREFRAME)
	// rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// 頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	//// コンパイル済みのShader、エラー時情報の格納場所の用意
	// ID3DBlob* vsBlob = nullptr;
	// ID3DBlob* psBlob = nullptr;
	// ID3DBlob* errorBlob = nullptr;

	//// 頂点シェーダーの読み込みとコンパイル
	// std::wstring vsFile = L"Resources/shaders/TestVS.hlsl";
	// hr = D3DCompileFromFile(
	//     vsFile.c_str(), // シェーダーファイル名
	//     nullptr,
	//     D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	//     "main", "vs_5_0",                                // エントリーポイント名、シェーダーモデル指定
	//     D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	//     0, &vsBlob, &errorBlob);
	// if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}
	//	assert(false);
	// }

	//// ピクセルシェーダーの読み込みとコンパイル
	// std::wstring psFile = L"Resources/shaders/TestPS.hlsl";
	// hr = D3DCompileFromFile(
	//     psFile.c_str(), // シェーダーファイル名
	//     nullptr,
	//     D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	//     "main", "ps_5_0",                                // エントリーポイント名、シェーダーモデル指定
	//     D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	//     0, &psBlob, &errorBlob);
	// if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}
	//	assert(false);
	//	return 0;
	// }

	//// PSO(PipelineStateObject)の生成 -------------
	//
	// D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	// graphicsPipelineStateDesc.pRootSignature = rs.Get();                             // RootSignature
	// graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                              // InputLayput
	// graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	// graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	// graphicsPipelineStateDesc.BlendState = blendDesc;                                     // BlendState
	// graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                           // RasterizerState
	// 書き込むRTVの情報

	// graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む
	// graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//// 利用するトボロジ(形状)のタイプ。三角形
	// graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//// どのように画面に色を打ち込むかの設定(今は気にしなくていい)
	// graphicsPipelineStateDesc.SampleDesc.Count = 1;
	// graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//// PSOを作成
	// ID3D12PipelineState* graphicsPipeLineState = nullptr;
	// HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipeLineState));
	// assert(SUCCEEDED(hr));

	// PinelineState作成
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);

	// リソースの確保含め、頂点情報を柔軟に対応できるようにVertexData構造体多新たに作成する
	// Vertex ⇒　VertexDataに変換して利用する
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	// 頂点データの準備  00_07追加
	VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, //  左上
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, //  右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, //  左下
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, //  右下
	};

	// VertexBuffer(VertexResource, VertexBufferView)の生成
	VertexBuffer vb;
	// vb.Create(sizeof(Vector4) * 3, sizeof(Vector4));
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	// 頂点リソースにデータを書き込む　　00_07追加
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	uint16_t indices[] = {
	    0, 1, 2, 2, 1, 3,
	};

	// indexBuffer(IndexResource, IndexResorceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点インデックスリソースにデータを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {

		pGpuIndices[i] = indices[i];
	}

	// RenderTexture関係 ★00_09 追加
	// =========================================================

	// Resource生成、Heap生成、View生成 で再利用される変数の準備
	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	// 0. RenderTextureResourceの作成 (1/3)
	// 画面クリア色 ※分かりやすいように赤とする
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	//----------------------------------------------------------
	// 1. RTV用の DescriptorHeapを作成する (2/3)
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // Descriptorの個数は 1

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//----------------------------------------------------------
	// 2. RTV用の Viewの生成 (3/3)
	device->CreateRenderTargetView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    nullptr,               // RTVの詳細情報(nullptrにするとDirectX12が自動で推測してくれる)
	    rtvHandleCPU           // RTV用ディスクリプタヒープの CPU Handle
	);

	// =========================================================
	// DepthStencilTexture関係 ★00_09 追加
	// =========================================================

	//----------------------------------------------------------
	// 0. DepthStencilTextureResourceの作成 (1/3)
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	//----------------------------------------------------------
	// 1. DSV用の DescriptorHeapの作成 (2/3)
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   // Heap Type
	dsvDescriptorHeapDesc.NumDescriptors = 1;                      // Heap Type の個数
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // DSVは Shaderで触らないとする

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//----------------------------------------------------------
	// 2. DSV用の Viewの生成 (3/3)
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;                // 基本的にResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2D Texture

	// DSVHeapの先頭に DSVを作る
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);

	// =========================================================
	// SRV(Shader Resource View)を準備する ★00_09 追加
	// ※ PixelShaderと連携をとるようにするため
	// =========================================================

	//----------------------------------------------------------
	// 1. SRV用の DescriptorHeapの作成 (1/2)
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // PixelShader から見える
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLE、GPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	//----------------------------------------------------------
	// 2. SRV(Shader Resource View)の作成 (2/2)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                           // RenderTargetResource と同じにする
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // RGBA値をそのまま Shaderに対応させる
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                      // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;                                            // MipLevel は 1 しかない

	device->CreateShaderResourceView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    &srvDesc,              // SRVの詳細情報
	    srvHandleCPU           // SRV用ディスクリプタヒープの CPU Handle
	);

	//// VertexResouceの生成 -------------------
	//// 頂点リソース用ヒープの設定
	//
	// D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	// uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	//
	//// 頂点リソースの設定
	//
	// D3D12_RESOURCE_DESC vertexResourceDesc{};
	// vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	// vertexResourceDesc.Width = sizeof(Vector4) * 3;
	//
	//// バッファの場合、これらは1にする決まり
	// vertexResourceDesc.Height = 1;
	// vertexResourceDesc.DepthOrArraySize = 1;
	// vertexResourceDesc.MipLevels = 1;
	// vertexResourceDesc.SampleDesc.Count = 1;
	//
	//// バッファの場合はこれにする決まり
	// vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//
	//// 実際に頂点リソースを生成する
	// ID3D12Resource* vertexResource = nullptr;
	// HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	// IID_PPV_ARGS(&vertexResource)); assert(SUCCEEDED(hr)); // うまくいかなかったときは起動できない
	//
	//// VertexBufferViewを作成する ----------
	// D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//// リソースの先頭アドレスから使う
	// vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//// 使用するリソースのサイズは頂点3つ分のサイズ
	// vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	//// 1つの頂点のサイズ
	// vertexBufferView.StrideInBytes = sizeof(Vector4);

	//// 頂点リソースにデータを書き込む --------------
	// Vector4* vertexData = nullptr;
	// vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f}; // 左下
	// vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};   // 上
	// vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};  // 右下
	//  頂点リソースのマップを解除する
	// vd.Get()->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// =========================================================
		// 1. レンダーテクスチャへの描画処理（オフスクリーン）
		// =========================================================

		// TransitionBarrier: SRV ⇒ RTV
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrier);

		// 描画先をレンダーテクスチャに切り替え
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// Viewport & Scissor 設定
		D3D12_VIEWPORT viewport{0.0f, 0.0f, (float)WinApp::kWindowWidth, (float)WinApp::kWindowHeight, 0.0f, 1.0f};
		D3D12_RECT scissorRect{0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight};
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		// レンダーテクスチャを「赤色」でクリア（★ここが背景の赤になります）
		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// ※パス1ではテクスチャを描画ターゲットにしているため、ここではポリゴン描画(Draw)を行わずに
		//  赤色クリアのみ保持します（または別シェーダーで3Dモデルを描く場所です）

		// TransitionBarrier: RTV ⇒ SRV (シェーダーから読める状態に戻す)
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);

		// =========================================================
		// 2. 画面（フレームバッファ）への描画処理
		// =========================================================

		// 画面描画開始（画面クリア＆バックバッファのRTV設定）
		dxCommon->PreDraw();

		// ルートシグネチャ・PSO・描画波形のセット
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(pipelineState.Get());

		commandList->IASetVertexBuffers(0, 1, vb.GetView());
		commandList->IASetIndexBuffer(ib.GetView());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 赤く塗ったレンダーテクスチャ(SRV)をセット
		ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap};
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		// 画面いっぱいのポリゴンを描画（レンダーテクスチャの赤色が画面に貼り付く）
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了・画面表示
		dxCommon->PostDraw();
	}
}
//// シェーダーコンパイル関数
////  filePath    : シェーダーファイルのパス    例) L"Resources/shaders/TestVS.hlsl"
////  shaderModel : シェーダーモデル          例) "vs_5_0"
//ID3DBlob* CompileShader(const std::wstring& filePath, const std::string& shaderModel) {
//
//	ID3DBlob* shaderBlob = nullptr;
//	ID3DBlob* errorBlob = nullptr;
//
//	HRESULT hr = D3DCompileFromFile(
//	    filePath.c_str(), // シェーダーファイル名
//	    nullptr,
//	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
//	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダモデル指定
//	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
//	    0, &shaderBlob, &errorBlob);
//	// エラーが発生した場合、止める
//	if (FAILED(hr)) {
//		if (errorBlob) {
//			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
//			errorBlob->Release();
//		}
//		assert(false);
//	}
//
//	// 生成したshaderBlobを返す
//	return shaderBlob;
//}