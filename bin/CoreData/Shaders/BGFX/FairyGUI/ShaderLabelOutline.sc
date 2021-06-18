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
uniform vec4 u_effectType;
SAMPLER2D(sDiffMap, 0);
void main()
{
    vec4 sample = texture2D(sDiffMap, v_texCoord);
    // fontAlpha == 1 means the area of solid text (without edge)
    // fontAlpha == 0 means the area outside text, including outline area
    // fontAlpha == (0, 1) means the edge of text
    float fontAlpha = sample.a;

    // outlineAlpha == 1 means the area of 'solid text' and 'solid outline'
    // outlineAlpha == 0 means the transparent area outside text and outline
    // outlineAlpha == (0, 1) means the edge of outline
    float outlineAlpha = sample.r;

    if (u_effectType.x == 0) // draw text
    {
        gl_FragColor = v_fragmentColor * vec4(u_textColor.rgb, u_textColor.a * fontAlpha);
    }
    else if (u_effectType.x == 1) // draw outline
    {
        // multipy (1.0 - fontAlpha) to make the inner edge of outline smoother and make the text itself transparent.
        gl_FragColor = v_fragmentColor * vec4(u_effectColor.rgb, u_effectColor.a * outlineAlpha * (1.0 - fontAlpha));
    }
    else // draw shadow
    {
        gl_FragColor = v_fragmentColor * vec4(u_effectColor.rgb, u_effectColor.a * outlineAlpha);
    }
}
#endif