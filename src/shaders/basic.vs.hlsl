struct Input
{
	// Position is using TEXCOORD semantic because of rules imposed by SDL
	// Per https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader#remarks
	float3 Position : TEXCOORD0; 
	float4 Color : TEXCOORD1;
};

struct Output
{
	float4 Color : TEXCOORD0;
	float4 Position : SV_Position;
};

Output main(Input input)
{
	Output output;
	output.Color = input.Color;
	output.Position = float4(input.Position, 1.0f);
	return output;
}
