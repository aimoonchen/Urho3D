#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_color0, a_texcoord0, a_texcoord1, i_data0, i_data1, i_data2
$output v_color0, v_texcoord0, v_wpos, v_normal, v_tangent, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_wpos, v_normal, v_tangent, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3
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
#ifdef NORMALMAP
    varying vec4 v_texcoord0;
    varying vec4 v_tangent;
#else
    varying vec2 v_texcoord0;
#endif
varying vec3 v_normal;
varying vec4 v_wpos;
#ifdef VERTEXCOLOR
    varying vec4 v_color0;
#endif
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
        varying vec3 vReflectionVec;
    #endif
    #if defined(LIGHTMAP) || defined(AO)
        varying vec2 v_texcoord1;
    #endif
#endif
*/
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    vec4 clipPos = GetClipPos(worldPos);
    clipPos.z -= cDepthBias.x;
    gl_Position = GetClipPos(worldPos);//clipPos;// 
    v_normal = GetWorldNormal(modelMatrix);
    v_wpos = vec4(worldPos, GetDepth(gl_Position));

    #ifdef VERTEXCOLOR
        v_color0 = a_color0;
    #endif

    #ifdef NORMALMAP
        vec4 tangent = GetWorldTangent(modelMatrix);
        vec3 bitangent = cross(tangent.xyz, v_normal) * tangent.w;
        v_texcoord0 = vec4(GetTexCoord(a_texcoord0), bitangent.xy);
        v_tangent = vec4(tangent.xyz, bitangent.z);
    #else
        v_texcoord0 = vec4(GetTexCoord(a_texcoord0), vec2_splat(0));
    #endif

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
            v_cube_mask_vec = mul((worldPos - cLightPos.xyz), mat3(cLightMatrices[0][0].xyz, cLightMatrices[0][1].xyz, cLightMatrices[0][2].xyz));
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
            vReflectionVec = worldPos - cCameraPos;
        #endif
    #endif
}
#elif defined(COMPILEPS)
void main()
{
    // Get material diffuse albedo
    #ifdef DIFFMAP
        vec4 diffInput = texture2D(sDiffMap, v_texcoord0.xy);
        #ifdef ALPHAMASK
            if (diffInput.a < 0.5)
                discard;
        #endif
        vec4 diffColor = cMatDiffColor * diffInput;
    #else
        vec4 diffColor = cMatDiffColor;
    #endif

    #ifdef VERTEXCOLOR
        diffColor *= v_color0;
    #endif
    
    // Get material specular albedo
    #ifdef SPECMAP
        vec3 specColor = cMatSpecColor.rgb * texture2D(sSpecMap, v_texcoord0.xy).rgb;
    #else
        vec3 specColor = cMatSpecColor.rgb;
    #endif

    // Get normal
    #ifdef NORMALMAP
        mat3 tbn = mat3(v_tangent.xyz, vec3(v_texcoord0.zw, v_tangent.w), v_normal);
        vec3 normal = normalize(tbn * DecodeNormal(texture2D(sNormalMap, v_texcoord0.xy)));
    #else
        vec3 normal = normalize(v_normal);
    #endif

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
            vec4 vShadowPos[NUMCASCADES];//{v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3};
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

        vec3 finalColor = v_vertex_light * diffColor.rgb;
        #ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, v_texcoord1).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif

        #ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif
        #ifdef LIGHTMAP
            finalColor += texture2D(sEmissiveMap, v_texcoord1).rgb * diffColor.rgb;
        #endif
        #ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor.rgb * texture2D(sEmissiveMap, v_texcoord0.xy).rgb;
        #else
            finalColor += cMatEmissiveColor.rgb;
        #endif

        gl_FragData[0] = vec4(GetFog(finalColor, fogFactor), 1.0);
        gl_FragData[1] = fogFactor * vec4(diffColor.rgb, specIntensity);
        gl_FragData[2] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[3] = vec4(EncodeDepth(v_wpos.w), 0.0);
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = v_vertex_light * diffColor.rgb;
        #ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, v_texcoord1).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif
        
        #ifdef MATERIAL
            // Add light pre-pass accumulation result
            // Lights are accumulated at half intensity. Bring back to full intensity now
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, v_screen_pos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif

        #ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif
        #ifdef LIGHTMAP
            finalColor += texture2D(sEmissiveMap, v_texcoord1).rgb * diffColor.rgb;
        #endif
        #ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor.rgb * texture2D(sEmissiveMap, v_texcoord0.xy).rgb;
        #else
            finalColor += cMatEmissiveColor.rgb;
        #endif

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
}
#endif