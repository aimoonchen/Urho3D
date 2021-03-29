#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_color0, a_texcoord0
$output v_texcoord0
#elif defined(COMPILEPS)
$input v_texcoord0
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"

//varying vec3 vTexCoord;
#if defined(COMPILEVS)
void main()
{
#ifdef IGNORENODETRANSFORM
    mat4 modelMatrix = mat4(
        vec4(1.0, 0.0, 0.0, cViewInv[0].w),
        vec4(0.0, 1.0, 0.0, cViewInv[1].w),
        vec4(0.0, 0.0, 1.0, cViewInv[2].w),
        vec4(0.0, 0.0, 0.0, 1.0));
#else
    mat4 modelMatrix = iModelMatrix;
#endif
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    gl_Position.z = gl_Position.w;
    v_texcoord0.xyz = a_position.xyz;
}
#elif defined(COMPILEPS)
void main()
{
    vec4 sky = cMatDiffColor * textureCube(sDiffCubeMap, v_texcoord0.xyz);
    #ifdef HDRSCALE
        sky = pow(sky + clamp((cAmbientColor.a - 1.0) * 0.1, 0.0, 0.25), max(vec4(cAmbientColor.a), 1.0)) * clamp(cAmbientColor.a, 0.0, 1.0);
    #endif
    gl_FragColor = sky;
}
#endif