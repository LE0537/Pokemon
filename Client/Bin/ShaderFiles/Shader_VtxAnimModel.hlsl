#include "Client_Shader_Defines.hpp"

matrix			g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D		g_DiffuseTexture;
texture2D		g_DissolveTexture;

matrix			g_BoneMatrices[256];

float			g_DissolveTimer;
float			g_DissolveLifespan = 1.f;


struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float3		vTangent : TANGENT;
	uint4		vBlendIndex : BLENDINDEX;
	float4		vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};


/* DrawIndexed함수를 호출하면. */
/* 인덱스버퍼에 담겨있는 인덱스번째의 정점을 VS_MAIN함수에 인자로 던진다. VS_IN에 저장된다. */
/* 일반적으로 TriangleList로 그릴경우, 정점 세개를 각각 VS_MAIN함수의 인자로 던진다. */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	float		fW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

	matrix		BoneMatrix = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
		g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
		g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
		g_BoneMatrices[In.vBlendIndex.w] * fW;

	vector		vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
	vector		vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
	vNormal = normalize(mul(vNormal, g_WorldMatrix));

	/* 정점의 위치에 월드 뷰 투영행렬을 곱한다. 현재 정점은 ViewSpace에 존재하낟. */
	/* 투영행렬까지 곱하면 정점위치의 w에 뷰스페이스 상의 z를 보관한다. == Out.vPosition이 반드시 float4이어야하는 이유. */
	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = vNormal;
	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float4		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;

};
struct PS_OUT_DISSOLVE
{
	float4      vDiffuse : SV_TARGET0;
	float4      vNormal : SV_TARGET1;
	float4      vDepth : SV_TARGET2;
	float4      vDissolve : SV_TARGET3;
};
struct PS_OUT_SHADOW
{
	float4			vLightDepth :  SV_TARGET0;
};

/* 이렇게 만들어진 픽셀을 PS_MAIN함수의 인자로 던진다. */
/* 리턴하는 색은 Target0 == 장치에 0번째에 바인딩되어있는 렌더타겟(일반적으로 백버퍼)에 그린다. */
/* 그래서 백버퍼에 색이 그려진다. */

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1300.f, 0.f, 0.f);

	if (Out.vDiffuse.a <= 0.3f)
		discard;

	return Out;
}
PS_OUT_DISSOLVE PS_DISSOLVE(PS_IN In)
{
	PS_OUT_DISSOLVE      Out = (PS_OUT_DISSOLVE)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 1.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1300.f, 0.f, 0.f);

	if (Out.vDiffuse.a <= 0.3f)
		discard;

	/* Dissolve with Highlight */
	float dissolveFactor = lerp(0, 1, g_DissolveTimer / g_DissolveLifespan);

	/* If dissolveFactor:
	== 0:   Should not Dissolve
	== 1:   Should Dissolve Everything. */

	float4 dissolveColor = g_DissolveTexture.Sample(LinearSampler, In.vTexUV);
	dissolveColor.a = dissolveColor.y;
	dissolveColor.yz = dissolveColor.x;

	float dissolveValue = dissolveColor.r - dissolveFactor;

	/* If dissolveValue:
	> .15:      No Dissolve
	0 ~ .15f:   Highlight
	<= 0:      Dissolve. */


	if (dissolveValue <= 0)
		discard;
	else if (dissolveValue < 0.15f)
	{
		float3 colorYellow = float3(1.f, 0.95f, 0.6f);
		float3 colorOrange = float3(0.92f, 0.36f, 0.2f);

		float3 lerpColor = lerp(colorOrange, colorYellow, dissolveValue / 0.15f);
		Out.vDissolve.rgb = lerpColor;
	}

	return Out;
}
PS_OUT_SHADOW PS_Shadow(PS_IN In)
{
	PS_OUT_SHADOW		Out = (PS_OUT_SHADOW)0;

	Out.vLightDepth.x = In.vProjPos.w / 1300.f;

	Out.vLightDepth.a = 1.f;

	return Out;
}
PS_OUT PS_Evolution(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	
	Out.vDiffuse = 1.f;
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1300.f, 0.f, 0.f);

	return Out;
}
PS_OUT PS_Ball(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexUV);
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 3.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1300.f, 0.f, 0.f);

	if (Out.vDiffuse.a <= 0.3f)
		discard;

	return Out;
}
technique11 DefaultTechnique
{
	pass Default
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Evolution //1
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Evolution();
	}
	pass Ball // 2
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Ball();
	}
	pass Shadow //3
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Shadow, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Shadow();
	}
	pass Dissolve   // 4
	{
		SetRasterizerState(RS_Default);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 1.f), 0xffffffff);
		SetDepthStencilState(DSS_Default, 0);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DISSOLVE();
	}
}