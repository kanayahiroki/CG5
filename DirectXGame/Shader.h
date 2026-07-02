#pragma once

#include <string> //wstring, string

#include <d3dcompiler.h> //D3DCompileFromFile関数を使うために必要
#include <dxcapi.h> //IDxcBlob


class Shader {
public:

    //シェーダーファイルを読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::wstring& shaderModel);
	void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);

    // 生成したコンパイル済みデータを取得する
	ID3DBlob* GetBlob();
	IDxcBlob* GetDxcBlob();


    // コンパイル
    Shader();
	// デストラクタ
    ~Shader();

private:
	ID3DBlob* blob_ = nullptr;//コンストラクタで初期化しないでいい
	IDxcBlob* dxcBlob_ = nullptr;//コンストラクタで初期化しないでいい
};