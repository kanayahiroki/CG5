struct vertexShaderOutput
{
	float3 position : POSITION;
};


struct vertexShaderInput
{
	float3 position : POSITION;
};

vertexShaderOutput main(vertexShaderInput input)
{
	vertexShaderOutput output;
	output.position = input.position;
	return output;
}