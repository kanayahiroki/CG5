struct PixelShaderOutput
{
	float4 position : SV_POSITION;
};

PixelShaderOutput main()
{
	PixelShaderOutput output;
	output.position = float4(1.0f, 0.0f, 0.0f, 1.0f);
	return output;
}