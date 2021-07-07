#include "../../Graphics/IndexBuffer.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
#include "EffekseerUrho3D.IndexBuffer.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
IndexBuffer::IndexBuffer(Urho3D::Context* context /*RendererImplemented* renderer*/, int maxCount, bool isDynamic,
                         int32_t stride)
	: IndexBufferBase(maxCount, isDynamic)
    , m_buffer(stride * maxCount)
{
    stride_ = stride;
    auto buffer = new Urho3D::IndexBuffer(context);
    buffer->SetSize(maxCount, stride > 2, isDynamic);
    m_urho3d_buffer = buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{
	delete m_urho3d_buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Effekseer::RefPtr<IndexBuffer> IndexBuffer::Create(Urho3D::Context* context /*RendererImplemented* renderer*/,
                                                   int maxCount, bool isDynamic,
                                                   int32_t stride)
{
    return IndexBufferRef(new IndexBuffer(context, maxCount, isDynamic, stride));
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void IndexBuffer::Lock()
{
	assert(!m_isLock);

	m_isLock = true;
	m_resource = m_buffer.data();
	m_indexCount = 0;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void IndexBuffer::Unlock()
{
	assert(m_isLock);
    m_urho3d_buffer->SetData(m_resource);
	m_resource = NULL;
	m_isLock = false;
}

bool IndexBuffer::IsValid()
{
    return m_urho3d_buffer->IsValid();
}

namespace Backend
{
IndexBuffer::IndexBuffer(Urho3D::Context* context, int count, bool isDynamic,
    Effekseer::Backend::IndexBufferStrideType strideType)
    : isDynamic_{ isDynamic }
{
    urho3d_buffer_ = std::make_unique<Urho3D::IndexBuffer>(context);
    urho3d_buffer_->SetSize(count, strideType == Effekseer::Backend::IndexBufferStrideType::Stride4, isDynamic);
}

IndexBuffer::~IndexBuffer()
{
    Deallocate();
}

bool IndexBuffer::Allocate(int32_t elementCount, int32_t stride)
{
    resources_.resize(elementCount_ * stride_);
//     GLExt::glGenBuffers(1, &buffer_);
// 
//     int elementArrayBufferBinding = 0;
//     glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBufferBinding);
// 
//     GLExt::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_);
//     GLExt::glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<uint32_t>(resources_.size()), nullptr, GL_STATIC_DRAW);
//     GLExt::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferBinding);
// 
    elementCount_ = elementCount;
    strideType_ = stride == 4 ? Effekseer::Backend::IndexBufferStrideType::Stride4
                              : Effekseer::Backend::IndexBufferStrideType::Stride2;
    return true;
}

void IndexBuffer::Deallocate()
{
    urho3d_buffer_ = nullptr;
}

bool IndexBuffer::Init(int32_t elementCount, int32_t stride)
{
    elementCount_ = elementCount;
    stride_ = stride;
    return Allocate(elementCount_, stride_);
}

void IndexBuffer::UpdateData(const void* src, int32_t size, int32_t offset)
{
    memcpy(resources_.data() + offset, src, size);
    urho3d_buffer_->SetData(resources_.data());
}

Effekseer::Backend::IndexBufferRef IndexBuffer::Create(Urho3D::Context* context,
                                                   int elementCount, Effekseer::Backend::IndexBufferStrideType strideType, const void* initialData, bool isDynamic)
{
    //return Effekseer::MakeRefPtr<IndexBuffer>(context, count, isDynamic, strideType);
    auto ret = Effekseer::MakeRefPtr<IndexBuffer>(context, elementCount, isDynamic, strideType);
    if (!ret->Init(elementCount, strideType == Effekseer::Backend::IndexBufferStrideType::Stride4 ? 4 : 2)) {
        return nullptr;
    }
    if (initialData) {
        ret->UpdateData(initialData, elementCount * (strideType == Effekseer::Backend::IndexBufferStrideType::Stride4 ? 4 : 2), 0);
    }
    return ret;
}

}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
