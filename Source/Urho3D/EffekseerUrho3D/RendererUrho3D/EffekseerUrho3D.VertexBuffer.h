#pragma once

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../EffekseerRendererCommon/EffekseerRenderer.VertexBufferBase.h"
#include "EffekseerUrho3D.Base.h"

namespace Urho3D
{
class Context;
class VertexBuffer;
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
class VertexBuffer
	: public EffekseerRenderer::VertexBufferBase
	, public Effekseer::ReferenceObject
{
private:
	std::vector<uint8_t> m_buffer;
    uint16_t m_stride;
    Urho3D::VertexBuffer* m_urho3d_buffer{nullptr};
    uint32_t m_vertexRingStart;
	uint32_t m_vertexRingOffset;
	bool m_ringBufferLock;

	int32_t m_ringLockedOffset;
	int32_t m_ringLockedSize;

public:
    VertexBuffer(Urho3D::Context* context/*RendererImplemented* renderer*/, int size, bool isDynamic, unsigned int layoutMask);

	virtual ~VertexBuffer();

	static Effekseer::RefPtr<VertexBuffer> Create(Urho3D::Context* context /*RendererImplemented* renderer*/, int size,
                                                  bool isDynamic, unsigned int layoutMask);

public:
	void Lock() override;
	bool RingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	bool TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	void Unlock() override;
    bool IsValid();
    Urho3D::VertexBuffer* GetInterface() const { return m_urho3d_buffer; }
	const uint8_t* Refer() const { return m_buffer.data(); }
};
using VertexBufferRef = Effekseer::RefPtr<VertexBuffer>;

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------