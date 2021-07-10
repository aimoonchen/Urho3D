#pragma once

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "../EffekseerRendererCommon/EffekseerRenderer.VertexBufferBase.h"
#include "Effekseer/Backend/GraphicsDevice.h"
#include "EffekseerUrho3D.Base.h"
#include "../../Container/Vector.h"

namespace Urho3D
{
class Context;
class VertexBuffer;
struct VertexElement;
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
    VertexBuffer(Urho3D::Context* context, int count, bool isDynamic, unsigned int layoutMask);
    VertexBuffer(Urho3D::Context* context, int count, bool isDynamic,
                 const Urho3D::PODVector<Urho3D::VertexElement>& elements);
	virtual ~VertexBuffer();

	static Effekseer::RefPtr<VertexBuffer> Create(Urho3D::Context* context, int count,
                                                  bool isDynamic, unsigned int layoutMask);
    static Effekseer::RefPtr<VertexBuffer> Create(Urho3D::Context* context, int count,
                                                  bool isDynamic, const Urho3D::PODVector<Urho3D::VertexElement>& elements);

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

namespace Backend
{
	class VertexBuffer : public Effekseer::Backend::VertexBuffer
	{
	public:
		static Effekseer::Backend::VertexBufferRef Create(Urho3D::Context* context, int count,
			unsigned int layoutMask, const void* initialData = nullptr, bool isDynamic = false);
        static Effekseer::Backend::VertexBufferRef Create(Urho3D::Context* context, int count, const Urho3D::PODVector<Urho3D::VertexElement>& elements,
                                                          const void* initialData = nullptr, bool isDynamic = false);
		VertexBuffer(Urho3D::Context* context, int size, unsigned int layoutMask, bool isDynamic = false);
		VertexBuffer(Urho3D::Context* context, int size, const Urho3D::PODVector<Urho3D::VertexElement>& elements, bool isDynamic = false);
		virtual ~VertexBuffer();
        bool Allocate(int32_t size, bool isDynamic);
        void Deallocate();
        bool Init(int32_t size, bool isDynamic);
		void UpdateData(const void* src, int32_t size, int32_t offset) override;
		Urho3D::VertexBuffer* GetInterface() const { return urho3d_buffer_.get(); }
	private:
		uint32_t stride_{ 0 };
        std::unique_ptr<Urho3D::VertexBuffer> urho3d_buffer_{nullptr};
        std::vector<uint8_t> resources_;
        int32_t size_ = 0;
        bool isDynamic_ = false;
	};
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------