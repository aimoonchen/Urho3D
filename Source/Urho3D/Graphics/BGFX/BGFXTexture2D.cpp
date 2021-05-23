//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rightsR
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../../Precompiled.h"

#include "../../Core/Context.h"
#include "../../Core/Profiler.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsEvents.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/Renderer.h"
#include "../../Graphics/Texture2D.h"
#include "../../IO/FileSystem.h"
#include "../../IO/Log.h"
#include "../../Resource/ResourceCache.h"
#include "../../Resource/XMLFile.h"

#include "bgfx/bgfx.h"

#include <vector>

#include "../../DebugNew.h"



namespace Urho3D
{

void Texture2D::OnDeviceLost()
{

}

void Texture2D::OnDeviceReset()
{

}

void Texture2D::Release()
{
    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (graphics_) {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        if (renderSurface_)
            renderSurface_->Release();

        object_.handle_ = bgfx::kInvalidHandle;
    }
    else
    {
        if (renderSurface_)
            renderSurface_->Release();
    }

    resolveDirty_ = false;
    levelsDirty_ = false;
}

bool Texture2D::SetData(unsigned level, int x, int y, int width, int height, const void* data)
{
    URHO3D_PROFILE(SetTextureData);

    if (object_.handle_ == bgfx::kInvalidHandle || !graphics_)
    {
        URHO3D_LOGERROR("No texture created, can not set data");
        return false;
    }

    if (!data)
    {
        URHO3D_LOGERROR("Null source for setting data");
        return false;
    }

    if (level >= levels_)
    {
        URHO3D_LOGERROR("Illegal mip level for setting data");
        return false;
    }

    if (graphics_->IsDeviceLost())
    {
        URHO3D_LOGWARNING("Texture data assignment while device is lost");
        dataPending_ = true;
        return true;
    }

    if (IsCompressed())
    {
        x &= ~3u;
        y &= ~3u;
    }

    int levelWidth = GetLevelWidth(level);
    int levelHeight = GetLevelHeight(level);
    if (x < 0 || x + width > levelWidth || y < 0 || y + height > levelHeight || width <= 0 || height <= 0)
    {
        URHO3D_LOGERROR("Illegal dimensions for setting data");
        return false;
    }

    //graphics_->SetTextureForUpdate(this);

    bool wholeLevel = x == 0 && y == 0 && width == levelWidth && height == levelHeight;

    bgfx::updateTexture2D(bgfx::TextureHandle{object_.handle_}, 0, level, x, y, width, height,
                          bgfx::copy(data, GetDataSize(width, height)));
    return true;
}

bool Texture2D::SetData(Image* image, bool useAlpha)
{
    if (!image)
    {
        URHO3D_LOGERROR("Null image, can not set data");
        return false;
    }

    // Use a shared ptr for managing the temporary mip images created during this function
    SharedPtr<Image> mipImage;
    unsigned memoryUse = sizeof(Texture2D);
    MaterialQuality quality = QUALITY_HIGH;
    auto* renderer = GetSubsystem<Renderer>();
    if (renderer)
        quality = renderer->GetTextureQuality();

    if (!image->IsCompressed())
    {
        // Convert unsuitable formats to RGBA
        unsigned components = image->GetComponents();
        if (Graphics::GetGL3Support() && ((components == 1 && !useAlpha) || components == 2))
        {
            mipImage = image->ConvertToRGBA(); image = mipImage;
            if (!image)
                return false;
            components = image->GetComponents();
        }

        unsigned char* levelData = image->GetData();
        int levelWidth = image->GetWidth();
        int levelHeight = image->GetHeight();
        unsigned format = 0;

        // Discard unnecessary mip levels
        for (unsigned i = 0; i < mipsToSkip_[quality]; ++i)
        {
            mipImage = image->GetNextLevel(); image = mipImage;
            levelData = image->GetData();
            levelWidth = image->GetWidth();
            levelHeight = image->GetHeight();
        }

        switch (components)
        {
        case 1:
            format = useAlpha ? Graphics::GetAlphaFormat() : Graphics::GetLuminanceFormat();
            break;

        case 2:
            format = Graphics::GetLuminanceAlphaFormat();
            break;

        case 3:
            format = Graphics::GetRGBFormat();
            break;

        case 4:
            format = Graphics::GetRGBAFormat();
            break;

        default:
            assert(false);  // Should not reach here
            break;
        }

        // If image was previously compressed, reset number of requested levels to avoid error if level count is too high for new size
        if (IsCompressed() && requestedLevels_ > 1)
            requestedLevels_ = 0;
        SetSize(levelWidth, levelHeight, format);
        if (object_.handle_ == bgfx::kInvalidHandle)
            return false;

        for (unsigned i = 0; i < levels_; ++i)
        {
            SetData(i, 0, 0, levelWidth, levelHeight, levelData);
            memoryUse += levelWidth * levelHeight * components;

            if (i < levels_ - 1)
            {
                mipImage = image->GetNextLevel();
                image = mipImage;
                levelData = image->GetData();
                levelWidth = image->GetWidth();
                levelHeight = image->GetHeight();
            }
        }
    }
    else
    {
        int width = image->GetWidth();
        int height = image->GetHeight();
        unsigned levels = image->GetNumCompressedLevels();
        unsigned format = graphics_->GetFormat(image->GetCompressedFormat());
        bool needDecompress = false;

        if (format == bgfx::TextureFormat::Unknown)
        {
            format = Graphics::GetRGBAFormat();
            needDecompress = true;
        }

        unsigned mipsToSkip = mipsToSkip_[quality];
        if (mipsToSkip >= levels)
            mipsToSkip = levels - 1;
        while (mipsToSkip && (width / (1u << mipsToSkip) < 4 || height / (1u << mipsToSkip) < 4))
            --mipsToSkip;
        width /= (1u << mipsToSkip);
        height /= (1u << mipsToSkip);

        SetNumLevels(Max((levels - mipsToSkip), 1U));
        SetSize(width, height, format);

        for (unsigned i = 0; i < levels_ && i < levels - mipsToSkip; ++i)
        {
            CompressedLevel level = image->GetCompressedLevel(i + mipsToSkip);
            if (!needDecompress)
            {
                SetData(i, 0, 0, level.width_, level.height_, level.data_);
                memoryUse += level.rows_ * level.rowSize_;
            }
            else
            {
                auto* rgbaData = new unsigned char[level.width_ * level.height_ * 4];
                level.Decompress(rgbaData);
                SetData(i, 0, 0, level.width_, level.height_, rgbaData);
                memoryUse += level.width_ * level.height_ * 4;
                delete[] rgbaData;
            }
        }
    }

    SetMemoryUse(memoryUse);
    return true;
}

bool Texture2D::GetData(unsigned level, void* dest) const
{
    return false;
}

bool Texture2D::Create()
{
    Release();

    if (!graphics_ || !width_ || !height_)
        return false;

    if (graphics_->IsDeviceLost())
    {
        URHO3D_LOGWARNING("Texture creation while device is lost");
        return true;
    }

#ifdef GL_ES_VERSION_2_0
    if (multiSample_ > 1)
    {
        URHO3D_LOGWARNING("Multisampled texture is not supported on OpenGL ES");
        multiSample_ = 1;
        autoResolve_ = false;
    }
#endif

    unsigned format = /*GetSRGB() ? GetSRGBFormat(format_) : */format_;
//     unsigned externalFormat = GetExternalFormat(format_);
//     unsigned dataType = GetDataType(format_);

    uint64_t textureFlags = 0;
    // Create a renderbuffer instead of a texture if depth texture is not properly supported, or if this will be a packed
    // depth stencil texture
#ifndef GL_ES_VERSION_2_0
    if (format == Graphics::GetDepthStencilFormat())
#else
    if (format == GL_DEPTH_COMPONENT16 || format == GL_DEPTH_COMPONENT24_OES || format == GL_DEPTH24_STENCIL8_OES ||
        (format == GL_DEPTH_COMPONENT && !graphics_->GetShadowMapFormat()))
#endif
    {
//         if (renderSurface_)
//         {
//             renderSurface_->CreateRenderBuffer(width_, height_, format, multiSample_);
//             return true;
//         }
//         else
//             return false;
    }
    else
    {
        if (multiSample_ > 1)
        {
            textureFlags |= BGFX_TEXTURE_MSAA_SAMPLE;
            if (autoResolve_)
            {
                // Multisample with autoresolve: create a renderbuffer for rendering, but also a texture
                renderSurface_->CreateRenderBuffer(width_, height_, format, multiSample_);
            }
            else
            {
                // Multisample without autoresolve: create a texture only
// #ifndef GL_ES_VERSION_2_0
//                 if (!Graphics::GetGL3Support() && !GLEW_ARB_texture_multisample)
//                 {
//                     URHO3D_LOGERROR("Multisampled texture extension not available");
//                     return false;
//                 }
// 
//                 target_ = GL_TEXTURE_2D_MULTISAMPLE;
//                 if (renderSurface_)
//                     renderSurface_->target_ = GL_TEXTURE_2D_MULTISAMPLE;
// #endif
            }
        }
    }
    
//     glGenTextures(1, &object_.name_);
// 
//     // Ensure that our texture is bound to OpenGL texture unit 0
//     graphics_->SetTextureForUpdate(this);

    // If not compressed, create the initial level 0 texture with null data
    bool success = true;

//     if (!IsCompressed())
//     {
//         glGetError();
// #ifndef GL_ES_VERSION_2_0
//         if (multiSample_ > 1 && !autoResolve_)
//         {
//             glTexImage2DMultisample(target_, multiSample_, format, width_, height_, GL_TRUE);
//         }
//         else
// #endif
//         {
//             glTexImage2D(target_, 0, format, width_, height_, 0, externalFormat, dataType, nullptr);
//         }
//         if (glGetError())
//         {
//             URHO3D_LOGERROR("Failed to create texture");
//             success = false;
//         }
//     }
    
//     if (usage_ >= TEXTURE_RENDERTARGET)
//     {
//         textureFlags |= BGFX_TEXTURE_RT;
//     }
    // Set mipmapping
    if (usage_ == TEXTURE_DEPTHSTENCIL || usage_ == TEXTURE_DYNAMIC)
        requestedLevels_ = 1;
    else if (usage_ == TEXTURE_RENDERTARGET)
    {
#if defined(__EMSCRIPTEN__) || defined(IOS) || defined(TVOS)
        // glGenerateMipmap appears to not be working on WebGL or iOS/tvOS, disable rendertarget mipmaps for now
        requestedLevels_ = 1;
#else
//         if (requestedLevels_ != 1)
//         {
//             // Generate levels for the first time now
//             RegenerateLevels();
//             // Determine max. levels automatically
//             requestedLevels_ = 0;
//         }
        requestedLevels_ = 1;
#endif
    }

    levels_ = CheckMaxLevels(width_, height_, requestedLevels_);
// #ifndef GL_ES_VERSION_2_0
//     glTexParameteri(target_, GL_TEXTURE_BASE_LEVEL, 0);
//     glTexParameteri(target_, GL_TEXTURE_MAX_LEVEL, levels_ - 1);
// #endif

    textureFlags |= (GetFilterMode() | GetCoordMode(COORD_U) | GetCoordMode(COORD_V));
    if (usage_ >= TEXTURE_RENDERTARGET)
    {
        textureFlags |= BGFX_TEXTURE_RT;
    }
    if (GetSRGB())
    {
        textureFlags |= BGFX_TEXTURE_SRGB;
    }
    auto textureHandle = bgfx::createTexture2D(width_, height_, levels_ > 1, 1, bgfx::TextureFormat::Enum(format_), textureFlags);
    if (!bgfx::isValid(textureHandle))
    {
        URHO3D_LOGERROR("Failed to create Texture2D");
        return false;
    }
    object_.handle_ = textureHandle.idx;
    return success;
}

bool Texture2D::IsValid() const { return GetGPUObjectHandle() != bgfx::kInvalidHandle; }
}
