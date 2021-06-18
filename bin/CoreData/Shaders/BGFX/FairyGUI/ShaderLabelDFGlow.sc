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
uniform vec4 u_effectColor;
uniform vec4 u_textColor;
SAMPLER2D(sDiffMap, 0);
void main()
{
    float dist = texture2D(sDiffMap, v_texCoord).a;
    //TODO: Implementation 'fwidth' for glsl 1.0
    //float width = fwidth(dist);
    //assign width for constant will lead to a little bit fuzzy,it's temporary measure.
    float width = 0.04;
    float alpha = smoothstep(0.5-width, 0.5+width, dist);
    //glow
    float mu = smoothstep(0.5, 1.0, sqrt(dist));
    vec4 color = u_effectColor*(1.0-alpha) + u_textColor*alpha;
    gl_FragColor = v_fragmentColor * vec4(color.rgb, max(alpha,mu)*color.a);
}
#endif