#include "Test.hlsli"


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float32_t4(input.texcoord.x, input.texcoord.y, 1.0f, 1.0f);
    return output;
}