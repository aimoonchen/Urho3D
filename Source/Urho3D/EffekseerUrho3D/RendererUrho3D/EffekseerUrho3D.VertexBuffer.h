﻿#pragma once

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../../Effekseer/EffekseerRendererCommon/EffekseerRenderer.VertexBufferBase.h"
#include "EffekseerUrho3D.Base.h"

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

	uint32_t m_vertexRingOffset;
	bool m_ringBufferLock;

	int32_t m_ringLockedOffset;
	int32_t m_ringLockedSize;

public:
	VertexBuffer(RendererImplemented* renderer, int size, bool isDynamic);

	virtual ~VertexBuffer();

	static Effekseer::RefPtr<VertexBuffer> Create(RendererImplemented* renderer, int size, bool isDynamic);

public:
	void Lock() override;
	bool RingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	bool TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment) override;
	void Unlock() override;

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