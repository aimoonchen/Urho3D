#pragma once
#include <vector>
#include "Container/Str.h"
#include "CEGUI/InputEvent.h"

namespace CEGUI {
class Renderer;
class ImageCodec;
class ResourceProvider;
class GeometryBuffer;
class EventArgs;
class InputAggregator;
}

namespace Urho3D {

class Graphics;

class CEGui
{
public:
	CEGui();

public:
	void Initialize(Graphics* graphics);
	void Update(float timeStep);
	void Render();
	void Clear();
	bool injectKeyDown(const CEGUI::Key::Scan& ceguiKey);
	bool injectKeyUp(const CEGUI::Key::Scan& ceguiKey);
	bool injectChar(int character);
	bool injectMouseButtonDown(const CEGUI::MouseButton& ceguiMouseButton);
	bool injectMouseButtonUp(const CEGUI::MouseButton& ceguiMouseButton);
	bool injectMouseWheelChange(float position);
	bool injectMousePosition(float x, float y);
private:
	void updateFPS(const float elapsed);
	void updateLogo(const float elapsed);
	void updateLogoGeometry();
	void updateFPSGeometry();
	void updateLogoGeometryRotation();
	CEGUI::InputAggregator* getCurrentInputAggregator();
	//! event handler function that draws the logo and FPS overlay elements.
	bool sampleBrowserOverlayHandler(const CEGUI::EventArgs& args);
	//! event handler function that draws the FPS overlay elements.
	bool sampleOverlayHandler(const CEGUI::EventArgs& args);
	//! event handler function called when main view is resized
	bool resizeHandler(const CEGUI::EventArgs& args);
	void initialiseResourceGroupDirectories();
	void initialiseDefaultResourceGroups();
	String getDataPathPrefix() const { return d_dataPathPrefix; }
	void initDataPathPrefix(const String &override);
protected:
private:
	//! Renderer to use.  This MUST be set in the subclass constructor.
	CEGUI::Renderer* d_renderer;
	//! ImageCodec to use.  Set in subclass constructor, may be 0.
	CEGUI::ImageCodec* d_imageCodec;
	//! ResourceProvider to use.  Set in subclass constructor, may be 0.
	CEGUI::ResourceProvider* d_resourceProvider;
	//! GeometryBuffer used for drawing the spinning CEGUI logo
	std::vector<CEGUI::GeometryBuffer*> d_logoGeometry;
	//! GeometryBuffers used for drawing the FPS value.
	std::vector<CEGUI::GeometryBuffer*> d_FPSGeometry;
	static const char DATAPATH_VAR_NAME[];
	//CEGUI::String d_dataPathPrefix;
	String d_dataPathPrefix;
	//! Fraction of second elapsed (used for counting frames per second).
	float d_FPSElapsed;
	//! Number of frames drawn so far.
	int d_FPSFrames;
	//! Last changed FPS value.
	int d_FPSValue;
	//! whether to spin the logo
	bool d_spinLogo;
	CEGUI::InputAggregator*         d_systemInputAggregator{ nullptr };
};

}