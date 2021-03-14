#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_ppos
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_ppos
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"

//#if defined(DIFFMAP) || defined(ALPHAMAP)
//    varying vec2 vTexCoord;
//#endif
//#ifdef VERTEXCOLOR
//    varying vec4 vColor;
//#endif
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    
    #ifdef DIFFMAP
        v_texcoord0 = a_texcoord0;
    #endif
    #ifdef VERTEXCOLOR
        v_color0 = a_color0;
    #endif
}
#elif defined(COMPILEPS)
void main()
{
    vec4 diffColor = cMatDiffColor;

    #ifdef VERTEXCOLOR
        diffColor *= v_color0;
    #endif

    #if (!defined(DIFFMAP)) && (!defined(ALPHAMAP))
        gl_FragColor = diffColor;
    #endif
    #ifdef DIFFMAP
        vec4 diffInput = texture2D(sDiffMap, v_texcoord0);
        #ifdef ALPHAMASK
            if (diffInput.a < 0.5)
                discard;
        #endif
        gl_FragColor = diffColor * diffInput;
    #endif
    #ifdef ALPHAMAP
        #ifdef GL3
            float alphaInput = texture2D(sDiffMap, v_texcoord0).r;
        #else
            float alphaInput = texture2D(sDiffMap, v_texcoord0).a;
        #endif
        gl_FragColor = vec4(diffColor.rgb, diffColor.a * alphaInput);
    #endif
}
#endif