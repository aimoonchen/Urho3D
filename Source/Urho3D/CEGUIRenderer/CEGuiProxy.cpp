#ifdef __APPLE__
#   include <Carbon/Carbon.h>
#endif
#include "CEGuiProxy.h"
// #include "CEGUISamplesConfig.h"
// #include "CEGuiBaseApplication.h"
// #include "SampleBrowser.h"
#include "CEGUI/System.h"
#include "CEGUI/DefaultResourceProvider.h"
#include "CEGUI/ImageManager.h"
#include "CEGUI/Image.h"
#include "CEGUI/Font.h"
#include "CEGUI/Scheme.h"
#include "CEGUI/WindowManager.h"
#include "CEGUI/falagard/WidgetLookManager.h"
#include "CEGUI/ScriptModule.h"
#include "CEGUI/XMLParser.h"
#include "CEGUI/GeometryBuffer.h"
#include "CEGUI/GUIContext.h"
#include "CEGUI/RenderTarget.h"
#include "CEGUI/AnimationManager.h"
#include "CEGUI/FontManager.h"
#include "CEGUI/InputAggregator.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sstream>

#include "Urho3D/Input/InputConstants.h"
// setup default-default path
#ifndef CEGUI_SAMPLE_DATAPATH
#define CEGUI_SAMPLE_DATAPATH "../datafiles"
#endif

/***********************************************************************
	Static / Const data
*************************************************************************/
const char CEGuiBaseApplication::DATAPATH_VAR_NAME[] = "CEGUI_SAMPLE_DATAPATH";
//SampleBrowserBase* CEGuiBaseApplication::d_sampleApp(nullptr);

CEGUI::Key::Scan urho3DKeyToCeguiKey(unsigned key)
{
	switch (key)
	{
	case Urho3D::KEY_ESCAPE: return CEGUI::Key::Scan::Esc;
	case Urho3D::KEY_F1: return CEGUI::Key::Scan::F1;
	case Urho3D::KEY_F2: return CEGUI::Key::Scan::F2;
	case Urho3D::KEY_F3: return CEGUI::Key::Scan::F3;
	case Urho3D::KEY_F4: return CEGUI::Key::Scan::F4;
	case Urho3D::KEY_F5: return CEGUI::Key::Scan::F5;
	case Urho3D::KEY_F6: return CEGUI::Key::Scan::F6;
	case Urho3D::KEY_F7: return CEGUI::Key::Scan::F7;
	case Urho3D::KEY_F8: return CEGUI::Key::Scan::F8;
	case Urho3D::KEY_F9: return CEGUI::Key::Scan::F9;
	case Urho3D::KEY_F10: return CEGUI::Key::Scan::F10;
	case Urho3D::KEY_F11: return CEGUI::Key::Scan::F11;
	case Urho3D::KEY_F12: return CEGUI::Key::Scan::F12;
	case Urho3D::KEY_F13: return CEGUI::Key::Scan::F13;
	case Urho3D::KEY_F14: return CEGUI::Key::Scan::F14;
	case Urho3D::KEY_F15: return CEGUI::Key::Scan::F15;
	case Urho3D::KEY_UP: return CEGUI::Key::Scan::ArrowUp;
	case Urho3D::KEY_DOWN: return CEGUI::Key::Scan::ArrowDown;
	case Urho3D::KEY_LEFT: return CEGUI::Key::Scan::ArrowLeft;
	case Urho3D::KEY_RIGHT: return CEGUI::Key::Scan::ArrowRight;
	case Urho3D::KEY_LSHIFT: return CEGUI::Key::Scan::LeftShift;
	case Urho3D::KEY_RSHIFT: return CEGUI::Key::Scan::RightShift;
	case Urho3D::KEY_LCTRL: return CEGUI::Key::Scan::LeftControl;
	case Urho3D::KEY_RCTRL: return CEGUI::Key::Scan::RightControl;
	case Urho3D::KEY_LALT: return CEGUI::Key::Scan::LeftAlt;
	case Urho3D::KEY_RALT: return CEGUI::Key::Scan::RightAlt;
	case Urho3D::KEY_TAB: return CEGUI::Key::Scan::Tab;
	case Urho3D::KEY_RETURN: return CEGUI::Key::Scan::Return;
	case Urho3D::KEY_BACKSPACE: return CEGUI::Key::Scan::Backspace;
	case Urho3D::KEY_INSERT: return CEGUI::Key::Scan::Insert;
	case Urho3D::KEY_DELETE: return CEGUI::Key::Scan::DeleteKey;
	case Urho3D::KEY_PAGEUP: return CEGUI::Key::Scan::PageUp;
	case Urho3D::KEY_PAGEDOWN: return CEGUI::Key::Scan::PageDown;
	case Urho3D::KEY_HOME: return CEGUI::Key::Scan::Home;
	case Urho3D::KEY_END: return CEGUI::Key::Scan::End;
	case Urho3D::KEY_KP_ENTER: return CEGUI::Key::Scan::NumpadEnter;
	case Urho3D::KEY_SPACE: return CEGUI::Key::Scan::Space;
	case Urho3D::KEY_A: return CEGUI::Key::Scan::A;
	case Urho3D::KEY_B: return CEGUI::Key::Scan::B;
	case Urho3D::KEY_C: return CEGUI::Key::Scan::C;
	case Urho3D::KEY_D: return CEGUI::Key::Scan::D;
	case Urho3D::KEY_E: return CEGUI::Key::Scan::E;
	case Urho3D::KEY_F: return CEGUI::Key::Scan::F;
	case Urho3D::KEY_G: return CEGUI::Key::Scan::G;
	case Urho3D::KEY_H: return CEGUI::Key::Scan::H;
	case Urho3D::KEY_I: return CEGUI::Key::Scan::I;
	case Urho3D::KEY_J: return CEGUI::Key::Scan::J;
	case Urho3D::KEY_K: return CEGUI::Key::Scan::K;
	case Urho3D::KEY_L: return CEGUI::Key::Scan::L;
	case Urho3D::KEY_M: return CEGUI::Key::Scan::M;
	case Urho3D::KEY_N: return CEGUI::Key::Scan::N;
	case Urho3D::KEY_O: return CEGUI::Key::Scan::O;
	case Urho3D::KEY_P: return CEGUI::Key::Scan::P;
	case Urho3D::KEY_Q: return CEGUI::Key::Scan::Q;
	case Urho3D::KEY_R: return CEGUI::Key::Scan::R;
	case Urho3D::KEY_S: return CEGUI::Key::Scan::S;
	case Urho3D::KEY_T: return CEGUI::Key::Scan::T;
	case Urho3D::KEY_U: return CEGUI::Key::Scan::U;
	case Urho3D::KEY_V: return CEGUI::Key::Scan::V;
	case Urho3D::KEY_W: return CEGUI::Key::Scan::W;
	case Urho3D::KEY_X: return CEGUI::Key::Scan::X;
	case Urho3D::KEY_Y: return CEGUI::Key::Scan::Y;
	case Urho3D::KEY_Z: return CEGUI::Key::Scan::Z;
	default: return CEGUI::Key::Scan::Unknown;
	}
}

//----------------------------------------------------------------------------//
CEGuiBaseApplication::CEGuiBaseApplication() :
	d_quitting(false),
	d_renderer(nullptr),
	d_imageCodec(nullptr),
	d_resourceProvider(nullptr),
	d_logoGeometry(0),
	d_FPSGeometry(0),
	d_FPSElapsed(1.0f),
	d_FPSFrames(0),
	d_FPSValue(0),
	d_spinLogo(false)
{
}

//----------------------------------------------------------------------------//
CEGuiBaseApplication::~CEGuiBaseApplication()
{
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::renderSingleFrame(const float elapsed)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());

	gui_system.injectTimePulse(elapsed);
	//d_sampleApp->update(static_cast<float>(elapsed));

	updateFPS(elapsed);
	updateLogo(elapsed);

	beginRendering(elapsed);

	CEGUI::Renderer* gui_renderer(gui_system.getRenderer());
	gui_renderer->beginRendering();

	//d_sampleApp->renderGUIContexts();

	//CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.getDefaultGUIContext().draw();

	gui_renderer->endRendering();
	CEGUI::WindowManager::getSingleton().cleanDeadPool();

	endRendering();
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::init(SampleBrowserBase* sampleApp,
	const CEGUI::String& logFile, const CEGUI::String& dataPathPrefixOverride)
{
	//d_sampleApp = sampleApp;

	d_systemInputAggregator = new CEGUI::InputAggregator(&CEGUI::System::getSingletonPtr()->getDefaultGUIContext());
	d_systemInputAggregator->initialise();

	if (!d_renderer)
		throw CEGUI::InvalidRequestException("CEGuiBaseApplication::run: "
			"Base application subclass did not create Renderer!");

	// start up CEGUI system using objects created in subclass constructor.
	CEGUI::System::create(*d_renderer, d_resourceProvider, nullptr, d_imageCodec, nullptr, "", logFile);

	// initialise resource system
	initDataPathPrefix(dataPathPrefixOverride);
	initialiseResourceGroupDirectories();
	initialiseDefaultResourceGroups();

	// create logo imageset and draw the image (we only ever draw this once)
	CEGUI::ImageManager::getSingleton().addBitmapImageFromFile("cegui_logo",
		"logo.png");

	CEGUI::Image& ceguiLogo = CEGUI::ImageManager::getSingleton().get("cegui_logo");

	CEGUI::ImageRenderSettings imgRenderSettings(CEGUI::Rectf(0, 0, 183, 89));

	auto ceguiLogoGeomBuffers = ceguiLogo.createRenderGeometry(imgRenderSettings);

	d_logoGeometry.insert(d_logoGeometry.end(), ceguiLogoGeomBuffers.begin(), ceguiLogoGeomBuffers.end());

	// initial position update of the logo
	updateLogoGeometry();
	// setup for spinning logo
	updateLogoGeometryRotation();

	// clearing this queue actually makes sure it's created(!)
	CEGUI::System::getSingleton().getDefaultGUIContext().clearGeometry(CEGUI::RenderQueueID::Overlay);

	// subscribe handler to render overlay items
	CEGUI::System::getSingleton().getDefaultGUIContext().
		subscribeEvent(CEGUI::RenderingSurface::EventRenderQueueStarted,
			CEGUI::Event::Subscriber(&CEGuiBaseApplication::sampleBrowserOverlayHandler,
				this));

	// subscribe handler to reposition logo when window is sized.
	CEGUI::System::getSingleton().subscribeEvent(
		CEGUI::System::EventDisplaySizeChanged,
		CEGUI::Event::Subscriber(&CEGuiBaseApplication::resizeHandler,
			this));

	const CEGUI::Rectf& area(CEGUI::System::getSingleton().getRenderer()->
		getDefaultRenderTarget().getArea());
	//d_sampleApp->setApplicationWindowSize(static_cast<int>(area.getWidth()), static_cast<int>(area.getHeight()));

	return true;
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::cleanup()
{
	if (d_renderer)
	{
		CEGUI::ImageManager::getSingleton().destroy("cegui_logo");

		const size_t logo_buffer_count = d_logoGeometry.size();
		for (size_t i = 0; i < logo_buffer_count; ++i)
			d_renderer->destroyGeometryBuffer(*d_logoGeometry.at(i));
		d_logoGeometry.clear();

		const size_t fps_buffer_count = d_FPSGeometry.size();
		for (size_t i = 0; i < fps_buffer_count; ++i)
			d_renderer->destroyGeometryBuffer(*d_FPSGeometry.at(i));
		d_FPSGeometry.clear();

		CEGUI::System::destroy();
		destroyRenderer();
		destroyWindow();
	}
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::destroyRenderer()
{
	delete d_renderer;
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::initialiseResourceGroupDirectories()
{
	// initialise the required dirs for the DefaultResourceProvider
	CEGUI::DefaultResourceProvider* rp =
		static_cast<CEGUI::DefaultResourceProvider*>
		(CEGUI::System::getSingleton().getResourceProvider());
	CEGUI::String dataPathPrefix(getDataPathPrefix());

	/* for each resource type, set a resource group directory. We cast strings
	   to "const CEGUI::utf8*" in order to support general Unicode strings,
	   rather than only ASCII strings (even though currently they're all ASCII).
	   */
	rp->setResourceGroupDirectory("schemes",
		dataPathPrefix + "/schemes/");
	rp->setResourceGroupDirectory("imagesets",
		dataPathPrefix + "/imagesets/");
	rp->setResourceGroupDirectory("fonts",
		dataPathPrefix + "/fonts/");
	rp->setResourceGroupDirectory("layouts",
		dataPathPrefix + "/layouts/");
	rp->setResourceGroupDirectory("looknfeels",
		dataPathPrefix + "/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts",
		dataPathPrefix + "/lua_scripts/");
	rp->setResourceGroupDirectory("schemas",
		dataPathPrefix + "/xml_schemas/");
	rp->setResourceGroupDirectory("animations",
		dataPathPrefix + "/animations/");
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::initialiseDefaultResourceGroups()
{
	// set the default resource groups to be used
	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
	CEGUI::AnimationManager::setDefaultResourceGroup("animations");

	// setup default group for validation schemas
	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::initDataPathPrefix(const CEGUI::String &override)
{

	if (override.empty())
	{

#       ifdef __APPLE__

		char c_str[PATH_MAX];
		CFURLRef datafilesURL = CFBundleCopyResourceURL(CFBundleGetMainBundle(),
			CFSTR("datafiles"),
			0, 0);
		CFURLGetFileSystemRepresentation(datafilesURL, true,
			reinterpret_cast<UInt8*>(c_str),
			PATH_MAX);
		CFRelease(datafilesURL);
		d_dataPathPrefix = c_str;

#       else

		// get data path from environment var
		char* envDataPath = getenv(DATAPATH_VAR_NAME);

		// set data path prefix / base directory.  This will
		// be either from an environment variable, or from
		// a compiled in default based on original configure
		// options
		d_dataPathPrefix = envDataPath ?
			envDataPath : CEGUI_SAMPLE_DATAPATH;

#       endif

	}
	else
		d_dataPathPrefix = override;

}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::sampleBrowserOverlayHandler(const CEGUI::EventArgs& args)
{
	if (static_cast<const CEGUI::RenderQueueEventArgs&>(args).queueID != CEGUI::RenderQueueID::Overlay)
		return false;

	// draw the logo
	const size_t logo_buffer_count = d_logoGeometry.size();
	for (size_t i = 0; i < logo_buffer_count; ++i)
	{
		d_logoGeometry[i]->draw();
	}

	// draw FPS value
	const size_t fps_buffer_count = d_FPSGeometry.size();
	for (size_t i = 0; i < fps_buffer_count; ++i)
		d_FPSGeometry.at(i)->draw();

	return true;
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::sampleOverlayHandler(const CEGUI::EventArgs& args)
{
	if (static_cast<const CEGUI::RenderQueueEventArgs&>(args).queueID != CEGUI::RenderQueueID::Overlay)
		return false;

	// Draw FPS value
	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
		d_FPSGeometry.at(i)->draw();

	return true;
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::updateFPS(const float elapsed)
{
	// another frame
	++d_FPSFrames;

	if ((d_FPSElapsed += elapsed) >= 1.0f)
	{
		if (d_FPSFrames != d_FPSValue)
		{
			d_FPSValue = d_FPSFrames;

			CEGUI::Font* fnt = &CEGUI::FontManager::getSingleton().get("DejaVuSans-12");
			if (!fnt)
				return;

			// update FPS imagery
			std::stringstream sstream;
			sstream << "FPS: " << d_FPSValue;

			const size_t bufferCount = d_FPSGeometry.size();
			for (size_t i = 0; i < bufferCount; ++i)
				d_renderer->destroyGeometryBuffer(*d_FPSGeometry.at(i));
			d_FPSGeometry.clear();

			auto textGeomBuffers = fnt->createTextRenderGeometry(
				sstream.str(), glm::vec2(0, 0), nullptr, false,
				CEGUI::Colour(0xFFFFFFFF), CEGUI::DefaultParagraphDirection::LeftToRight);

			d_FPSGeometry.insert(d_FPSGeometry.end(), textGeomBuffers.begin(),
				textGeomBuffers.end());

			updateFPSGeometry();
		}

		// reset counter state
		d_FPSFrames = 0;

		float modValue = 1.0f;
		d_FPSElapsed = std::modf(d_FPSElapsed, &modValue);
	}
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::updateLogo(const float elapsed)
{
	if (!d_spinLogo)
		return;

	static float rot = 0.0f;

	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
	{
		d_logoGeometry[i]->setRotation(glm::quat(glm::vec3(glm::radians(rot), 0, 0)));
	}

	rot = fmodf(rot + 180.0f * elapsed, 360.0f);
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::updateLogoGeometry()
{
	const CEGUI::Rectf scrn(d_renderer->getDefaultRenderTarget().getArea());
	const glm::vec3 position(10.0f, scrn.getSize().d_height - 89.0f, 0.0f);

	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
	{
		d_logoGeometry[i]->setClippingRegion(scrn);
		d_logoGeometry[i]->setTranslation(position);
	}
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::updateFPSGeometry()
{
	const CEGUI::Rectf scrn(d_renderer->getDefaultRenderTarget().getArea());
	const glm::vec3 position(scrn.getSize().d_width - 120.0f, 0.0f, 0.0f);

	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
	{
		d_FPSGeometry[i]->setClippingRegion(scrn);
		d_FPSGeometry[i]->setTranslation(position);
	}
}


//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::resizeHandler(const CEGUI::EventArgs& /*args*/)
{
	// clear FPS geometry and see that it gets recreated in the next frame
	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
		d_renderer->destroyGeometryBuffer(*d_FPSGeometry.at(i));
	d_FPSGeometry.clear();

	d_FPSValue = 0;

	const CEGUI::Rectf& area(CEGUI::System::getSingleton().getRenderer()->getDefaultRenderTarget().getArea());
	//d_sampleApp->handleNewWindowSize(area.getWidth(), area.getHeight());

	updateLogoGeometry();
	updateFPSGeometry();

	return true;
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::registerSampleOverlayHandler(CEGUI::GUIContext* gui_context)
{
	// clearing this queue actually makes sure it's created(!)
	gui_context->clearGeometry(CEGUI::RenderQueueID::Overlay);

	// subscribe handler to render overlay items
	gui_context->subscribeEvent(CEGUI::RenderingSurface::EventRenderQueueStarted,
		CEGUI::Event::Subscriber(&CEGuiBaseApplication::sampleOverlayHandler,
			this));
}

//----------------------------------------------------------------------------//
void CEGuiBaseApplication::updateLogoGeometryRotation()
{
	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
	{
		d_logoGeometry[i]->setPivot(glm::vec3(91.5f, 44.5f, 0));
	}
}

//

CEGUI::InputAggregator* CEGuiBaseApplication::getCurrentInputAggregator()
{
// 	if (d_selectedSampleData != nullptr)
// 		return d_selectedSampleData->getInputAggregator();

	return d_systemInputAggregator;
}

bool CEGuiBaseApplication::injectKeyDown(const CEGUI::Key::Scan& ceguiKey)
{
	if (CEGUI::Key::Scan::Esc != ceguiKey)
		return getCurrentInputAggregator()->injectKeyDown(ceguiKey);
// 	else
// 	{
// 		if (d_selectedSampleData)
// 			stopDisplaySample();
// 		else
// 			setQuitting(true);
// 	}

	return false;
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectKeyUp(const CEGUI::Key::Scan& ceguiKey)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectKeyUp(ceguiKey);
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectChar(int character)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectChar(character);
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectMouseButtonDown(const CEGUI::MouseButton& ceguiMouseButton)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseButtonDown(ceguiMouseButton);
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectMouseButtonUp(const CEGUI::MouseButton& ceguiMouseButton)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseButtonUp(ceguiMouseButton);
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectMouseWheelChange(float position)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseWheelChange(position);
}

//----------------------------------------------------------------------------//
bool CEGuiBaseApplication::injectMousePosition(float x, float y)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMousePosition(x, y);
}