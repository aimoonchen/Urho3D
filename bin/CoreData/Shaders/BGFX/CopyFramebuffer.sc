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

//varying vec2 vScreenPos;
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
    gl_FragColor = texture2D(sDiffMap, v_screen_pos.xy);
}
#endif