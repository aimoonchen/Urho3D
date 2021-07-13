#include "../../Graphics/VertexBuffer.h"
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
#include "EffekseerUrho3D.VertexBuffer.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerUrho3D
{
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
VertexBuffer::VertexBuffer(Urho3D::Context* context,
    int count, bool isDynamic, unsigned int layoutMask)
    : VertexBufferBase(count, isDynamic)
	//, m_buffer((size_t)size)
    , m_vertexRingStart(0)
	, m_vertexRingOffset(0)
	, m_ringBufferLock(false)
	, m_ringLockedOffset(0)
	, m_ringLockedSize(0)
{
    auto buffer = new Urho3D::VertexBuffer(context);
    buffer->SetSize(count, layoutMask, isDynamic);
    m_stride = buffer->GetVertexSize();
    m_size = count * m_stride;
    //m_buffer.resize(m_size);
    m_resource = new uint8_t[m_size];
    memset(m_resource, 0, (size_t)m_size);
    m_urho3d_buffer = buffer;
}

VertexBuffer::VertexBuffer(Urho3D::Context* context, int count, bool isDynamic,
    const Urho3D::PODVector<Urho3D::VertexElement>& elements)
    : VertexBufferBase(count, isDynamic)
    //, m_buffer((size_t)size)
    , m_vertexRingStart(0)
    , m_vertexRingOffset(0)
    , m_ringBufferLock(false)
    , m_ringLockedOffset(0)
    , m_ringLockedSize(0)
{
    auto buffer = new Urho3D::VertexBuffer(context);
    buffer->SetSize(count, elements, isDynamic);
    m_stride = buffer->GetVertexSize();
    m_size = count * m_stride;
    //m_buffer.resize(m_size);
    m_resource = new uint8_t[m_size];
    memset(m_resource, 0, (size_t)m_size);
    m_urho3d_buffer = buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
    delete [] m_resource;
	delete m_urho3d_buffer;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
Effekseer::RefPtr<VertexBuffer> VertexBuffer::Create(Urho3D::Context* context, int count, bool isDynamic,
                                                     unsigned int layoutMask)
{
    return VertexBufferRef(new VertexBuffer(context, count, isDynamic, layoutMask));
}

Effekseer::RefPtr<VertexBuffer> VertexBuffer::Create(Urho3D::Context* context, int count,
                                              bool isDynamic, const Urho3D::PODVector<Urho3D::VertexElement>& elements)
{
    return VertexBufferRef(new VertexBuffer(context, count, isDynamic, elements));
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void VertexBuffer::Lock()
{
	assert(!m_isLock);
	//assert(!m_ringBufferLock);

	m_isLock = true;
	//m_resource = m_buffer.data();
	m_offset = 0;
    m_vertexRingStart = 0;
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
bool VertexBuffer::RingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment)
{
    m_urho3d_buffer->AllocTransientVertexBuffer(size / m_urho3d_buffer->GetVertexSize());
    data = m_urho3d_buffer->GetTransientVertexData();
    return true;

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
        m_vertexRingOffset = size;
        m_vertexRingStart = offset;
    }
    else
    {
        offset = m_vertexRingOffset;
        m_vertexRingOffset += size;
        m_vertexRingStart = offset;
    }

    m_offset = size;
	m_ringBufferLock = true;
    data = m_resource;// = m_buffer.data();

	return true;
}

bool VertexBuffer::TryRingBufferLock(int32_t size, int32_t& offset, void*& data, int32_t alignment)
{
    if ((int32_t)m_vertexRingOffset + size > m_size)
        return false;

	return RingBufferLock(size, offset, data, alignment);
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void VertexBuffer::Unlock()
{
    return;

	assert(m_isLock || m_ringBufferLock);
    m_urho3d_buffer->SetDataRange(m_resource, m_vertexRingStart / m_stride, m_offset / m_stride);
    if (m_isLock) {
        m_vertexRingOffset += m_offset;
    }
	//m_resource = NULL;
	m_isLock = false;
	m_ringBufferLock = false;
}

bool VertexBuffer::IsValid() { return m_urho3d_buffer && m_urho3d_buffer->IsValid(); }

namespace Backend
{
    VertexBuffer::VertexBuffer(Urho3D::Context* context, int count, unsigned int layoutMask, bool isDynamic)
        : isDynamic_{ isDynamic }
    {
        urho3d_buffer_ = std::make_unique<Urho3D::VertexBuffer>(context);
        urho3d_buffer_->SetSize(count, layoutMask, isDynamic);
        stride_ = urho3d_buffer_->GetVertexSize();
    }
    VertexBuffer::VertexBuffer(Urho3D::Context* context, int count, const Urho3D::PODVector<Urho3D::VertexElement>& elements, bool isDynamic)
        : isDynamic_{ isDynamic }
    {
        urho3d_buffer_ = std::make_unique<Urho3D::VertexBuffer>(context);
        urho3d_buffer_->SetSize(count, elements, isDynamic);
        stride_ = urho3d_buffer_->GetVertexSize();
    }
    
    VertexBuffer::~VertexBuffer()
    {
        Deallocate();
    }

    void VertexBuffer::UpdateData(const void* src, int32_t count, int32_t offset)
    {
        urho3d_buffer_->SetDataRange(src, offset, count);
    }

    Effekseer::Backend::VertexBufferRef VertexBuffer::Create(Urho3D::Context* context, int count,
        unsigned int layoutMask, const void* initialData, bool isDynamic)
    {
        //return Effekseer::MakeRefPtr<VertexBuffer>(context, size, isDynamic, layoutMask);
        auto ret = Effekseer::MakeRefPtr<VertexBuffer>(context, count, layoutMask, isDynamic);

        if (!ret->Init(count, isDynamic)) {
            return nullptr;
        }
        if (initialData) {
            ret->UpdateData(initialData, count, 0);
        }
        return ret;
    }

    Effekseer::Backend::VertexBufferRef VertexBuffer::Create(Urho3D::Context* context, int count,
        const Urho3D::PODVector<Urho3D::VertexElement>& elements, const void* initialData, bool isDynamic)
    {
        // return Effekseer::MakeRefPtr<VertexBuffer>(context, size, isDynamic, layoutMask);
        auto ret = Effekseer::MakeRefPtr<VertexBuffer>(context, count, elements, isDynamic);

        if (!ret->Init(count, isDynamic))
        {
            return nullptr;
        }
        if (initialData)
        {
            ret->UpdateData(initialData, count, 0);
        }
        return ret;
    }

    bool VertexBuffer::Allocate(int32_t size, bool isDynamic)
    {
        resources_.resize(static_cast<size_t>(size));
        return true;
    }

    void VertexBuffer::Deallocate()
    {
        urho3d_buffer_ = nullptr;
    }

    bool VertexBuffer::Init(int32_t count, bool isDynamic)
    {
        size_ = count * stride_;
        isDynamic_ = isDynamic;

        return Allocate(size_, isDynamic_);
    }
}

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
} // namespace EffekseerUrho3D
  //-----------------------------------------------------------------------------------
  //
  //-----------------------------------------------------------------------------------