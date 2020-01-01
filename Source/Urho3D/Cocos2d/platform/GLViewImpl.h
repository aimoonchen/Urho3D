#pragma once
#include "CCGLView.h"
#include "../Input/InputConstants.h"

NS_CC_BEGIN

class GLViewImpl : public cocos2d::GLView
{
public:
	GLViewImpl(bool initglfw = true);
	virtual ~GLViewImpl();
	float getFrameZoomFactor() const override;
	//void centerWindow();

	virtual void setViewPortInPoints(float x, float y, float w, float h) override;
	virtual void setScissorInPoints(float x, float y, float w, float h) override;
	virtual Rect getScissorRect() const override;

	void onGLFWMouseCallBack(Urho3D::MouseButton button, bool isPressed, int modify);
	void onGLFWMouseMoveCallBack(double x, double y);
	void onGLFWMouseScrollCallback(double x, double y);
	void onGLFWKeyCallback(int key, int scancode, bool isPressed, int mods);
	void onGLFWCharCallback(unsigned int character);
private:
	bool _captured;
	bool _supportTouch;
	bool _isInRetinaMonitor;
	bool _isRetinaEnabled;
	int  _retinaFactor;  // Should be 1 or 2

	float _frameZoomFactor;
	float _mouseX;
	float _mouseY;
};

NS_CC_END