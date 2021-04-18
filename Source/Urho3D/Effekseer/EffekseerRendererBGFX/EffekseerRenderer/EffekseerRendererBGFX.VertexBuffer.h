#pragma once
#include "../../EffekseerRendererCommon/EffekseerRenderer.VertexBufferBase.h"
#include "EffekseerRendererBGFX.RendererImplemented.h"

namespace EffekseerRendererBGFX {
class VertexBuffer : public ::EffekseerRenderer::VertexBufferBase
{
private:
	uint16_t m_stride;
	bgfx::DynamicVertexBufferHandle m_buffer{ BGFX_INVALID_HANDLE };
	uint32_t m_vertexRingStart;
	uint32_t m_vertexRingOffset;
	bool m_ringBufferLock;
	VertexBuffer(int size, bool isDynamic, const bgfx::VertexLayout& layout);
public:
	virtual ~VertexBuffer();
	static VertexBuffer* Create(int size, bool isDynamic, const bgfx::VertexLayout& layout);
	bgfx::DynamicVertexBufferHandle GetInterface();
	void Lock();
	bool RingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	bool TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	void Unlock();
	bool IsValid();
};
} // namespace EffekseerRendererBGFX
