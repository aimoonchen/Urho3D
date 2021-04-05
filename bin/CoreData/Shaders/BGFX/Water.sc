#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_color0, a_texcoord0, a_texcoord1, i_data0, i_data1, i_data2
$output v_texcoord0, v_texcoord1, v_texcoord2, v_wpos, v_normal, v_tangent, v_screen_pos
#elif defined(COMPILEPS)
$input v_texcoord0, v_texcoord1, v_texcoord2, v_wpos, v_normal, v_tangent, v_screen_pos
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"
#include "ScreenPos.sh"
#include "Fog.sh"

/*
#ifndef GL_ES
varying vec4 v_screen_pos;
varying vec2 vReflectUV;
varying vec2 vWaterUV;
varying vec4 vEyeVec;
#else
varying highp vec4 v_screen_pos;
varying highp vec2 vReflectUV;
varying highp vec2 vWaterUV;
varying highp vec4 vEyeVec;
#endif
varying vec3 v_normal;

#ifdef COMPILEVS
uniform vec2 cNoiseSpeed;
uniform float cNoiseTiling;
#endif
#ifdef COMPILEPS
uniform float cNoiseStrength;
uniform float cFresnelPower;
uniform vec3 cWaterTint;
#endif
*/
#ifdef COMPILEVS
uniform vec4 cNoiseSpeed;
uniform vec4 cNoiseTiling;
#endif
#ifdef COMPILEPS
uniform vec4 cNoiseStrength;
uniform vec4 cFresnelPower;
uniform vec4 cWaterTint;
#endif

#define vReflectUV v_texcoord1
#define vWaterUV v_texcoord2
#define vEyeVec v_wpos

#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_screen_pos = GetScreenPos(gl_Position);
    // GetQuadTexCoord() returns a vec2 that is OK for quad rendering; multiply it with output W
    // coordinate to make it work with arbitrary meshes such as the water plane (perform divide in pixel shader)
    // Also because the quadTexCoord is based on the clip position, and Y is flipped when rendering to a texture
    // on OpenGL, must flip again to cancel it out
    vReflectUV = GetQuadTexCoord(gl_Position);
    vReflectUV.y = 1.0 - vReflectUV.y;
    vReflectUV *= gl_Position.w;
    v_texcoord0.xy = a_texcoord0 * cNoiseTiling.x + cElapsedTime.x * cNoiseSpeed.xy;
    v_normal = GetWorldNormal(modelMatrix);
    vEyeVec = vec4(cCameraPos.xyz - worldPos, GetDepth(gl_Position));
}
#elif defined(COMPILEPS)
void main()
{
    vec2 refractUV = v_screen_pos.xy / v_screen_pos.w;
    vec2 reflectUV = vReflectUV.xy / v_screen_pos.w;

    vec2 noise = (texture2D(sNormalMap, v_texcoord0.xy).rg - 0.5) * cNoiseStrength.x;
    refractUV += noise;
    // Do not shift reflect UV coordinate upward, because it will reveal the clipping of geometry below water
    if (noise.y < 0.0)
        noise.y = 0.0;
    reflectUV += noise;

    float fresnel = pow(1.0 - clamp(dot(normalize(vEyeVec.xyz), v_normal), 0.0, 1.0), cFresnelPower.x);
    vec3 refractColor = texture2D(sEnvMap, refractUV).rgb * cWaterTint.rgb;
    vec3 reflectColor = texture2D(sDiffMap, reflectUV).rgb;
    vec3 finalColor = mix(refractColor, reflectColor, fresnel);

    gl_FragColor = vec4(GetFog(finalColor, GetFogFactor(vEyeVec.w)), 1.0);
}
#endif