#include "EffekseerRendererBGFX.VertexBuffer.h"

#include "../../../Cocos2d/Urho3DContext.h"
#include "../../../Graphics/VertexBuffer.h"

namespace EffekseerRendererBGFX {

VertexBuffer::VertexBuffer(int size, bool isDynamic, unsigned int elementMask/*const bgfx::VertexLayout& layout*/)
	: VertexBufferBase(size, isDynamic)
	, m_vertexRingStart(0)
	, m_vertexRingOffset(0)
	, m_ringBufferLock(false)
{
	m_resource = new uint8_t[m_size];
	memset(m_resource, 0, (size_t)m_size);
//	m_stride = layout.getStride();
// 	m_buffer = bgfx::createDynamicVertexBuffer(
// 		bgfx::makeRef(m_resource, m_size),
// 		layout
// 	);
    m_buffer = new Urho3D::VertexBuffer(GetUrho3DContext());
    m_buffer->SetSize(size / 24, elementMask, isDynamic);
    m_stride = m_buffer->GetVertexSize();
}

VertexBuffer::~VertexBuffer()
{
	delete[] m_resource;
	//bgfx::destroy(m_buffer);
    delete m_buffer;
}

VertexBuffer* VertexBuffer::Create(int size, bool isDynamic, unsigned int elementMask/*const bgfx::VertexLayout& layout*/)
{
    return new VertexBuffer(size, isDynamic, elementMask /*layout*/);
}

// bgfx::DynamicVertexBufferHandle VertexBuffer::GetInterface()
// {
// 	return m_buffer;
// }

void VertexBuffer::Lock()
{
	assert(!m_isLock);

	m_isLock = true;
	m_offset = 0;
	m_vertexRingStart = 0;
}

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
		data = m_resource;
		m_vertexRingOffset = size;

		m_vertexRingStart = offset;
		m_offset = size;
	}
	else
	{
		offset = m_vertexRingOffset;
		data = m_resource;
		m_vertexRingOffset += size;

		m_vertexRingStart = offset;
		m_offset = size;
	}

	m_ringBufferLock = true;

	return true;
}

bool VertexBuffer::TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment)
{
	if ((int32_t)m_vertexRingOffset + size > m_size)
		return false;

	return RingBufferLock(size, offset, data, alignment);
}

void VertexBuffer::Unlock()
{
	assert(m_isLock || m_ringBufferLock);
	//bgfx::update(m_buffer, m_vertexRingStart / m_stride, bgfx::copy(m_resource, m_offset));
    m_buffer->SetDataRange(m_resource, m_vertexRingStart / m_stride, m_offset / m_stride);
	if (m_isLock)
	{
		m_vertexRingOffset += m_offset;
	}

	m_isLock = false;
	m_ringBufferLock = false;
}

bool VertexBuffer::IsValid()
{
	return m_buffer->IsValid(); // bgfx::isValid(m_buffer);
}

} // namespace EffekseerRendererBGFX
