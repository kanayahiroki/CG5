#include "Test.hlsli"

Texture2D<float32_t4> gTexture : register(t0); // SRV register => t
SamplerState gSampler : register(s0); // Sampler register => s



PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // ★【訂正】UV座標を使ってテクスチャの色をサンプリング
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);

    output.color = textureColor; // non effect

    return output;
}