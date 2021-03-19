//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
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

#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/Material.h"
#include "../../Graphics/RenderSurface.h"
#include "../../IO/Log.h"
#include "../../Resource/ResourceCache.h"
#include "../../Resource/XMLFile.h"

#include "../../DebugNew.h"

#include "bgfx/bgfx.h"

namespace Urho3D
{

static GLenum glWrapModes[] =
{
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
#ifndef GL_ES_VERSION_2_0
    GL_CLAMP
#else
    GL_CLAMP_TO_EDGE
#endif
};

#ifndef GL_ES_VERSION_2_0
static GLenum gl3WrapModes[] =
{
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER
};
#endif

static uint64_t GetWrapMode(TextureAddressMode mode, TextureCoordinate coord)
{
#ifndef GL_ES_VERSION_2_0
    return Graphics::GetGL3Support() ? gl3WrapModes[mode] : glWrapModes[mode];
#else
    return glWrapModes[mode];
#endif
}

void Texture::SetSRGB(bool enable)
{
    if (graphics_)
        enable &= graphics_->GetSRGBSupport();

    if (enable != sRGB_)
    {
        sRGB_ = enable;
        // If texture had already been created, must recreate it to set the sRGB texture format
        if (object_.name_)
            Create();

        // If texture in use in the framebuffer, mark it dirty
        if (graphics_ && graphics_->GetRenderTarget(0) && graphics_->GetRenderTarget(0)->GetParentTexture() == this)
            graphics_->MarkFBODirty();
    }
}

void Texture::UpdateParameters()
{
    /*
    if (!object_.name_ || !graphics_)
        return;

    // If texture is multisampled, do not attempt to set parameters as it's illegal, just return
#ifndef GL_ES_VERSION_2_0
    if (target_ == GL_TEXTURE_2D_MULTISAMPLE)
    {
        parametersDirty_ = false;
        return;
    }
#endif

    // Wrapping
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, GetWrapMode(addressModes_[COORD_U]));
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, GetWrapMode(addressModes_[COORD_V]));
#ifndef GL_ES_VERSION_2_0
    glTexParameteri(target_, GL_TEXTURE_WRAP_R, GetWrapMode(addressModes_[COORD_W]));
#endif

    TextureFilterMode filterMode = filterMode_;
    if (filterMode == FILTER_DEFAULT)
        filterMode = graphics_->GetDefaultTextureFilterMode();

    // Filtering
    switch (filterMode)
    {
    case FILTER_NEAREST:
        if (levels_ < 2)
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

    case FILTER_BILINEAR:
        if (levels_ < 2)
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        else
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;

    case FILTER_ANISOTROPIC:
    case FILTER_TRILINEAR:
        if (levels_ < 2)
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        else
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;

    case FILTER_NEAREST_ANISOTROPIC:
        if (levels_ < 2)
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

    default:
        break;
    }

#ifndef GL_ES_VERSION_2_0
    // Anisotropy
    if (graphics_->GetAnisotropySupport())
    {
        unsigned maxAnisotropy = anisotropy_ ? anisotropy_ : graphics_->GetDefaultTextureAnisotropy();
        glTexParameterf(target_, GL_TEXTURE_MAX_ANISOTROPY_EXT,
            (filterMode == FILTER_ANISOTROPIC || filterMode == FILTER_NEAREST_ANISOTROPIC) ? (float)maxAnisotropy : 1.0f);
    }

    // Shadow compare
    if (shadowCompare_)
    {
        glTexParameteri(target_, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(target_, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    else
        glTexParameteri(target_, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glTexParameterfv(target_, GL_TEXTURE_BORDER_COLOR, borderColor_.Data());
#endif

    parametersDirty_ = false;
    */
}

bool Texture::GetParametersDirty() const
{
    return parametersDirty_;
}

bool Texture::IsCompressed() const
{
    return format_ == bgfx::TextureFormat::BC1 || format_ == bgfx::TextureFormat::BC2 ||
           format_ == bgfx::TextureFormat::BC3 || format_ == bgfx::TextureFormat::ETC1 ||
           format_ == bgfx::TextureFormat::ETC2 || format_ == bgfx::TextureFormat::ETC2A ||
           format_ == bgfx::TextureFormat::PTC14 || format_ == bgfx::TextureFormat::PTC14A ||
           format_ == bgfx::TextureFormat::PTC12 || format_ == bgfx::TextureFormat::PTC12A;
}

unsigned Texture::GetRowDataSize(int width) const
{
    switch (format_)
    {
    case bgfx::TextureFormat::R1:
    case bgfx::TextureFormat::A8:
        return (unsigned)width;

//     case GL_LUMINANCE_ALPHA:
//         return (unsigned)(width * 2);

    case bgfx::TextureFormat::RGB8:
        return (unsigned)(width * 3);

    case bgfx::TextureFormat::RGBA8:
#ifndef GL_ES_VERSION_2_0
    case bgfx::TextureFormat::D24S8:
    case bgfx::TextureFormat::RG16:
    case bgfx::TextureFormat::RG16F:
    case bgfx::TextureFormat::R32F:
#endif
        return (unsigned)(width * 4);

#ifndef GL_ES_VERSION_2_0
    case bgfx::TextureFormat::R8:
        return (unsigned)width;

    case bgfx::TextureFormat::RG8:
    case bgfx::TextureFormat::R16F:
        return (unsigned)(width * 2);

    case bgfx::TextureFormat::RGBA16:
    case bgfx::TextureFormat::RGBA16F:
        return (unsigned)(width * 8);

    case bgfx::TextureFormat::RGBA32F:
        return (unsigned)(width * 16);
#endif

    case bgfx::TextureFormat::BC1:
        return ((unsigned)(width + 3) >> 2u) * 8;

    case bgfx::TextureFormat::BC2:
    case bgfx::TextureFormat::BC3:
        return ((unsigned)(width + 3) >> 2u) * 16;

    case bgfx::TextureFormat::ETC1:
    case bgfx::TextureFormat::ETC2:
        return ((unsigned)(width + 3) >> 2u) * 8;

    case bgfx::TextureFormat::ETC2A:
        return ((unsigned)(width + 3) >> 2u) * 16;

    case bgfx::TextureFormat::PTC14:
    case bgfx::TextureFormat::PTC14A:
        return ((unsigned)(width + 3) >> 2u) * 8;

    case bgfx::TextureFormat::PTC12:
    case bgfx::TextureFormat::PTC12A:
        return ((unsigned)(width + 7) >> 3u) * 8;

    default:
        return 0;
    }
}

unsigned Texture::GetExternalFormat(unsigned format)
{
#ifndef GL_ES_VERSION_2_0
    if (format == GL_DEPTH_COMPONENT16 || format == GL_DEPTH_COMPONENT24 || format == GL_DEPTH_COMPONENT32)
        return GL_DEPTH_COMPONENT;
    else if (format == GL_DEPTH24_STENCIL8_EXT)
        return GL_DEPTH_STENCIL_EXT;
    else if (format == GL_SLUMINANCE_EXT)
        return GL_LUMINANCE;
    else if (format == GL_SLUMINANCE_ALPHA_EXT)
        return GL_LUMINANCE_ALPHA;
    else if (format == GL_R8 || format == GL_R16F || format == GL_R32F)
        return GL_RED;
    else if (format == GL_RG8 || format == GL_RG16 || format == GL_RG16F || format == GL_RG32F)
        return GL_RG;
    else if (format == GL_RGBA16 || format == GL_RGBA16F_ARB || format == GL_RGBA32F_ARB || format == GL_SRGB_ALPHA_EXT)
        return GL_RGBA;
    else if (format == GL_SRGB_EXT)
        return GL_RGB;
    else
        return format;
#else
    return format;
#endif
}

unsigned Texture::GetDataType(unsigned format)
{
#ifndef GL_ES_VERSION_2_0
    if (format == GL_DEPTH24_STENCIL8_EXT)
        return GL_UNSIGNED_INT_24_8_EXT;
    else if (format == GL_RG16 || format == GL_RGBA16)
        return GL_UNSIGNED_SHORT;
    else if (format == GL_RGBA32F_ARB || format == GL_RG32F || format == GL_R32F)
        return GL_FLOAT;
    else if (format == GL_RGBA16F_ARB || format == GL_RG16F || format == GL_R16F)
        return GL_HALF_FLOAT_ARB;
    else
        return GL_UNSIGNED_BYTE;
#else
    if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_COMPONENT24_OES)
        return GL_UNSIGNED_INT;
    else if (format == GL_DEPTH_COMPONENT16)
        return GL_UNSIGNED_SHORT;
    else
        return GL_UNSIGNED_BYTE;
#endif
}

unsigned Texture::GetSRGBFormat(unsigned format)
{
#ifndef GL_ES_VERSION_2_0
    if (!graphics_ || !graphics_->GetSRGBSupport())
        return format;

    switch (format)
    {
    case GL_RGB:
        return GL_SRGB_EXT;
    case GL_RGBA:
        return GL_SRGB_ALPHA_EXT;
    case GL_LUMINANCE:
        return GL_SLUMINANCE_EXT;
    case GL_LUMINANCE_ALPHA:
        return GL_SLUMINANCE_ALPHA_EXT;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
    default:
        return format;
    }
#else
    return format;
#endif
}

void Texture::RegenerateLevels()
{
//     if (!object_.name_)
//         return;
// 
// #ifndef GL_ES_VERSION_2_0
//     if (Graphics::GetGL3Support())
//         glGenerateMipmap(target_);
//     else
//         glGenerateMipmapEXT(target_);
// #else
//     glGenerateMipmap(target_);
// #endif

    levelsDirty_ = false;
}

uint64_t Texture::GetCoordMode(TextureCoordinate coord) {
    auto mode = addressModes_[coord];
    if (coord == COORD_U)
    {
        if (mode == ADDRESS_MIRROR)
        {
            return BGFX_SAMPLER_U_MIRROR;
        }
        else if (mode == ADDRESS_CLAMP)
        {
            return BGFX_SAMPLER_U_CLAMP;
        }
        else if (mode == ADDRESS_BORDER)
        {
            return BGFX_SAMPLER_U_BORDER;
        }
    }
    else if (coord == COORD_V)
    {
        if (mode == ADDRESS_MIRROR)
        {
            return BGFX_SAMPLER_V_MIRROR;
        }
        else if (mode == ADDRESS_CLAMP)
        {
            return BGFX_SAMPLER_V_CLAMP;
        }
        else if (mode == ADDRESS_BORDER)
        {
            return BGFX_SAMPLER_V_BORDER;
        }
    }
    else if (coord == COORD_W)
    {
        if (mode == ADDRESS_MIRROR)
        {
            return BGFX_SAMPLER_W_MIRROR;
        }
        else if (mode == ADDRESS_CLAMP)
        {
            return BGFX_SAMPLER_W_CLAMP;
        }
        else if (mode == ADDRESS_BORDER)
        {
            return BGFX_SAMPLER_W_BORDER;
        }
    }
    else
    {
        return 0;
    }
}

uint64_t Texture::GetFilterMode()
{
    uint64_t flags = 0;
    switch (filterMode_) {
    case FILTER_NEAREST:
        flags |= BGFX_SAMPLER_MIN_POINT;
        flags |= BGFX_SAMPLER_MAG_POINT;
//         if (levels_ < 2)
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//         else
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        if (levels_ > 1) {
            flags |= BGFX_SAMPLER_MIP_POINT;
        }
        glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case FILTER_BILINEAR:
//         if (levels_ < 2)
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         else
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//         glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (levels_ > 1) {
            flags = BGFX_SAMPLER_MIP_POINT;
        }
        break;
    case FILTER_ANISOTROPIC:
    case FILTER_TRILINEAR:
//         if (levels_ < 2)
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         else
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        flags |= BGFX_SAMPLER_MIN_ANISOTROPIC;
        flags |= BGFX_SAMPLER_MAG_ANISOTROPIC;
        break;
    case FILTER_NEAREST_ANISOTROPIC:
//         if (levels_ < 2)
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//         else
//             glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
//         glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        flags |= BGFX_SAMPLER_MIN_POINT;
        flags |= BGFX_SAMPLER_MAG_POINT;
        break;
    default:
        break;
    }
    return flags;
}

}
