#if defined(COMPILEVS)
$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_ppos
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_ppos
#endif

#include "bgfx_shader.sh"

#if defined(COMPILEVS)
//uniform mat4 mCamera;
//uniform mat4 mCameraProj;
uniform vec4 mUVInversed;
uniform vec4 mflipbookParameter;
void main()
{
//	vec3 wpos = mul(vec4(a_position, 1.0), u_model[0]).xyz;
//	vec4 proj_pos = mul(vec4(wpos, 1.0), u_viewProj);
	vec4 proj_pos = mul(u_viewProj, vec4(a_position, 1.0));
	v_ppos = proj_pos;
	gl_Position = proj_pos;
	v_color0 = a_color0;
	vec2 uv1 = a_texcoord0;
    uv1.y = mUVInversed.x + (mUVInversed.y * uv1.y);
	v_texcoord0 = uv1;
}
#elif defined(COMPILEPS)
SAMPLER2D(sColorTex, 0);
SAMPLER2D(sDepthTex, 1);
uniform vec4 fLightDirection;
uniform vec4 fLightColor;
uniform vec4 fLightAmbient;
uniform vec4 fFlipbookParameter;
uniform vec4 fUVDistortionParameter;
uniform vec4 fBlendTextureParameter;
uniform vec4 fCameraFrontDirection;
uniform vec4 fFalloffParameter;
uniform vec4 fFalloffBeginColor;
uniform vec4 fFalloffEndColor;
uniform vec4 fEmissiveScaling;
uniform vec4 fEdgeColor;
uniform vec4 fEdgeParameter;
uniform vec4 softParticleParam;
uniform vec4 reconstructionParam1;
uniform vec4 reconstructionParam2;

float SoftParticle(float backgroundZ, float meshZ, vec4 softparticleParam, vec4 reconstruct1, vec4 reconstruct2)
{
    float distanceFar = softparticleParam.x;
    float distanceNear = softparticleParam.y;
    float distanceNearOffset = softparticleParam.z;
    vec2 rescale = reconstruct1.xy;
    vec4 params = reconstruct2;
    vec2 zs = vec2((backgroundZ * rescale.x) + rescale.y, meshZ);
    vec2 depth = ((zs * params.w) - vec2_splat(params.y)) / (vec2_splat(params.x) - (zs * params.z));
    float alphaFar = (depth.y - depth.x) / distanceFar;
    float alphaNear = ((-distanceNearOffset) - depth.y) / distanceNear;
    return min(max(min(alphaFar, alphaNear), 0.0), 1.0);
}

void main()
{
	vec4 Output = texture2D(sColorTex, v_texcoord0) * v_color0;
    vec4 screenPos = v_ppos / vec4_splat(v_ppos.w);
    vec2 screenUV = (screenPos.xy + vec2_splat(1.0)) / vec2_splat(2.0);
    screenUV.y = 1.0 - screenUV.y;
    screenUV.y = 1.0 - screenUV.y;
    if (!(softParticleParam.w == 0.0))
    {
        float backgroundZ = texture2D(sDepthTex, screenUV).x;
        float param = backgroundZ;
        float param_1 = screenPos.z;
        vec4 param_2 = softParticleParam;
        vec4 param_3 = reconstructionParam1;
        vec4 param_4 = reconstructionParam2;
        Output.w *= SoftParticle(param, param_1, param_2, param_3, param_4);
    }
    if (Output.w == 0.0)
    {
        discard;
    }
	gl_FragColor = Output;
}
#endif