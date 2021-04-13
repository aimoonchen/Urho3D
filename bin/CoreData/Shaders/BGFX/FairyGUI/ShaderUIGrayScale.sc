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
SAMPLER2D(sDiffMap, 0);
void main()
{
    vec4 c = texture2D(sDiffMap, v_texCoord);
    c = v_fragmentColor * c;
    gl_FragColor.xyz = vec3(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
    gl_FragColor.w = c.w;
}
#endif