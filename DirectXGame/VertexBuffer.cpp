#include "VertexBuffer.h"
#include "kamataEngine.h"

#include <cassert>
#include <d3d12.h>

using namespace KamataEngine;


void VertexBuffer::Create(const UINT size, const UINT stride) {
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();


	// 頂点リソースの設定
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファリソース
	vertexResourceDesc.Width = size;                                // 頂点バッファのサイズ
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを生成する
	ID3D12Resource* vertexBuffer = nullptr;

	// HRESULT　追加
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties,             // ヒープ設定
	    D3D12_HEAP_FLAG_NONE,              // ヒープフラグ
	    &vertexResourceDesc,               // リソース設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // リソースの使用状態
	    nullptr,                           // クリア最適値
	    IID_PPV_ARGS(&vertexBuffer));
	assert(SUCCEEDED(hr)); // 生成するリソースポインタのアドレス

	// 生成したリソースを取っておく
	vertexBuffer_ = vertexBuffer;

	// VertexBufferViewの作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// 作成した頂点バッファの情報を設定
	vertexBufferView.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	// 作成した頂点バッファのサイズを設定
	vertexBufferView.SizeInBytes = size;
	// 作成した頂点バッファの1頂点分のサイズを設定
	vertexBufferView.StrideInBytes = stride;

	// 作成したVertexbufferViewをメンバ変数に格納
	vertexBufferView_ = vertexBufferView;
}

// 生成した頂点バッファを返す
ID3D12Resource* VertexBuffer::Get() { return vertexBuffer_; }

// 用意済みの頂点バッファビューを返す
D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() { return &vertexBufferView_; }

// コンストラクタ
VertexBuffer::VertexBuffer() {}
// デストラクタ
VertexBuffer::~VertexBuffer() {
	if (vertexBuffer_) {
		vertexBuffer_->Release();
		vertexBuffer_ = nullptr;
	}
}