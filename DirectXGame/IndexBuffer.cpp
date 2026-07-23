#include "indexBuffer.h"
#include "KamataEngine.h"

#include <cassert>
#include <d3d11.h>

using namespace KamataEngine;

//生成
void IndexBuffer::Create(const UINT size, const UINT stride) 
{
	//strideの値によって、1つのインデックスフォーマットを作成する
	assert(stride == 2 || stride == 4);
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	//クラス内でdxCommonを利用するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//インデックスリソースを生成
	//インデックスリソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;  //CPUから書き込むヒープ

	//インデックスリソースの設定
	D3D12_RESOURCE_DESC indexResourceDesc{};
	indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexResourceDesc.Width = size;

	//バッファの場合はこれらは１にする決まり
	indexResourceDesc.Height = 1;
	indexResourceDesc.DepthOrArraySize = 1;
	indexResourceDesc.MipLevels = 1;
	indexResourceDesc.SampleDesc.Count = 1;

	//バッファの場合はこれにする決まり
	indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際にインデックスリソースを生成する
	ID3D12Resource* indexResource = nullptr;

	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexResource));

	assert(SUCCEEDED(hr));

	//生成したインデックスリソースをっとっておく
	indexBuffer_ = indexResource;

	//indexBUfferViewを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	//リソースの先頭アドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するインデックスデータの全サイズ
	indexBufferView.SizeInBytes = size;
	//インデックスのフォーマット
	indexBufferView.Format = format; //インデックス一つ分のサイズ
	
	//indexBufferViewを取っておく
	indexBufferView_ = indexBufferView;
	
}

//生成したインデックスバッファビューを返す
ID3D12Resource* IndexBuffer::Get() 
{ return indexBuffer_; }

//用意済みのインデックスバッファービューを返す
D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() 
{ return &indexBufferView_; }

//コンストラクタ
IndexBuffer::IndexBuffer() {}

//デストラクタ
IndexBuffer::~IndexBuffer() 
{
	if (indexBuffer_) 
	{
		indexBuffer_->Release();
		indexBuffer_ = nullptr;
	}
}