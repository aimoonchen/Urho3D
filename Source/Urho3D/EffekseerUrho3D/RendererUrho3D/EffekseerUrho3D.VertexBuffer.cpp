#include "../../Graphics/VertexBuffer.h"
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
#include "EffekseerUrho3D.VertexBuffer.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
VertexBuffer::VertexBuffer(Urho3D::Context* context/*RendererImplemented* renderer*/, int size, bool isDynamic, unsigned int layoutMask)
	: VertexBufferBase(size, isDynamic)
	, m_buffer((size_t)size)
    , m_vertexRingStart(0)
	, m_vertexRingOffset(0)
	, m_ringBufferLock(false)
	, m_ringLockedOffset(0)
	, m_ringLockedSize(0)
{
    auto buffer = new Urho3D::VertexBuffer(context);
    buffer->SetSize(size / 24, layoutMask, isDynamic);
    m_stride = buffer->GetVertexSize();
    m_urho3d_buffer = buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
	delete m_urho3d_buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Effekseer::RefPtr<VertexBuffer> VertexBuffer::Create(Urho3D::Context* context /*RendererImplemented* renderer*/,
                                                     int size, bool isDynamic,
                                                     unsigned int layoutMask)
{
    return VertexBufferRef(new VertexBuffer(context, size, isDynamic, layoutMask));
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void VertexBuffer::Lock()
{
	assert(!m_isLock);
	//assert(!m_ringBufferLock);

	m_isLock = true;
	m_resource = m_buffer.data();
	m_offset = 0;
    m_vertexRingStart = 0;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
bool VertexBuffer::RingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment)
{
	assert(!m_isLock);
	assert(!m_ringBufferLock);
    assert(this->m_isDynamic);

	if (size > m_size)
        return false;

    m_vertexRingOffset = (m_vertexRingOffset + alignment - 1) / alignment * alignment;

#ifdef __ANDROID__
    if (true)
#else
    if ((int32_t)m_vertexRingOffset + size > m_size)
#endif
    {
        offset = 0;
        m_vertexRingOffset = size;
        m_vertexRingStart = offset;
    }
    else
    {
        offset = m_vertexRingOffset;
        m_vertexRingOffset += size;
        m_vertexRingStart = offset;
    }

    m_offset = size;
	m_ringBufferLock = true;
	data = m_resource = m_buffer.data();

	return true;
}

bool VertexBuffer::TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment)
{
    if ((int32_t)m_vertexRingOffset + size > m_size)
        return false;

	return RingBufferLock(size, offset, data, alignment);
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void VertexBuffer::Unlock()
{
	assert(m_isLock || m_ringBufferLock);
    m_urho3d_buffer->SetDataRange(m_resource, m_vertexRingStart / m_stride, m_offset / m_stride);
    if (m_isLock) {
        m_vertexRingOffset += m_offset;
    }
	m_resource = NULL;
	m_isLock = false;
	m_ringBufferLock = false;
}

bool VertexBuffer::IsValid() { return m_urho3d_buffer && m_urho3d_buffer->IsValid(); }

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------