$input a_position, a_color0, a_normal, a_tangent, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3
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
SAMPLER2D(efk_texture_343, 0);
SAMPLER2D(efk_texture_901, 1);
SAMPLER2D(efk_texture_904, 2);
SAMPLER2D(efk_background, 3);
SAMPLER2D(efk_depth, 4);

uniform mat4 uMatCamera;
uniform mat4 uMatProjection;
uniform vec4 mUVInversed;
uniform vec4 vs_predefined_uniform;
uniform vec4 vs_cameraPosition;

uniform vec4 vs_efk_uniform_380;
uniform vec4 vs_efk_uniform_463;
uniform vec4 vs_efk_uniform_508;
uniform vec4 vs_efk_uniform_709;
uniform vec4 vs_efk_uniform_1065;


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

void main() {
	vec3 worldPos = a_position.xyz;
	vec3 objectScale = vec3(1.0, 1.0, 1.0);

	// Dummy
	vec2 screenUV = vec2(0.0, 0.0);
	float meshZ = 0.0;

	// UV
	vec2 uv1 = a_texcoord0.xy;
	//uv1.y = mUVInversed.x + mUVInversed.y * uv1.y;
	vec2 uv2 = a_texcoord1.xy;
	//uv2.y = mUVInversed.x + mUVInversed.y * uv2.y;

	// NBT
	vec3 worldNormal = (a_normal - vec3(0.5, 0.5, 0.5)) * 2.0;
	vec3 worldTangent = (a_tangent - vec3(0.5, 0.5, 0.5)) * 2.0;
	vec3 worldBinormal = cross(worldNormal, worldTangent);

	v_WorldN_PX.xyz = worldNormal;
	v_WorldB_PY.xyz = worldBinormal;
	v_WorldT_PZ.xyz = worldTangent;
	vec3 pixelNormalDir = worldNormal;
	vec4 vcolor = a_color0;
vec4 customData1 = a_texcoord2;
v_CustomData1 = customData1.xyzw;


float val0=vs_efk_uniform_709.x;
float val1=vs_efk_uniform_380.x;
vec2 val2=uv1;
vec2 val3=(val2-float(0.5));
vec4 val4_CompMask=vec4(val3.x,val3.y, 0.0, 1.0);
float val4=val4_CompMask.x;
vec4 val5_CompMask=vec4(val3.x,val3.y, 0.0, 1.0);
float val5=val5_CompMask.y;
float val6=atan2(val5,val4);
float val7=(val6+float(3.142));
float val8=(val7/float(6.284));
vec2 val9=vs_efk_uniform_1065.xy;
vec4 val10_CompMask=vec4(val9.x,val9.y, 0.0, 1.0);
float val10=val10_CompMask.y;
float val11=(val10*val8);
float val12=vs_predefined_uniform.x;
float val13=vs_efk_uniform_463.x;
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
vec4 val24 = TEX2D(efk_texture_904,GetUV(val22), 0.0);
vec4 val25=(val24-float(0.5));
vec4 val26=(val25*vec4(val1,val1,val1,val1));
vec4 val27_CompMask=val26;
vec2 val27=val27_CompMask.xy;
float val28=vs_efk_uniform_508.x;
float val29=(val28*val12);
float val30=(val29+val20);
vec2 val31=vec2(val30,val11);
vec2 val32=(val31+val27);
vec4 val34 = TEX2D(efk_texture_901,GetUV(val32), 0.0);
vec4 val35= pow(val34,vec4(val0,val0,val0,val0));
vec2 val36=uv1;
vec4 val38 = TEX2D(efk_texture_343,GetUV(val36), 0.0);
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



	worldPos = worldPos + worldPositionOffset;

	vec4 cameraPos = mul(uMatCamera, vec4(worldPos, 1.0));
	cameraPos = cameraPos / cameraPos.w;

	gl_Position = mul(uMatProjection, cameraPos);

	v_WorldN_PX.w = worldPos.x;
	v_WorldB_PY.w = worldPos.y;
	v_WorldT_PZ.w = worldPos.z;
	v_VColor = vcolor;

	v_UV1 = uv1;
	v_UV2 = uv2;
	//v_ScreenUV.xy = gl_Position.xy / gl_Position.w;
	//v_ScreenUV.xy = vec2(v_ScreenUV.x + 1.0, v_ScreenUV.y + 1.0) * 0.5;

	v_PosP = gl_Position;

	#ifdef _Y_INVERTED_
	gl_Position.y = - gl_Position.y;
	#endif
}
 