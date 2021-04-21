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
#include "../../Graphics/VertexBuffer.h"
#include "../../IO/Log.h"

#include "../../DebugNew.h"

#include "bgfx/bgfx.h"

namespace Urho3D
{

static bgfx::AttribType::Enum Urho3DTypeToBGFXAttribType(VertexElementType elementType)
{
    switch (elementType)
    {
    case Urho3D::TYPE_INT:
        return bgfx::AttribType::Int16;
    case Urho3D::TYPE_FLOAT:
    case Urho3D::TYPE_VECTOR2:
    case Urho3D::TYPE_VECTOR3:
    case Urho3D::TYPE_VECTOR4:
        return bgfx::AttribType::Float;
    case Urho3D::TYPE_UBYTE4:
    case Urho3D::TYPE_UBYTE4_NORM:
        return bgfx::AttribType::Uint8;
    default:
        URHO3D_LOGERROR("Can not map urho3d type to bgfx attribtype");
        break;
    }
}

static bgfx::Attrib::Enum Urho3DSemanticToBGFXAttrib(VertexElementSemantic elementSemantic, uint8_t index)
{
    switch (elementSemantic)
    {
    case Urho3D::SEM_POSITION:
        return bgfx::Attrib::Position;
        break;
    case Urho3D::SEM_NORMAL:
        return bgfx::Attrib::Normal;
        break;
    case Urho3D::SEM_BINORMAL:
        return bgfx::Attrib::Bitangent;
        break;
    case Urho3D::SEM_TANGENT:
        return bgfx::Attrib::Tangent;
        break;
    case Urho3D::SEM_TEXCOORD:
        return bgfx::Attrib::Enum(bgfx::Attrib::TexCoord0 + index);
        break;
    case Urho3D::SEM_COLOR:
        return bgfx::Attrib::Color0;
        break;
    case Urho3D::SEM_BLENDWEIGHTS:
        return bgfx::Attrib::Weight;
        break;
    case Urho3D::SEM_BLENDINDICES:
        return bgfx::Attrib::Indices;
        break;
        //     case Urho3D::SEM_OBJECTINDEX:
        //         return bgfx::Attrib::Count;
        //         break;
    default:
        break;
    }
}

static uint8_t GetTypeNum(VertexElementType elementType)
{
    if (elementType == Urho3D::TYPE_VECTOR2)
    {
        return 2;
    }
    else if (elementType == Urho3D::TYPE_VECTOR3)
    {
        return 3;
    }
    else if (elementType == Urho3D::TYPE_VECTOR4
        || elementType == Urho3D::TYPE_UBYTE4
        || elementType == Urho3D::TYPE_UBYTE4_NORM)
    {
        return 4;
    }
    else
    {
        return 1;
    }
}

static bgfx::VertexLayout Urho3DLayoutToBGFXLayout(const PODVector<VertexElement>& elements)
{
    bgfx::VertexLayout layout;
    layout.begin();
    for (int i = 0; i < elements.Size(); i++)
    {
        bool asInt = ((elements[i].semantic_ == SEM_NORMAL) || (elements[i].semantic_ == SEM_TANGENT)) &&
                     ((elements[i].type_ == TYPE_UBYTE4) || (elements[i].type_ == TYPE_UBYTE4_NORM));
        layout.add(Urho3DSemanticToBGFXAttrib(elements[i].semantic_, elements[i].index_),
                   GetTypeNum(elements[i].type_),
                   Urho3DTypeToBGFXAttribType(elements[i].type_), (elements[i].type_ == TYPE_UBYTE4_NORM)/*false*/, asInt);
    }
    layout.end();
    return layout;
}

void VertexBuffer::OnDeviceLost()
{

}

void VertexBuffer::OnDeviceReset()
{
}

void VertexBuffer::Release()
{
    Unlock();

    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (graphics_)
        {
            for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
            {
                if (graphics_->GetVertexBuffer(i) == this)
                    graphics_->SetVertexBuffer(nullptr);
            }
        }
        
        dynamic_ ? bgfx::destroy(bgfx::DynamicVertexBufferHandle{object_.handle_})
                 : bgfx::destroy(bgfx::VertexBufferHandle{object_.handle_});
        object_.handle_ = bgfx::kInvalidHandle;
    }
}

bool VertexBuffer::SetData(const void* data)
{
    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }

    if (!vertexSize_)
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }

    if (shadowData_ && data != shadowData_.Get())
        memcpy(shadowData_.Get(), data, vertexCount_ * (size_t)vertexSize_);

    auto mem = bgfx::copy(data, vertexCount_ * (size_t)vertexSize_);
    if (dynamic_)
    {
        if (object_.handle_ != bgfx::kInvalidHandle)
        {
            bgfx::update(bgfx::DynamicVertexBufferHandle{object_.handle_}, 0, mem);
        }
        else
        {
            URHO3D_LOGERROR("try update invalid index buffer.");
        }
    }
    else
    {
        if (object_.handle_ == bgfx::kInvalidHandle)
        {
            auto handle = bgfx::createVertexBuffer(mem, Urho3DLayoutToBGFXLayout(elements_));
            if (bgfx::isValid(handle))
            {
                object_.handle_ = handle.idx;
            }
            else
            {
                URHO3D_LOGERROR("createVertexBuffer failed!");
            }
        }
        else
        {
            URHO3D_LOGERROR("try update static index buffer.");
        }
    }
    dataLost_ = false;
    return true;
}

bool VertexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    if (start == 0 && count == vertexCount_)
        return SetData(data);

    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }

    if (!vertexSize_)
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }

    if (start + count > vertexCount_)
    {
        URHO3D_LOGERROR("Illegal range for setting new vertex buffer data");
        return false;
    }

    if (!count)
        return true;

    if (shadowData_ && shadowData_.Get() + start * vertexSize_ != data)
        memcpy(shadowData_.Get() + start * vertexSize_, data, count * (size_t)vertexSize_);

    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (dynamic_)
        {
            bgfx::update(bgfx::DynamicVertexBufferHandle{object_.handle_}, start * (size_t)vertexSize_,
                         bgfx::copy(data, count * vertexSize_));
        }
        else
        {
            URHO3D_LOGERROR("try update static index buffer!");
        }
    }
    else
    {
        URHO3D_LOGERROR("try update invalid index buffer!");
    }

    return true;
}

bool VertexBuffer::IsValid() const { return GetGPUObjectHandle() != bgfx::kInvalidHandle; }

void* VertexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    if (lockState_ != LOCK_NONE)
    {
        URHO3D_LOGERROR("Vertex buffer already locked");
        return nullptr;
    }

    if (!vertexSize_)
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not lock vertex buffer");
        return nullptr;
    }

    if (start + count > vertexCount_)
    {
        URHO3D_LOGERROR("Illegal range for locking vertex buffer");
        return nullptr;
    }

    if (!count)
        return nullptr;

    lockStart_ = start;
    lockCount_ = count;
    discardLock_ = discard;

    if (shadowData_)
    {
        lockState_ = LOCK_SHADOW;
        return shadowData_.Get() + start * vertexSize_;
    }
    else if (graphics_)
    {
        lockState_ = LOCK_SCRATCH;
        lockScratchData_ = graphics_->ReserveScratchBuffer(count * vertexSize_);
        return lockScratchData_;
    }
    else
        return nullptr;
}

void VertexBuffer::Unlock()
{
    switch (lockState_)
    {
    case LOCK_SHADOW:
        SetDataRange(shadowData_.Get() + lockStart_ * vertexSize_, lockStart_, lockCount_, discardLock_);
        lockState_ = LOCK_NONE;
        break;

    case LOCK_SCRATCH:
        SetDataRange(lockScratchData_, lockStart_, lockCount_, discardLock_);
        if (graphics_)
            graphics_->FreeScratchBuffer(lockScratchData_);
        lockScratchData_ = nullptr;
        lockState_ = LOCK_NONE;
        break;

    default:
        break;
    }
}

bool VertexBuffer::Create()
{
    if (!vertexCount_ || !elementMask_)
    {
        Release();
        return true;
    }

    if (graphics_)
    {
        auto size = vertexCount_ * (size_t)vertexSize_;
        if (dynamic_)
        {
            auto handle = bgfx::createDynamicVertexBuffer(size, Urho3DLayoutToBGFXLayout(elements_));
            if (!bgfx::isValid(handle))
            {
                URHO3D_LOGERROR("createDynamicIndexBuffer Failed!");
            }
            object_.handle_ = handle.idx;
        }
    }

    return true;
}

bool VertexBuffer::UpdateToGPU()
{
    if (object_.name_ && shadowData_)
        return SetData(shadowData_.Get());
    else
        return false;
}

void* VertexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    // Never called on OpenGL
    return nullptr;
}

void VertexBuffer::UnmapBuffer()
{
    // Never called on OpenGL
}

}
