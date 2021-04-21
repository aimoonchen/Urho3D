#pragma once
#include "../Graphics/Drawable.h"

#include "Effekseer.h"
#include "EffekseerRendererCommon/EffekseerRenderer.Renderer.h"

namespace Urho3D {
    class Node;
    struct FrameInfo;
}
namespace efk
{
class EffectManager;
class InternalManager;

class Effect : public Urho3D::RefCount
{
private:
    ::Effekseer::EffectRef effect = nullptr;
    InternalManager* internalManager_ = nullptr;

public:
    static std::unique_ptr<Effect> create(const std::string& filename, float maginification = 1.0f);

    Effect(InternalManager* internalManager = nullptr);

    virtual ~Effect();
    ::Effekseer::EffectRef getInternalPtr() { return effect; }
};

class EffectEmitter : public Urho3D::Drawable
{
    friend class EffectManager;

private:
    bool playOnEnter = false;
    bool removeOnStop = true;
    bool isLooping = false;

    //! whether this effect is played once at least
    bool isPlayedAtLeastOnce = false;

    Urho3D::Vector3 targetPosition_;
    float speed_ = 1.0f;
    Urho3D::Color color_{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, 4> dynamicInputs_;

    std::shared_ptr<EffectManager> manager_ = nullptr;
    std::shared_ptr<Effect> effect_ = nullptr;
    ::Effekseer::Handle handle = -1;

    //cocos2d::CallbackCommand renderCommand;

    void beforeRender(EffekseerRenderer::RendererRef, Effekseer::RefPtr<EffekseerRenderer::CommandList>);
    void afterRender(EffekseerRenderer::RendererRef, Effekseer::RefPtr<EffekseerRenderer::CommandList>);

public:
    static std::unique_ptr<EffectEmitter> create(const std::shared_ptr<EffectManager>& manager);
    static std::unique_ptr<EffectEmitter> create(const std::shared_ptr<EffectManager>& manager, const std::string& filename, float maginification = 1.0f);
    EffectEmitter(Urho3D::Context* context, const std::shared_ptr<EffectManager>& manager);
    virtual ~EffectEmitter();
    Effect* getEffect();
    void setEffect(const std::shared_ptr<Effect>& effect);
    ::Effekseer::Handle getInternalHandle() const;
    void play();
    void play(int32_t startTime);
    bool getPlayOnEnter();
    void setPlayOnEnter(bool value);
    bool getIsLooping();
    void setIsLooping(bool value);
    bool getRemoveOnStop();
    void setRemoveOnStop(bool value);
    void setColor(const Urho3D::Color& color);
    float getSpeed();
    void setSpeed(float speed);
    void setTargetPosition(const Urho3D::Vector3& position);
    float getDynamicInput(int32_t index);
    void setDynamicInput(int32_t index, float value);
    bool isPlaying();
    void stop();
    void stopRoot();
//     void onEnter() override;
//     void onExit() override;
//    void update(float delta) override;
    void Update(const Urho3D::FrameInfo& frame) override;
//    void draw(/*cocos2d::Renderer* renderer, */const Urho3D::Matrix4& parentTransform, uint32_t parentFlags) override;
protected:
    void OnNodeSet(Urho3D::Node* node) override;
    void OnWorldBoundingBoxUpdate() override;
};

class EffectManager : public Urho3D::Object
{
    URHO3D_OBJECT(EffectManager, Object);
	friend class EffectEmitter;
private:
	::Effekseer::ManagerRef manager_ = nullptr;
	::EffekseerRenderer::RendererRef renderer_ = nullptr;
	::EffekseerRenderer::DistortingCallback* distortingCallback = NULL;
	::Effekseer::RefPtr<::EffekseerRenderer::SingleFrameMemoryPool> memoryPool_ = nullptr;
	::Effekseer::RefPtr<::EffekseerRenderer::CommandList> commandList_ = nullptr;
	bool isDistortionEnabled = false;
	bool isDistorted = false;
	float time_ = 0.0f;
	InternalManager* internalManager_ = nullptr;
// 	cocos2d::CustomCommand distortionCommand;
// 	cocos2d::CustomCommand beginCommand;
// 	cocos2d::CustomCommand endCommand;
	::Effekseer::Handle play(Effect* effect, float x, float y, float z);
	::Effekseer::Handle play(Effect* effect, float x, float y, float z, int startTime);
	void setMatrix(::Effekseer::Handle handle, const Urho3D::Matrix4& mat);
	void setPotation(::Effekseer::Handle handle, float x, float y, float z);
	void setRotation(::Effekseer::Handle handle, float x, float y, float z);
	void setScale(::Effekseer::Handle handle, float x, float y, float z);
	
	void CreateRenderer(int32_t spriteSize);
	void onDestructor();

public:
    bool Initialize(int visibleWidth, int visibleHeight);
    //void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	static std::unique_ptr<EffectManager> create(int visibleWidth, int visibleHeight);
    EffectManager(Urho3D::Context* context);
	virtual ~EffectManager();
	void setIsDistortionEnabled(bool value);
	void begin(/*cocos2d::Renderer* renderer, */float globalZOrder);
	void end(/*cocos2d::Renderer* renderer, */float globalZOrder);
	void newFrame();
	void Update(float delta = 1.0f / 60.0f);
    void Render(/*const Urho3D::Matrix4& viewMat*/);
	::Effekseer::ManagerRef getInternalManager() { return manager_; }
	::EffekseerRenderer::RendererRef getInternalRenderer() { return renderer_; }
	void setCameraMatrix(const Urho3D::Matrix4& mat);
	void setProjectionMatrix(const Urho3D::Matrix4& mat);
	Effekseer::RefPtr<::EffekseerRenderer::CommandList> getInternalCommandList() { return commandList_; }
};

class NetworkServer : public Urho3D::RefCount
{
private:
	InternalManager* internalManager_ = nullptr;

public:
	NetworkServer();

	virtual ~NetworkServer();
	static NetworkServer* create();
	bool makeNetworkServerEnabled(uint16_t port);
	void update();
};

}