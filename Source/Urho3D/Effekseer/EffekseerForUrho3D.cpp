#include "EffekseerForUrho3D.h"

#ifdef CC_USE_METAL
#include "renderer/backend/Device.h"
#endif
#include "Urho3DContext.h"
#include "../Scene/Node.h"
#include "../Core/CoreEvents.h"
#include "../Graphics/Texture2D.h"

namespace efk
{
// 	class ImageAccessor : public cocos2d::Image
// 	{
// 	public:
// 		static bool getPngPremultipledAlphaEnabled() { return PNG_PREMULTIPLIED_ALPHA_ENABLED; }
// 	};

	Effekseer::ModelLoaderRef CreateModelLoader(Effekseer::FileInterface*);

	::Effekseer::MaterialLoaderRef CreateMaterialLoader(Effekseer::FileInterface*);

	void UpdateTextureData(::Effekseer::TextureRef textureData, Urho3D::Texture2D* texture);

	void CleanupTextureData(::Effekseer::TextureRef textureData);

	::EffekseerRenderer::DistortingCallback* CreateDistortingCallback(::EffekseerRenderer::RendererRef, Effekseer::RefPtr<::EffekseerRenderer::CommandList>);

	void ResetBackground(::EffekseerRenderer::RendererRef renderer);

	int ccNextPOT(int x)
	{
		x = x - 1;
		x = x | (x >> 1);
		x = x | (x >> 2);
		x = x | (x >> 4);
		x = x | (x >> 8);
		x = x | (x >> 16);
		return x + 1;
	}

	static std::u16string getFilenameWithoutExt(const char16_t* path)
	{
		int start = 0;
		int end = 0;

		for (int i = 0; path[i] != 0; i++)
		{
			if (path[i] == u'/' || path[i] == u'\\')
			{
				start = i + 1;
			}
		}

		for (int i = start; path[i] != 0; i++)
		{
			if (path[i] == u'.')
			{
				end = i;
			}
		}

		std::vector<char16_t> ret;

		for (int i = start; i < end; i++)
		{
			ret.push_back(path[i]);
		}
		ret.push_back(0);

		return std::u16string(ret.data());
	}

	class EffekseerFileReader : public Effekseer::FileReader
	{
		std::vector<uint8_t> data;
		int32_t position;

	public:
		EffekseerFileReader(std::vector<uint8_t>& data)
		{
			this->data = data;
			position = 0;
		}

		virtual ~EffekseerFileReader() {}

		size_t Read(void* buffer, size_t size)
		{
			int32_t readable = size;
			if (data.size() - position < size)
				readable = data.size() - position;

			memcpy(buffer, &(data[position]), readable);
			position += readable;
			return readable;
		}

		void Seek(int position)
		{
			this->position = position;
			if (this->position < 0)
				this->position = 0;
			if (this->position > static_cast<int32_t>(data.size()))
				this->position = static_cast<int32_t>(data.size());
		}

		int GetPosition() { return position; }

		size_t GetLength() { return data.size(); }
	};

	class EffekseerFile : public Effekseer::FileInterface
	{
	public:
		EffekseerFile();
		virtual ~EffekseerFile();
		Effekseer::FileReader* OpenRead(const EFK_CHAR* path);
		Effekseer::FileWriter* OpenWrite(const EFK_CHAR* path);
	};

	EffekseerFile::EffekseerFile() {}
	EffekseerFile::~EffekseerFile() {}

	Effekseer::FileReader* EffekseerFile::OpenRead(const EFK_CHAR* path)
	{
// 		char path_[300];
// 		::Effekseer::ConvertUtf16ToUtf8(path_, 300, path);
// 
// 		cocos2d::Data data_ = cocos2d::FileUtils::getInstance()->getDataFromFile(path_);
// 
// 		if (data_.isNull())
// 		{
// 			return nullptr;
// 		}

		std::vector<uint8_t> data;
// 		data.resize(data_.getSize());
// 		memcpy(data.data(), data_.getBytes(), data.size());
		return new EffekseerFileReader(data);
	}

	Effekseer::FileWriter* EffekseerFile::OpenWrite(const EFK_CHAR* path) { return nullptr; }

	static std::map<Effekseer::TextureRef, std::basic_string<EFK_CHAR>> g_glTex2FilePath;
	static std::map<std::basic_string<EFK_CHAR>, std::shared_ptr<Urho3D::Texture2D>> g_filePath2CTex;
	static std::map<std::basic_string<EFK_CHAR>, Effekseer::TextureRef> g_filePath2EffectData;

	class TextureLoader : public ::Effekseer::TextureLoader
	{
	private:
		::Effekseer::FileInterface* m_fileInterface;
		::Effekseer::DefaultFileInterface m_defaultFileInterface;

	public:
		TextureLoader(::Effekseer::FileInterface* fileInterface = NULL);
		virtual ~TextureLoader();

	public:
		Effekseer::TextureRef Load(const EFK_CHAR* path, ::Effekseer::TextureType textureType) override;

		void Unload(Effekseer::TextureRef data) override;
	};

	TextureLoader::TextureLoader(::Effekseer::FileInterface* fileInterface) : m_fileInterface(fileInterface)
	{
		if (m_fileInterface == NULL)
		{
			m_fileInterface = &m_defaultFileInterface;
		}
	}

	TextureLoader::~TextureLoader() {}

	Effekseer::TextureRef TextureLoader::Load(const EFK_CHAR* path, ::Effekseer::TextureType textureType)
	{
// 		auto key = std::basic_string<EFK_CHAR>(path);
// 		if (g_filePath2CTex.find(key) != g_filePath2CTex.end())
// 		{
// 			auto texture = g_filePath2CTex[key];
// 			texture->retain();
// 			return g_filePath2EffectData[key];
// 		}
// 
// 		std::unique_ptr<Effekseer::FileReader> reader(m_fileInterface->OpenRead(path));
// 
// 		if (reader.get() != NULL)
// 		{
// 			size_t size_texture = reader->GetLength();
// 			char* data_texture = new char[size_texture];
// 			reader->Read(data_texture, size_texture);
// 
// 			cocos2d::Image* image = new cocos2d::Image();
// 			cocos2d::Texture2D* texture = new cocos2d::Texture2D();
// 
// 			auto backup = ImageAccessor::getPngPremultipledAlphaEnabled();
// 			cocos2d::Image::setPNGPremultipliedAlphaEnabled(false);
// 
// 			if (image != nullptr && texture != nullptr && image->initWithImageData((const uint8_t*)data_texture, size_texture))
// 			{
// 				if (texture->initWithImage(image))
// 				{
// 
// #ifdef CC_USE_METAL
// 					texture->generateMipmap();
// #else
// 					if (texture->getPixelsWide() == ccNextPOT(texture->getPixelsWide()) &&
// 						texture->getPixelsHigh() == ccNextPOT(texture->getPixelsHigh()))
// 					{
// 						texture->generateMipmap();
// 					}
// 					else
// 					{
// 						char path8[300];
// 						::Effekseer::ConvertUtf16ToUtf8(path8, 300, path);
// 						CCLOG("%s : The texture is not shown on a mobile. The size is not power of two.", path8);
// 					}
// #endif
// 				}
// 				else
// 				{
// 					CC_SAFE_DELETE(texture);
// 					CC_SAFE_DELETE(image);
// 				}
// 			}
// 			CC_SAFE_DELETE(image);
// 
// 			delete[] data_texture;
// 
// 			Effekseer::TextureRef textureData = Effekseer::MakeRefPtr<Effekseer::Texture>();;
// 			UpdateTextureData(textureData, texture);
// 			g_filePath2CTex[key] = texture;
// 			g_filePath2EffectData[key] = textureData;
// 			g_glTex2FilePath[textureData] = key;
// 
// 			cocos2d::Image::setPNGPremultipliedAlphaEnabled(backup);
// 
// 			return textureData;
// 		}
 		return NULL;
	}

	void TextureLoader::Unload(Effekseer::TextureRef data)
	{
// 		if (data != NULL)
// 		{
// 			auto path = g_glTex2FilePath[data];
// 			auto tex = g_filePath2CTex[path];
// 
// 			if (tex->getReferenceCount() == 1)
// 			{
// 				CleanupTextureData(data);
// 				g_glTex2FilePath.erase(data);
// 				g_filePath2EffectData.erase(path);
// 				g_filePath2CTex.erase(path);
// 			}
// 			tex->release();
// 		}
	}

	class EffekseerSetting;

	static Effekseer::RefPtr<EffekseerSetting> g_effekseerSetting = nullptr;

	class EffekseerSetting : public ::Effekseer::Setting
	{
	protected:
		Effekseer::FileInterface* effectFile = nullptr;

	public:

		EffekseerSetting()
		{
			effectFile = new EffekseerFile();
			SetEffectLoader(Effekseer::Effect::CreateEffectLoader(effectFile));
			SetTextureLoader(Effekseer::MakeRefPtr<TextureLoader>(effectFile));
			SetModelLoader(CreateModelLoader(effectFile));
			SetMaterialLoader(CreateMaterialLoader(effectFile));
			SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>(effectFile));
			// TODO sound
		}

		virtual ~EffekseerSetting()
		{
			delete effectFile;
		}

		static Effekseer::RefPtr<EffekseerSetting> create()
		{
			if (g_effekseerSetting == nullptr)
			{
				g_effekseerSetting = Effekseer::MakeRefPtr<EffekseerSetting>();
			}

			return g_effekseerSetting;
		}

		int Release() override
		{
			auto ret = ::Effekseer::Setting::Release();
			if (ret == 1)
			{
				g_effekseerSetting = nullptr;
			}

			return ret;
		}
	};

	struct EffectResource
	{
		Effekseer::EffectRef effect = nullptr;
		int counter = 0;
	};

	static InternalManager* g_internalManager = nullptr;

	class InternalManager : public Effekseer::ReferenceObject
	{
		std::map<std::u16string, EffectResource> path2effect;
		std::map<Effekseer::EffectRef, std::u16string> effect2path;

		std::set<Effekseer::ManagerRef> managers;
		std::vector<Effekseer::ManagerRef> managersVector;

		Effekseer::Server* server = nullptr;

	public:
		InternalManager() {}

		~InternalManager()
		{
			if (server != nullptr)
			{
				server->Stop();
				ES_SAFE_DELETE(server);
			}

			g_internalManager = nullptr;
		}

		Effekseer::EffectRef loadEffect(const EFK_CHAR* path, float maginification)
		{
			auto it_effect = path2effect.find(path);

			if (it_effect == path2effect.end())
			{
				EffectResource resource;
				auto setting = EffekseerSetting::create();
				resource.effect = Effekseer::Effect::Create(setting.DownCast<Effekseer::Setting>(), path, maginification);
				resource.counter = 1;

				if (resource.effect != nullptr)
				{
					path2effect[path] = resource;
					effect2path[resource.effect] = path;

					if (server != nullptr)
					{
						auto key = getFilenameWithoutExt(path);
						server->Register(key.c_str(), resource.effect);
					}

					return resource.effect;
				}
				return nullptr;
			}
			else
			{
				it_effect->second.counter++;
				return it_effect->second.effect;
			}
		}

		void unloadEffect(Effekseer::EffectRef effect)
		{
			auto it_path = effect2path.find(effect);
			if (it_path == effect2path.end())
				return;

			auto it_effect = path2effect.find(it_path->second);
			if (it_effect == path2effect.end())
				return;

			it_effect->second.counter--;
			if (it_effect->second.counter == 0)
			{
				if (server != nullptr)
				{
					server->Unregister(it_effect->second.effect);
				}

				it_effect->second.effect = nullptr;
				effect2path.erase(it_path);
				path2effect.erase(it_effect);
			}
		}

		void registerManager(Effekseer::ManagerRef manager)
		{
			managers.insert(manager);

			managersVector.clear();
			for (auto m : managers)
			{
				managersVector.push_back(m);
			}
		}

		void unregisterManager(Effekseer::ManagerRef manager)
		{
			managers.erase(manager);
			managersVector.clear();
			for (auto m : managers)
			{
				managersVector.push_back(m);
			}
		}

		bool makeNetworkServerEnabled(uint16_t port)
		{
			if (server != nullptr)
				return false;
			server = Effekseer::Server::Create();
			if (!server->Start(port))
			{
				ES_SAFE_DELETE(server);
				return false;
			}

			return true;
		}

		void update()
		{
			if (server != nullptr)
			{
				if (managersVector.size() > 0)
				{
					server->Update(managersVector.data(), managersVector.size());
				}
				else
				{
					server->Update();
				}
			}
		}
	};

	InternalManager* getGlobalInternalManager()
	{
		if (g_internalManager == nullptr)
		{
			g_internalManager = new InternalManager();
		}
		else
		{
			g_internalManager->AddRef();
		}

		return g_internalManager;
	}

	std::unique_ptr<Effect> Effect::create(const std::string& filename, float maginification)
	{
		EFK_CHAR path_[300];
		::Effekseer::ConvertUtf8ToUtf16(path_, 300, filename.c_str());

		auto internalManager = getGlobalInternalManager();

		auto effect = internalManager->loadEffect(path_, maginification);

		if (effect != nullptr)
		{
			auto e = std::make_unique<Effect>(internalManager);
			e->effect = effect;
			ES_SAFE_RELEASE(internalManager);
			return e;
		}

		ES_SAFE_RELEASE(internalManager);
		return nullptr;
	}

	Effect::Effect(InternalManager* internalManager)
	{
		internalManager_ = internalManager;
		ES_SAFE_ADDREF(internalManager_);
	}

	Effect::~Effect()
	{
		if (internalManager_ != nullptr)
		{
			internalManager_->unloadEffect(effect);
		}

		ES_SAFE_RELEASE(internalManager_);
	}

	std::unique_ptr<EffectEmitter> EffectEmitter::create(const std::shared_ptr<EffectManager>& manager) { return std::make_unique<EffectEmitter>(nullptr, manager); }

	std::unique_ptr<EffectEmitter> EffectEmitter::create(const std::shared_ptr<EffectManager>& manager, const std::string& filename, float maginification)
	{
		auto effectEmitter = std::make_unique<EffectEmitter>(nullptr, manager);
		std::shared_ptr<Effect> effect = Effect::create(filename, maginification);
		effectEmitter->setEffect(effect);
		effectEmitter->playOnEnter = true;
		return effectEmitter;
	}

	EffectEmitter::EffectEmitter(Urho3D::Context* context, const std::shared_ptr<EffectManager>& manager)
		: Urho3D::Drawable(context, Urho3D::DRAWABLE_GEOMETRY)
	{
		manager_ = manager;
		dynamicInputs_.fill(0.0f);
	}

	EffectEmitter::~EffectEmitter()
	{

	}

	Effect* EffectEmitter::getEffect() { return effect_.get(); }

	void EffectEmitter::setEffect(const std::shared_ptr<Effect>& effect)
	{
		effect_ = effect;
	}

	::Effekseer::Handle EffectEmitter::getInternalHandle() const { return handle; }

	void EffectEmitter::play() { play(0); }

	void EffectEmitter::play(int32_t startTime)
	{
		if (effect_ == nullptr)
			return;
		if (manager_ == nullptr)
			return;

		if (startTime == 0)
		{
			handle = manager_->play(effect_.get(), 0, 0, 0, 0);
		}
		else
		{
			handle = manager_->play(effect_.get(), 0, 0, 0, startTime);
		}

		auto transform = node_->GetWorldTransform().ToMatrix4();// this->getNodeToWorldTransform();
		manager_->setMatrix(handle, transform);
		isPlayedAtLeastOnce = true;

		setTargetPosition(targetPosition_);
		setColor(color_);
		setSpeed(speed_);

		for (size_t i = 0; i < 4; i++)
		{
			setDynamicInput(i, dynamicInputs_[i]);
		}
	}

	bool EffectEmitter::getPlayOnEnter() { return playOnEnter; }

	void EffectEmitter::setPlayOnEnter(bool value) { playOnEnter = value; }

	bool EffectEmitter::getIsLooping() { return isLooping; }

	void EffectEmitter::setIsLooping(bool value) { isLooping = value; }

	bool EffectEmitter::getRemoveOnStop() { return removeOnStop; }

	void EffectEmitter::setRemoveOnStop(bool value) { removeOnStop = value; }

	void EffectEmitter::setColor(const Urho3D::Color& color)
	{
		color_ = color;
		Effekseer::Color col;
		col.R = color.r_ * 255.0f;
		col.G = color.g_ * 255.0f;
		col.B = color.b_ * 255.0f;
		col.A = color.a_ * 255.0f;
		manager_->getInternalManager()->SetAllColor(handle, col);
	}

	float EffectEmitter::getSpeed()
	{
		return speed_;
		// return manager->getInternalManager()->GetSpeed(handle);
	}

	void EffectEmitter::setSpeed(float speed)
	{
		speed_ = speed;
		manager_->getInternalManager()->SetSpeed(handle, speed);
	}

	void EffectEmitter::setTargetPosition(const Urho3D::Vector3& position)
	{
		targetPosition_ = position;
		manager_->getInternalManager()->SetTargetLocation(handle, position.x_, position.y_, position.z_);
	}

	float EffectEmitter::getDynamicInput(int32_t index)
	{
		return dynamicInputs_.at(index);
	}

	void EffectEmitter::setDynamicInput(int32_t index, float value)
	{
		dynamicInputs_.at(index) = value;
		manager_->getInternalManager()->SetDynamicInput(handle, index, value);
	}

	bool EffectEmitter::isPlaying() { return manager_->getInternalManager()->Exists(handle); }

	void EffectEmitter::stop() { manager_->getInternalManager()->StopEffect(handle); }

	void EffectEmitter::stopRoot() { manager_->getInternalManager()->StopRoot(handle); }

// 	void EffectEmitter::update(float delta)
// 	{
// 		auto m = manager_->getInternalManager();
// 		if (!m->Exists(handle))
// 		{
// 			if (isLooping)
// 			{
// 				play();
// 			}
// 			else if (removeOnStop && isPlayedAtLeastOnce)
// 			{
// 				auto transform = node_->GetWorldTransform().ToMatrix4();// this->getNodeToWorldTransform();
// 				manager_->setMatrix(handle, transform);
// 				cocos2d::Node::update(delta);
// 
// 				this->removeFromParent();
// 				return;
// 			}
// 		}
// 
// 		{
// 			auto transform = node_->GetWorldTransform().ToMatrix4();// this->getNodeToWorldTransform();
// 			manager_->setMatrix(handle, transform);
// 
// 			cocos2d::Node::update(delta);
// 		}
// 	}
    
// 	void EffectEmitter::draw(/*cocos2d::Renderer* renderer, */const Urho3D::Matrix4& parentTransform, uint32_t parentFlags)
// 	{
// 		if (!manager_->getInternalManager()->GetShown(handle) ||
// 			manager_->getInternalManager()->GetTotalInstanceCount() < 1)
// 			return; // nothing to draw
// 
// #ifdef CC_USE_METAL
// 		if (!manager_->isDistorted)
// 		{
// 			// allow frame buffer texture to be copied for distortion
// 			cocos2d::backend::Device::getInstance()->setFrameBufferOnly(false);
// 		}
// #endif
// 
// 		renderCommand.init(_globalZOrder);
// 
// 		auto renderer2d = manager_->getInternalRenderer();
// 		Effekseer::Matrix44 mCamera = renderer2d->GetCameraMatrix();
// 		Effekseer::Matrix44 mProj = renderer2d->GetProjectionMatrix();
// 		renderCommand.func = [=]() -> void {
// 			renderer2d->SetCameraMatrix(mCamera);
// 			renderer2d->SetProjectionMatrix(mProj);
// 
// #ifdef CC_USE_METAL
// 			auto commandList = manager_->getInternalCommandList();
// 			beforeRender(renderer2d, commandList);
// #endif
// 			renderer2d->SetRestorationOfStatesFlag(true);
// 			renderer2d->BeginRendering();
// 			manager_->getInternalManager()->DrawHandle(handle);
// 			renderer2d->EndRendering();
// 
// 			// Count drawcall and vertex
// 			renderer->addDrawnBatches(renderer2d->GetDrawCallCount());
// 			renderer->addDrawnVertices(renderer2d->GetDrawVertexCount());
// 			renderer2d->ResetDrawCallCount();
// 			renderer2d->ResetDrawVertexCount();
// 
// #ifdef CC_USE_METAL
// 			afterRender(renderer2d, commandList);
// #endif
// 
// 		};
// 
// 		renderer->addCommand(&renderCommand);
// 
// 		cocos2d::Node::draw(renderer, parentTransform, parentFlags);
// 	}
	
	void EffectEmitter::Update(const Urho3D::FrameInfo& frame)
	{
        auto m = manager_->getInternalManager();
        if (!m->Exists(handle))
        {
            if (isLooping)
            {
                play();
            }
            else if (removeOnStop && isPlayedAtLeastOnce)
            {
                auto transform = node_->GetWorldTransform().ToMatrix4(); // this->getNodeToWorldTransform();
                manager_->setMatrix(handle, transform);
                //cocos2d::Node::update(delta);
				Drawable::Update(frame);
				Remove();
                //this->removeFromParent();
                return;
            }
        }

        {
            auto transform = node_->GetWorldTransform().ToMatrix4(); // this->getNodeToWorldTransform();
            manager_->setMatrix(handle, transform);

            //cocos2d::Node::update(delta);
			Drawable::Update(frame);
        }
	}
    
	void EffectEmitter::OnWorldBoundingBoxUpdate()
	{
        worldBoundingBox_ = boundingBox_.Transformed(node_->GetWorldTransform());
	}

	void EffectEmitter::OnNodeSet(Urho3D::Node* node)
	{
		if (node)
		{
			EffectEmitter::OnNodeSet(node);

			if (playOnEnter)
			{
				play();
			}

			//scheduleUpdate();
		}
		else
		{
            auto m = manager_->getInternalManager();
            if (m->Exists(handle))
            {
                manager_->getInternalManager()->StopEffect(handle);
            }

			EffectEmitter::OnNodeSet(node);
		}
	}

	::Effekseer::Handle EffectManager::play(Effect* effect, float x, float y, float z)
	{
		return manager_->Play(effect->getInternalPtr(), x, y, z);
	}

	::Effekseer::Handle EffectManager::play(Effect* effect, float x, float y, float z, int startTime)
	{
		return manager_->Play(effect->getInternalPtr(), Effekseer::Vector3D(x, y, z), startTime);
	}

	void EffectManager::setMatrix(::Effekseer::Handle handle, const Urho3D::Matrix4& mat)
	{
		Effekseer::Matrix43 mat_;
		const float* p = &mat.m00_;
		int size = sizeof(float) * 3;
		memcpy(mat_.Value[0], p, size);
		p += 4;
		memcpy(mat_.Value[1], p, size);
		p += 4;
		memcpy(mat_.Value[2], p, size);
		p += 4;
		memcpy(mat_.Value[3], p, size);
		manager_->SetMatrix(handle, mat_);
	}

	void EffectManager::setPotation(::Effekseer::Handle handle, float x, float y, float z) { manager_->SetLocation(handle, x, y, z); }

	void EffectManager::setRotation(::Effekseer::Handle handle, float x, float y, float z) { manager_->SetRotation(handle, x, y, z); }

	void EffectManager::setScale(::Effekseer::Handle handle, float x, float y, float z) { manager_->SetScale(handle, x, y, z); }
    
	bool EffectManager::Initialize(int visibleWidth, int visibleHeight)
	{
		// large buffer make application slow on Android
		int32_t spriteSize = 600;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		spriteSize = 4000;
#endif

		CreateRenderer(spriteSize);

		manager_ = ::Effekseer::Manager::Create(8000);

		// set camera and projection matrix for 2d
		// If you special camera or 3d, please set yourself with setCameraMatrix and setProjectionMatrix
		renderer_->SetProjectionMatrix(::Effekseer::Matrix44().OrthographicLH(visibleWidth, visibleHeight, 1.0f, 400.0f));

		renderer_->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtLH(::Effekseer::Vector3D(visibleWidth / 2.0f, visibleHeight / 2.0f, 200.0f),
				::Effekseer::Vector3D(visibleWidth / 2.0f, visibleHeight / 2.0f, -200.0f),
				::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

		distortingCallback = CreateDistortingCallback(renderer_, commandList_);

		manager_->SetSpriteRenderer(renderer_->CreateSpriteRenderer());
		manager_->SetRibbonRenderer(renderer_->CreateRibbonRenderer());
		manager_->SetRingRenderer(renderer_->CreateRingRenderer());
		manager_->SetModelRenderer(renderer_->CreateModelRenderer());
		manager_->SetTrackRenderer(renderer_->CreateTrackRenderer());
		//
		manager_->SetTextureLoader(renderer_->CreateTextureLoader());
        manager_->SetModelLoader(renderer_->CreateModelLoader());
        manager_->SetMaterialLoader(renderer_->CreateMaterialLoader());
        manager_->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

		internalManager_ = getGlobalInternalManager();
		internalManager_->registerManager(manager_);

        //SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(EffectManager, HandleUpdate));
		SubscribeToEvent(Urho3D::E_RENDERUPDATE, URHO3D_HANDLER(EffectManager, HandleRenderUpdate));
		return true;
	}

	std::unique_ptr<EffectManager> EffectManager::create(int visibleWidth, int visibleHeight)
	{
		auto ret = std::make_unique<EffectManager>(GetUrho3DContext());
		if (ret->Initialize(visibleWidth, visibleHeight))
		{
			return ret;
		}
		return nullptr;
	}

	EffectManager::EffectManager(Urho3D::Context* context)
        : Object(context)
	{
	}

	EffectManager::~EffectManager()
	{
		if (distortingCallback != nullptr &&
			renderer_->GetDistortingCallback() != distortingCallback)
		{
			delete distortingCallback;
			distortingCallback = nullptr;
		}

		onDestructor();

		if (manager_ != nullptr)
		{
			internalManager_->unregisterManager(manager_);
			manager_ = nullptr;
		}

		if (renderer_ != nullptr)
		{
			renderer_ = nullptr;
		}

		memoryPool_.Reset();
		commandList_.Reset();
		ES_SAFE_RELEASE(internalManager_);
	}
	
	void EffectManager::HandleRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
	{
		float timeStep = eventData[Urho3D::RenderUpdate::P_TIMESTEP].GetFloat();
		Update(timeStep);
	}

	void EffectManager::setIsDistortionEnabled(bool value)
	{
		isDistortionEnabled = value;
		if (isDistortionEnabled)
		{
			renderer_->SetDistortingCallback(distortingCallback);
		}
		else
		{
			renderer_->SetDistortingCallback(nullptr);
		}
	}

	void EffectManager::begin(/*cocos2d::Renderer* renderer, */float globalZOrder)
	{
		if (isDistortionEnabled)
		{
			isDistorted = false;
		}
		else
		{
			isDistorted = true;
			ResetBackground(renderer_);
		}

		newFrame();

		// TODO Batch render
		/*
		beginCommand.init(globalZOrder);
		beginCommand.func = [this]() -> void
		{
			renderer2d->SetRestorationOfStatesFlag(true);
			renderer2d->BeginRendering();
			manager2d->Draw();

		};




		renderer->addCommand(&beginCommand);
		*/
	}

	void EffectManager::end(/*cocos2d::Renderer* renderer, */float globalZOrder)
	{
		// TODO Batch render
		/*
		endCommand.init(globalZOrder);
		endCommand.func = [this]() -> void
		{
			renderer2d->ResetRenderState();
			renderer2d->EndRendering();
		};

		renderer->addCommand(&endCommand);
		*/
	}

	void EffectManager::setCameraMatrix(const Urho3D::Matrix4& mat)
	{
		Effekseer::Matrix44 mat_;
		memcpy(mat_.Values, &mat.m00_, sizeof(float) * 16);
		getInternalRenderer()->SetCameraMatrix(mat_);
	}

	void EffectManager::setProjectionMatrix(const Urho3D::Matrix4& mat)
	{
		Effekseer::Matrix44 mat_;
		memcpy(mat_.Values, &mat.m00_, sizeof(float) * 16);
		getInternalRenderer()->SetProjectionMatrix(mat_);
	}

	void EffectManager::Update(float delta)
	{
		// Stabilize in a variable frame environment
        float deltaFrames = delta * 60.0f;
        int iterations = std::max(1, (int)roundf(deltaFrames));
        float advance = deltaFrames / iterations;
        for (int i = 0; i < iterations; i++)
        {
            manager_->Update(advance);
        }
		time_ += delta;
		renderer_->SetTime(time_);
	}
	void EffectManager::Render(/*const Urho3D::Matrix4& viewMat*/)
	{
//         Effekseer::Matrix44 matrix = EffekseerGodot::ToEfkMatrix44(camera_transform.inverse());
//         renderer_->SetCameraMatrix(matrix);
		renderer_->BeginRendering();
		manager_->Draw();
		renderer_->EndRendering();
	}
	NetworkServer* NetworkServer::create() { return new NetworkServer(); }

	NetworkServer::NetworkServer() { internalManager_ = getGlobalInternalManager(); }

	NetworkServer::~NetworkServer() { ES_SAFE_RELEASE(internalManager_); }

	bool NetworkServer::makeNetworkServerEnabled(uint16_t port) { return internalManager_->makeNetworkServerEnabled(port); }

	void NetworkServer::update() { internalManager_->update(); }
} // namespace efk
