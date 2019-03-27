//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "HelloWorld.h"

#include <Urho3D/DebugNew.h>

#include <Urho3D/CEGUIRenderer/CEGuiProxy.h>

// Expands to this example's entry-point
URHO3D_DEFINE_APPLICATION_MAIN(HelloWorld)

HelloWorld::HelloWorld(Context* context) :
    Sample(context)
{
}

void HelloWorld::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create "Hello World" Text
    CreateText();

    // Finally subscribe to the update event. Note that by subscribing events at this point we have already missed some events
    // like the ScreenMode event sent by the Graphics subsystem when opening the application window. To catch those as well we
    // could subscribe in the constructor instead.
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);

	cegui_proxy_ = std::make_unique<CEGuiBaseApplication>();
	cegui_proxy_->init(nullptr, "CEGUI.log", CEGUI::String());
}

void HelloWorld::CreateText()
{
    auto* cache = GetSubsystem<ResourceCache>();

    // Construct new Text object
    SharedPtr<Text> helloText(new Text(context_));

    // Set String to display
    helloText->SetText("Hello World from Urho3D!");

    // Set font and text color
    helloText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 30);
    helloText->SetColor(Color(0.0f, 1.0f, 0.0f));

    // Align Text center-screen
    helloText->SetHorizontalAlignment(HA_CENTER);
    helloText->SetVerticalAlignment(VA_CENTER);

    // Add Text instance to the UI root element
    GetSubsystem<UI>()->GetRoot()->AddChild(helloText);
}

void HelloWorld::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(HelloWorld, HandleUpdate));

	SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(HelloWorld, HandleMouseButtonDown));
	SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(HelloWorld, HandleMouseButtonUp));
	//SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(HelloWorld, HandleMouseMove));
	SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(HelloWorld, HandleMouseWheel));
	SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(HelloWorld, HandleTouchBegin));
	SubscribeToEvent(E_TOUCHEND, URHO3D_HANDLER(HelloWorld, HandleTouchEnd));
	SubscribeToEvent(E_TOUCHMOVE, URHO3D_HANDLER(HelloWorld, HandleTouchMove));
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyHelloWorldApp, HandleKeyDown));
	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(HelloWorld, HandleKeyUp));
	SubscribeToEvent(E_TEXTINPUT, URHO3D_HANDLER(HelloWorld, HandleTextInput));

}

void HelloWorld::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Do nothing for now, could be extended to eg. animate the display
}

void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseButtonDown;

	auto mouseButtons = MouseButtonFlags(eventData[P_BUTTONS].GetUInt());

	OnMouseButtonDown(MouseButton(eventData[P_BUTTON].GetUInt()));
}

void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseButtonUp;

	auto mouseButtons = MouseButtonFlags(eventData[P_BUTTONS].GetUInt());

	OnMouseButtonUp(MouseButton(eventData[P_BUTTONS].GetUInt()));
}

void HandleMouseWheel(StringHash eventType, VariantMap& eventData)
{
	auto* input = GetSubsystem<Input>();
	if (input->IsMouseGrabbed())
		return;

	using namespace MouseWheel;

	auto mouseButtons = MouseButtonFlags(eventData[P_BUTTONS].GetInt());
	cegui_proxy_->injectMouseWheelChange(eventData[P_WHEEL].GetInt());
}

void HandleTouchBegin(StringHash eventType, VariantMap& eventData)
{
	auto* input = GetSubsystem<Input>();
	if (input->IsMouseGrabbed())
		return;

	using namespace TouchBegin;

	IntVector2 pos(eventData[P_X].GetInt(), eventData[P_Y].GetInt());
	// 		pos.x_ = int(pos.x_ / uiScale_);
	// 		pos.y_ = int(pos.y_ / uiScale_);
	// 		usingTouchInput_ = true;
	// 
	MouseButton touchId = MakeTouchIDMask(eventData[P_TOUCHID].GetInt());
	// 		WeakPtr<UIElement> element(GetElementAt(pos));
	// 
	// 		if (element)
	// 		{
	// 			ProcessClickBegin(pos, touchId, touchDragElements_[element], QUAL_NONE, nullptr, true);
	// 			touchDragElements_[element] |= touchId;
	// 		}
	// 		else
	// 			ProcessClickBegin(pos, touchId, touchId, QUAL_NONE, nullptr, true);
	OnMouseButtonDown(touchId);
}

void HandleTouchEnd(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchEnd;

	IntVector2 pos(eventData[P_X].GetInt(), eventData[P_Y].GetInt());
	// 		pos.x_ = int(pos.x_ / uiScale_);
	// 		pos.y_ = int(pos.y_ / uiScale_);

			// Get the touch index
	MouseButton touchId = MakeTouchIDMask(eventData[P_TOUCHID].GetInt());
	OnMouseButtonUp(touchId);
	// 		// Transmit hover end to the position where the finger was lifted
	// 		WeakPtr<UIElement> element(GetElementAt(pos));
	// 
	// 		// Clear any drag events that were using the touch id
	// 		for (auto i = touchDragElements_.Begin(); i != touchDragElements_.End();)
	// 		{
	// 			const MouseButtonFlags touches = i->second_;
	// 			if (touches & touchId)
	// 				i = touchDragElements_.Erase(i);
	// 			else
	// 				++i;
	// 		}
	// 
	// 		if (element && element->IsEnabled())
	// 			element->OnHover(element->ScreenToElement(pos), pos, 0, 0, nullptr);
	// 
	// 		ProcessClickEnd(pos, touchId, MOUSEB_NONE, QUAL_NONE, nullptr, true);
}

void HandleTouchMove(StringHash eventType, VariantMap& eventData)
{
	using namespace TouchMove;

	IntVector2 pos(eventData[P_X].GetInt(), eventData[P_Y].GetInt());
	IntVector2 deltaPos(eventData[P_DX].GetInt(), eventData[P_DY].GetInt());

	MouseButton touchId = MakeTouchIDMask(eventData[P_TOUCHID].GetInt());
}

void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	// Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
	int key = eventData[P_KEY].GetInt();
	if (key == KEY_ESCAPE)
		engine_->Exit();

	cegui_proxy_->injectKeyDown(urho3DKeyToCeguiKey(Key(key)));
}

void HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	int key = eventData[P_KEY].GetInt();

	cegui_proxy_->injectKeyUp(urho3DKeyToCeguiKey(Key(key)));
}

void HandleTextInput(StringHash eventType, VariantMap& eventData)
{
	using namespace TextInput;
	auto str = eventData[P_TEXT].GetString();
	cegui_proxy_->injectChar(str[0]);
}
