#include "GuiImpl.h"

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

#include "CEGUIRenderer/Renderer.h"

//test
#include "CEGUI/widgets/DefaultWindow.h"
#include "CEGUI/widgets/FrameWindow.h"

#if defined(__WIN32__) || defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Urho3D {

#ifndef CEGUI_SAMPLE_DATAPATH
//#define CEGUI_SAMPLE_DATAPATH "../datafiles"
#define CEGUI_SAMPLE_DATAPATH "D:/cegui/datafiles"
#endif

const char CEGui::DATAPATH_VAR_NAME[] = "CEGUI_SAMPLE_DATAPATH";

CEGui::CEGui()
{
	
}

void CEGui::Initialize(Graphics* graphics)
{
	CEGUI::String logFile{ "CEGUI.log" };
	String dataPathPrefixOverride;

	d_renderer = &CEGUI::Urho3DRenderer::create(graphics);

	if (!d_renderer)
		throw CEGUI::InvalidRequestException("CEGuiBaseApplication::run: Base application subclass did not create Renderer!");

	// start up CEGUI system using objects created in subclass constructor.
	CEGUI::System::create(*d_renderer, d_resourceProvider, nullptr, d_imageCodec, nullptr, "", logFile);

	auto& cegui_system = CEGUI::System::getSingleton();
	// initialise resource system
	initDataPathPrefix(dataPathPrefixOverride);
	initialiseResourceGroupDirectories();
	initialiseDefaultResourceGroups();

	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
	CEGUI::FrameWindow* wnd = static_cast<CEGUI::FrameWindow*>(winMgr.createWindow("TaharezLook/FrameWindow", "Sample Window"));
	//wnd->setUsingAutoRenderingSurface(true);
	wnd->setPosition(CEGUI::UVector2(cegui_reldim(0.25f), cegui_reldim(0.25f)));
	wnd->setSize(CEGUI::USize(cegui_reldim(0.5f), cegui_reldim(0.5f)));
	wnd->setMaxSize(CEGUI::USize(cegui_reldim(1.0f), cegui_reldim(1.0f)));
	wnd->setMinSize(CEGUI::USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));
	wnd->setText("Hello World!");
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(wnd);
	/*
	// create logo imageset and draw the image (we only ever draw this once)
	CEGUI::ImageManager::getSingleton().addBitmapImageFromFile("cegui_logo", "logo.png");

	CEGUI::Image& ceguiLogo = CEGUI::ImageManager::getSingleton().get("cegui_logo");

	CEGUI::ImageRenderSettings imgRenderSettings(CEGUI::Rectf(0, 0, 183, 89));

	auto ceguiLogoGeomBuffers = ceguiLogo.createRenderGeometry(imgRenderSettings);

	d_logoGeometry.insert(d_logoGeometry.end(), ceguiLogoGeomBuffers.begin(), ceguiLogoGeomBuffers.end());

	// initial position update of the logo
	updateLogoGeometry();
	// setup for spinning logo
	updateLogoGeometryRotation();

	// clearing this queue actually makes sure it's created(!)
	cegui_system.getDefaultGUIContext().clearGeometry(CEGUI::RenderQueueID::Overlay);

	// subscribe handler to render overlay items
	cegui_system.getDefaultGUIContext().subscribeEvent(CEGUI::RenderingSurface::EventRenderQueueStarted, CEGUI::Event::Subscriber(&CEGui::sampleBrowserOverlayHandler, this));

	// subscribe handler to reposition logo when window is sized.
	cegui_system.subscribeEvent(CEGUI::System::EventDisplaySizeChanged, CEGUI::Event::Subscriber(&CEGui::resizeHandler, this));

	const CEGUI::Rectf& area(cegui_system.getRenderer()->getDefaultRenderTarget().getArea());
	//d_sampleApp->setApplicationWindowSize(static_cast<int>(area.getWidth()), static_cast<int>(area.getHeight()));


	d_systemInputAggregator = new CEGUI::InputAggregator(&cegui_system.getDefaultGUIContext());
	d_systemInputAggregator->initialise();


// 	CEGUI::FontManager::FontList loadedFonts = CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");
// 	CEGUI::Font* defaultFont = loadedFonts.empty() ? 0 : loadedFonts.front();
// 
// 	CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont(defaultFont);

	//
	d_guiContext = &CEGUI::System::getSingleton().createGUIContext(d_renderer->getDefaultRenderTarget());

	auto guiContext = d_guiContext;

	// 	auto d_textureTarget = d_renderer->createTextureTarget(false);
	// 	test_context_ = &CEGUI::System::getSingleton().createGUIContext(static_cast<RenderTarget&>(*d_textureTarget));
	// 	auto guiContext = test_context_;
	// 	d_textureTarget->declareRenderSize({1280, 720});


	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");

	// The next thing we do is to set a default cursor image.  This is
	// not strictly essential, although it is nice to always have a visible
	// indicator if a window or widget does not explicitly set one of its own.
	//
	// The TaharezLook Imageset contains an Image named "MouseArrow" which is
	// the ideal thing to have as a defult cursor image.
	guiContext->getCursor().setDefaultImage("TaharezLook/MouseArrow");

	// Now the system is initialised, we can actually create some UI elements, for
	// this first example, a full-screen 'root' window is set as the active GUI
	// sheet, and then a simple frame window will be created and attached to it.

	// All windows and widgets are created via the WindowManager singleton.
	CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

	// Here we create a "DefaultWindow".  This is a native type, that is, it does
	// not have to be loaded via a scheme, it is always available.  One common use
	// for the DefaultWindow is as a generic container for other windows.  Its
	// size defaults to 1.0f x 1.0f using the Relative metrics mode, which means
	// when it is set as the root GUI sheet window, it will cover the entire display.
	// The DefaultWindow does not perform any rendering of its own, so is invisible.
	//
	// Create a DefaultWindow called 'Root'.
	auto d_root = static_cast<CEGUI::DefaultWindow*>(winMgr.createWindow("DefaultWindow", "Root"));

	// load font and setup default if not loaded via scheme
	CEGUI::FontManager::FontList loadedFonts = CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");
	CEGUI::Font* defaultFont = loadedFonts.empty() ? 0 : loadedFonts.front();
	// Set default font for the gui context
	guiContext->setDefaultFont(defaultFont);

	// Set the root window as root of our GUI Context
	guiContext->setRootWindow(d_root);

	// A FrameWindow is a window with a frame and a titlebar which may be moved around
	// and resized.
	//
	// Create a FrameWindow in the TaharezLook style, and name it 'Sample Window'
	CEGUI::FrameWindow* wnd = static_cast<CEGUI::FrameWindow*>(winMgr.createWindow("TaharezLook/FrameWindow", "Sample Window"));

	// Here we attach the newly created FrameWindow to the previously created
	// DefaultWindow which we will be using as the root of the displayed gui.
	d_root->addChild(wnd);

	// Windows are in Relative metrics mode by default.  This means that we can
	// specify sizes and positions without having to know the exact pixel size
	// of the elements in advance.  The relative metrics mode co-ordinates are
	// relative to the parent of the window where the co-ordinates are being set.
	// This means that if 0.5f is specified as the width for a window, that window
	// will be half as its parent window.
	//
	// Here we set the FrameWindow so that it is half the size of the display,
	// and centered within the display.
	wnd->setPosition(CEGUI::UVector2(cegui_reldim(0.25f), cegui_reldim(0.25f)));
	wnd->setSize(CEGUI::USize(cegui_reldim(0.5f), cegui_reldim(0.5f)));

	// now we set the maximum and minum sizes for the new window.  These are
	// specified using relative co-ordinates, but the important thing to note
	// is that these settings are aways relative to the display rather than the
	// parent window.
	//
	// here we set a maximum size for the FrameWindow which is equal to the size
	// of the display, and a minimum size of one tenth of the display.
	wnd->setMaxSize(CEGUI::USize(cegui_reldim(1.0f), cegui_reldim(1.0f)));
	wnd->setMinSize(CEGUI::USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));

	// As a final step in the initialisation of our sample window, we set the window's
	// text to "Hello World!", so that this text will appear as the caption in the
	// FrameWindow's titlebar.
	wnd->setText("Hello World!");

	//wnd->subscribeEvent(CEGUI::Window::EventCursorActivate, Event::Subscriber(&CEGuiBaseApplication::handleHelloWorldClicked, this));

	wnd->setUsingAutoRenderingSurface(false);

	d_inputAggregator = new CEGUI::InputAggregator(d_guiContext);
	d_inputAggregator->initialise();
	*/
	d_systemInputAggregator = new CEGUI::InputAggregator(&CEGUI::System::getSingletonPtr()->getDefaultGUIContext());
	d_systemInputAggregator->initialise();
}

void CEGui::initialiseResourceGroupDirectories()
{
	// initialise the required dirs for the DefaultResourceProvider
	auto rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
	CEGUI::String dataPathPrefix(getDataPathPrefix().CString());

	/* for each resource type, set a resource group directory. We cast strings
	   to "const CEGUI::utf8*" in order to support general Unicode strings,
	   rather than only ASCII strings (even though currently they're all ASCII).
	   */
	rp->setResourceGroupDirectory("schemes", dataPathPrefix + "/schemes/");
	rp->setResourceGroupDirectory("imagesets", dataPathPrefix + "/imagesets/");
	rp->setResourceGroupDirectory("fonts", dataPathPrefix + "/fonts/");
	rp->setResourceGroupDirectory("layouts", dataPathPrefix + "/layouts/");
	rp->setResourceGroupDirectory("looknfeels", dataPathPrefix + "/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts", dataPathPrefix + "/lua_scripts/");
	rp->setResourceGroupDirectory("schemas", dataPathPrefix + "/xml_schemas/");
	rp->setResourceGroupDirectory("animations", dataPathPrefix + "/animations/");
}

//----------------------------------------------------------------------------//
void CEGui::initialiseDefaultResourceGroups()
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

void CEGui::initDataPathPrefix(const String &override)
{

	if (override.Empty()) {

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
		d_dataPathPrefix = envDataPath ? envDataPath : CEGUI_SAMPLE_DATAPATH;

#       endif

	}
	else
		d_dataPathPrefix = override;

}
//----------------------------------------------------------------------------//
bool CEGui::resizeHandler(const CEGUI::EventArgs& /*args*/)
{
	// clear FPS geometry and see that it gets recreated in the next frame
	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
		d_renderer->destroyGeometryBuffer(*d_FPSGeometry.at(i));
	d_FPSGeometry.clear();

	d_FPSValue = 0;

	// 		const CEGUI::Rectf& area(CEGUI::System::getSingleton().getRenderer()->getDefaultRenderTarget().getArea());
	// 		d_sampleApp->handleNewWindowSize(area.getWidth(), area.getHeight());

	updateLogoGeometry();
	updateFPSGeometry();

	return true;
}

//----------------------------------------------------------------------------//
bool CEGui::sampleBrowserOverlayHandler(const CEGUI::EventArgs& args)
{
	if (static_cast<const CEGUI::RenderQueueEventArgs&>(args).queueID != CEGUI::RenderQueueID::Overlay)
		return false;

	// draw the logo
// 	d_renderer->uploadBuffers(d_logoGeometry);
// 	const size_t logo_buffer_count = d_logoGeometry.size();
// 	for (size_t i = 0; i < logo_buffer_count; ++i) {
// 		d_logoGeometry[i]->draw();
// 	}

	// draw FPS value
	d_renderer->uploadBuffers(d_FPSGeometry);
	const size_t fps_buffer_count = d_FPSGeometry.size();
	for (size_t i = 0; i < fps_buffer_count; ++i)
		d_FPSGeometry.at(i)->draw();

	return true;
}

//----------------------------------------------------------------------------//
bool CEGui::sampleOverlayHandler(const CEGUI::EventArgs& args)
{
	if (static_cast<const CEGUI::RenderQueueEventArgs&>(args).queueID != CEGUI::RenderQueueID::Overlay)
		return false;

	// Draw FPS value
	d_renderer->uploadBuffers(d_FPSGeometry);
	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i)
		d_FPSGeometry.at(i)->draw();

	return true;
}

//----------------------------------------------------------------------------//
void CEGui::updateFPS(const float elapsed)
{
	/*
	// another frame
	++d_FPSFrames;

	if ((d_FPSElapsed += elapsed) >= 1.0f) {
		if (d_FPSFrames != d_FPSValue) {
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
	*/

	CEGUI::Font* fnt = &CEGUI::FontManager::getSingleton().get("DejaVuSans-12");
	if (!fnt)
		return;

	// update FPS imagery
	std::stringstream sstream;
	sstream << "FPS: ";// << d_FPSValue;

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

//----------------------------------------------------------------------------//
void CEGui::updateLogo(const float elapsed)
{
	if (!d_spinLogo)
		return;

	static float rot = 0.0f;

	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i) {
		d_logoGeometry[i]->setRotation(glm::quat(glm::vec3(glm::radians(rot), 0, 0)));
	}

	rot = fmodf(rot + 180.0f * elapsed, 360.0f);
}

void CEGui::updateLogoGeometry()
{
	const CEGUI::Rectf scrn(d_renderer->getDefaultRenderTarget().getArea());
	const glm::vec3 position(10.0f, scrn.getSize().d_height - 89.0f, 0.0f);

	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i) {
		d_logoGeometry[i]->setClippingRegion(scrn);
		d_logoGeometry[i]->setTranslation(position);
	}
}

//----------------------------------------------------------------------------//
void CEGui::updateFPSGeometry()
{
	const CEGUI::Rectf scrn(d_renderer->getDefaultRenderTarget().getArea());
	const glm::vec3 position(scrn.getSize().d_width - 120.0f, 0.0f, 0.0f);

	const size_t bufferCount = d_FPSGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i) {
		d_FPSGeometry[i]->setClippingRegion(scrn);
		d_FPSGeometry[i]->setTranslation(position);
	}
}

void CEGui::updateLogoGeometryRotation()
{
	const size_t bufferCount = d_logoGeometry.size();
	for (size_t i = 0; i < bufferCount; ++i) {
		d_logoGeometry[i]->setPivot(glm::vec3(91.5f, 44.5f, 0));
	}
}
CEGUI::InputAggregator* CEGui::getCurrentInputAggregator()
{
	// 		if (d_selectedSampleData != nullptr)
	// 			return d_selectedSampleData->getInputAggregator();

	return d_systemInputAggregator;
	//return d_inputAggregator;
}

void CEGui::Update(float timeStep)
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	gui_system.injectTimePulse(timeStep);

	// TODO: current context inject time pulse;
	gui_system.getDefaultGUIContext().injectTimePulse(timeStep);
	//gui_system.getDefaultGUIContext().getRootWindow()->invalidate(true);
// 	d_guiContext->injectTimePulse(timeStep);
// 
// 	updateFPS(timeStep);
// 	updateLogo(timeStep);
}

void CEGui::Render()
{
	CEGUI::System& gui_system(CEGUI::System::getSingleton());
	//		gui_system.injectTimePulse(elapsed);
	// 		d_sampleApp->update(static_cast<float>(elapsed));
	// 
	// 		updateFPS(elapsed);
	//		updateLogo(elapsed);
	// 
	//beginRendering(elapsed);

	CEGUI::Renderer* gui_renderer(gui_system.getRenderer());
	gui_renderer->beginRendering();

	//d_sampleApp->renderGUIContexts();
	//CEGUI::System& gui_system(CEGUI::System::getSingleton());

	gui_system.getDefaultGUIContext().draw();

	//d_guiContext->draw();

	gui_renderer->endRendering();

	CEGUI::WindowManager::getSingleton().cleanDeadPool();

	//endRendering();
}

void CEGui::Clear()
{
	if (d_systemInputAggregator != nullptr) {
		delete d_systemInputAggregator;
		d_systemInputAggregator = nullptr;
	}
}


bool CEGui::injectKeyDown(const CEGUI::Key::Scan& ceguiKey)
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
bool CEGui::injectKeyUp(const CEGUI::Key::Scan& ceguiKey)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectKeyUp(ceguiKey);
}

//----------------------------------------------------------------------------//
bool CEGui::injectChar(int character)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectChar(character);
}

//----------------------------------------------------------------------------//
bool CEGui::injectMouseButtonDown(const CEGUI::MouseButton& ceguiMouseButton)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseButtonDown(ceguiMouseButton);
}

//----------------------------------------------------------------------------//
bool CEGui::injectMouseButtonUp(const CEGUI::MouseButton& ceguiMouseButton)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseButtonUp(ceguiMouseButton);
}

//----------------------------------------------------------------------------//
bool CEGui::injectMouseWheelChange(float position)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMouseWheelChange(position);
}

//----------------------------------------------------------------------------//
bool CEGui::injectMousePosition(float x, float y)
{
	if (getCurrentInputAggregator() == nullptr)
		return false;

	return getCurrentInputAggregator()->injectMousePosition(x, y);
}

}
