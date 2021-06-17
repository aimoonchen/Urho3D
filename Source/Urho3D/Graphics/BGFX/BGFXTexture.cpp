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

void Texture::SetSRGB(bool enable)
{
    if (graphics_)
        enable &= graphics_->GetSRGBSupport();

    if (enable != sRGB_)
    {
        sRGB_ = enable;
        // If texture had already been created, must recreate it to set the sRGB texture format
        if (object_.handle_ != bgfx::kInvalidHandle)
            Create();

        // If texture in use in the framebuffer, mark it dirty
        if (graphics_ && graphics_->GetRenderTarget(0) && graphics_->GetRenderTarget(0)->GetParentTexture() == this)
            graphics_->MarkFBODirty();
    }
}

void Texture::UpdateParameters()
{

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
    case bgfx::TextureFormat::D24S8:
    case bgfx::TextureFormat::RG16:
    case bgfx::TextureFormat::RG16F:
    case bgfx::TextureFormat::R32F:
        return (unsigned)(width * 4);
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
    assert(false);
    return format;
}

unsigned Texture::GetDataType(unsigned format)
{
    assert(false);
    return format;
    //     if (format == GL_DEPTH24_STENCIL8_EXT)
//         return GL_UNSIGNED_INT_24_8_EXT;
//     else if (format == GL_RG16 || format == GL_RGBA16)
//         return GL_UNSIGNED_SHORT;
//     else if (format == bgfx::TextureFormat::RGBA32F  || format == bgfx::TextureFormat::RG32F || format == bgfx::TextureFormat::R32F)
//         return GL_FLOAT;
//     else if (format == GL_RGBA16F_ARB || format == GL_RG16F || format == GL_R16F)
//         return GL_HALF_FLOAT_ARB;
//     else
//         return GL_UNSIGNED_BYTE;
}

unsigned Texture::GetSRGBFormat(unsigned format)
{
    assert(false);
    return format;
}

void Texture::RegenerateLevels()
{
    levelsDirty_ = false;
}

}
