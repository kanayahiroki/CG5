#include "VertexBuffer.h"
#include "kamataEngine.h"

#include <cassert>
#include <d3d12.h>

using namespace KamataEngine;

VertexBuffer::~VertexBuffer() {}



void VertexBuffer::Create(const UINT size, const UINT stride) 
{
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//頂点リソースの生成
	//  頂点バッファのリソースを作成する
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};	
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
}

