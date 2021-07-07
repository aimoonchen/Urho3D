#if defined(COMPILEVS)
$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_ppos
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_ppos
#endif

#include "../bgfx_shader.sh"

#if defined(COMPILEVS)
uniform mat4 mCameraProj;
uniform mat4 mModel_Inst[10];
uniform vec4 fUV[10];
uniform vec4 fModelColor[10];
uniform vec4 fLightDirection;
uniform vec4 fLightColor;
uniform vec4 fLightAmbient;
uniform vec4 mUVInversed;

void main()
{
    //ivec4 index = ivec4(a_indices);
    int index = gl_InstanceID;
    mat4 mModel = mModel_Inst[index];
    vec4 worldPos = mul(mModel, vec4(a_position, 1.0));
    vec4 proj_pos = mul(mCameraProj, worldPos);
    v_ppos = proj_pos;
    gl_Position = proj_pos;
    vec2 outputUV = a_texcoord0;
    vec4 uv = fUV[index];
    outputUV.x = (outputUV.x * uv.z) + uv.x;
    outputUV.y = (outputUV.y * uv.w) + uv.y;
    outputUV.y = mUVInversed.x + (mUVInversed.y * outputUV.y);
    v_texcoord0 = outputUV;
    v_color0 = fModelColor[index] * a_color0;
}
#elif defined(COMPILEPS)
SAMPLER2D(sDiffMap, 0);
SAMPLER2D(sNormalMap, 1);
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
	vec4 Output = texture2D(sDiffMap, v_texcoord0) * v_color0;
    vec4 screenPos = v_ppos / vec4_splat(v_ppos.w);
    vec2 screenUV = (screenPos.xy + vec2_splat(1.0)) / vec2_splat(2.0);
    screenUV.y = 1.0 - screenUV.y;
    screenUV.y = 1.0 - screenUV.y;
    if (!(softParticleParam.w == 0.0))
    {
        float backgroundZ = texture2D(sNormalMap, screenUV).x;
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