#pragma once
#include "CCGLView.h"
#include "../../Input/InputConstants.h"

NS_CC_BEGIN

class GLViewImpl : public cocos2d::GLView
{
public:
	GLViewImpl(bool initglfw = true);
	virtual ~GLViewImpl();
	float getFrameZoomFactor() const override;
	//void centerWindow();
    void end() override {}
    bool isOpenGLReady() override { return true; }
    void swapBuffers() override {}

	virtual void setViewPortInPoints(float x, float y, float w, float h) override;
	virtual void setScissorInPoints(float x, float y, float w, float h) override;
	virtual Rect getScissorRect() const override;

	void onGLFWMouseCallBack(Urho3D::MouseButton button, bool isPressed, int modify);
	void onGLFWMouseMoveCallBack(double x, double y);
	void onGLFWMouseScrollCallback(double x, double y);
	void onGLFWKeyCallback(int key, int scancode, bool isPressed, int mods);
	void onGLFWCharCallback(unsigned int character);
	virtual void setIMEKeyboardState(bool bOpen) override;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	HWND getWin32Window() override;
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    id getCocoaWindow() override { return {};/*glfwGetCocoaWindow(_mainWindow);*/ }
    id getNSGLContext() override { return {};/*glfwGetNSGLContext(_mainWindow);*/ } // stevetranby: added
#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
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
