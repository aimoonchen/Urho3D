#if defined(COMPILEVS)
$input a_position, a_texcoord0, a_color0
$output v_fragmentColor, v_texCoord
#elif defined(COMPILEPS)
$input v_fragmentColor, v_texCoord
#endif

#include "bgfx_shader.sh"

#if defined(COMPILEVS)
uniform mat4 CC_PMatrix;
void main()
{
    gl_Position = mul(vec4(a_position, 1.0), CC_PMatrix);
    v_fragmentColor = a_color0;
    v_texCoord = a_texcoord0;
}
#elif defined(COMPILEPS)
uniform float CC_alpha_value;
SAMPLER2D(sDiffMap, 0);
void main()
{
    vec4 texColor = texture2D(sDiffMap, v_texCoord);

// mimic: glAlphaFunc(GL_GREATER)
// pass if ( incoming_pixel >= CC_alpha_value ) => fail if incoming_pixel < CC_alpha_value

    if ( texColor.a <= CC_alpha_value )
        discard;

    gl_FragColor = texColor * v_fragmentColor;
}
#endif