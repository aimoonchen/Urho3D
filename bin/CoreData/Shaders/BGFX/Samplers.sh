#ifndef SAMPLERS_H_HEADER_GUARD
#define SAMPLERS_H_HEADER_GUARD
#ifdef COMPILEPS
SAMPLER2D(sDiffMap, 0);
SAMPLERCUBE(sDiffCubeMap, 0);
SAMPLER2D(sNormalMap, 1);
SAMPLER2D(sSpecMap, 2);
SAMPLER2D(sEmissiveMap, 3);
SAMPLER2D(sEnvMap, 4);
SAMPLERCUBE(sEnvCubeMap, 4);
SAMPLER2D(sLightRampMap, 8);
SAMPLER2D(sLightSpotMap, 9);
SAMPLERCUBE(sLightCubeMap, 9);
//SAMPLER3D(sVolumeMap, 5);
SAMPLER2D(sAlbedoBuffer, 0);
SAMPLER2D(sNormalBuffer, 1);
SAMPLER2D(sDepthBuffer, 13);
SAMPLER2D(sLightBuffer, 14);
#ifdef VSM_SHADOW
    SAMPLER2D(sShadowMap, 10);
#else
    SAMPLER2DSHADOW(sShadowMap, 10);
#endif
SAMPLERCUBE(sFaceSelectCubeMap, 11);
SAMPLERCUBE(sIndirectionCubeMap, 12);
SAMPLERCUBE(sZoneCubeMap, 15);
//SAMPLER3D(sZoneVolumeMap, 15);

vec3 DecodeNormal(vec4 normalInput)
{
    #ifdef PACKEDNORMAL
        vec3 normal;
        normal.xy = normalInput.ag * 2.0 - 1.0;
        normal.z = sqrt(max(1.0 - dot(normal.xy, normal.xy), 0.0));
        return normal;
    #else
        return normalize(normalInput.rgb * 2.0 - 1.0);
    #endif
}

vec3 EncodeDepth(float depth)
{
    // OpenGL 3 can use different MRT formats, so no need for encoding
    return vec3(depth, 0.0, 0.0);
}

float DecodeDepth(vec3 depth)
{
    // OpenGL 3 can use different MRT formats, so no need for encoding
    return depth.r;
}

float ReconstructDepth(float hwDepth)
{
    return dot(vec2(hwDepth, cDepthReconstruct.y / (hwDepth - cDepthReconstruct.x)), cDepthReconstruct.zw);
}
#endif
#endif // SAMPLERS_H_HEADER_GUARD