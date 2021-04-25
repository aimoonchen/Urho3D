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

#include "../../Core/Context.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/IndexBuffer.h"
#include "../../IO/Log.h"

#include "../../DebugNew.h"

#include "bgfx/bgfx.h"

namespace Urho3D
{

void IndexBuffer::OnDeviceLost()
{

}

void IndexBuffer::OnDeviceReset()
{

}

void IndexBuffer::Release()
{
    Unlock();

    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (graphics_ && graphics_->GetIndexBuffer() == this)
            graphics_->SetIndexBuffer(nullptr);
        dynamic_ ? bgfx::destroy(bgfx::DynamicIndexBufferHandle{object_.handle_})
                 : bgfx::destroy(bgfx::IndexBufferHandle{object_.handle_});
        object_.handle_ = bgfx::kInvalidHandle;
    }
}

bool IndexBuffer::SetData(const void* data)
{
    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for index buffer data");
        return false;
    }

    if (!indexSize_)
    {
        URHO3D_LOGERROR("Index size not defined, can not set index buffer data");
        return false;
    }

    if (shadowData_ && data != shadowData_.Get())
        memcpy(shadowData_.Get(), data, indexCount_ * (size_t)indexSize_);

    auto mem = bgfx::copy(data, indexCount_ * (size_t)indexSize_);
    if (dynamic_)
    {
        if (object_.handle_ != bgfx::kInvalidHandle)
        {
            bgfx::update(bgfx::DynamicIndexBufferHandle{object_.handle_}, 0, mem);
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
            auto handle = bgfx::createIndexBuffer(mem);
            if (bgfx::isValid(handle))
            {
                object_.handle_ = handle.idx;
            }
            else
            {
                URHO3D_LOGERROR("createIndexBuffer failed!");
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

bool IndexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    if (start == 0 && count == indexCount_)
        return SetData(data);

    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for index buffer data");
        return false;
    }

    if (!indexSize_)
    {
        URHO3D_LOGERROR("Index size not defined, can not set index buffer data");
        return false;
    }

    if (start + count > indexCount_)
    {
        URHO3D_LOGERROR("Illegal range for setting new index buffer data");
        return false;
    }

    if (!count)
        return true;

    if (shadowData_ && shadowData_.Get() + start * indexSize_ != data)
        memcpy(shadowData_.Get() + start * indexSize_, data, count * (size_t)indexSize_);

    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (dynamic_)
        {
            bgfx::update(bgfx::DynamicIndexBufferHandle{object_.handle_}, start/* * (size_t)indexSize_*/,
                         bgfx::copy(data, count * indexSize_));
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

void* IndexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    if (lockState_ != LOCK_NONE)
    {
        URHO3D_LOGERROR("Index buffer already locked");
        return nullptr;
    }

    if (!indexSize_)
    {
        URHO3D_LOGERROR("Index size not defined, can not lock index buffer");
        return nullptr;
    }

    if (start + count > indexCount_)
    {
        URHO3D_LOGERROR("Illegal range for locking index buffer");
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
        return shadowData_.Get() + start * indexSize_;
    }
    else if (graphics_)
    {
        lockState_ = LOCK_SCRATCH;
        lockScratchData_ = graphics_->ReserveScratchBuffer(count * indexSize_);
        return lockScratchData_;
    }
    else
        return nullptr;
}

void IndexBuffer::Unlock()
{
    switch (lockState_)
    {
    case LOCK_SHADOW:
        SetDataRange(shadowData_.Get() + lockStart_ * indexSize_, lockStart_, lockCount_, discardLock_);
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

bool IndexBuffer::IsValid() const
{
    return GetGPUObjectHandle() != bgfx::kInvalidHandle;
}

bool IndexBuffer::Create()
{
    if (!indexCount_)
    {
        Release();
        return true;
    }

    if (graphics_)
    {
        auto size = indexCount_ * (size_t)indexSize_;
        if (dynamic_)
        {
            auto handle = bgfx::createDynamicIndexBuffer(size);
            if (!bgfx::isValid(handle))
            {
                URHO3D_LOGERROR("createDynamicIndexBuffer Failed!");
            }
            object_.handle_ = handle.idx;
        }
    }

    return true;
}

bool IndexBuffer::UpdateToGPU()
{
    if (object_.name_ && shadowData_)
        return SetData(shadowData_.Get());
    else
        return false;
}

void* IndexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    // Never called on OpenGL
    return nullptr;
}

void IndexBuffer::UnmapBuffer()
{
    // Never called on OpenGL
}

}
