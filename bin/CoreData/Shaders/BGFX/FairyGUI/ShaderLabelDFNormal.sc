#if defined(COMPILEVS)
$input a_position, a_texcoord0, a_color0
$output v_fragmentColor, v_texCoord
#elif defined(COMPILEPS)
$input v_fragmentColor, v_texCoord
#endif

#include "../bgfx_shader.sh"

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
    vec4 color = texture2D(sDiffMap, v_texCoord);
    //the texture use dual channel 16-bit output for distance_map
    //float dist = color.b+color.g/256.0;
    // the texture use single channel 8-bit output for distance_map
    float dist = color.a;
    //TODO: Implementation 'fwidth' for glsl 1.0
    //float width = fwidth(dist);
    //assign width for constant will lead to a little bit fuzzy,it's temporary measure.
    float width = 0.04;
    float alpha = smoothstep(0.5-width, 0.5+width, dist) * u_textColor.a;
    gl_FragColor = v_fragmentColor * vec4(u_textColor.rgb,alpha);
}
#endif