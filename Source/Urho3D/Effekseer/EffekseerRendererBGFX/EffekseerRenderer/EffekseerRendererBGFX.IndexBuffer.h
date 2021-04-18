#pragma once
#include "../../EffekseerRendererCommon/EffekseerRenderer.IndexBufferBase.h"
#include "EffekseerRendererBGFX.RendererImplemented.h"

namespace EffekseerRendererBGFX
{
class IndexBuffer : public ::EffekseerRenderer::IndexBufferBase
{
private:
	bgfx::DynamicIndexBufferHandle m_buffer{ BGFX_INVALID_HANDLE };
	IndexBuffer(bgfx::DynamicIndexBufferHandle buffer, int maxCount, bool isDynamic, int32_t stride);

public:
	virtual ~IndexBuffer();
	static IndexBuffer* Create(int maxCount, bool isDynamic, int32_t stride);
	bgfx::DynamicIndexBufferHandle GetInterface() { return m_buffer; }
	void Lock() override;
	void Unlock() override;
	bool IsValid();
	int32_t GetStride() const { return stride_; }
};

} // namespace EffekseerRendererBGFX
