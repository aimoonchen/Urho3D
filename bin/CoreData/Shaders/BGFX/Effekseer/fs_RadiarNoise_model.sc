$input v_VColor, v_UV1, v_UV2, v_WorldN_PX, v_WorldB_PY, v_WorldT_PZ, v_PosP, v_CustomData1, v_CustomData2
#include "../bgfx_shader.sh"
#define FRAGCOLOR gl_FragColor
#define MOD mod
#define FRAC fract
#define LERP mix

//float atan2(in float y, in float x) {
//    return x == 0.0 ? sign(y)* 3.141592 / 2.0 : atan(y, x);
//}



#define TEX2D texture2D

SAMPLER2D(efk_texture_343, 0);
SAMPLER2D(efk_texture_901, 1);
SAMPLER2D(efk_texture_904, 2);
SAMPLER2D(efk_background, 3);
SAMPLER2D(efk_depth, 4);


uniform vec4 mUVInversedBack;
uniform vec4 fs_predefined_uniform;
uniform vec4 fs_cameraPosition;
uniform vec4 reconstructionParam1;
uniform vec4 reconstructionParam2;

uniform vec4 fs_efk_uniform_380;
uniform vec4 fs_efk_uniform_463;
uniform vec4 fs_efk_uniform_508;
uniform vec4 fs_efk_uniform_709;
uniform vec4 fs_efk_uniform_1065;


vec2 GetUV(vec2 uv)
{
	uv.y = mUVInversedBack.x + mUVInversedBack.y * uv.y;
	return uv;
}

vec2 GetUVBack(vec2 uv)
{
	uv.y = mUVInversedBack.z + mUVInversedBack.w * uv.y;
	return uv;
}

float CalcDepthFade(vec2 screenUV, float meshZ, float softParticleParam)
{
	float backgroundZ = TEX2D(efk_depth, GetUVBack(screenUV)).x;

	float distance = softParticleParam * fs_predefined_uniform.y;
	vec2 rescale = reconstructionParam1.xy;
	vec4 params = reconstructionParam2;

	vec2 zs = vec2(backgroundZ * rescale.x + rescale.y, meshZ);

	vec2 depth = (zs * params.w - params.y) / (params.x - zs * params.z);
	float dir = sign(depth.x);
	depth *= dir;
	return min(max((depth.x - depth.y) / distance, 0.0), 1.0);
}

#ifdef _MATERIAL_LIT_

const float lightScale = 3.14;

float saturate(float v)
{
	return max(min(v, 1.0), 0.0);
}

float calcD_GGX(float roughness, float dotNH)
{
	float alpha = roughness*roughness;
	float alphaSqr = alpha*alpha;
	float pi = 3.14159;
	float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0;
	return (alpha / denom) * (alpha / denom) / pi;
}

float calcF(float F0, float dotLH)
{
	float dotLH5 = pow(1.0-dotLH,5.0);
	return F0 + (1.0-F0)*(dotLH5);
}

float calcG_Schlick(float roughness, float dotNV, float dotNL)
{
	// UE4
	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	// float k = roughness * roughness / 2.0;

	float gV = dotNV*(1.0 - k) + k;
	float gL = dotNL*(1.0 - k) + k;

	return 1.0 / (gV * gL);
}

float calcLightingGGX(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
	vec3 H = normalize(V+L);

	float dotNL = saturate( dot(N,L) );
	float dotLH = saturate( dot(L,H) );
	float dotNH = saturate( dot(N,H) ) - 0.001;
	float dotNV = saturate( dot(N,V) ) + 0.001;

	float D = calcD_GGX(roughness, dotNH);
	float F = calcF(F0, dotLH);
	float G = calcG_Schlick(roughness, dotNV, dotNL);

	return dotNL * D * F * G / 4.0;
}

vec3 calcDirectionalLightDiffuseColor(vec3 diffuseColor, vec3 normal, vec3 lightDir, float ao)
{
	vec3 color = vec3(0.0,0.0,0.0);

	float NoL = dot(normal,lightDir);
	color.xyz = lightColor.xyz * lightScale * max(NoL,0.0) * ao / 3.14;
	color.xyz = color.xyz * diffuseColor.xyz;
	return color;
}

#endif

void main()
{
	vec2 uv1 = v_UV1;
	vec2 uv2 = v_UV2;
	vec3 worldPos = vec3(v_WorldN_PX.w, v_WorldB_PY.w, v_WorldT_PZ.w);
	vec3 worldNormal = v_WorldN_PX.xyz;
	vec3 worldTangent = v_WorldT_PZ.xyz;
	vec3 worldBinormal = v_WorldB_PY.xyz;
	vec3 pixelNormalDir = worldNormal;
	vec4 vcolor = v_VColor;
	vec3 objectScale = vec3(1.0, 1.0, 1.0);

	vec2 screenUV = v_PosP.xy / v_PosP.w;
	float meshZ =   v_PosP.z / v_PosP.w;
	screenUV.xy = vec2(screenUV.x + 1.0, screenUV.y + 1.0) * 0.5;

#ifdef _SCREEN_FLIPPED_
	screenUV.y = 1.0 - screenUV.y;
#endif
vec4 customData1 = v_CustomData1;


float val0=fs_efk_uniform_709.x;
float val1=fs_efk_uniform_380.x;
vec2 val2=uv1;
vec2 val3=(val2-float(0.5));
vec4 val4_CompMask=vec4(val3.x,val3.y, 0.0, 1.0);
float val4=val4_CompMask.x;
vec4 val5_CompMask=vec4(val3.x,val3.y, 0.0, 1.0);
float val5=val5_CompMask.y;
float val6=atan2(val5,val4);
float val7=(val6+float(3.142));
float val8=(val7/float(6.284));
vec2 val9=fs_efk_uniform_1065.xy;
vec4 val10_CompMask=vec4(val9.x,val9.y, 0.0, 1.0);
float val10=val10_CompMask.y;
float val11=(val10*val8);
float val12=fs_predefined_uniform.x;
float val13=fs_efk_uniform_463.x;
float val14=(val13*val12);
vec4 val15_CompMask=vec4(val9.x,val9.y, 0.0, 1.0);
float val15=val15_CompMask.x;
float val16=(val5*val5);
float val17=(val4*val4);
float val18=(val17+val16);
float val19=sqrt(val18);
float val20=(val19*val15);
float val21=(val20+val14);
vec2 val22=vec2(val21,val11);
vec4 val24 = TEX2D(efk_texture_904,GetUV(val22));
vec4 val25=(val24-float(0.5));
vec4 val26=(val25*vec4(val1,val1,val1,val1));
vec4 val27_CompMask=val26;
vec2 val27=val27_CompMask.xy;
float val28=fs_efk_uniform_508.x;
float val29=(val28*val12);
float val30=(val29+val20);
vec2 val31=vec2(val30,val11);
vec2 val32=(val31+val27);
vec4 val34 = TEX2D(efk_texture_901,GetUV(val32));
vec4 val35= pow(val34,vec4(val0,val0,val0,val0));
vec2 val36=uv1;
vec4 val38 = TEX2D(efk_texture_343,GetUV(val36));
vec4 val39=(val38*val35);
vec4 val40=customData1.xyzw;
vec4 val41_CompMask=val40;
float val41=val41_CompMask.w;
vec4 val42=(vec4(val41,val41,val41,val41)*val39);
vec4 val43_CompMask=val40;
vec3 val43=val43_CompMask.xyz;
vec3 normalDir = vec3(0.5,0.5,1.0);
vec3 tempNormalDir = ((normalDir -vec3 (0.5, 0.5, 0.5)) * 2.0);
pixelNormalDir = tempNormalDir.x * worldTangent + tempNormalDir.y * worldBinormal + tempNormalDir.z * worldNormal;
vec3 worldPositionOffset = vec3(0.0,0.0,0.0);
vec3 baseColor = vec3(0.0,0.0,0.0);
vec3 emissive = val43;
float metallic = float(0.5);
float roughness = float(0.5);
float ambientOcclusion = float(1.0);
float opacity = val42.x;
float opacityMask = float(1.0);
float refraction = float(0.0);




	if(opacityMask <= 0.0) discard;
	if(opacity <= 0.0) discard;

	FRAGCOLOR = vec4(emissive, opacity);
}
 