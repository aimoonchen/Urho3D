#pragma once

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../EffekseerRendererCommon/EffekseerRenderer.IndexBufferBase.h"
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

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------