#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight
$output v_texcoord0, v_screen_pos
#elif defined(COMPILEPS)
$input v_texcoord0, v_screen_pos
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"
#include "ScreenPos.sh"
#include "PostProcess.sh"

//varying vec2 v_texcoord0;
//varying vec2 v_screen_pos;

#ifdef COMPILEPS
uniform float cAutoExposureAdaptRate;
uniform vec2 cAutoExposureLumRange;
uniform float cAutoExposureMiddleGrey;
uniform vec2 cHDR128InvSize;
uniform vec2 cLum64InvSize;
uniform vec2 cLum16InvSize;
uniform vec2 cLum4InvSize;

float GatherAvgLum(sampler2D texSampler, vec2 texCoord, vec2 texelSize)
{
    float lumAvg = 0.0;
    lumAvg += texture2D(texSampler, texCoord + vec2(-1.0, -1.0) * texelSize).r;
    lumAvg += texture2D(texSampler, texCoord + vec2(-1.0, 1.0) * texelSize).r;
    lumAvg += texture2D(texSampler, texCoord + vec2(1.0, 1.0) * texelSize).r;
    lumAvg += texture2D(texSampler, texCoord + vec2(1.0, -1.0) * texelSize).r;
    return lumAvg / 4.0;
}
#endif
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_texcoord0.xy = GetQuadTexCoord(gl_Position);
    v_screen_pos.xy = GetScreenPosPreDiv(gl_Position);
}
#elif defined(COMPILEPS)
void main()
{
    #ifdef LUMINANCE64
    float logLumSum = 0.0;
    logLumSum += log(dot(texture2D(sDiffMap, v_texcoord0.xy + vec2(-1.0, -1.0) * cHDR128InvSize).rgb, LumWeights) + 1e-5);
    logLumSum += log(dot(texture2D(sDiffMap, v_texcoord0.xy + vec2(-1.0, 1.0) * cHDR128InvSize).rgb, LumWeights) + 1e-5);
    logLumSum += log(dot(texture2D(sDiffMap, v_texcoord0.xy + vec2(1.0, 1.0) * cHDR128InvSize).rgb, LumWeights) + 1e-5);
    logLumSum += log(dot(texture2D(sDiffMap, v_texcoord0.xy + vec2(1.0, -1.0) * cHDR128InvSize).rgb, LumWeights) + 1e-5);
    gl_FragColor.r = logLumSum;
    #endif

    #ifdef LUMINANCE16
    gl_FragColor.r = GatherAvgLum(sDiffMap, v_texcoord0.xy, cLum64InvSize);
    #endif

    #ifdef LUMINANCE4
    gl_FragColor.r = GatherAvgLum(sDiffMap, v_texcoord0.xy, cLum16InvSize);
    #endif

    #ifdef LUMINANCE1
    gl_FragColor.r = exp(GatherAvgLum(sDiffMap, v_texcoord0.xy, cLum4InvSize) / 16.0);
    #endif

    #ifdef ADAPTLUMINANCE
    float adaptedLum = texture2D(sDiffMap, v_texcoord0.xy).r;
    float lum = clamp(texture2D(sNormalMap, v_texcoord0.xy).r, cAutoExposureLumRange.x, cAutoExposureLumRange.y);
    gl_FragColor.r = adaptedLum + (lum - adaptedLum) * (1.0 - exp(-cDeltaTimePS.x * cAutoExposureAdaptRate));
    #endif

    #ifdef EXPOSE
    vec3 color = texture2D(sDiffMap, v_screen_pos.xy).rgb;
    float adaptedLum = texture2D(sNormalMap, v_texcoord0.xy).r;
    gl_FragColor = vec4(color * (cAutoExposureMiddleGrey / adaptedLum), 1.0);
    #endif
}
#endif