#if defined(COMPILEVS)
$input a_position
$output v_fragmentColor
#elif defined(COMPILEPS)
$input v_fragmentColor
#endif

#include "bgfx_shader.sh"

#if defined(COMPILEVS)
uniform mat4 CC_MVPMatrix;
uniform vec4 u_color;
uniform vec4 u_pointSize;
void main()
{
    gl_Position = mul(vec4(a_position, 1.0), CC_MVPMatrix);
//    gl_PointSize = u_pointSize.x;
    v_fragmentColor = u_color;
}
#elif defined(COMPILEPS)
void main()
{
    gl_FragColor = v_fragmentColor;
}
#endif