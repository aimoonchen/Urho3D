#if defined(COMPILEVS)
$input a_position, a_color0, a_normal, a_tangent, a_texcoord0, a_texcoord1
$outColor v_vspos, v_texcoord0, v_binormal, v_tangent, v_ppos, v_color0
#elif defined(COMPILEPS)
$input v_vspos, v_texcoord0, v_binormal, v_tangent, v_ppos, v_color0
#endif

#include "../bgfx_shader.sh"

#if defined(COMPILEVS)
//uniform mat4 mCamera;
uniform mat4 mCameraProj;
uniform vec4 mUVInversed;
uniform vec4 u_flipbookParameter;

void main()
{
    vec4 worldNormal = vec4((a_normal.xyz - vec3(0.5)) * 2.0, 0.0);
    vec4 worldTangent = vec4((a_tangent.xyz - vec3(0.5)) * 2.0, 0.0);
    vec4 worldBinormal = vec4(cross(worldNormal.xyz, worldTangent.xyz), 0.0);
    vec4 worldPos = vec4(a_position, 1.0);
    vec4 PosVS = mul(mCameraProj, worldPos);
    v_tangent = mul(mCameraProj, (worldPos + worldTangent));
    v_binormal = mul(mCameraProj, (worldPos + worldBinormal));
    v_vspos = PosVS;
    v_ppos = PosVS;
    v_color0 = a_color0;
    vec2 uv1 = a_texcoord0;
    uv1.y = mUVInversed.x + (mUVInversed.y * uv1.y);
    v_texcoord0 = uv1;
    gl_Position = PosVS;
}
#elif defined(COMPILEPS)
SAMPLER2D(sDiffMap, 0);     // colorTex
SAMPLER2D(sNormalMap, 1);   // depthTex
SAMPLER2D(sSpecMap, 2);     // backTex

uniform vec4 g_scale;
uniform vec4 mUVInversedBack;
uniform vec4 fFlipbookParameter;
uniform vec4 fUVDistortionParameter;
uniform vec4 fBlendTextureParameter;
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
    vec2 depth = ((zs * params.w) - vec2(params.y)) / (vec2(params.x) - (zs * params.z));
    float alphaFar = (depth.y - depth.x) / distanceFar;
    float alphaNear = ((-distanceNearOffset) - depth.y) / distanceNear;
    return min(max(min(alphaFar, alphaNear), 0.0), 1.0);
}

void main()
{
    vec4 outColor = texture2D(sDiffMap, v_texcoord0);
    outColor.w *= v_color0.w;
    vec2 pos = v_ppos.xy / v_ppos.w;
    vec2 posR = v_tangent.xy / v_tangent.w;
    vec2 posU = v_binormal.xy / v_binormal.w;
    float xscale = (((outColor.x * 2.0) - 1.0) * v_color0.x) * g_scale.x;
    float yscale = (((outColor.y * 2.0) - 1.0) * v_color0.y) * g_scale.x;
    vec2 uv = (pos + ((posR - pos) * xscale)) + ((posU - pos) * yscale);
    uv.x = (uv.x + 1.0) * 0.5;
    uv.y = 1.0 - ((uv.y + 1.0) * 0.5);
    uv.y = mUVInversedBack.x + (mUVInversedBack.y * uv.y);
    uv.y = 1.0 - uv.y;
    vec3 color = vec3(texture2D(sSpecMap, uv).xyz);
    outColor = vec4(color.x, color.y, color.z, outColor.w);
    vec4 screenPos = v_ppos / v_ppos.w;
    vec2 screenUV = (screenPos.xy + vec2_splat(1.0)) / 2.0;
    screenUV.y = 1.0 - screenUV.y;
    screenUV.y = 1.0 - screenUV.y;
    if (!(softParticleParam.w == 0.0)) {
        float backgroundZ = texture2D(sNormalMap, screenUV).x;
        float param = backgroundZ;
        float param_1 = screenPos.z;
        vec4 param_2 = softParticleParam;
        vec4 param_3 = reconstructionParam1;
        vec4 param_4 = reconstructionParam2;
        outColor.w *= SoftParticle(param, param_1, param_2, param_3, param_4);
    }
    if (outColor.w == 0.0) {
        discard;
    }
    gl_FragColor = outColor;
}
#endif