#pragma once

#include <string> //wstring, string
#include <d3d12.h> //ID3DBlod

class Shader {
public:

    //シェーダーファイルを読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::string& shaderModel);

	ID3DBlob* GetBlob();

    // 生成したコンパイル済みデータを取得する
	ID3DBlob* GetBlob();


    // コンパイル
    Shader();
	// デストラクタ
    ~Shader();

private:
	ID3DBlob* blob_ = nullptr;//コンストラクタで初期化しないでいい
};