#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_color0, a_texcoord0, a_texcoord1, i_data0, i_data1, i_data2
$output v_color0, v_texcoord0, v_texcoord2, v_wpos, v_normal, v_tangent, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3, v_reflection_vec
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_texcoord2, v_wpos, v_normal, v_tangent, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3, v_reflection_vec
#endif
#include "bgfx_shader.sh"
#include "shaderlib.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"
#include "ScreenPos.sh"
#include "Lighting.sh"
#include "Fog.sh"

/*
varying vec2 vTexCoord;

#ifndef GL_ES
varying vec2 v_texcoord2;
#else
varying mediump vec2 v_texcoord2;
#endif

varying vec3 v_normal;
varying vec4 v_wpos;
#ifdef PERPIXEL
    #ifdef SHADOW
        #ifndef GL_ES
            varying vec4 vShadowPos[NUMCASCADES];
        #else
            varying highp vec4 vShadowPos[NUMCASCADES];
        #endif
    #endif
    #ifdef SPOTLIGHT
        varying vec4 v_spot_pos;
    #endif
    #ifdef POINTLIGHT
        varying vec3 v_cube_mask_vec;
    #endif
#else
    varying vec3 v_vertex_light;
    varying vec4 v_screen_pos;
    #ifdef ENVCUBEMAP
        varying vec3 v_reflection_vec;
    #endif
    #if defined(LIGHTMAP) || defined(AO)
        varying vec2 vTexCoord2;
    #endif
#endif
*/

SAMPLER2D(sWeightMap0, 0);
SAMPLER2D(sDetailMap1, 1);
SAMPLER2D(sDetailMap2, 2);
SAMPLER2D(sDetailMap3, 3);

uniform vec4 cDetailTiling;

#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_normal = GetWorldNormal(modelMatrix);
    v_wpos = vec4(worldPos, GetDepth(gl_Position));
    v_texcoord0.xy = GetTexCoord(a_texcoord0);
    v_texcoord2 = cDetailTiling.xy * v_texcoord0.xy;

    #ifdef PERPIXEL
        // Per-pixel forward lighting
        vec4 projWorldPos = vec4(worldPos, 1.0);

        #ifdef SHADOW
            // Shadow projection: transform from world space to shadow space
            //for (int i = 0; i < NUMCASCADES; i++)
            //    vShadowPos[i] = GetShadowPos(i, v_normal, projWorldPos);
            v_shadow_pos0 = GetShadowPos(0, v_normal, projWorldPos);
            v_shadow_pos1 = GetShadowPos(1, v_normal, projWorldPos);
            v_shadow_pos2 = GetShadowPos(2, v_normal, projWorldPos);
            v_shadow_pos3 = GetShadowPos(3, v_normal, projWorldPos);
        #endif

        #ifdef SPOTLIGHT
            // Spotlight projection: transform from world space to projector texture coordinates
            v_spot_pos = mul(projWorldPos, cLightMatrices[0]);
        #endif
    
        #ifdef POINTLIGHT
            v_cube_mask_vec = (worldPos - cLightPos.xyz) * mat3(cLightMatrices[0][0].xyz, cLightMatrices[0][1].xyz, cLightMatrices[0][2].xyz);
        #endif
    #else
        // Ambient & per-vertex lighting
        #if defined(LIGHTMAP) || defined(AO)
            // If using lightmap, disregard zone ambient light
            // If using AO, calculate ambient in the PS
            v_vertex_light = vec3(0.0, 0.0, 0.0);
            v_texcoord1 = a_texcoord1;
        #else
            v_vertex_light = GetAmbient(GetZonePos(worldPos));
        #endif
        
        #ifdef NUMVERTEXLIGHTS
            for (int i = 0; i < NUMVERTEXLIGHTS; ++i)
                v_vertex_light += GetVertexLight(i, worldPos, v_normal) * cVertexLights[i * 3].rgb;
        #endif
        
        v_screen_pos = GetScreenPos(gl_Position);

        #ifdef ENVCUBEMAP
            v_reflection_vec = worldPos - cCameraPos.xyz;
        #endif
    #endif
}
#elif defined(COMPILEPS)
void main()
{
    // Get material diffuse albedo
    vec3 weights = texture2D(sWeightMap0, v_texcoord0.xy).rgb;
    float sumWeights = weights.r + weights.g + weights.b;
    weights /= sumWeights;
    vec4 diffColor = cMatDiffColor * (
        weights.r * texture2D(sDetailMap1, v_texcoord2) +
        weights.g * texture2D(sDetailMap2, v_texcoord2) + 
        weights.b * texture2D(sDetailMap3, v_texcoord2)
    );

    // Get material specular albedo
    vec3 specColor = cMatSpecColor.rgb;

    // Get normal
    vec3 normal = normalize(v_normal);

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(v_wpos.w, v_wpos.y);
    #else
        float fogFactor = GetFogFactor(v_wpos.w);
    #endif

    #if defined(PERPIXEL)
        // Per-pixel forward lighting
        vec3 lightColor;
        vec3 lightDir;
        vec3 finalColor;
        
        float diff = GetDiffuse(normal, v_wpos.xyz, lightDir);

        #ifdef SHADOW
            vec4 vShadowPos[NUMCASCADES];
            vShadowPos[0] = v_shadow_pos0;
            #ifdef DIRLIGHT
                vShadowPos[1] = v_shadow_pos1;
                vShadowPos[2] = v_shadow_pos2;
                vShadowPos[3] = v_shadow_pos3;
            #endif
            diff *= GetShadow(vShadowPos, v_wpos.w);
        #endif
    
        #if defined(SPOTLIGHT)
            lightColor = v_spot_pos.w > 0.0 ? texture2DProj(sLightSpotMap, v_spot_pos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        #elif defined(CUBEMASK)
            lightColor = textureCube(sLightCubeMap, v_cube_mask_vec).rgb * cLightColor.rgb;
        #else
            lightColor = cLightColor.rgb;
        #endif
    
        #ifdef SPECULAR
            float spec = GetSpecular(normal, cCameraPosPS.xyz - v_wpos.xyz, lightDir, cMatSpecColor.a);
            finalColor = diff * lightColor * (diffColor.rgb + spec * specColor * cLightColor.a);
        #else
            finalColor = diff * lightColor * diffColor.rgb;
        #endif

        #ifdef AMBIENT
            finalColor += cAmbientColor.rgb * diffColor.rgb;
            finalColor += cMatEmissiveColor.rgb;
            gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        #endif
    #elif defined(PREPASS)
        // Fill light pre-pass G-Buffer
        float specPower = cMatSpecColor.a / 255.0;

        gl_FragData[0] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[1] = vec4(EncodeDepth(v_wpos.w), 0.0);
    #elif defined(DEFERRED)
        // Fill deferred G-buffer
        float specIntensity = specColor.g;
        float specPower = cMatSpecColor.a / 255.0;

        gl_FragData[0] = vec4(GetFog(v_vertex_light * diffColor.rgb, fogFactor), 1.0);
        gl_FragData[1] = fogFactor * vec4(diffColor.rgb, specIntensity);
        gl_FragData[2] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[3] = vec4(EncodeDepth(v_wpos.w), 0.0);
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = v_vertex_light * diffColor.rgb;

        #ifdef MATERIAL
            // Add light pre-pass accumulation result
            // Lights are accumulated at half intensity. Bring back to full intensity now
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, v_screen_pos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
}
#endif