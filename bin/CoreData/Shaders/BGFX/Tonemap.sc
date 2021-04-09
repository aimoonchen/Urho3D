#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight
$output v_screen_pos
#elif defined(COMPILEPS)
$input v_screen_pos
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"
#include "ScreenPos.sh"
#include "PostProcess.sh"

//varying vec2 v_screen_pos;

#ifdef COMPILEPS
//uniform vec4 cTonemapExposureBiasMaxWhite;
//#define cTonemapExposureBias cTonemapExposureBiasMaxWhite.x
//#define cTonemapMaxWhite cTonemapExposureBiasMaxWhite.y
uniform vec4 cTonemapExposureBias;
uniform vec4 cTonemapMaxWhite;
#endif
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_screen_pos.xy = GetScreenPosPreDiv(gl_Position);
}
#elif defined(COMPILEPS)
void main()
{
    #ifdef REINHARDEQ3
    vec3 color = ReinhardEq3Tonemap(max(texture2D(sDiffMap, v_screen_pos.xy).rgb * cTonemapExposureBias.x, 0.0));
    gl_FragColor = vec4(color, 1.0);
    #endif

    #ifdef REINHARDEQ4
    vec3 color = ReinhardEq4Tonemap(max(texture2D(sDiffMap, v_screen_pos.xy).rgb * cTonemapExposureBias.x, 0.0), cTonemapMaxWhite.x);
    gl_FragColor = vec4(color, 1.0);
    #endif

    #ifdef UNCHARTED2
    vec3 color = Uncharted2Tonemap(max(texture2D(sDiffMap, v_screen_pos.xy).rgb * cTonemapExposureBias.x, 0.0)) / 
        Uncharted2Tonemap(vec3(cTonemapMaxWhite.x, cTonemapMaxWhite.x, cTonemapMaxWhite.x));
    gl_FragColor = vec4(color, 1.0);
    #endif
}
#endif