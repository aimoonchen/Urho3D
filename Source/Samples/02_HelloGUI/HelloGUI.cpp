//
// Copyright (c) 2008-2020 the Urho3D project.
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
#include "FairyGUIExamples/MenuScene.h"
#include "FairyGUIExamples/BasicsScene.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UIComponent.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

#include "HelloGUI.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(
    HelloGUI
    , "02-HelloGUI"
    , "Loading textures."
    , "https://bkaradzic.github.io/bgfx/examples.html#bump");

HelloGUI::HelloGUI(Context* context)
    : Sample(context)
    , uiRoot_(GetSubsystem<UI>()->GetRoot())
    , dragBeginPosition_(IntVector2::ZERO)
{
}

void HelloGUI::Setup()
{
    // m_renderdocdll = bgfx::loadRenderDoc();

    engineParameters_[EP_WINDOW_WIDTH] = 1280;
    engineParameters_[EP_WINDOW_HEIGHT] = 720;
    Sample::Setup();
}

void HelloGUI::Start()
{
    // Execute base class startup
    Sample::Start();

    // Enable OS cursor
    GetSubsystem<Input>()->SetMouseVisible(true);

    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Set the loaded style as default style
    uiRoot_->SetDefaultStyle(style);

    /*


    scene_ = new Scene(context_);

    // Create octree, use also default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    scene_->CreateComponent<Octree>();

    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light without shadows
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.5f, -1.0f, 0.5f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetColor(Color(0.2f, 0.2f, 0.2f));
    light->SetSpecularIntensity(1.0f);

    // Create a "floor" consisting of several tiles
    for (int y = -5; y <= 5; ++y)
    {
        for (int x = -5; x <= 5; ++x)
        {
            Node* floorNode = scene_->CreateChild("FloorTile");
            floorNode->SetPosition(Vector3(x * 20.5f, -0.5f, y * 20.5f));
            floorNode->SetScale(Vector3(20.0f, 1.0f, 20.f));
            auto* floorObject = floorNode->CreateComponent<StaticModel>();
            floorObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            floorObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        }
    }

    // Create a "screen" like object for viewing the second scene. Construct it from two StaticModels, a box for the
frame
    // and a plane for the actual view
    {
        Node* boxNode = scene_->CreateChild("ScreenBox");
        boxNode->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
        boxNode->SetScale(Vector3(21.0f, 16.0f, 0.5f));
        auto* boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

        Node* screenNode = scene_->CreateChild("Screen");
        screenNode->SetPosition(Vector3(0.0f, 10.0f, -0.27f));
        screenNode->SetRotation(Quaternion(-90.0f, 0.0f, 0.0f));
        screenNode->SetScale(Vector3(20.0f, 0.0f, 15.0f));
        auto* screenObject = screenNode->CreateComponent<StaticModel>();
        screenObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));

        // Create a renderable texture (1024x768, RGB format), enable bilinear filtering on it
        SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
        renderTexture->SetSize(1024, 768, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
        renderTexture->SetFilterMode(FILTER_BILINEAR);

        // Create a new material from scratch, use the diffuse unlit technique, assign the render texture
        // as its diffuse texture, then assign the material to the screen plane object
        SharedPtr<Material> renderMaterial(new Material(context_));
        renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffUnlit.xml"));
        renderMaterial->SetTexture(TU_DIFFUSE, renderTexture);
        // Since the screen material is on top of the box model and may Z-fight, use negative depth bias
        // to push it forward (particularly necessary on mobiles with possibly less Z resolution)
        renderMaterial->SetDepthBias(BiasParameters(-0.001f, 0.0f));
        screenObject->SetMaterial(renderMaterial);

        // Get the texture's RenderSurface object (exists when the texture has been created in rendertarget mode)
        // and define the viewport for rendering the second scene, similarly as how backbuffer viewports are defined
        // to the Renderer subsystem. By default the texture viewport will be updated when the texture is visible
        // in the main view
// 		RenderSurface* surface = renderTexture->GetRenderSurface();
// 		SharedPtr<Viewport> rttViewport(new Viewport(context_, rttScene_, rttCameraNode_->GetComponent<Camera>()));
// 		surface->SetViewport(0, rttViewport);
        auto uiNode = scene_->CreateChild("UINode");
        uiCom_ = uiNode->CreateComponent<UIComponent>();
    }

    // Create the camera which we will move around. Limit far clip distance to match the fog
    cameraNode_ = scene_->CreateChild("Camera");
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 7.0f, -30.0f));

    auto* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
    */
    // Initialize Window
    InitWindow();

    // Create and add some controls to the Window
    InitControls();

    // Create a draggable Fish
    CreateDraggableFish();
    // GetSubsystem<Graphics>()->SetViewport
#ifdef CC_PLATFORM_PC
    //UIConfig::registerFont(UIConfig::defaultFont, "fonts/DroidSansFallback.ttf");
#endif
    // create a scene. it's an autorelease object
    //auto scene = BasicsScene::create();
    auto scene = MenuScene::create();

    // run
    cocos2d::Director::getInstance()->runWithScene(scene);

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}

void HelloGUI::InitControls()
{
    return;
    // Create a CheckBox
    auto* checkBox = new CheckBox(context_);
    checkBox->SetName("CheckBox");

    // Create a Button
    auto* button = new Button(context_);
    button->SetName("Button");
    button->SetMinHeight(24);

    // Create a LineEdit
    auto* lineEdit = new LineEdit(context_);
    lineEdit->SetName("LineEdit");
    lineEdit->SetMinHeight(24);

    // Add controls to Window
    window_->AddChild(checkBox);
    window_->AddChild(button);
    window_->AddChild(lineEdit);

    // Apply previously set default style
    checkBox->SetStyleAuto();
    button->SetStyleAuto();
    lineEdit->SetStyleAuto();
}

void HelloGUI::InitWindow()
{
    return;
    // Create the Window and add it to the UI's root node
    window_ = new Urho3D::Window(context_);
    uiRoot_->AddChild(window_);
    // uiCom_->GetRoot()->AddChild(window_);
    // Set Window size and layout settings
    window_->SetMinWidth(384);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_CENTER, VA_CENTER);
    window_->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Hello GUI!");

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    window_->AddChild(titleBar);

    // Apply styles
    window_->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(HelloGUI, HandleClosePressed));

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(HelloGUI, HandleControlClicked));
}

void HelloGUI::CreateDraggableFish()
{
    return;
    auto* cache = GetSubsystem<ResourceCache>();
    auto* graphics = GetSubsystem<Graphics>();

    // Create a draggable Fish button
    auto* draggableFish = new Button(context_);
    draggableFish->SetTexture(cache->GetResource<Texture2D>("Textures/UrhoDecal.dds")); // Set texture
    draggableFish->SetBlendMode(BLEND_ADD);
    draggableFish->SetSize(128, 128);
    // draggableFish->SetPosition((graphics->GetWidth() - draggableFish->GetWidth()) / 2, 200);
    draggableFish->SetPosition(0, 100);
    draggableFish->SetName("Fish");
    uiRoot_->AddChild(draggableFish);

    // Add a tooltip to Fish button
    auto* toolTip = new ToolTip(context_);
    draggableFish->AddChild(toolTip);
    toolTip->SetPosition(
        IntVector2(draggableFish->GetWidth() + 5, draggableFish->GetWidth() / 2)); // slightly offset from close button
    auto* textHolder = new BorderImage(context_);
    toolTip->AddChild(textHolder);
    textHolder->SetStyle("ToolTipBorderImage");
    auto* toolTipText = new Text(context_);
    textHolder->AddChild(toolTipText);
    toolTipText->SetStyle("ToolTipText");
    toolTipText->SetText("Please drag me!");

    // Subscribe draggableFish to Drag Events (in order to make it draggable)
    // See "Event list" in documentation's Main Page for reference on available Events and their eventData
    SubscribeToEvent(draggableFish, E_DRAGBEGIN, URHO3D_HANDLER(HelloGUI, HandleDragBegin));
    SubscribeToEvent(draggableFish, E_DRAGMOVE, URHO3D_HANDLER(HelloGUI, HandleDragMove));
    SubscribeToEvent(draggableFish, E_DRAGEND, URHO3D_HANDLER(HelloGUI, HandleDragEnd));
}

void HelloGUI::HandleDragBegin(StringHash eventType, VariantMap& eventData)
{
    // Get UIElement relative position where input (touch or click) occurred (top-left = IntVector2(0,0))
    dragBeginPosition_ = IntVector2(eventData["ElementX"].GetInt(), eventData["ElementY"].GetInt());
}

void HelloGUI::HandleDragMove(StringHash eventType, VariantMap& eventData)
{
    IntVector2 dragCurrentPosition = IntVector2(eventData["X"].GetInt(), eventData["Y"].GetInt());
    UIElement* draggedElement = static_cast<UIElement*>(eventData["Element"].GetPtr());
    draggedElement->SetPosition(dragCurrentPosition - dragBeginPosition_);
}

void HelloGUI::HandleDragEnd(StringHash eventType, VariantMap& eventData) // For reference (not used here)
{
}

void HelloGUI::HandleClosePressed(StringHash eventType, VariantMap& eventData)
{
    if (GetPlatform() != "Web")
        engine_->Exit();
}

void HelloGUI::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    auto* windowTitle = window_->GetChildStaticCast<Text>("WindowTitle", true);

    // Get control that was clicked
    auto* clicked = static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());

    String name = "...?";
    if (clicked)
    {
        // Get the name of the control that was clicked
        name = clicked->GetName();
    }

    // Update the Window's title text
    windowTitle->SetText("Hello " + name + "!");
}
