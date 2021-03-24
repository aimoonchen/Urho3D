#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent
$output v_color0
#elif defined(COMPILEPS)
$input v_color0
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Transform.sh"

#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
}
#elif defined(COMPILEPS)
void main()
{
    gl_FragColor = vec4(1.0);
}
#endif
