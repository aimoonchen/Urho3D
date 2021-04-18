#include "EffekseerRendererBGFX.IndexBuffer.h"

namespace EffekseerRendererBGFX
{
IndexBuffer::IndexBuffer(bgfx::DynamicIndexBufferHandle buffer, int maxCount, bool isDynamic, int32_t stride)
	: IndexBufferBase(maxCount, isDynamic)
	, m_buffer(buffer)
{
	stride_ = stride;
	m_resource = new uint8_t[m_indexMaxCount * stride_];
}

IndexBuffer::~IndexBuffer()
{
	delete[] m_resource;
	bgfx::destroy(m_buffer);
}

IndexBuffer* IndexBuffer::Create(int maxCount, bool isDynamic, int32_t stride)
{
	uint16_t flags = 0
		| ((stride == 4) ? BGFX_BUFFER_INDEX32 : 0);
	auto ib = bgfx::createDynamicIndexBuffer(maxCount, flags);

	return new IndexBuffer(ib, maxCount, isDynamic, stride);
}

void IndexBuffer::Lock()
{
	assert(!m_isLock);

	m_isLock = true;
	m_indexCount = 0;
}

void IndexBuffer::Unlock()
{
	assert(m_isLock);
	const bgfx::Memory* mem = bgfx::makeRef(m_resource, m_indexCount * stride_);
	bgfx::update(m_buffer, 0, mem);
	m_isLock = false;
}

bool IndexBuffer::IsValid()
{
	return bgfx::isValid(m_buffer);
}

} // namespace EffekseerRendererBGFX
