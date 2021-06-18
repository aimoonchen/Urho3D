#ifndef UNIFORMS_H_HEADER_GUARD
#define UNIFORMS_H_HEADER_GUARD
#ifdef COMPILEVS
// Vertex shader uniforms
uniform vec4 cAmbientStartColor;
uniform vec4 cAmbientEndColor;
uniform mat3 cBillboardRot;
uniform vec4 cCameraPos;
uniform float cNearClip;
uniform float cFarClip;
uniform vec4 cDepthMode;
uniform vec3 cFrustumSize;
uniform float cDeltaTime;
uniform vec4 cElapsedTime;
uniform vec4 cGBufferOffsets;
uniform vec4 cLightPos;
uniform vec3 cLightDir;
uniform vec4 cNormalOffsetScale;
uniform mat4 cModel;
uniform mat4 cView;
uniform mat4 cViewInv;
uniform mat4 cViewProj;
uniform vec4 cUOffset;
uniform vec4 cVOffset;
uniform mat4 cZone;
uniform mat4 cLightMatrices[4];
#ifdef SKINNED
    uniform vec4 cSkinMatrices[MAXBONES*3];
#endif
#ifdef NUMVERTEXLIGHTS
    uniform vec4 cVertexLights[4*3];
#endif
uniform vec4 cClipPlane;
uniform vec4 cDepthBias;
#else
// Fragment shader uniforms
uniform vec4 cAmbientColor;
uniform vec4 cCameraPosPS;
uniform vec4 cDeltaTimePS;
uniform vec4 cDepthReconstruct;
uniform float cElapsedTimePS;
uniform vec4 cFogParams;
uniform vec4 cFogColor;
uniform vec4 cGBufferInvSize;
uniform vec4 cLightColor;
uniform vec4 cLightPosPS;
uniform vec4 cLightDirPS;
uniform vec4 cNormalOffsetScalePS;
uniform vec4 cMatDiffColor;
uniform vec4 cMatEmissiveColor;
uniform vec4 cMatEnvMapColor;
uniform vec4 cMatSpecColor;
#ifdef PBR
// uniform vec4 cRoughnessMetallicLightRadLightLength
// #define cRoughness cRoughnessMetallicLightRadLightLength.x
// #define cMetallic cRoughnessMetallicLightRadLightLength.y
// #define cLightRad cRoughnessMetallicLightRadLightLength.z
// #define cLightLength cRoughnessMetallicLightRadLightLength.w
    uniform vec4 cRoughness;
    uniform vec4 cMetallic;
    uniform vec4 cLightRad;
    uniform vec4 cLightLength;
#endif
uniform vec4 cZoneMin;
uniform vec4 cZoneMax;
uniform float cNearClipPS;
uniform float cFarClipPS;
uniform vec4 cShadowCubeAdjust;
uniform vec4 cShadowDepthFade;
uniform vec4 cShadowIntensity;
uniform vec4 cShadowMapInvSize;
uniform vec4 cShadowSplits;
uniform mat4 cLightMatricesPS[4];
#ifdef VSM_SHADOW
uniform vec4 cVSMShadowParams;
#endif
#endif // COMPILEVS
#endif // UNIFORMS_H_HEADER_GUARD