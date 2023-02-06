struct PixelShaderInput
{
	min16float4 pos         : SV_POSITION;
	min16float2 texCoord    : TEXCOORD0;
};

Texture2D<float>  luminanceChannel   : t0;
Texture2D<float2> chrominanceChannel : t1;
SamplerState      defaultSampler     : s0;

static const float3x3 YUVtoRGBCoeffMatrix =
{
	1.164383f,  1.164383f, 1.164383f,
	2.017232f, -0.391762f, 0.000000f,
	0.000000f, -0.812968f, 1.596027f
};

float3 ConvertYUVtoRGB(float3 yuv)
{
	yuv -= float3(0.062745f, 0.501960f, 0.501960f);
	yuv = mul(yuv, YUVtoRGBCoeffMatrix);

	return saturate(yuv);
}

min16float4 ps_main(PixelShaderInput input) : SV_TARGET
{
	float y = luminanceChannel.Sample(defaultSampler, input.texCoord);
	float2 uv = chrominanceChannel.Sample(defaultSampler, input.texCoord);

	return min16float4(ConvertYUVtoRGB(float3(y, uv)), 1.f);
}