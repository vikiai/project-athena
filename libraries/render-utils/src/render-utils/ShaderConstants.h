// <!
//  Created by Bradley Austin Davis on 2018/05/25
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
// !>

// <@if not RENDER_UTILS_SHADER_CONSTANTS_H@>
// <@def RENDER_UTILS_SHADER_CONSTANTS_H@>

// Hack comment to absorb the extra '//' scribe prepends

#ifndef RENDER_UTILS_SHADER_CONSTANTS_H
#define RENDER_UTILS_SHADER_CONSTANTS_H

#define RENDER_UTILS_ATTR_TEXCOORD0 0
#define RENDER_UTILS_ATTR_TEXCOORD1 1
#define RENDER_UTILS_ATTR_COLOR 2
#define RENDER_UTILS_ATTR_ALPHA 3

// World space
#define RENDER_UTILS_ATTR_POSITION_WS 4
#define RENDER_UTILS_ATTR_NORMAL_WS 5
#define RENDER_UTILS_ATTR_TANGENT_WS 6

// Model space 
#define RENDER_UTILS_ATTR_POSITION_MS 7
#define RENDER_UTILS_ATTR_NORMAL_MS 8

// Eye space
#define RENDER_UTILS_ATTR_POSITION_ES 9

// Fade
#define RENDER_UTILS_ATTR_FADE1 10
#define RENDER_UTILS_ATTR_FADE2 11
#define RENDER_UTILS_ATTR_FADE3 12


#define RENDER_UTILS_BUFFER_DEFERRED_FRAME_TRANSFORM 0
#define RENDER_UTILS_BUFFER_LIGHT_MODEL 3
#define RENDER_UTILS_BUFFER_AMBIENT_LIGHT 6
#define RENDER_UTILS_BUFFER_LIGHT_INDEX 7

#define RENDER_UTILS_UNIFORM_LIGHT_RADIUS 0
#define RENDER_UTILS_UNIFORM_LIGHT_TEXCOORD_TRANSFORM 1

// Deferred lighting resolution
#define RENDER_UTILS_TEXTURE_DEFERRRED_COLOR 0
#define RENDER_UTILS_TEXTURE_DEFERRRED_NORMAL 1
#define RENDER_UTILS_TEXTURE_DEFERRRED_SPECULAR 2
#define RENDER_UTILS_TEXTURE_DEFERRRED_DEPTH 3
#define RENDER_UTILS_TEXTURE_DEFERRED_OBSCURANCE 4
#define RENDER_UTILS_TEXTURE_DEFERRRED_LINEAR_Z_EYE 5
#define RENDER_UTILS_TEXTURE_DEFERRED_CURVATURE 6
#define RENDER_UTILS_TEXTURE_DEFERRED_DIFFUSED_CURVATURE 7
#define RENDER_UTILS_TEXTURE_DEFERRED_LIGHTING 10
#define RENDER_UTILS_TEXTURE_SKYBOX 11

#define RENDER_UTILS_BUFFER_SHADOW_PARAMS 2
#define RENDER_UTILS_TEXTURE_SHADOW 12

#define RENDER_UTILS_BUFFER_LIGHT_CLUSTER_FRUSTUM_GRID 10
#define RENDER_UTILS_BUFFER_LIGHT_CLUSTER_GRID 11
#define RENDER_UTILS_BUFFER_LIGHT_CLUSTER_CONTENT 12

// Haze
#define RENDER_UTILS_BUFFER_HAZE_PARAMS 7
#define RENDER_UTILS_TEXTURE_HAZE_COLOR 0
#define RENDER_UTILS_TEXTURE_HAZE_LINEAR_DEPTH 1

// Fading
#define RENDER_UTILS_BUFFER_FADE_PARAMS 8
#define RENDER_UTILS_BUFFER_FADE_OBJECT_PARAMS 9
#define RENDER_UTILS_TEXTURE_FADE_MASK 10

// Highlighting
#define RENDER_UTILS_BUFFER_HIGHLIGHT_PARAMS 2
#define RENDER_UTILS_TEXTURE_HIGHLIGHT_SCENE_DEPTH 0
#define RENDER_UTILS_TEXTURE_HIGHLIGHT_DEPTH 1

// Subsurface scattering
#define RENDER_UTILS_BUFFER_SSSC_PARAMS 13
#define RENDER_UTILS_TEXTURE_SSSC_PROFILE 12
#define RENDER_UTILS_TEXTURE_SSSC_LUT 8
#define RENDER_UTILS_TEXTURE_SSSC_SPECULAR_BECKMANN 9

// Ambient occlusion
#define RENDER_UTILS_BUFFER_SSAO_PARAMS 2
#define RENDER_UTILS_BUFFER_SSAO_DEBUG_PARAMS 3
#define RENDER_UTILS_TEXTURE_SSAO_PYRAMID 1
#define RENDER_UTILS_TEXTURE_SSAO_OCCLUSION 0

// Temporal anti-aliasing
#define RENDER_UTILS_BUFFER_TAA_PARAMS 2
#define RENDER_UTILS_TEXTURE_TAA_HISTORY 0
#define RENDER_UTILS_TEXTURE_TAA_SOURCE 1
#define RENDER_UTILS_TEXTURE_TAA_VELOCITY 2
#define RENDER_UTILS_TEXTURE_TAA_DEPTH 3
#define RENDER_UTILS_TEXTURE_TAA_NEXT 4

// Surface Geometry 
#define RENDER_UTILS_BUFFER_SG_PARAMS 1
#define RENDER_UTILS_TEXTURE_SG_DEPTH 0
#define RENDER_UTILS_TEXTURE_SG_NORMAL 1

// Blur
#define RENDER_UTILS_BUFFER_BLUR_PARAMS 0
#define RENDER_UTILS_TEXTURE_BLUR_SOURCE 0
#define RENDER_UTILS_TEXTURE_BLUR_DEPTH 1

// Tone Mapping
#define RENDER_UTILS_BUFFER_TM_PARAMS 0
#define RENDER_UTILS_TEXTURE_TM_COLOR 0

// Bloom
#define RENDER_UTILS_BUFFER_BLOOM_PARAMS 1
#define RENDER_UTILS_TEXTURE_BLOOM_COLOR 0

// SDF Text rendering
#define RENDER_UTILS_TEXTURE_TEXT_FONT 0
#define RENDER_UTILS_UNIFORM_TEXT_COLOR 0
#define RENDER_UTILS_UNIFORM_TEXT_OUTLINE 1


// Debugging 
#define RENDER_UTILS_BUFFER_DEBUG_SKYBOX 5
#define RENDER_UTILS_TEXTURE_DEBUG_DEPTH 11
#define RENDER_UTILS_TEXTURE_DEBUG_HALF_DEPTH 12
#define RENDER_UTILS_TEXTURE_DEBUG_OCCLUSION 13
#define RENDER_UTILS_TEXTURE_DEBUG_OCCLUSION_BLURRED 14
#define RENDER_UTILS_TEXTURE_DEBUG_VELOCITY 15
#define RENDER_UTILS_TEXTURE_DEBUG_SHADOWS 16
#define RENDER_UTILS_TEXTURE_DEBUG_HALF_NORMAL 17
#define RENDER_UTILS_TEXTURE_DEBUG_SCATTERING 18

// <!
#include <gpu/ShaderConstants.h>

namespace render_utils { namespace slot {

namespace uniform {
enum Uniform {
    TextColor = RENDER_UTILS_UNIFORM_TEXT_COLOR,
    TextOutline = RENDER_UTILS_UNIFORM_TEXT_OUTLINE,
    TaaSharpenIntensity = GPU_UNIFORM_EXTRA0,
    HighlightOutlineWidth = GPU_UNIFORM_EXTRA0,
    LightRadius = RENDER_UTILS_UNIFORM_LIGHT_RADIUS,
    TexcoordTransform = RENDER_UTILS_UNIFORM_LIGHT_TEXCOORD_TRANSFORM,
};
}

namespace buffer {
enum Buffer {
    DeferredFrameTransform = RENDER_UTILS_BUFFER_DEFERRED_FRAME_TRANSFORM,
    LightModel = RENDER_UTILS_BUFFER_LIGHT_MODEL,
    AmbientLight = RENDER_UTILS_BUFFER_AMBIENT_LIGHT,
    HazeParams = RENDER_UTILS_BUFFER_HAZE_PARAMS,
    FadeParameters = RENDER_UTILS_BUFFER_FADE_PARAMS,
    FadeObjectParameters = RENDER_UTILS_BUFFER_FADE_OBJECT_PARAMS,
    LightClusterFrustumGrid = RENDER_UTILS_BUFFER_LIGHT_CLUSTER_FRUSTUM_GRID,
    LightClusterGrid = RENDER_UTILS_BUFFER_LIGHT_CLUSTER_GRID,
    LightClusterContent = RENDER_UTILS_BUFFER_LIGHT_CLUSTER_CONTENT,
    SsscParams = RENDER_UTILS_BUFFER_SSSC_PARAMS,
    SsaoParams = RENDER_UTILS_BUFFER_SSAO_PARAMS,
    SsaoDebugParams = RENDER_UTILS_BUFFER_SSAO_DEBUG_PARAMS,
    LightIndex = RENDER_UTILS_BUFFER_LIGHT_INDEX,
    TaaParams = RENDER_UTILS_BUFFER_TAA_PARAMS,
    HighlightParams = RENDER_UTILS_BUFFER_HIGHLIGHT_PARAMS,
    DebugSkyboxParams = RENDER_UTILS_BUFFER_DEBUG_SKYBOX,
    SurfaceGeometryParams = RENDER_UTILS_BUFFER_SG_PARAMS,
    BlurParams = RENDER_UTILS_BUFFER_BLUR_PARAMS,
    BloomParams = RENDER_UTILS_BUFFER_BLOOM_PARAMS,
    ToneMappingParams = RENDER_UTILS_BUFFER_TM_PARAMS,
    ShadowParams = RENDER_UTILS_BUFFER_SHADOW_PARAMS,
};
} // namespace buffer

namespace texture {
enum Texture {
    DeferredColor = RENDER_UTILS_TEXTURE_DEFERRRED_COLOR,
    DeferredNormal = RENDER_UTILS_TEXTURE_DEFERRRED_NORMAL,
    DeferredSpecular = RENDER_UTILS_TEXTURE_DEFERRRED_SPECULAR,
    DeferredDepth = RENDER_UTILS_TEXTURE_DEFERRRED_DEPTH,
    DeferredLinearZEye = RENDER_UTILS_TEXTURE_DEFERRRED_LINEAR_Z_EYE,
    DeferredObscurance = RENDER_UTILS_TEXTURE_DEFERRED_OBSCURANCE,
    DeferredLighting = RENDER_UTILS_TEXTURE_DEFERRED_LIGHTING,
    DeferredCurvature = RENDER_UTILS_TEXTURE_DEFERRED_CURVATURE,
    DeferredDiffusedCurvature = RENDER_UTILS_TEXTURE_DEFERRED_DIFFUSED_CURVATURE,
    SsscLut = RENDER_UTILS_TEXTURE_SSSC_LUT,
    SsscSpecularBeckmann = RENDER_UTILS_TEXTURE_SSSC_SPECULAR_BECKMANN,
    SsscProfile = RENDER_UTILS_TEXTURE_SSSC_PROFILE,
    FadeMask = RENDER_UTILS_TEXTURE_FADE_MASK,
    Skybox = RENDER_UTILS_TEXTURE_SKYBOX,
    HazeColor = RENDER_UTILS_TEXTURE_HAZE_COLOR,
    HazeLinearDepth = RENDER_UTILS_TEXTURE_HAZE_LINEAR_DEPTH,
    Shadow = RENDER_UTILS_TEXTURE_SHADOW,
    TaaHistory = RENDER_UTILS_TEXTURE_TAA_HISTORY,
    TaaSource = RENDER_UTILS_TEXTURE_TAA_SOURCE,
    TaaVelocity = RENDER_UTILS_TEXTURE_TAA_VELOCITY,
    TaaDepth = RENDER_UTILS_TEXTURE_TAA_DEPTH,
    TaaNext = RENDER_UTILS_TEXTURE_TAA_NEXT,
    SsaoOcclusion = RENDER_UTILS_TEXTURE_SSAO_OCCLUSION,
    SsaoPyramid = RENDER_UTILS_TEXTURE_SSAO_PYRAMID,
    HighlightSceneDepth = RENDER_UTILS_TEXTURE_HIGHLIGHT_SCENE_DEPTH,
    HighlightDepth = RENDER_UTILS_TEXTURE_HIGHLIGHT_DEPTH,
    SurfaceGeometryDepth = RENDER_UTILS_TEXTURE_SG_DEPTH,
    SurfaceGeometryNormal = RENDER_UTILS_TEXTURE_SG_NORMAL,
    BlurSource = RENDER_UTILS_TEXTURE_BLUR_SOURCE,
    BlurDepth = RENDER_UTILS_TEXTURE_BLUR_DEPTH,
    BloomColor = RENDER_UTILS_TEXTURE_BLOOM_COLOR,
    ToneMappingColor = RENDER_UTILS_TEXTURE_TM_COLOR,
    TextFont = RENDER_UTILS_TEXTURE_TEXT_FONT,
    DebugDepth = RENDER_UTILS_TEXTURE_DEBUG_DEPTH,
    DebugHalfDepth = RENDER_UTILS_TEXTURE_DEBUG_HALF_DEPTH,
    DebugOcclusion = RENDER_UTILS_TEXTURE_DEBUG_OCCLUSION,
    DebugOcclusionBlurred = RENDER_UTILS_TEXTURE_DEBUG_OCCLUSION_BLURRED,
    DebugVelocity = RENDER_UTILS_TEXTURE_DEBUG_VELOCITY,
    DebugShadows = RENDER_UTILS_TEXTURE_DEBUG_SHADOWS,
    DebugHalfNormal = RENDER_UTILS_TEXTURE_DEBUG_HALF_NORMAL,
    DebugScattering = RENDER_UTILS_TEXTURE_DEBUG_SCATTERING,
};
} // namespace texture

} } // namespace render_utils::slot

// !>
// Hack Comment

#endif // RENDER_UTILS_SHADER_CONSTANTS_H

// <@if 1@>
// Trigger Scribe include
// <@endif@> <!def that !>

// <@endif@>

// Hack Comment
