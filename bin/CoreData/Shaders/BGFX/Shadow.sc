#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_texcoord0
$output v_texcoord0
#elif defined(COMPILEPS)
$input v_texcoord0
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"

/*
#ifdef VSM_SHADOW
    varying vec4 vTexCoord;
#else
    varying vec2 vTexCoord;
#endif
*/
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    #ifdef VSM_SHADOW
        v_texcoord0 = vec4(GetTexCoord(a_texcoord0), gl_Position.z, gl_Position.w);
    #else
        v_texcoord0.xy = GetTexCoord(a_texcoord0);
    #endif
}
#elif defined(COMPILEPS)
void main()
{
    #ifdef ALPHAMASK
        float alpha = texture2D(sDiffMap, v_texcoord0.xy).a;
        if (alpha < 0.5)
            discard;
    #endif

    #ifdef VSM_SHADOW
        float depth = v_texcoord0.z / v_texcoord0.w * 0.5 + 0.5;
        gl_FragColor = vec4(depth, depth * depth, 1.0, 1.0);
    #else
        gl_FragColor = vec4_splat(1.0);
    #endif
}
#endif