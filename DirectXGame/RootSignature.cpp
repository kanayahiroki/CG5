#include "RootSignature.h"
#include "KamataEngine.h"
#include <cassert>

using namespace KamataEngine;

// ルートシグネチャの生成
void RootSignature::Create() 
{
	// ルートシグネチャの設定
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 構造体のデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) 
	{
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に作成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// signatureはRootSignatureの生成後開放して良い
	signatureBlob->Release();

	// 生成したルートシグネチャを取っておく
	rootSignature_ = rootSignature;
}

// 生成したarootSignatureをに返す
ID3D12RootSignature* RootSignature::Get() { return rootSignature_; }

// コンストラクタ
RootSignature::RootSignature() {}

// デストラクタ
RootSignature::~RootSignature() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}
