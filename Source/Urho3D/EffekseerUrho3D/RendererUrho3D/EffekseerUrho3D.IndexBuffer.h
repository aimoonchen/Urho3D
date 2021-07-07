#pragma once

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../EffekseerRendererCommon/EffekseerRenderer.IndexBufferBase.h"
#include "Effekseer/Backend/GraphicsDevice.h"
#include "EffekseerUrho3D.Base.h"

namespace Urho3D
{
class IndexBuffer;
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
class IndexBuffer
	: public EffekseerRenderer::IndexBufferBase
	, public Effekseer::ReferenceObject
{
private:
	std::vector<uint8_t> m_buffer;
    Urho3D::IndexBuffer* m_urho3d_buffer{nullptr};

public:
    IndexBuffer(Urho3D::Context* context /*RendererImplemented* renderer*/, int maxCount, bool isDynamic,
                int32_t stride);

	virtual ~IndexBuffer();

	static Effekseer::RefPtr<IndexBuffer> Create(Urho3D::Context* context /*RendererImplemented* renderer*/,
                                                 int maxCount, bool isDynamic,
                                                 int32_t stride);
    Urho3D::IndexBuffer* GetInterface() const { return m_urho3d_buffer; }
    int32_t GetStride() const { return stride_; }
    bool IsValid();

public:
	void Lock() override;
	void Unlock() override;

	const uint8_t* Refer() const { return m_buffer.data(); }
};
using IndexBufferRef = Effekseer::RefPtr<IndexBuffer>;

namespace Backend
{
class IndexBuffer : public Effekseer::Backend::IndexBuffer
{
private:
    std::unique_ptr<Urho3D::IndexBuffer> urho3d_buffer_;
    std::vector<uint8_t> resources_;
    int32_t stride_ = 0;
    bool isDynamic_ = false;
public:
    static Effekseer::Backend::IndexBufferRef Create(Urho3D::Context* context, int elementCount,
        Effekseer::Backend::IndexBufferStrideType strideType,
        const void* initialData = nullptr, bool isDynamic = false);
    IndexBuffer(Urho3D::Context* context, int maxCount, bool isDynamic,
        Effekseer::Backend::IndexBufferStrideType stride);

    virtual ~IndexBuffer();
    bool Allocate(int32_t elementCount, int32_t stride);
    void Deallocate();
    bool Init(int32_t elementCount, int32_t stride);
    void UpdateData(const void* src, int32_t size, int32_t offset) override;
    Urho3D::IndexBuffer* GetInterface() const { return urho3d_buffer_.get(); }
};

} // namespace Backend

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------