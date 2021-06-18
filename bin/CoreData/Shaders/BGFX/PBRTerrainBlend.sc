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
#include "Constants.sh"
#include "Fog.sh"
#include "PBR.sh"
#include "IBL.sh"
/*
#line 30010

varying vec2 v_texcoord0;

#ifndef GL_ES
varying vec2 vDetailTexCoord;
#else
varying mediump vec2 vDetailTexCoord;
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
        varying vec2 v_texcoord1;
    #endif
#endif
*/

SAMPLER2D(sWeightMap0, 6);
SAMPLER2D(sDetailMap1, 13);
SAMPLER2D(sDetailMap2, 14);
SAMPLER2D(sDetailMap3, 15);
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
    //vDetailTexCoord = cDetailTiling.xy * v_texcoord0.xy;
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

    #ifdef METALLIC
        vec4 roughMetalSrc = texture2D(sSpecMap, v_texcoord0.xy);

        float roughness = roughMetalSrc.r + cRoughness.x;
        float metalness = roughMetalSrc.g + cMetallic.x;
    #else
        float roughness = cRoughness.x;
        float metalness = cMetallic.x;
    #endif

    roughness *= roughness;

    roughness = clamp(roughness, ROUGHNESS_FLOOR, 1.0);
    metalness = clamp(metalness, METALNESS_FLOOR, 1.0);

    vec3 specColor = mix(0.08 * cMatSpecColor.rgb, diffColor.rgb, metalness);
    diffColor.rgb = diffColor.rgb - diffColor.rgb * metalness;

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

        #if defined(DIRLIGHT)
            float atten = GetAtten(normal, v_wpos.xyz, lightDir);
        #elif defined(SPOTLIGHT)
            float atten = GetAttenSpot(normal, v_wpos.xyz, lightDir);
        #else
            float atten = GetAttenPoint(normal, v_wpos.xyz, lightDir);
        #endif

        float shadow = 1.0;
        #ifdef SHADOW
            //shadow = GetShadow(vShadowPos, v_wpos.w);
            vec4 vShadowPos[NUMCASCADES];
            vShadowPos[0] = v_shadow_pos0;
            #ifdef DIRLIGHT
                vShadowPos[1] = v_shadow_pos1;
                vShadowPos[2] = v_shadow_pos2;
                vShadowPos[3] = v_shadow_pos3;
            #endif
            shadow = GetShadow(vShadowPos, v_wpos.w);
        #endif

        #if defined(SPOTLIGHT)
            lightColor = v_spot_pos.w > 0.0 ? texture2DProj(sLightSpotMap, v_spot_pos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        #elif defined(CUBEMASK)
            lightColor = textureCube(sLightCubeMap, v_cube_mask_vec).rgb * cLightColor.rgb;
        #else
            lightColor = cLightColor.rgb;
        #endif
        vec3 toCamera = normalize(cCameraPosPS.xyz - v_wpos.xyz);
        vec3 lightVec = normalize(lightDir);
        float ndl = clamp((dot(normal, lightVec)), M_EPSILON, 1.0);

        vec3 BRDF = GetBRDF(v_wpos.xyz, lightDir, lightVec, toCamera, normal, roughness, diffColor.rgb, specColor);

        finalColor.rgb = BRDF * lightColor * (atten * shadow) / M_PI;

        #ifdef AMBIENT
            finalColor += cAmbientColor.rgb * diffColor.rgb;
            finalColor += cMatEmissiveColor.rgb;
            gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        #endif
    #elif defined(DEFERRED)
        // Fill deferred G-buffer
        const vec3 spareData = vec3(0.0, 0.0, 0.0); // Can be used to pass more data to deferred renderer
        gl_FragData[0] = vec4(specColor, spareData.r);
        gl_FragData[1] = vec4(diffColor.rgb, spareData.g);
        gl_FragData[2] = vec4(normal * roughness, spareData.b);
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

        vec3 toCamera = normalize(v_wpos.xyz - cCameraPosPS.xyz);
        vec3 reflection = normalize(reflect(toCamera, normal));

        vec3 cubeColor = v_vertex_light.rgb;

        #ifdef IBL
          vec3 iblColor = ImageBasedLighting(reflection, normal, toCamera, diffColor.rgb, specColor.rgb, roughness, cubeColor);
          finalColor.rgb += iblColor;
        #endif

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
}
#endif