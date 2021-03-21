#if defined(COMPILEVS)
$input a_position
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

#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_screen_pos.xy = GetScreenPosPreDiv(gl_Position);
}
#elif defined(COMPILEPS)
uniform vec4 cBlurOffsets;
void main()
{
    vec2 color = vec2_splat(0.0);
    
    color += 0.015625 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(-3.0) * cBlurOffsets.xy).rg;
    color += 0.09375 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(-2.0) * cBlurOffsets.xy).rg;
    color += 0.234375 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(-1.0) * cBlurOffsets.xy).rg;
    color += 0.3125 * texture2D(sDiffMap, v_screen_pos.xy).rg;
    color += 0.234375 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(1.0) * cBlurOffsets.xy).rg;
    color += 0.09375 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(2.0) * cBlurOffsets.xy).rg;
    color += 0.015625 * texture2D(sDiffMap, v_screen_pos.xy + vec2_splat(3.0) * cBlurOffsets.xy).rg;
    
    gl_FragColor = vec4(color, 0.0, 0.0);
}
#endif
