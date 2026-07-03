#pragma once

#include <d3d12.h> // ID3D12RootSignature

class RootSignature {
public:
	void Create();

	// ゲッター
	ID3D12RootSignature* Get();

	// コンストラクタ
	RootSignature();

	// デストラクタ
	~RootSignature();

private:
	ID3D12RootSignature* rootSignature_ = nullptr;
};