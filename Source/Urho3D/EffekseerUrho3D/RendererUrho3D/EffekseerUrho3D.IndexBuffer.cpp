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

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
