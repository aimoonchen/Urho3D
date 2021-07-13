$input a_position, a_normal, a_bitangent, a_tangent, a_texcoord0, a_color0
$output v_VColor, v_UV1, v_UV2, v_WorldN_PX, v_WorldB_PY, v_WorldT_PZ, v_PosP, v_CustomData1, v_CustomData2
#include "../bgfx_shader.sh"
#define EFK__INSTANCING_DISABLED__ 1
#define MOD mod
#define FRAC fract
#define LERP mix

//float atan2(in float y, in float x) {
//    return x == 0.0 ? sign(y)* 3.141592 / 2.0 : atan(y, x);
//}




// Dummy
float CalcDepthFade(vec2 screenUV, float meshZ, float softParticleParam) { return 1.0; }


#define TEX2D texture2DLod

#define _INSTANCE_COUNT_ 10
SAMPLER2D(efk_texture_268, 0);
SAMPLER2D(efk_texture_343, 1);
SAMPLER2D(efk_texture_1388, 2);
SAMPLER2D(efk_background, 3);
SAMPLER2D(efk_depth, 4);

uniform mat4 ProjectionMatrix;

#ifdef EFK__INSTANCING_DISABLED__

uniform mat4 ModelMatrix;
uniform vec4 UVOffset;
uniform vec4 ModelColor;

#else

uniform mat4 ModelMatrix[_INSTANCE_COUNT_];
uniform vec4 UVOffset[_INSTANCE_COUNT_];
uniform vec4 ModelColor[_INSTANCE_COUNT_];

#endif

uniform vec4 mUVInversed;
uniform vec4 vs_predefined_uniform;
uniform vec4 vs_cameraPosition;


#ifdef EFK__INSTANCING_DISABLED__
uniform vec4 customData1;
#else
uniform vec4 customData1s[_INSTANCE_COUNT_];
#endif

uniform vec4 vs_efk_uniform_709;
uniform vec4 vs_efk_uniform_1146;
uniform vec4 vs_efk_uniform_1397;
uniform vec4 vs_efk_uniform_1507;


vec2 GetUV(vec2 uv)
{
	uv.y = mUVInversed.x + mUVInversed.y * uv.y;
	return uv;
}

vec2 GetUVBack(vec2 uv)
{
	uv.y = mUVInversed.z + mUVInversed.w * uv.y;
	return uv;
}


void main()
{
#ifdef EFK__INSTANCING_DISABLED__
	mat4 modelMatrix = ModelMatrix;
	vec4 uvOffset = UVOffset;
	vec4 modelColor = ModelColor * a_color0;
#else
	mat4 modelMatrix = ModelMatrix[int(gl_InstanceID)];
	vec4 uvOffset = UVOffset[int(gl_InstanceID)];
	vec4 modelColor = ModelColor[int(gl_InstanceID)] * a_color0;
#endif

	vec3 worldPos = mul(modelMatrix, vec4(a_position, 1.0)).xyz;
	mat3 modelMatRot = mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);//mat3(modelMatrix);
	vec3 worldNormal = normalize(mul(modelMatRot, a_normal));
	vec3 worldBinormal = normalize(mul(modelMatRot, a_bitangent));
	vec3 worldTangent = normalize(mul(modelMatRot, a_tangent));
	vec3 objectScale = vec3(1.0, 1.0, 1.0);

	// Calculate ObjectScale
	objectScale.x = length(mul(modelMatRot, vec3(1.0, 0.0, 0.0)));
	objectScale.y = length(mul(modelMatRot, vec3(0.0, 1.0, 0.0)));
	objectScale.z = length(mul(modelMatRot, vec3(0.0, 0.0, 1.0)));

	// UV
	vec2 uv1 = a_texcoord0.xy * uvOffset.zw + uvOffset.xy;
	vec2 uv2 = a_texcoord0.xy;

	//uv1.y = mUVInversed.x + mUVInversed.y * uv1.y;
	//uv1.y = mUVInversed.x + mUVInversed.y * uv1.y;

	vec3 pixelNormalDir = worldNormal;
	
	vec4 vcolor = modelColor;

	// Dummy
	vec2 screenUV = vec2(0.0, 0.0);
	float meshZ = 0.0;

#ifndef EFK__INSTANCING_DISABLED__
vec4 customData1 = customData1s[int(gl_InstanceID)];
#endif
v_CustomData1 = customData1.xyzw;


float val0=vs_efk_uniform_709.x;
float val1=vs_efk_uniform_1146.x;
float val2=vs_predefined_uniform.x;
float val3=(val2*val1);
vec2 val4=vs_efk_uniform_1507.xy;
vec4 val5_CompMask=vec4(val4.x,val4.y, 0.0, 1.0);
float val5=val5_CompMask.y;
float val6=(val5+val3);
float val7=0.0;
vec2 val8=vec2(val7,val6);
vec2 val9=uv1;
vec2 val10=(val4*val9);
vec2 val11=(val10+val8);
vec4 val13 = TEX2D(efk_texture_268,GetUV(val11), 0.0);
vec2 val14=uv1;
vec4 val16 = TEX2D(efk_texture_343,GetUV(val14), 0.0);
vec4 val17=(val16*val13);
vec4 val18= pow(val17,vec4(val0,val0,val0,val0));
vec4 val19=customData1.xyzw;
vec4 val20_CompMask=val19;
float val20=val20_CompMask.w;
vec4 val21=(vec4(val20,val20,val20,val20)*val18);
float val22=vs_efk_uniform_1397.x;
vec4 val24 = TEX2D(efk_texture_1388,GetUV(val11), 0.0);
vec4 val25=(val24*vec4(val22,val22,val22,val22));
vec4 val26=(val25+val19);
vec4 val27_CompMask=val26;
vec3 val27=val27_CompMask.xyz;
vec3 normalDir = vec3(0.5,0.5,1.0);
vec3 tempNormalDir = ((normalDir -vec3 (0.5, 0.5, 0.5)) * 2.0);
pixelNormalDir = tempNormalDir.x * worldTangent + tempNormalDir.y * worldBinormal + tempNormalDir.z * worldNormal;
vec3 worldPositionOffset = vec3(0.0,0.0,0.0);
vec3 baseColor = vec3(0.0,0.0,0.0);
vec3 emissive = val27;
float metallic = float(0.5);
float roughness = float(0.5);
float ambientOcclusion = float(1.0);
float opacity = val21.x;
float opacityMask = float(1.0);
float refraction = float(0.0);



	worldPos = worldPos + worldPositionOffset;

	v_WorldN_PX.w = worldPos.x;
	v_WorldB_PY.w = worldPos.y;
	v_WorldT_PZ.w = worldPos.z;
	v_WorldN_PX.xyz = worldNormal;
	v_WorldB_PY.xyz = worldBinormal;
	v_WorldT_PZ.xyz = worldTangent;
	v_UV1 = uv1;
	v_UV2 = uv2;
	v_VColor = vcolor;
	gl_Position = mul(ProjectionMatrix, vec4(worldPos, 1.0));
//	v_ScreenUV.xy = gl_Position.xy / gl_Position.w;
//	v_ScreenUV.xy = vec2(v_ScreenUV.x + 1.0, v_ScreenUV.y + 1.0) * 0.5;

	v_PosP = gl_Position;

	#ifdef _Y_INVERTED_
	gl_Position.y = - gl_Position.y;
	#endif
}
 