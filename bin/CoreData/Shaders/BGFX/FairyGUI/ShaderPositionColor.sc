#if defined(COMPILEVS)
$input a_position, a_color0
$output v_fragmentColor
#elif defined(COMPILEPS)
$input v_fragmentColor
#endif

#include "../bgfx_shader.sh"

#if defined(COMPILEVS)
uniform mat4 CC_MVPMatrix;
void main()
{
    gl_Position = mul(vec4(a_position, 1.0), CC_MVPMatrix);
    v_fragmentColor = a_color0;
}
#elif defined(COMPILEPS)
void main()
{
    gl_FragColor = v_fragmentColor;
}
#endif