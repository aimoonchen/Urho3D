#if defined(COMPILEVS)
$input a_position, a_texcoord0, a_color0
$output v_fragmentColor, v_texCoord
#elif defined(COMPILEPS)
$input v_fragmentColor, v_texCoord
#endif

#include "bgfx_shader.sh"

#if defined(COMPILEVS)
uniform vec4 u_alpha;
uniform mat4 CC_MVPMatrix;
void main()
{
    v_color = vec4(a_color0.rgb * a_color0.a * u_alpha.x, a_color0.a * u_alpha.x);
    v_texcoord = a_texcoord0;

    gl_Position = mul(vec4(a_position, 1.0), CC_MVPMatrix);
}
#elif defined(COMPILEPS)
SAMPLER2D(sDiffMap, 0);
void main()
{
    // #if defined GL_OES_standard_derivatives
// gl_FragColor = v_color*smoothstep(0.0, length(fwidth(v_texcoord)), 1.0 - length(v_texcoord));
// #else
    gl_FragColor = v_color*step(0.0, 1.0 - length(v_texcoord));
// #endif
}
#endif