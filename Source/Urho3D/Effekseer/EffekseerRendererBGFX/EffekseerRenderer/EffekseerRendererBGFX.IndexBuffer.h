#pragma once
#include "../../EffekseerRendererCommon/EffekseerRenderer.IndexBufferBase.h"
#include "EffekseerRendererBGFX.RendererImplemented.h"
namespace Urho3D
{
class IndexBuffer;
}
namespace EffekseerRendererBGFX
{
class IndexBuffer : public ::EffekseerRenderer::IndexBufferBase
{
private:
	//bgfx::DynamicIndexBufferHandle m_buffer{ BGFX_INVALID_HANDLE };
    Urho3D::IndexBuffer* m_buffer{nullptr};
    IndexBuffer(/*bgfx::DynamicIndexBufferHandle buffer*/ Urho3D::IndexBuffer* buffer, int maxCount, bool isDynamic,
                int32_t stride);

public:
	virtual ~IndexBuffer();
	static IndexBuffer* Create(int maxCount, bool isDynamic, int32_t stride);
	//bgfx::DynamicIndexBufferHandle GetInterface() { return m_buffer; }
    Urho3D::IndexBuffer* GetInterface() const { return m_buffer; }
	void Lock() override;
	void Unlock() override;
	bool IsValid();
	int32_t GetStride() const { return stride_; }
};

} // namespace EffekseerRendererBGFX
