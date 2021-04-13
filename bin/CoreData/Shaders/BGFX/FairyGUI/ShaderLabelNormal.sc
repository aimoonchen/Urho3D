#if defined(COMPILEVS)
$input a_position, a_texcoord0, a_color0
$output v_fragmentColor, v_texCoord
#elif defined(COMPILEPS)
$input v_fragmentColor, v_texCoord
#endif

#include "bgfx_shader.sh"

#if defined(COMPILEVS)
uniform mat4 CC_MVPMatrix;
void main()
{
    gl_Position = mul(vec4(a_position, 1.0), CC_MVPMatrix);
    v_fragmentColor = a_color0;
    v_texCoord = a_texcoord0;
}
#elif defined(COMPILEPS)
uniform vec4 u_textColor;
SAMPLER2D(sDiffMap, 0);
void main()
{
    gl_FragColor =  v_fragmentColor * vec4(u_textColor.rgb,// RGB from uniform
        u_textColor.a * texture2D(sDiffMap, v_texCoord).a// A from texture & uniform
    );
}
#endif