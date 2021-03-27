#if defined(COMPILEVS)
$input a_position, a_normal, a_tangent, a_indices, a_weight, a_texcoord0, a_texcoord1, i_data0, i_data1, i_data2
$output v_color0, v_texcoord0, v_wpos, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3
#elif defined(COMPILEPS)
$input v_color0, v_texcoord0, v_wpos, v_screen_pos, v_vertex_light, v_spot_pos, v_cube_mask_vec, v_shadow_pos0, v_shadow_pos1, v_shadow_pos2, v_shadow_pos3
#endif
#include "bgfx_shader.sh"
#include "Uniforms.sh"
#include "Samplers.sh"
#include "Transform.sh"
#include "ScreenPos.sh"
#include "Lighting.sh"
#include "Fog.sh"
#ifdef SOFTPARTICLES
    uniform vec4 cSoftParticleFadeScale;
#endif
/*
varying vec2 vTexCoord;
varying vec4 vWorldPos;
#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif
#ifdef SOFTPARTICLES
    varying vec4 vScreenPos;
    uniform float cSoftParticleFadeScale;
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
        varying vec4 vSpotPos;
    #endif
    #ifdef POINTLIGHT
        varying vec3 vCubeMaskVec;
    #endif
#else
    varying vec3 vVertexLight;
#endif
*/
#if defined(COMPILEVS)
void main()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    v_texcoord0.xy = GetTexCoord(a_texcoord0);
    v_wpos = vec4(worldPos, GetDepth(gl_Position));

    #ifdef SOFTPARTICLES
        v_screen_pos = GetScreenPos(gl_Position);
    #endif

    #ifdef VERTEXCOLOR
        v_color0 = iColor;
    #endif

    #ifdef PERPIXEL
        // Per-pixel forward lighting
        vec4 projWorldPos = vec4(worldPos, 1.0);

        #ifdef SHADOW
            // Shadow projection: transform from world space to shadow space
            //for (int i = 0; i < NUMCASCADES; i++)
            //    vShadowPos[i] = GetShadowPos(i, vec3(0.0, 0.0, 0.0), projWorldPos);
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
        v_vertex_light = GetAmbient(GetZonePos(worldPos));

        #ifdef NUMVERTEXLIGHTS
            for (int i = 0; i < NUMVERTEXLIGHTS; ++i)
                v_vertex_light += GetVertexLightVolumetric(i, worldPos) * cVertexLights[i * 3].rgb;
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

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(v_wpos.w, v_wpos.y);
    #else
        float fogFactor = GetFogFactor(v_wpos.w);
    #endif

    // Soft particle fade
    // In expand mode depth test should be off. In that case do manual alpha discard test first to reduce fill rate
    #ifdef SOFTPARTICLES
        #ifdef EXPAND
            if (diffColor.a < 0.01)
                discard;
        #endif

        float particleDepth = v_wpos.w;
        #ifdef HWDEPTH
            float depth = ReconstructDepth(texture2DProj(sDepthBuffer, v_screen_pos).r);
        #else
            float depth = DecodeDepth(texture2DProj(sDepthBuffer, v_screen_pos).rgb);
        #endif

        #ifdef EXPAND
            float diffZ = max(particleDepth - depth, 0.0) * (cFarClipPS - cNearClipPS);
            float fade = clamp(diffZ * cSoftParticleFadeScale.x, 0.0, 1.0);
        #else
            float diffZ = (depth - particleDepth) * (cFarClipPS - cNearClipPS);
            float fade = clamp(1.0 - diffZ * cSoftParticleFadeScale.x, 0.0, 1.0);
        #endif

        diffColor.a = max(diffColor.a - fade, 0.0);
    #endif

    #ifdef PERPIXEL
        // Per-pixel forward lighting
        vec3 lightColor;
        vec3 lightDir;
        vec3 finalColor;

        float diff = GetDiffuseVolumetric(v_wpos.xyz);

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

        finalColor = diff * lightColor * diffColor.rgb;
        gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = v_vertex_light * diffColor.rgb;

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
}
#endif