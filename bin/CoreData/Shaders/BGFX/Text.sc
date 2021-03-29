#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_color0, a_texcoord0
$output v_color0, v_texcoord0
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"

//varying vec2 v_texcoord0;
//varying vec4 v_color0;

#ifdef TEXT_EFFECT_SHADOW
uniform vec4 cShadowOffset;
uniform vec4 cShadowColor;
#endif

#ifdef TEXT_EFFECT_STROKE
uniform vec4 cStrokeColor;
#endif
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);

    v_texcoord0.xy = a_texcoord0;
    v_color0 = a_color0;
}

/*
    1) Simplest SDF shader:

    float distance = texture2D(sDiffMap, v_texcoord0.xy).a;
    if (distance >= 0.5)
        gl_FragColor.a = v_color0.a; // This is glyph
    else
        gl_FragColor.a = 0.0; // Outside glyph

    2) Glyph with antialiazed border:

    float distance = texture2D(sDiffMap, v_texcoord0.xy).a;
    gl_FragColor.a = v_color0.a * smoothstep(0.495, 0.505, distance);

    3) Quality improvement for far and small text:

    float distance = texture2D(sDiffMap, v_texcoord0.xy).a;
    // How much "distance" is changed for neighboring pixels.
    // If text is far then width is big. Far text will be blurred.
    float width = fwidth(distance);
    gl_FragColor.a = v_color0.a * smoothstep(0.5 - width, 0.5 + width, distance);
*/

#if defined(COMPILEPS) && defined(SIGNED_DISTANCE_FIELD)
    float GetAlpha(float distance, float width)
    {
        return smoothstep(0.5 - width, 0.5 + width, distance);
    }

    // Comment this define to turn off supersampling
    #define SUPERSAMPLING
#endif
#elif defined(COMPILEPS)
void main()
{
#ifdef SIGNED_DISTANCE_FIELD
    gl_FragColor.rgb = v_color0.rgb;
    float distance = texture2D(sDiffMap, v_texcoord0.xy).a;

    #ifdef TEXT_EFFECT_STROKE
        #ifdef SUPERSAMPLING
            float outlineFactor = smoothstep(0.5, 0.525, distance); // Border of glyph
            gl_FragColor.rgb = mix(cStrokeColor.rgb, v_color0.rgb, outlineFactor);
        #else
            if (distance < 0.525)
               gl_FragColor.rgb = cStrokeColor.rgb;
        #endif
    #endif

    #ifdef TEXT_EFFECT_SHADOW
        if (texture2D(sDiffMap, v_texcoord0.xy - cShadowOffset.xy).a > 0.5 && distance <= 0.5)
            gl_FragColor = cShadowColor;
        #ifndef SUPERSAMPLING
        else if (distance <= 0.5)
            gl_FragColor.a = 0.0;
        #endif
        else
    #endif
        {
            float width = fwidth(distance);
            float alpha = GetAlpha(distance, width);

            #ifdef SUPERSAMPLING
                vec2 deltaUV = 0.354 * fwidth(v_texcoord0.xy); // (1.0 / sqrt(2.0)) / 2.0 = 0.354
                vec4 square = vec4(v_texcoord0.xy - deltaUV, v_texcoord0.xy + deltaUV);

                float distance2 = texture2D(sDiffMap, square.xy).a;
                float distance3 = texture2D(sDiffMap, square.zw).a;
                float distance4 = texture2D(sDiffMap, square.xw).a;
                float distance5 = texture2D(sDiffMap, square.zy).a;

                alpha += GetAlpha(distance2, width)
                       + GetAlpha(distance3, width)
                       + GetAlpha(distance4, width)
                       + GetAlpha(distance5, width);

                // For calculating of average correct would be dividing by 5.
                // But when text is blurred, its brightness is lost. Therefore divide by 4.
                alpha = alpha * 0.25;
            #endif

            gl_FragColor.a = alpha;
        }
#else
    #ifdef ALPHAMAP
        gl_FragColor.rgb = v_color0.rgb;
        #ifdef GL3
            gl_FragColor.a = v_color0.a * texture2D(sDiffMap, v_texcoord0.xy).r;
        #else
            gl_FragColor.a = v_color0.a * texture2D(sDiffMap, v_texcoord0.xy).a;
        #endif
    #else
        gl_FragColor = v_color0 * texture2D(sDiffMap, v_texcoord0.xy);
    #endif
#endif
}
#endif