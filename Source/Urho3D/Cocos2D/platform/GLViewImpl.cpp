#include "GLViewImpl.h"
#include <set>
#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "base/CCEventMouse.h"
#include "base/CCIMEDispatcher.h"
#include "base/CCEventDispatcher.h"
#include "2d/CCCamera.h"
#include "../../Input/InputConstants.h"
#include "../../Core/Context.h"
#include "../../Graphics/Graphics.h"
#include "Urho3DContext.h"
//#include "../../Source/ThirdParty/SDL/src/video/SDL_sysvideo.h"

NS_CC_BEGIN

struct keyCodeItem
{
	int glfwKeyCode;
	EventKeyboard::KeyCode keyCode;
};

static std::unordered_map<int, EventKeyboard::KeyCode> g_keyCodeMap;

static keyCodeItem g_keyCodeStructArray[] = {
	/* The unknown key */
	{ Urho3D::KEY_UNKNOWN         , EventKeyboard::KeyCode::KEY_NONE          },

	/* Printable keys */
	{ Urho3D::KEY_SPACE           , EventKeyboard::KeyCode::KEY_SPACE         },
	{ Urho3D::KEY_COMMA           , EventKeyboard::KeyCode::KEY_COMMA         },
	{ Urho3D::KEY_MINUS           , EventKeyboard::KeyCode::KEY_MINUS         },
	{ Urho3D::KEY_PERIOD          , EventKeyboard::KeyCode::KEY_PERIOD        },
	{ Urho3D::KEY_SLASH           , EventKeyboard::KeyCode::KEY_SLASH         },
	{ Urho3D::KEY_0               , EventKeyboard::KeyCode::KEY_0             },
	{ Urho3D::KEY_1               , EventKeyboard::KeyCode::KEY_1             },
	{ Urho3D::KEY_2               , EventKeyboard::KeyCode::KEY_2             },
	{ Urho3D::KEY_3               , EventKeyboard::KeyCode::KEY_3             },
	{ Urho3D::KEY_4               , EventKeyboard::KeyCode::KEY_4             },
	{ Urho3D::KEY_5               , EventKeyboard::KeyCode::KEY_5             },
	{ Urho3D::KEY_6               , EventKeyboard::KeyCode::KEY_6             },
	{ Urho3D::KEY_7               , EventKeyboard::KeyCode::KEY_7             },
	{ Urho3D::KEY_8               , EventKeyboard::KeyCode::KEY_8             },
	{ Urho3D::KEY_9               , EventKeyboard::KeyCode::KEY_9             },
	{ Urho3D::KEY_SEMICOLON       , EventKeyboard::KeyCode::KEY_SEMICOLON     },
	{ Urho3D::KEY_EQUALS          , EventKeyboard::KeyCode::KEY_EQUAL         },
	{ Urho3D::KEY_A               , EventKeyboard::KeyCode::KEY_A             },
	{ Urho3D::KEY_B               , EventKeyboard::KeyCode::KEY_B             },
	{ Urho3D::KEY_C               , EventKeyboard::KeyCode::KEY_C             },
	{ Urho3D::KEY_D               , EventKeyboard::KeyCode::KEY_D             },
	{ Urho3D::KEY_E               , EventKeyboard::KeyCode::KEY_E             },
	{ Urho3D::KEY_F               , EventKeyboard::KeyCode::KEY_F             },
	{ Urho3D::KEY_G               , EventKeyboard::KeyCode::KEY_G             },
	{ Urho3D::KEY_H               , EventKeyboard::KeyCode::KEY_H             },
	{ Urho3D::KEY_I               , EventKeyboard::KeyCode::KEY_I             },
	{ Urho3D::KEY_J               , EventKeyboard::KeyCode::KEY_J             },
	{ Urho3D::KEY_K               , EventKeyboard::KeyCode::KEY_K             },
	{ Urho3D::KEY_L               , EventKeyboard::KeyCode::KEY_L             },
	{ Urho3D::KEY_M               , EventKeyboard::KeyCode::KEY_M             },
	{ Urho3D::KEY_N               , EventKeyboard::KeyCode::KEY_N             },
	{ Urho3D::KEY_O               , EventKeyboard::KeyCode::KEY_O             },
	{ Urho3D::KEY_P               , EventKeyboard::KeyCode::KEY_P             },
	{ Urho3D::KEY_Q               , EventKeyboard::KeyCode::KEY_Q             },
	{ Urho3D::KEY_R               , EventKeyboard::KeyCode::KEY_R             },
	{ Urho3D::KEY_S               , EventKeyboard::KeyCode::KEY_S             },
	{ Urho3D::KEY_T               , EventKeyboard::KeyCode::KEY_T             },
	{ Urho3D::KEY_U               , EventKeyboard::KeyCode::KEY_U             },
	{ Urho3D::KEY_V               , EventKeyboard::KeyCode::KEY_V             },
	{ Urho3D::KEY_W               , EventKeyboard::KeyCode::KEY_W             },
	{ Urho3D::KEY_X               , EventKeyboard::KeyCode::KEY_X             },
	{ Urho3D::KEY_Y               , EventKeyboard::KeyCode::KEY_Y             },
	{ Urho3D::KEY_Z               , EventKeyboard::KeyCode::KEY_Z             },
	{ Urho3D::KEY_LEFTBRACKET     , EventKeyboard::KeyCode::KEY_LEFT_BRACKET  },
	{ Urho3D::KEY_BACKSLASH       , EventKeyboard::KeyCode::KEY_BACK_SLASH    },
	{ Urho3D::KEY_RIGHTBRACKET    , EventKeyboard::KeyCode::KEY_RIGHT_BRACKET },

	/* Function keys */
	{ Urho3D::KEY_ESCAPE          , EventKeyboard::KeyCode::KEY_ESCAPE        },
	{ Urho3D::KEY_TAB             , EventKeyboard::KeyCode::KEY_TAB           },
	{ Urho3D::KEY_BACKSPACE       , EventKeyboard::KeyCode::KEY_BACKSPACE     },
	{ Urho3D::KEY_INSERT          , EventKeyboard::KeyCode::KEY_INSERT        },
	{ Urho3D::KEY_DELETE          , EventKeyboard::KeyCode::KEY_DELETE        },
	{ Urho3D::KEY_RIGHT           , EventKeyboard::KeyCode::KEY_RIGHT_ARROW   },
	{ Urho3D::KEY_LEFT            , EventKeyboard::KeyCode::KEY_LEFT_ARROW    },
	{ Urho3D::KEY_DOWN            , EventKeyboard::KeyCode::KEY_DOWN_ARROW    },
	{ Urho3D::KEY_UP              , EventKeyboard::KeyCode::KEY_UP_ARROW      },
	{ Urho3D::KEY_PAGEUP          , EventKeyboard::KeyCode::KEY_PG_UP         },
	{ Urho3D::KEY_PAGEDOWN        , EventKeyboard::KeyCode::KEY_PG_DOWN       },
	{ Urho3D::KEY_HOME            , EventKeyboard::KeyCode::KEY_HOME          },
	{ Urho3D::KEY_END             , EventKeyboard::KeyCode::KEY_END           },
	{ Urho3D::KEY_CAPSLOCK        , EventKeyboard::KeyCode::KEY_CAPS_LOCK     },
	{ Urho3D::KEY_SCROLLLOCK      , EventKeyboard::KeyCode::KEY_SCROLL_LOCK   },
	{ Urho3D::KEY_NUMLOCKCLEAR    , EventKeyboard::KeyCode::KEY_NUM_LOCK      },
	{ Urho3D::KEY_PRINTSCREEN     , EventKeyboard::KeyCode::KEY_PRINT         },
	{ Urho3D::KEY_PAUSE           , EventKeyboard::KeyCode::KEY_PAUSE         },
	{ Urho3D::KEY_F1              , EventKeyboard::KeyCode::KEY_F1            },
	{ Urho3D::KEY_F2              , EventKeyboard::KeyCode::KEY_F2            },
	{ Urho3D::KEY_F3              , EventKeyboard::KeyCode::KEY_F3            },
	{ Urho3D::KEY_F4              , EventKeyboard::KeyCode::KEY_F4            },
	{ Urho3D::KEY_F5              , EventKeyboard::KeyCode::KEY_F5            },
	{ Urho3D::KEY_F6              , EventKeyboard::KeyCode::KEY_F6            },
	{ Urho3D::KEY_F7              , EventKeyboard::KeyCode::KEY_F7            },
	{ Urho3D::KEY_F8              , EventKeyboard::KeyCode::KEY_F8            },
	{ Urho3D::KEY_F9              , EventKeyboard::KeyCode::KEY_F9            },
	{ Urho3D::KEY_F10             , EventKeyboard::KeyCode::KEY_F10           },
	{ Urho3D::KEY_F11             , EventKeyboard::KeyCode::KEY_F11           },
	{ Urho3D::KEY_F12             , EventKeyboard::KeyCode::KEY_F12           },
	{ Urho3D::KEY_F13             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F14             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F15             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F16             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F17             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F18             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F19             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F20             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F21             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F22             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F23             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_F24             , EventKeyboard::KeyCode::KEY_NONE          },
	{ Urho3D::KEY_KP_0            , EventKeyboard::KeyCode::KEY_0             },
	{ Urho3D::KEY_KP_1            , EventKeyboard::KeyCode::KEY_1             },
	{ Urho3D::KEY_KP_2            , EventKeyboard::KeyCode::KEY_2             },
	{ Urho3D::KEY_KP_3            , EventKeyboard::KeyCode::KEY_3             },
	{ Urho3D::KEY_KP_4            , EventKeyboard::KeyCode::KEY_4             },
	{ Urho3D::KEY_KP_5            , EventKeyboard::KeyCode::KEY_5             },
	{ Urho3D::KEY_KP_6            , EventKeyboard::KeyCode::KEY_6             },
	{ Urho3D::KEY_KP_7            , EventKeyboard::KeyCode::KEY_7             },
	{ Urho3D::KEY_KP_8            , EventKeyboard::KeyCode::KEY_8             },
	{ Urho3D::KEY_KP_9            , EventKeyboard::KeyCode::KEY_9             },
	{ Urho3D::KEY_KP_DECIMAL      , EventKeyboard::KeyCode::KEY_PERIOD        },
	{ Urho3D::KEY_KP_DIVIDE       , EventKeyboard::KeyCode::KEY_KP_DIVIDE     },
	{ Urho3D::KEY_KP_MULTIPLY     , EventKeyboard::KeyCode::KEY_KP_MULTIPLY   },
	{ Urho3D::KEY_KP_MINUS		  , EventKeyboard::KeyCode::KEY_KP_MINUS      },
	{ Urho3D::KEY_KP_PLUS		  , EventKeyboard::KeyCode::KEY_KP_PLUS       },
	{ Urho3D::KEY_KP_ENTER        , EventKeyboard::KeyCode::KEY_KP_ENTER      },
	{ Urho3D::KEY_LSHIFT          , EventKeyboard::KeyCode::KEY_LEFT_SHIFT    },
	{ Urho3D::KEY_LCTRL           , EventKeyboard::KeyCode::KEY_LEFT_CTRL     },
	{ Urho3D::KEY_LALT            , EventKeyboard::KeyCode::KEY_LEFT_ALT      },
	{ Urho3D::KEY_RSHIFT		  , EventKeyboard::KeyCode::KEY_RIGHT_SHIFT   },
	{ Urho3D::KEY_RCTRL			  , EventKeyboard::KeyCode::KEY_RIGHT_CTRL    },
	{ Urho3D::KEY_RALT			  , EventKeyboard::KeyCode::KEY_RIGHT_ALT     },
	{ Urho3D::KEY_MENU            , EventKeyboard::KeyCode::KEY_MENU          },
};

GLViewImpl::GLViewImpl(bool initglfw)
	: _captured(false)
	, _supportTouch(false)
	, _isInRetinaMonitor(false)
	, _isRetinaEnabled(false)
	, _retinaFactor(1)
	, _frameZoomFactor(1.0f)
	, _mouseX(0.0f)
	, _mouseY(0.0f)
{
	g_keyCodeMap.clear();
	for (auto& item : g_keyCodeStructArray)
	{
		g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
	}
}

GLViewImpl::~GLViewImpl()
{

}

float GLViewImpl::getFrameZoomFactor() const
{
	return _frameZoomFactor;
}

void GLViewImpl::setViewPortInPoints(float x, float y, float w, float h)
{
	Viewport vp((float)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
		(float)(y * _scaleY * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
		(float)(w * _scaleX * _retinaFactor * _frameZoomFactor),
		(float)(h * _scaleY * _retinaFactor * _frameZoomFactor));
	Camera::setDefaultViewport(vp);
}

void GLViewImpl::setScissorInPoints(float x, float y, float w, float h)
{
// 	glScissor((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
// 		(GLint)(y * _scaleY * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
// 		(GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
// 		(GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));

	int left = (int)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor);
	int bottom = (int)(y * _scaleY * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor);
	int top = bottom + (int)(h * _scaleY * _retinaFactor * _frameZoomFactor);
	int right = left + (int)(w * _scaleX * _retinaFactor * _frameZoomFactor);
	auto rtsize = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetRenderTargetDimensions();
 	top = rtsize.y_ - top;
 	bottom = rtsize.y_ - bottom;
	GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->SetScissorTest(true, Urho3D::IntRect{ left, top, right, bottom });
}

Rect GLViewImpl::getScissorRect() const
{
	GLfloat params[4];
	//glGetFloatv(GL_SCISSOR_BOX, params);
	auto rtsize = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetRenderTargetDimensions();
	auto urho3dRect = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetScissorRect();

	params[0] = urho3dRect.Left();
	params[1] = rtsize.y_ - urho3dRect.Bottom();
	params[2] = urho3dRect.Width();
	params[3] = urho3dRect.Height();
	float x = (params[0] - _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor) / (_scaleX * _retinaFactor * _frameZoomFactor);
	float y = (params[1] - _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor) / (_scaleY * _retinaFactor * _frameZoomFactor);
	float w = params[2] / (_scaleX * _retinaFactor * _frameZoomFactor);
	float h = params[3] / (_scaleY * _retinaFactor * _frameZoomFactor);
	return Rect(x, y, w, h);
}

void GLViewImpl::onGLFWMouseCallBack(Urho3D::MouseButton button, bool isPressed, int /*modify*/)
{
	if (Urho3D::MOUSEB_LEFT == button)
	{
		if (isPressed)
		{
			_captured = true;
			if (this->getViewPortRect().equals(Rect::ZERO) || this->getViewPortRect().containsPoint(Vec2(_mouseX, _mouseY)))
			{
				intptr_t id = 0;
				this->handleTouchesBegin(1, &id, &_mouseX, &_mouseY);
			}
		}
		else
		{
			if (_captured)
			{
				_captured = false;
				intptr_t id = 0;
				this->handleTouchesEnd(1, &id, &_mouseX, &_mouseY);
			}
		}
	}

	//Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
	float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

	auto cocosButton = static_cast<cocos2d::EventMouse::MouseButton>((button == Urho3D::MOUSEB_LEFT) ? 0 : (button == Urho3D::MOUSEB_RIGHT) ? 1 : 2);
	if (isPressed)
	{
		EventMouse event(EventMouse::MouseEventType::MOUSE_DOWN);
		event.setCursorPosition(cursorX, cursorY);
		event.setMouseButton(cocosButton);
		Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}
	else
	{
		EventMouse event(EventMouse::MouseEventType::MOUSE_UP);
		event.setCursorPosition(cursorX, cursorY);
		event.setMouseButton(cocosButton);
		Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
	}
}

void GLViewImpl::onGLFWMouseMoveCallBack(double x, double y)
{
	_mouseX = (float)x;
	_mouseY = (float)y;

	_mouseX /= this->getFrameZoomFactor();
	_mouseY /= this->getFrameZoomFactor();

	if (_isInRetinaMonitor)
	{
		if (_retinaFactor == 1)
		{
			_mouseX *= 2;
			_mouseY *= 2;
		}
	}

	if (_captured)
	{
		intptr_t id = 0;
		this->handleTouchesMove(1, &id, &_mouseX, &_mouseY);
	}

	//Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
	float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

	EventMouse event(EventMouse::MouseEventType::MOUSE_MOVE);
	// Set current button
// 	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
// 	{
// 		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_LEFT));
// 	}
// 	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
// 	{
// 		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_RIGHT));
// 	}
// 	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
// 	{
// 		event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_MIDDLE));
// 	}
	if (_captured)
	{
		event.setMouseButton(cocos2d::EventMouse::MouseButton::BUTTON_LEFT);
	}
	event.setCursorPosition(cursorX, cursorY);
	Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void GLViewImpl::onGLFWMouseScrollCallback(double x, double y)
{
	EventMouse event(EventMouse::MouseEventType::MOUSE_SCROLL);
	//Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
	float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
	float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
	event.setScrollData((float)x, -(float)y);
	event.setCursorPosition(cursorX, cursorY);
	Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void GLViewImpl::onGLFWKeyCallback(int key, int /*scancode*/, bool isPressed, int /*mods*/)
{
	if (true/*GLFW_REPEAT != action*/)
	{
		EventKeyboard event(g_keyCodeMap[key], isPressed);
		auto dispatcher = Director::getInstance()->getEventDispatcher();
		dispatcher->dispatchEvent(&event);
	}

	if (false/*GLFW_RELEASE != action*/)
	{
		switch (g_keyCodeMap[key])
		{
		case EventKeyboard::KeyCode::KEY_BACKSPACE:
			IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
			break;
		case EventKeyboard::KeyCode::KEY_HOME:
		case EventKeyboard::KeyCode::KEY_KP_HOME:
		case EventKeyboard::KeyCode::KEY_DELETE:
		case EventKeyboard::KeyCode::KEY_KP_DELETE:
		case EventKeyboard::KeyCode::KEY_END:
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_ESCAPE:
			IMEDispatcher::sharedDispatcher()->dispatchControlKey(g_keyCodeMap[key]);
			break;
		default:
			break;
		}
	}
}

void GLViewImpl::onGLFWCharCallback(unsigned int character)
{
	char16_t wcharString[2] = { (char16_t)character, 0 };
	std::string utf8String;

	StringUtils::UTF16ToUTF8(wcharString, utf8String);
	static std::set<std::string> controlUnicode = {
		"\xEF\x9C\x80", // up
		"\xEF\x9C\x81", // down
		"\xEF\x9C\x82", // left
		"\xEF\x9C\x83", // right
		"\xEF\x9C\xA8", // delete
		"\xEF\x9C\xA9", // home
		"\xEF\x9C\xAB", // end
		"\xEF\x9C\xAC", // pageup
		"\xEF\x9C\xAD", // pagedown
		"\xEF\x9C\xB9"  // clear
	};
	// Check for send control key
	if (controlUnicode.find(utf8String) == controlUnicode.end())
	{
		IMEDispatcher::sharedDispatcher()->dispatchInsertText(utf8String.c_str(), utf8String.size());
	}
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
HWND GLViewImpl::getWin32Window()
{
    return (HWND)GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetWindow();
    // 	SDL_Window* sdl_win = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetWindow();
// 	auto ptr_int = (long*)sdl_win;
// 	//sizeof(SDL_Window) == 156
// 	ptr_int += 36;
// 	long* phwnd = (long*)(*ptr_int);
// 	return (HWND)(*(++phwnd));
}
#endif /* (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) */

void GLViewImpl::setIMEKeyboardState(bool bOpen)
{

}

NS_CC_END