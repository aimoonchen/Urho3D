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

#include "../../Precompiled.h"

#include "../../Core/Context.h"
#include "../../Core/Mutex.h"
#include "../../Core/ProcessUtils.h"
#include "../../Core/Profiler.h"
#include "../../Graphics/ConstantBuffer.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsEvents.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/IndexBuffer.h"
#include "../../Graphics/RenderSurface.h"
#include "../../Graphics/Shader.h"
#include "../../Graphics/ShaderPrecache.h"
#include "../../Graphics/ShaderProgram.h"
#include "../../Graphics/ShaderVariation.h"
#include "../../Graphics/Texture2D.h"
#include "../../Graphics/TextureCube.h"
#include "../../Graphics/VertexBuffer.h"
#include "../../IO/File.h"
#include "../../IO/Log.h"
#include "../../Resource/ResourceCache.h"
#include "../../Input/Input.h"

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
#include <wayland-egl.h>
#endif
#elif BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif

#include <bx/os.h>

BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wextern-c-compat")
//#include <SDL/SDL_syswm.h>
BX_PRAGMA_DIAGNOSTIC_POP()

#include <bgfx/platform.h>
#if defined(None) // X11 defines this...
#undef None
#endif // defined(None)

#include "../../DebugNew.h"

#include "../BGFX/BGFXGraphics.h"

// #ifdef GL_ES_VERSION_2_0
// #define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
// #define glClearDepth glClearDepthf
// #endif

#ifdef __EMSCRIPTEN__0
#include "../../Input/Input.h"
#include "../../UI/Cursor.h"
#include "../../UI/UI.h"
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

// Emscripten provides even all GL extension functions via static linking. However there is
// no GLES2-specific extension header at the moment to include instanced rendering declarations,
// so declare them manually from GLES3 gl2ext.h. Emscripten will provide these when linking final output.
// extern "C"
// {
//     GL_APICALL void GL_APIENTRY glDrawArraysInstancedANGLE (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
//     GL_APICALL void GL_APIENTRY glDrawElementsInstancedANGLE (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
//     GL_APICALL void GL_APIENTRY glVertexAttribDivisorANGLE (GLuint index, GLuint divisor);
// }

// Helper functions to support emscripten canvas resolution change
static const Urho3D::Context *appContext;

static void JSCanvasSize(int width, int height, bool fullscreen, float scale)
{
    URHO3D_LOGINFOF("JSCanvasSize: width=%d height=%d fullscreen=%d ui scale=%f", width, height, fullscreen, scale);

    using namespace Urho3D;

    if (appContext)
    {
        bool uiCursorVisible = false;
        bool systemCursorVisible = false;
        MouseMode mouseMode{};

        // Detect current system pointer state
        Input* input = appContext->GetSubsystem<Input>();
        if (input)
        {
            systemCursorVisible = input->IsMouseVisible();
            mouseMode = input->GetMouseMode();
        }

        UI* ui = appContext->GetSubsystem<UI>();
        if (ui)
        {
            ui->SetScale(scale);

            // Detect current UI pointer state
            Cursor* cursor = ui->GetCursor();
            if (cursor)
                uiCursorVisible = cursor->IsVisible();
        }

        // Apply new resolution
        appContext->GetSubsystem<Graphics>()->SetMode(width, height);

        // Reset the pointer state as it was before resolution change
        if (input)
        {
            if (uiCursorVisible)
                input->SetMouseVisible(false);
            else
                input->SetMouseVisible(systemCursorVisible);

            input->SetMouseMode(mouseMode);
        }

        if (ui)
        {
            Cursor* cursor = ui->GetCursor();
            if (cursor)
            {
                cursor->SetVisible(uiCursorVisible);

                IntVector2 pos = input->GetMousePosition();
                pos = ui->ConvertSystemToUI(pos);

                cursor->SetPosition(pos);
            }
        }
    }
}

using namespace emscripten;
EMSCRIPTEN_BINDINGS(Module) {
    function("JSCanvasSize", &JSCanvasSize);
}
#endif

// #ifdef _WIN32
// // Prefer the high-performance GPU on switchable GPU systems
// #include <windows.h>
// extern "C"
// {
//     __declspec(dllexport) DWORD NvOptimusEnablement = 1;
//     __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
// }
// #endif

namespace Urho3D
{

// static const unsigned glCmpFunc[] =
// {
//     GL_ALWAYS,
//     GL_EQUAL,
//     GL_NOTEQUAL,
//     GL_LESS,
//     GL_LEQUAL,
//     GL_GREATER,
//     GL_GEQUAL
// };
// 
// static const unsigned glSrcBlend[] =
// {
//     GL_ONE,
//     GL_ONE,
//     GL_DST_COLOR,
//     GL_SRC_ALPHA,
//     GL_SRC_ALPHA,
//     GL_ONE,
//     GL_ONE_MINUS_DST_ALPHA,
//     GL_ONE,
//     GL_SRC_ALPHA
// };
// 
// static const unsigned glDestBlend[] =
// {
//     GL_ZERO,
//     GL_ONE,
//     GL_ZERO,
//     GL_ONE_MINUS_SRC_ALPHA,
//     GL_ONE,
//     GL_ONE_MINUS_SRC_ALPHA,
//     GL_DST_ALPHA,
//     GL_ONE,
//     GL_ONE
// };
// 
// static const unsigned glBlendOp[] =
// {
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_ADD,
//     GL_FUNC_REVERSE_SUBTRACT,
//     GL_FUNC_REVERSE_SUBTRACT
// };
// 
// #ifndef GL_ES_VERSION_2_0
// static const unsigned glFillMode[] =
// {
//     GL_FILL,
//     GL_LINE,
//     GL_POINT
// };
// 
// static const unsigned glStencilOps[] =
// {
//     GL_KEEP,
//     GL_ZERO,
//     GL_REPLACE,
//     GL_INCR_WRAP,
//     GL_DECR_WRAP
// };
// #endif
// 
// static const unsigned glElementTypes[] =
// {
//     GL_INT,
//     GL_FLOAT,
//     GL_FLOAT,
//     GL_FLOAT,
//     GL_FLOAT,
//     GL_UNSIGNED_BYTE,
//     GL_UNSIGNED_BYTE
// };

static const unsigned glElementComponents[] =
{
    1,
    1,
    2,
    3,
    4,
    4,
    4
};

// #ifdef GL_ES_VERSION_2_0
// static unsigned glesDepthStencilFormat = GL_DEPTH_COMPONENT16;
// static unsigned glesReadableDepthFormat = GL_DEPTH_COMPONENT;
// #endif

static String extensions;

// bool CheckExtension(const String& name)
// {
//     if (extensions.Empty())
//         extensions = (const char*)glGetString(GL_EXTENSIONS);
//     return extensions.Contains(name);
// }

static void GetGLPrimitiveType(unsigned elementCount, PrimitiveType type, unsigned& primitiveCount)
{
    switch (type)
    {
    case TRIANGLE_LIST:
        primitiveCount = elementCount / 3;
        break;

    case LINE_LIST:
        primitiveCount = elementCount / 2;
        break;

    case POINT_LIST:
        primitiveCount = elementCount;
        break;

    case TRIANGLE_STRIP:
        primitiveCount = elementCount - 2;
        break;

    case LINE_STRIP:
        primitiveCount = elementCount - 1;
        break;

    case TRIANGLE_FAN:
        primitiveCount = elementCount - 2;
        break;
    }
}

const Vector2 Graphics::pixelUVOffset(0.0f, 0.0f);
bool Graphics::gl3Support = false;

Graphics::Graphics(Context* context) :
    Object(context),
    impl_(new GraphicsImpl()),
    position_(0/*SDL_WINDOWPOS_UNDEFINED*/, 0/*SDL_WINDOWPOS_UNDEFINED*/),
    shadowMapFormat_(bgfx::TextureFormat::D16/*GL_DEPTH_COMPONENT16*/),
    hiresShadowMapFormat_(bgfx::TextureFormat::D24/*GL_DEPTH_COMPONENT24*/),
//     shaderPath_("Shaders/GLSL/"),
//     shaderExtension_(".glsl"),
    shaderPath_("Shaders/BGFX/"),
    shaderExtension_(".sc"),
    orientations_("LandscapeLeft LandscapeRight"),
    render_state_{BGFX_STATE_CULL_CW | BGFX_STATE_FRONT_CCW}
// #ifndef GL_ES_VERSION_2_0
//     apiName_("GL2")
// #else
//     apiName_("GLES2")
// #endif
{
    SetTextureUnitMappings();
    ResetCachedState();

//    context_->RequireSDL(SDL_INIT_VIDEO);

    // Register Graphics library object factories
    RegisterGraphicsLibrary(context_);

#ifdef __EMSCRIPTEN__0
    appContext = context_;
#endif
}

Graphics::~Graphics()
{
    Close();

    delete impl_;
    impl_ = nullptr;

    context_->ReleaseSDL();
}

bool Graphics::SetScreenMode(int width, int height, const ScreenModeParams& params, bool maximize)
{
    URHO3D_PROFILE(SetScreenMode);

    // Ensure that parameters are properly filled
    ScreenModeParams newParams = params;
    AdjustScreenMode(width, height, newParams, maximize);

    if (IsInitialized() && width == width_ && height == height_ && screenParams_ == newParams)
        return true;
#ifndef __EMSCRIPTEN__
    entry::setWindowSize(default_window_, width, height);
#endif
    width_ = width;
    height_ = height;
    window_ = true;//(SDL_Window*)entry::getNativeWindow(); // (SDL_Window*) 1;
    OnScreenModeChanged();
    CheckFeatureSupport();
    return true;
    /*
    // If only vsync changes, do not destroy/recreate the context
    if (IsInitialized() && width == width_ && height == height_
        && screenParams_.EqualsExceptVSync(newParams) && screenParams_.vsync_ != newParams.vsync_)
    {
        SDL_GL_SetSwapInterval(newParams.vsync_ ? 1 : 0);
        screenParams_.vsync_ = newParams.vsync_;
        return true;
    }

    // Track if the window was repositioned and don't update window position in this case
    bool reposition = false;

    // With an external window, only the size can change after initial setup, so do not recreate context
    if (!externalWindow_ || !impl_->context_)
    {
        // Close the existing window and OpenGL context, mark GPU objects as lost
        Release(false, true);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifndef GL_ES_VERSION_2_0
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

        if (externalWindow_)
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        else
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        if (!forceGL2_)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        }
        else
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
        }
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        SDL_Rect display_rect;
        SDL_GetDisplayBounds(newParams.monitor_, &display_rect);
        reposition = newParams.fullscreen_ || (newParams.borderless_ && width >= display_rect.w && height >= display_rect.h);

        const int x = reposition ? display_rect.x : position_.x_;
        const int y = reposition ? display_rect.y : position_.y_;

        unsigned flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (newParams.fullscreen_)
            flags |= SDL_WINDOW_FULLSCREEN;
        if (newParams.borderless_)
            flags |= SDL_WINDOW_BORDERLESS;
        if (newParams.resizable_)
            flags |= SDL_WINDOW_RESIZABLE;

#ifndef __EMSCRIPTEN__
        if (newParams.highDPI_)
            flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

        SDL_SetHint(SDL_HINT_ORIENTATIONS, orientations_.CString());

        // Try 24-bit depth first, fallback to 16-bit
        for (const int depthSize : { 24, 16 })
        {
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthSize);

            // Try requested multisample level first, fallback to lower levels and no multisample
            for (int multiSample = newParams.multiSample_; multiSample > 0; multiSample /= 2)
            {
                if (multiSample > 1)
                {
                    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
                    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multiSample);
                }
                else
                {
                    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
                    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
                }

                if (!externalWindow_)
                    window_ = SDL_CreateWindow(windowTitle_.CString(), x, y, width, height, flags);
                else
                {
    #ifndef __EMSCRIPTEN__
                    if (!window_)
                        window_ = SDL_CreateWindowFrom(externalWindow_, SDL_WINDOW_OPENGL);
                    newParams.fullscreen_ = false;
    #endif
                }

                if (window_)
                {
                    // TODO: We probably want to keep depthSize as well
                    newParams.multiSample_ = multiSample;
                    break;
                }
            }

            if (window_)
                break;
        }

        if (!window_)
        {
            URHO3D_LOGERRORF("Could not create window, root cause: '%s'", SDL_GetError());
            return false;
        }

        // Reposition the window on the specified monitor
        if (reposition)
            SDL_SetWindowPosition(window_, display_rect.x, display_rect.y);

        CreateWindowIcon();

        if (maximize)
        {
            Maximize();
            SDL_GL_GetDrawableSize(window_, &width, &height);
        }

        // Create/restore context and GPU objects and set initial renderstate
        Restore();

        // Specific error message is already logged by Restore() when context creation or OpenGL extensions check fails
        if (!impl_->context_)
            return false;
    }

    // Set vsync
    SDL_GL_SetSwapInterval(newParams.vsync_ ? 1 : 0);

    // Store the system FBO on iOS/tvOS now
#if defined(IOS) || defined(TVOS)
    //glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&impl_->systemFBO_);
#endif

    screenParams_ = newParams;

    SDL_GL_GetDrawableSize(window_, &width_, &height_);
    if (!reposition)
        SDL_GetWindowPosition(window_, &position_.x_, &position_.y_);

    int logicalWidth, logicalHeight;
    SDL_GetWindowSize(window_, &logicalWidth, &logicalHeight);
    screenParams_.highDPI_ = (width_ != logicalWidth) || (height_ != logicalHeight);

    // Reset rendertargets and viewport for the new screen mode
    ResetRenderTargets();

    // Clear the initial window contents to black
    Clear(CLEAR_COLOR);
    SDL_GL_SwapWindow(window_);

    CheckFeatureSupport();

// #ifdef URHO3D_LOGGING
//     URHO3D_LOGINFOF("Adapter used %s %s", (const char *) glGetString(GL_VENDOR), (const char *) glGetString(GL_RENDERER));
// #endif

    OnScreenModeChanged();

    return true;
    */
}

void Graphics::SetSRGB(bool enable)
{
    enable &= sRGBWriteSupport_;

    if (enable != sRGB_)
    {
        sRGB_ = enable;
        impl_->fboDirty_ = true;
    }
}

void Graphics::SetDither(bool enable)
{
//     if (enable)
//         glEnable(GL_DITHER);
//     else
//         glDisable(GL_DITHER);
}

void Graphics::SetFlushGPU(bool enable)
{
    // Currently unimplemented on OpenGL
}

void Graphics::SetForceGL2(bool enable)
{
    if (IsInitialized())
    {
        URHO3D_LOGERROR("OpenGL 2 can only be forced before setting the initial screen mode");
        return;
    }

    forceGL2_ = enable;
}

void Graphics::Close()
{
    if (!IsInitialized())
        return;

    // Actually close the window
    Release(true, true);
}

bool Graphics::TakeScreenShot(Image& destImage)
{
    /*
    URHO3D_PROFILE(TakeScreenShot);

    if (!IsInitialized())
        return false;

    if (IsDeviceLost())
    {
        URHO3D_LOGERROR("Can not take screenshot while device is lost");
        return false;
    }

    ResetRenderTargets();

#ifndef GL_ES_VERSION_2_0
    destImage.SetSize(width_, height_, 3);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, destImage.GetData());
#else
    // Use RGBA format on OpenGL ES, as otherwise (at least on Android) the produced image is all black
    destImage.SetSize(width_, height_, 4);
    glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, destImage.GetData());
#endif

    // On OpenGL we need to flip the image vertically after reading
    destImage.FlipVertical();
    */
    return true;
}

bool Graphics::BeginFrame()
{
    if (!IsInitialized() || IsDeviceLost())
        return false;

    // If using an external window, check it for size changes, and reset screen mode if necessary
    if (externalWindow_)
    {
//         int width, height;
// 
//         SDL_GL_GetDrawableSize(window_, &width, &height);
//         if (width != width_ || height != height_)
//             SetMode(width, height);
    }

    // Re-enable depth test and depth func in case a third party program has modified it
//     glEnable(GL_DEPTH_TEST);
//     glDepthFunc(glCmpFunc[depthTestMode_]);

    // Set default rendertarget and depth buffer
    ResetRenderTargets();
    //
    last_view_id_ = 0xff;
    current_view_id_ = 0;
    bgfx::setViewFrameBuffer(current_view_id_, BGFX_INVALID_HANDLE);
    bgfx::setViewRect(current_view_id_, viewport_.left_, viewport_.top_, viewport_.Width(), viewport_.Height());
    bgfx::setViewScissor(current_view_id_);
    bgfx::setScissor();

    // Cleanup textures from previous frame
    for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
        SetTexture(i, nullptr);

    // Enable color and depth write
    SetColorWrite(true);
    SetDepthWrite(true);

    numPrimitives_ = 0;
    numBatches_ = 0;

    SendEvent(E_BEGINRENDERING);

    return true;
}

void Graphics::EndFrame()
{
    if (!IsInitialized())
        return;

    bgfx::frame();

    for (uint16_t vid = 0; vid <= current_view_id_; vid++)
    {
        bgfx::resetView(vid);
    }

    URHO3D_PROFILE(Present);

    SendEvent(E_ENDRENDERING);

    //SDL_GL_SwapWindow(window_);

    // Clean up too large scratch buffers
    CleanupScratchBuffers();
}

void Graphics::Clear(ClearTargetFlags flags, const Color& color, float depth, unsigned stencil)
{
    PrepareDraw();

#ifdef GL_ES_VERSION_2_0
    flags &= ~CLEAR_STENCIL;
#endif

    bool oldColorWrite = colorWrite_;
    bool oldDepthWrite = depthWrite_;

    if (flags & CLEAR_COLOR && !oldColorWrite)
        SetColorWrite(true);
    if (flags & CLEAR_DEPTH && !oldDepthWrite)
        SetDepthWrite(true);
//     if (flags & CLEAR_STENCIL && stencilWriteMask_ != M_MAX_UNSIGNED)
//         glStencilMask(M_MAX_UNSIGNED);

    unsigned glFlags = 0;
    uint64_t clearFlag = 0;
    if (flags & CLEAR_COLOR)
        clearFlag |= BGFX_CLEAR_COLOR;
    if (flags & CLEAR_DEPTH)
        clearFlag |= BGFX_CLEAR_DEPTH;
    if (flags & CLEAR_STENCIL)
        clearFlag |= BGFX_CLEAR_STENCIL;

    auto ToRGBA = [](const Color& color) {
        auto r = (unsigned)Clamp(((int)(color.r_ * 255.0f)), 0, 255);
        auto g = (unsigned)Clamp(((int)(color.g_ * 255.0f)), 0, 255);
        auto b = (unsigned)Clamp(((int)(color.b_ * 255.0f)), 0, 255);
        auto a = (unsigned)Clamp(((int)(color.a_ * 255.0f)), 0, 255);
        return (r << 24u) | (g << 16u) | (b << 8u) | a;
    };
    bgfx::setViewClear(current_view_id_, clearFlag, ToRGBA(color), depth, stencil);

    // If viewport is less than full screen, set a scissor to limit the clear
    /// \todo Any user-set scissor test will be lost
    IntVector2 viewSize = GetRenderTargetDimensions();
    if (viewport_.left_ != 0 || viewport_.top_ != 0 || viewport_.right_ != viewSize.x_ || viewport_.bottom_ != viewSize.y_)
        SetScissorTest(true, IntRect(0, 0, viewport_.Width(), viewport_.Height()));
    else
        SetScissorTest(false);

    bgfx::setViewRect(current_view_id_, viewport_.left_, viewport_.top_, viewport_.Width(), viewport_.Height());
    bgfx::setViewScissor(current_view_id_, scissorRect_.left_, scissorRect_.top_, scissorRect_.Width(), scissorRect_.Height());
    bgfx::touch(current_view_id_);

    SetScissorTest(false);
    SetColorWrite(oldColorWrite);
    SetDepthWrite(oldDepthWrite);
}

bool Graphics::ResolveToTexture(Texture2D* destination, const IntRect& viewport)
{
//     if (!destination || !destination->GetRenderSurface())
//         return false;
// 
//     URHO3D_PROFILE(ResolveToTexture);
// 
//     IntRect vpCopy = viewport;
//     if (vpCopy.right_ <= vpCopy.left_)
//         vpCopy.right_ = vpCopy.left_ + 1;
//     if (vpCopy.bottom_ <= vpCopy.top_)
//         vpCopy.bottom_ = vpCopy.top_ + 1;
//     vpCopy.left_ = Clamp(vpCopy.left_, 0, width_);
//     vpCopy.top_ = Clamp(vpCopy.top_, 0, height_);
//     vpCopy.right_ = Clamp(vpCopy.right_, 0, width_);
//     vpCopy.bottom_ = Clamp(vpCopy.bottom_, 0, height_);
// 
//     // Make sure the FBO is not in use
//     ResetRenderTargets();
// 
//     // Use Direct3D convention with the vertical coordinates ie. 0 is top
//     SetTextureForUpdate(destination);
//     glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vpCopy.left_, height_ - vpCopy.bottom_, vpCopy.Width(), vpCopy.Height());
//     SetTexture(0, nullptr);

    return true;
}

bool Graphics::ResolveToTexture(Texture2D* texture)
{
    return true;
// #ifndef GL_ES_VERSION_2_0
//     if (!texture)
//         return false;
//     RenderSurface* surface = texture->GetRenderSurface();
//     if (!surface || !surface->GetRenderBuffer())
//         return false;
// 
//     URHO3D_PROFILE(ResolveToTexture);
// 
//     texture->SetResolveDirty(false);
//     surface->SetResolveDirty(false);
// 
//     // Use separate FBOs for resolve to not disturb the currently set rendertarget(s)
//     if (!impl_->resolveSrcFBO_)
//         impl_->resolveSrcFBO_ = CreateFramebuffer();
//     if (!impl_->resolveDestFBO_)
//         impl_->resolveDestFBO_ = CreateFramebuffer();
// 
//     if (!gl3Support)
//     {
//         glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, impl_->resolveSrcFBO_);
//         glFramebufferRenderbufferEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT,
//             surface->GetRenderBuffer());
//         glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, impl_->resolveDestFBO_);
//         glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->GetGPUObjectName(),
//             0);
//         glBlitFramebufferEXT(0, 0, texture->GetWidth(), texture->GetHeight(), 0, 0, texture->GetWidth(), texture->GetHeight(),
//             GL_COLOR_BUFFER_BIT, GL_NEAREST);
//         glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
//         glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
//     }
//     else
//     {
//         glBindFramebuffer(GL_READ_FRAMEBUFFER, impl_->resolveSrcFBO_);
//         glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, surface->GetRenderBuffer());
//         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, impl_->resolveDestFBO_);
//         glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetGPUObjectName(), 0);
//         glBlitFramebuffer(0, 0, texture->GetWidth(), texture->GetHeight(), 0, 0, texture->GetWidth(), texture->GetHeight(),
//             GL_COLOR_BUFFER_BIT, GL_NEAREST);
//         glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//     }
// 
//     // Restore previously bound FBO
//     BindFramebuffer(impl_->boundFBO_);
//     return true;
// #else
//     // Not supported on GLES
//     return false;
// #endif
}

bool Graphics::ResolveToTexture(TextureCube* texture)
{
    return true;
// #ifndef GL_ES_VERSION_2_0
//     if (!texture)
//         return false;
// 
//     URHO3D_PROFILE(ResolveToTexture);
// 
//     texture->SetResolveDirty(false);
// 
//     // Use separate FBOs for resolve to not disturb the currently set rendertarget(s)
//     if (!impl_->resolveSrcFBO_)
//         impl_->resolveSrcFBO_ = CreateFramebuffer();
//     if (!impl_->resolveDestFBO_)
//         impl_->resolveDestFBO_ = CreateFramebuffer();
// 
//     if (!gl3Support)
//     {
//         for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
//         {
//             // Resolve only the surface(s) that were actually rendered to
//             RenderSurface* surface = texture->GetRenderSurface((CubeMapFace)i);
//             if (!surface->IsResolveDirty())
//                 continue;
// 
//             surface->SetResolveDirty(false);
//             glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, impl_->resolveSrcFBO_);
//             glFramebufferRenderbufferEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT,
//                 surface->GetRenderBuffer());
//             glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, impl_->resolveDestFBO_);
//             glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//                 texture->GetGPUObjectName(), 0);
//             glBlitFramebufferEXT(0, 0, texture->GetWidth(), texture->GetHeight(), 0, 0, texture->GetWidth(), texture->GetHeight(),
//                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
//         }
// 
//         glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
//         glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
//     }
//     else
//     {
//         for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
//         {
//             RenderSurface* surface = texture->GetRenderSurface((CubeMapFace)i);
//             if (!surface->IsResolveDirty())
//                 continue;
// 
//             surface->SetResolveDirty(false);
//             glBindFramebuffer(GL_READ_FRAMEBUFFER, impl_->resolveSrcFBO_);
//             glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, surface->GetRenderBuffer());
//             glBindFramebuffer(GL_DRAW_FRAMEBUFFER, impl_->resolveDestFBO_);
//             glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//                 texture->GetGPUObjectName(), 0);
//             glBlitFramebuffer(0, 0, texture->GetWidth(), texture->GetHeight(), 0, 0, texture->GetWidth(), texture->GetHeight(),
//                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
//         }
// 
//         glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//     }
// 
//     // Restore previously bound FBO
//     BindFramebuffer(impl_->boundFBO_);
//     return true;
// #else
//     // Not supported on GLES
//     return false;
// #endif
}

void Graphics::DoDraw(PrimitiveType type, uint32_t start, uint32_t count, bool useIndex)
{
    PrepareDraw();

    unsigned primitiveCount;
    GetGLPrimitiveType(count, type, primitiveCount);

    const auto& samples = impl_->shaderProgram_->GetSamplers();
    for (auto [unit, handle] : samples)
    {
        if (textures_[unit].texture)
        {
            bgfx::setTexture(unit, {handle}, {textures_[unit].texture->GetGPUObjectHandle()}, textures_[unit].flags);
        }
        else
        {
            URHO3D_LOGERRORF("texture unit %d invalid.", unit);
        }
    }
    if (useIndex) {
        indexBuffer_->IsDynamic()
            ? bgfx::setIndexBuffer(bgfx::DynamicIndexBufferHandle{indexBuffer_->GetGPUObjectHandle()}, start, count)
            : bgfx::setIndexBuffer(bgfx::IndexBufferHandle{indexBuffer_->GetGPUObjectHandle()}, start, count);
        start = 0;
        count = UINT32_MAX;
    }

    for (unsigned i = MAX_VERTEX_STREAMS - 1; i < MAX_VERTEX_STREAMS; --i)
    {
        auto buffer = vertexBuffers_[i];
        if (!buffer || (buffer->GetGPUObjectHandle() == bgfx::kInvalidHandle && !buffer->GetTransientBuffer()))
            continue;
        auto tvb = buffer->GetTransientBuffer();
        if (tvb)
        {
            bgfx::setVertexBuffer(i, tvb);
        }
        else
        {
            buffer->IsDynamic()
                ? bgfx::setVertexBuffer(i, bgfx::DynamicVertexBufferHandle{buffer->GetGPUObjectHandle()}, start, count)
                : bgfx::setVertexBuffer(i, bgfx::VertexBufferHandle{buffer->GetGPUObjectHandle()}, start, count);
        }
    }

    if (instance_info_.buffer || instance_info_.count < UINT32_MAX)
    {
        bgfx::setInstanceDataBuffer((bgfx::InstanceDataBuffer*)instance_info_.buffer, instance_info_.start,
                                    instance_info_.count);
    }
    render_state_ &= ~BGFX_STATE_PT_MASK;
    if (type != TRIANGLE_LIST)
    {
        render_state_ |= bgfxRSPrimitiveType(type);
    }
    bgfx::setState(render_state_);
    bgfx::setStencil(front_stencil_);
    bgfx::submit(current_view_id_, {impl_->shaderProgram_->GetGPUObjectHandle()});

    numPrimitives_ += primitiveCount;
    ++numBatches_;
}

void Graphics::Draw(PrimitiveType type, unsigned vertexStart, unsigned vertexCount)
{
    if (!vertexCount || !impl_->shaderProgram_)
        return;

    DoDraw(type, vertexStart, vertexCount, false);
    
    return;

//     PrepareDraw();
// 
//     unsigned primitiveCount;
//     GetGLPrimitiveType(vertexCount, type, primitiveCount);
//     
//     const auto& samples = impl_->shaderProgram_->GetSamplers();
//     for (auto [unit, handle] : samples) {
//         if (textures_[unit].texture) {
//             bgfx::setTexture(unit, {handle}, {textures_[unit].texture->GetGPUObjectHandle()}, textures_[unit].flags);
//         } else {
//             URHO3D_LOGERRORF("texture unit %d invalid.", unit);
//         }
//     }
// 
//     for (unsigned i = MAX_VERTEX_STREAMS - 1; i < MAX_VERTEX_STREAMS; --i) {
//         auto buffer = vertexBuffers_[i];
//         if (!buffer || (buffer->GetGPUObjectHandle() == bgfx::kInvalidHandle && !buffer->GetTransientBuffer()))
//             continue;
//         auto tvb = buffer->GetTransientBuffer();
//         if (tvb) {
//             bgfx::setVertexBuffer(i, tvb);
//         } else {
//             buffer->IsDynamic()
//                 ? bgfx::setVertexBuffer(i, bgfx::DynamicVertexBufferHandle{ buffer->GetGPUObjectHandle() }, vertexStart, vertexCount)
//                 : bgfx::setVertexBuffer(i, bgfx::VertexBufferHandle{ buffer->GetGPUObjectHandle() }, vertexStart, vertexCount);
//         }
//     }
//     if (instance_info_.buffer || instance_info_.count < UINT32_MAX) {
//         bgfx::setInstanceDataBuffer((bgfx::InstanceDataBuffer*)instance_info_.buffer, instance_info_.start,
//                                     instance_info_.count);
//     }
//     render_state_ &= ~BGFX_STATE_PT_MASK;
//     if (type != TRIANGLE_LIST)
//     {
//         render_state_ |= bgfxRSPrimitiveType(type);
//     }
//     bgfx::setState(render_state_);
//     bgfx::setStencil(front_stencil_);
//     bgfx::submit(current_view_id_, {impl_->shaderProgram_->GetGPUObjectHandle()});
// 
//     numPrimitives_ += primitiveCount;
//     ++numBatches_;
}

void Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount)
{
    if (!indexCount || !indexBuffer_ || !impl_->shaderProgram_ ||
        indexBuffer_->GetGPUObjectHandle() == bgfx::kInvalidHandle)
    {
        URHO3D_LOGERROR("indexbuffer invalid.");
        return;
    }
    
    DoDraw(type, indexStart, indexCount);

    return;

//     PrepareDraw();
// 
//     unsigned primitiveCount;
//     GetGLPrimitiveType(indexCount, type, primitiveCount);
// 
//     const auto& samples = impl_->shaderProgram_->GetSamplers();
//     for (auto [unit, handle] : samples) {
//         if (textures_[unit].texture) {
//             bgfx::setTexture(unit, {handle}, {textures_[unit].texture->GetGPUObjectHandle()}, textures_[unit].flags);
//         } else {
//             URHO3D_LOGERRORF("texture unit %d invalid.", unit);
//         }
//     }
// 
//     indexBuffer_->IsDynamic() ? bgfx::setIndexBuffer(bgfx::DynamicIndexBufferHandle{ indexBuffer_->GetGPUObjectHandle()}, indexStart, indexCount)
//                         : bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ indexBuffer_->GetGPUObjectHandle()}, indexStart, indexCount);
//     for (unsigned i = MAX_VERTEX_STREAMS - 1; i < MAX_VERTEX_STREAMS; --i) {
//         auto buffer = vertexBuffers_[i];
//         if (!buffer || (buffer->GetGPUObjectHandle() == bgfx::kInvalidHandle && !buffer->GetTransientBuffer()))
//             continue;
//         auto tvb = buffer->GetTransientBuffer();
//         if (tvb) {
//             bgfx::setVertexBuffer(i, tvb);
//         } else {
//             buffer->IsDynamic()
//                 ? bgfx::setVertexBuffer(i, bgfx::DynamicVertexBufferHandle{buffer->GetGPUObjectHandle()})
//                 : bgfx::setVertexBuffer(i, bgfx::VertexBufferHandle{buffer->GetGPUObjectHandle()});
//         }
//     }
// 
//     if (instance_info_.buffer || instance_info_.count < UINT32_MAX) {
//         bgfx::setInstanceDataBuffer((bgfx::InstanceDataBuffer*)instance_info_.buffer, instance_info_.start,
//                                     instance_info_.count);
//     }
//     render_state_ &= ~BGFX_STATE_PT_MASK;
//     if (type != TRIANGLE_LIST) {
//         render_state_ |= bgfxRSPrimitiveType(type);
//     }
//     bgfx::setState(render_state_);
//     bgfx::setStencil(front_stencil_);
//     bgfx::submit(current_view_id_, { impl_->shaderProgram_->GetGPUObjectHandle() });
// 
//     numPrimitives_ += primitiveCount;
//     ++numBatches_;
}

void Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex, unsigned vertexCount)
{
    assert(false);
}

void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount,
    unsigned instanceCount)
{
    assert(false);
}

void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex,
        unsigned vertexCount, unsigned instanceCount)
{
    assert(false);
}

void Graphics::SetVertexBuffer(VertexBuffer* buffer)
{
    // Note: this is not multi-instance safe
    static PODVector<VertexBuffer*> vertexBuffers(1);
    vertexBuffers[0] = buffer;
    SetVertexBuffers(vertexBuffers);
}

bool Graphics::SetVertexBuffers(const PODVector<VertexBuffer*>& buffers, unsigned instanceOffset)
{
    if (buffers.Size() > MAX_VERTEX_STREAMS)
    {
        URHO3D_LOGERROR("Too many vertex buffers");
        return false;
    }

    if (instanceOffset != impl_->lastInstanceOffset_)
    {
        impl_->lastInstanceOffset_ = instanceOffset;
        impl_->vertexBuffersDirty_ = true;
    }

    for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
    {
        VertexBuffer* buffer = nullptr;
        if (i < buffers.Size())
            buffer = buffers[i];
        if (buffer != vertexBuffers_[i])
        {
            vertexBuffers_[i] = buffer;
            impl_->vertexBuffersDirty_ = true;
        }
    }

    return true;
}

bool Graphics::SetVertexBuffers(const Vector<SharedPtr<VertexBuffer> >& buffers, unsigned instanceOffset)
{
    return SetVertexBuffers(reinterpret_cast<const PODVector<VertexBuffer*>&>(buffers), instanceOffset);
}

void Graphics::SetIndexBuffer(IndexBuffer* buffer)
{
    if (indexBuffer_ == buffer)
        return;

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer ? buffer->GetGPUObjectName() : 0);
//     buffer->IsDynamic() ? bgfx::setIndexBuffer(bgfx::DynamicIndexBufferHandle{buffer->GetGPUObjectHandle()})
//                         : bgfx::setIndexBuffer(bgfx::IndexBufferHandle{buffer->GetGPUObjectHandle()});
    indexBuffer_ = buffer;
}

void Graphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps)
{
    if (vs == vertexShader_ && ps == pixelShader_)
        return;

    // Compile the shaders now if not yet compiled. If already attempted, do not retry
    if (vs && vs->GetGPUObjectHandle() == bgfx::kInvalidHandle/*!vs->GetGPUObjectName()*/)
    {
        if (vs->GetCompilerOutput().Empty())
        {
            URHO3D_PROFILE(CompileVertexShader);

            bool success = vs->Create();
            if (success)
                URHO3D_LOGDEBUG("Compiled vertex shader " + vs->GetFullName());
            else
            {
                URHO3D_LOGERROR("Failed to compile vertex shader " + vs->GetFullName() + ":\n" + vs->GetCompilerOutput());
                vs = nullptr;
            }
        }
        else
            vs = nullptr;
    }

    if (ps && ps->GetGPUObjectHandle() == bgfx::kInvalidHandle /*!ps->GetGPUObjectName()*/)
    {
        if (ps->GetCompilerOutput().Empty())
        {
            URHO3D_PROFILE(CompilePixelShader);

            bool success = ps->Create();
            if (success)
                URHO3D_LOGDEBUG("Compiled pixel shader " + ps->GetFullName());
            else
            {
                URHO3D_LOGERROR("Failed to compile pixel shader " + ps->GetFullName() + ":\n" + ps->GetCompilerOutput());
                ps = nullptr;
            }
        }
        else
            ps = nullptr;
    }

    if (!vs || !ps)
    {
        //glUseProgram(0);
        vertexShader_ = nullptr;
        pixelShader_ = nullptr;
        impl_->shaderProgram_ = nullptr;
    }
    else
    {
        vertexShader_ = vs;
        pixelShader_ = ps;

        Pair<ShaderVariation*, ShaderVariation*> combination(vs, ps);
        ShaderProgramMap::Iterator i = impl_->shaderPrograms_.Find(combination);

        if (i != impl_->shaderPrograms_.End())
        {
            // Use the existing linked program
            if (i->second_->GetGPUObjectHandle() != bgfx::kInvalidHandle /*i->second_->GetGPUObjectName()*/)
            {
                //glUseProgram(i->second_->GetGPUObjectName());
                lastShaderProgram_ = impl_->shaderProgram_;
                impl_->shaderProgram_ = i->second_;
            }
            else
            {
                //glUseProgram(0);
                impl_->shaderProgram_ = nullptr;
            }
        }
        else
        {
            // Link a new combination
            URHO3D_PROFILE(LinkShaders);

            SharedPtr<ShaderProgram> newProgram(new ShaderProgram(this, vs, ps));
            if (newProgram->Link())
            {
                URHO3D_LOGDEBUG("Linked vertex shader " + vs->GetFullName() + " and pixel shader " + ps->GetFullName());
                // Note: Link() calls glUseProgram() to set the texture sampler uniforms,
                // so it is not necessary to call it again
                lastShaderProgram_ = impl_->shaderProgram_;
                impl_->shaderProgram_ = newProgram;
            }
            else
            {
                URHO3D_LOGERROR("Failed to link vertex shader " + vs->GetFullName() + " and pixel shader " + ps->GetFullName() + ":\n" +
                         newProgram->GetLinkerOutput());
                //glUseProgram(0);
                impl_->shaderProgram_ = nullptr;
            }

            impl_->shaderPrograms_[combination] = newProgram;
        }
    }

    // Update the clip plane uniform on GL3, and set constant buffers
#ifndef GL_ES_VERSION_2_0
    if (false/*gl3Support && impl_->shaderProgram_*/)
    {
//         const SharedPtr<ConstantBuffer>* constantBuffers = impl_->shaderProgram_->GetConstantBuffers();
//         for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS * 2; ++i)
//         {
//             ConstantBuffer* buffer = constantBuffers[i].Get();
//             if (buffer != impl_->constantBuffers_[i])
//             {
//                 unsigned object = buffer ? buffer->GetGPUObjectName() : 0;
//                 glBindBufferBase(GL_UNIFORM_BUFFER, i, object);
//                 // Calling glBindBufferBase also affects the generic buffer binding point
//                 impl_->boundUBO_ = object;
//                 impl_->constantBuffers_[i] = buffer;
//                 ShaderProgram::ClearGlobalParameterSource((ShaderParameterGroup)(i % MAX_SHADER_PARAMETER_GROUPS));
//             }
//         }

        SetShaderParameter(VSP_CLIPPLANE, useClipPlane_ ? clipPlane_ : Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    }
#endif

    // Store shader combination if shader dumping in progress
    if (shaderPrecache_)
        shaderPrecache_->StoreShaders(vertexShader_, pixelShader_);

    if (impl_->shaderProgram_)
    {
        impl_->usedVertexAttributes_ = impl_->shaderProgram_->GetUsedVertexAttributes();
        impl_->vertexAttributes_ = &impl_->shaderProgram_->GetVertexAttributes();
    }
    else
    {
        impl_->usedVertexAttributes_ = 0;
        impl_->vertexAttributes_ = nullptr;
    }

    impl_->vertexBuffersDirty_ = true;
}

void Graphics::SetShaderParameter(StringHash param, const float* data, unsigned count)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle)) {
            bgfx::setUniform(uniformHandle, data, count);
        }
        /*
        const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
        if (info)
        {
            if (info->bufferPtr_)
            {
                ConstantBuffer* buffer = info->bufferPtr_;
                if (!buffer->IsDirty())
                    impl_->dirtyConstantBuffers_.Push(buffer);
                buffer->SetParameter(info->offset_, (unsigned)(count * sizeof(float)), data);
                return;
            }

            switch (info->glType_)
            {
            case GL_FLOAT:
                glUniform1fv(info->location_, count, data);
                break;

            case GL_FLOAT_VEC2:
                glUniform2fv(info->location_, count / 2, data);
                break;

            case GL_FLOAT_VEC3:
                glUniform3fv(info->location_, count / 3, data);
                break;

            case GL_FLOAT_VEC4:
                glUniform4fv(info->location_, count / 4, data);
                break;

            case GL_FLOAT_MAT3:
                glUniformMatrix3fv(info->location_, count / 9, GL_FALSE, data);
                break;

            case GL_FLOAT_MAT4:
                glUniformMatrix4fv(info->location_, count / 16, GL_FALSE, data);
                break;

            default: break;
            }
        }
        */
    }
}

void Graphics::SetShaderParameter(StringHash param, float value)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle)) {
            bgfx::setUniform(uniformHandle, &value);
        }
        /*
        const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
        if (info)
        {
            if (info->bufferPtr_)
            {
                ConstantBuffer* buffer = info->bufferPtr_;
                if (!buffer->IsDirty())
                    impl_->dirtyConstantBuffers_.Push(buffer);
                buffer->SetParameter(info->offset_, sizeof(float), &value);
                return;
            }

            glUniform1fv(info->location_, 1, &value);
        }
        */
    }
}

void Graphics::SetShaderParameter(StringHash param, int value)
{
    /*
    if (impl_->shaderProgram_)
    {
        const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
        if (info)
        {
            if (info->bufferPtr_)
            {
                ConstantBuffer* buffer = info->bufferPtr_;
                if (!buffer->IsDirty())
                    impl_->dirtyConstantBuffers_.Push(buffer);
                buffer->SetParameter(info->offset_, sizeof(int), &value);
                return;
            }

            glUniform1i(info->location_, value);
        }
    }
    */
}

void Graphics::SetShaderParameter(StringHash param, bool value)
{
    /*
    // \todo Not tested
    if (impl_->shaderProgram_)
    {
        const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
        if (info)
        {
            if (info->bufferPtr_)
            {
                ConstantBuffer* buffer = info->bufferPtr_;
                if (!buffer->IsDirty())
                    impl_->dirtyConstantBuffers_.Push(buffer);
                buffer->SetParameter(info->offset_, sizeof(bool), &value);
                return;
            }

            glUniform1i(info->location_, (int)value);
        }
    }
    */
}

void Graphics::SetShaderParameter(StringHash param, const Color& color)
{
    SetShaderParameter(param, color.Data(), 1);
}

void Graphics::SetShaderParameter(StringHash param, const Vector2& vector)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            bgfx::setUniform(uniformHandle, vector.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetParameter(info->offset_, sizeof(Vector2), &vector);
//                 return;
//             }
// 
//             // Check the uniform type to avoid mismatch
//             switch (info->glType_)
//             {
//             case GL_FLOAT:
//                 glUniform1fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC2:
//                 glUniform2fv(info->location_, 1, vector.Data());
//                 break;
// 
//             default: break;
//             }
//         }
    }
}

void Graphics::SetShaderParameter(StringHash param, const Matrix3& matrix)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            bgfx::setUniform(uniformHandle, matrix.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetVector3ArrayParameter(info->offset_, 3, &matrix);
//                 return;
//             }
// 
//             glUniformMatrix3fv(info->location_, 1, GL_FALSE, matrix.Data());
//         }
    }
}

void Graphics::SetShaderParameter(StringHash param, const Vector3& vector)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            bgfx::setUniform(uniformHandle, vector.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetParameter(info->offset_, sizeof(Vector3), &vector);
//                 return;
//             }
// 
//             // Check the uniform type to avoid mismatch
//             switch (info->glType_)
//             {
//             case GL_FLOAT:
//                 glUniform1fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC2:
//                 glUniform2fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC3:
//                 glUniform3fv(info->location_, 1, vector.Data());
//                 break;
// 
//             default: break;
//             }
//         }
    }
}

void Graphics::SetShaderParameter(StringHash param, const Matrix4& matrix)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            bgfx::setUniform(uniformHandle, matrix.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetParameter(info->offset_, sizeof(Matrix4), &matrix);
//                 return;
//             }
// 
//             glUniformMatrix4fv(info->location_, 1, GL_FALSE, matrix.Data());
//         }
    }
}

void Graphics::SetShaderParameter(StringHash param, const Vector4& vector)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            bgfx::setUniform(uniformHandle, vector.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetParameter(info->offset_, sizeof(Vector4), &vector);
//                 return;
//             }
// 
//             // Check the uniform type to avoid mismatch
//             switch (info->glType_)
//             {
//             case GL_FLOAT:
//                 glUniform1fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC2:
//                 glUniform2fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC3:
//                 glUniform3fv(info->location_, 1, vector.Data());
//                 break;
// 
//             case GL_FLOAT_VEC4:
//                 glUniform4fv(info->location_, 1, vector.Data());
//                 break;
// 
//             default: break;
//             }
//         }
    }
}

void Graphics::SetShaderParameter(StringHash param, const Matrix3x4& matrix)
{
    if (impl_->shaderProgram_)
    {
        bgfx::UniformHandle uniformHandle{impl_->shaderProgram_->GetUniform(param)};
        if (bgfx::isValid(uniformHandle))
        {
            static Matrix4 fullMatrix;
            fullMatrix.m00_ = matrix.m00_;
            fullMatrix.m01_ = matrix.m01_;
            fullMatrix.m02_ = matrix.m02_;
            fullMatrix.m03_ = matrix.m03_;
            fullMatrix.m10_ = matrix.m10_;
            fullMatrix.m11_ = matrix.m11_;
            fullMatrix.m12_ = matrix.m12_;
            fullMatrix.m13_ = matrix.m13_;
            fullMatrix.m20_ = matrix.m20_;
            fullMatrix.m21_ = matrix.m21_;
            fullMatrix.m22_ = matrix.m22_;
            fullMatrix.m23_ = matrix.m23_;
            bgfx::setUniform(uniformHandle, fullMatrix.Data());
        }
        //         const ShaderParameter* info = impl_->shaderProgram_->GetParameter(param);
//         if (info)
//         {
//             // Expand to a full Matrix4
//             static Matrix4 fullMatrix;
//             fullMatrix.m00_ = matrix.m00_;
//             fullMatrix.m01_ = matrix.m01_;
//             fullMatrix.m02_ = matrix.m02_;
//             fullMatrix.m03_ = matrix.m03_;
//             fullMatrix.m10_ = matrix.m10_;
//             fullMatrix.m11_ = matrix.m11_;
//             fullMatrix.m12_ = matrix.m12_;
//             fullMatrix.m13_ = matrix.m13_;
//             fullMatrix.m20_ = matrix.m20_;
//             fullMatrix.m21_ = matrix.m21_;
//             fullMatrix.m22_ = matrix.m22_;
//             fullMatrix.m23_ = matrix.m23_;
// 
//             if (info->bufferPtr_)
//             {
//                 ConstantBuffer* buffer = info->bufferPtr_;
//                 if (!buffer->IsDirty())
//                     impl_->dirtyConstantBuffers_.Push(buffer);
//                 buffer->SetParameter(info->offset_, sizeof(Matrix4), &fullMatrix);
//                 return;
//             }
// 
//             glUniformMatrix4fv(info->location_, 1, GL_FALSE, fullMatrix.Data());
//         }
    }
}

bool Graphics::NeedParameterUpdate(ShaderParameterGroup group, const void* source)
{
    return impl_->shaderProgram_ ? impl_->shaderProgram_->NeedParameterUpdate(group, source) : false;
}

bool Graphics::HasShaderParameter(StringHash param)
{
    return impl_->shaderProgram_ && impl_->shaderProgram_->HasParameter(param);
}

bool Graphics::HasTextureUnit(TextureUnit unit)
{
    return impl_->shaderProgram_ && impl_->shaderProgram_->HasTextureUnit(unit);
}

void Graphics::ClearParameterSource(ShaderParameterGroup group)
{
    if (impl_->shaderProgram_)
        impl_->shaderProgram_->ClearParameterSource(group);
}

void Graphics::ClearParameterSources()
{
    ShaderProgram::ClearParameterSources();
}

void Graphics::ClearTransformSources()
{
    if (impl_->shaderProgram_)
    {
        impl_->shaderProgram_->ClearParameterSource(SP_CAMERA);
        impl_->shaderProgram_->ClearParameterSource(SP_OBJECT);
    }
}

void Graphics::SetTexture(unsigned index, Texture* texture) { SetTextureEx(index, {}, texture, UINT32_MAX); }

void Graphics::SetTextureEx(unsigned index, StringHash uniformHandle, Texture* texture, uint32_t flags)
{
    if (index >= MAX_TEXTURE_UNITS)
        return;

    // Check if texture is currently bound as a rendertarget. In that case, use its backup texture, or blank if not defined
    if (texture)
    {
        if (renderTargets_[0] && renderTargets_[0]->GetParentTexture() == texture)
            texture = texture->GetBackupTexture();
        else
        {
            // Resolve multisampled texture now as necessary
            if (texture->GetMultiSample() > 1 && texture->GetAutoResolve() && texture->IsResolveDirty())
            {
                if (texture->GetType() == Texture2D::GetTypeStatic())
                    ResolveToTexture(static_cast<Texture2D*>(texture));
                if (texture->GetType() == TextureCube::GetTypeStatic())
                    ResolveToTexture(static_cast<TextureCube*>(texture));
            }
        }
    }

//     static StringHash samplerName[TextureUnit::MAX_TEXTURE_UNITS] = {
//         {"DiffMap"},
//         {"NormalMap"},
//         {"SpecMap"},
//         {"EmissiveMap"},
//         /*{"EnvMap"},*/
//         {"EnvCubeMap"},
//         {"WeightMap0"},
//         {"DetailMap1"},
//         {"DetailMap2"},
//         {"LightRampMap"},
//         {"LightSpotMap"},
//         {"ShadowMap"},
//         {"FaceSelectCubeMap"},
//         {"IndirectionCubeMap"},
//         {"DetailMap3"},
//         {""},
//         {"ZoneCubeMap"}
//     };
    if (textures_[index].texture != texture)
    {
        textures_[index].texture = texture;
        textures_[index].uniform_handle = uniformHandle;
        textures_[index].flags = flags;
    }
//     if (texture && impl_->shaderProgram_)
//     {
//         auto sampler_handle = impl_->shaderProgram_->GetSampler(index);
//         if (sampler_handle != bgfx::kInvalidHandle)
//         {
//             bgfx::setTexture(index, {sampler_handle}, {texture->GetGPUObjectHandle()});
//         }
//         else
//         {
//             if (index == TU_FACESELECT || index == TU_INDIRECTION || index == TU_ENVIRONMENT)
//             {
//                 textures_[index] = texture;
//             }
//             else
//             {
//                 URHO3D_LOGERROR("Can not found texture unit : %d.", index);
//             }
//         }
//     }
    /*
    if (textures_[index] != texture)
    {
        if (impl_->activeTexture_ != index)
        {
            glActiveTexture(GL_TEXTURE0 + index);
            impl_->activeTexture_ = index;
        }

        if (texture)
        {
            unsigned glType = texture->GetTarget();
            // Unbind old texture type if necessary
            if (impl_->textureTypes_[index] && impl_->textureTypes_[index] != glType)
                glBindTexture(impl_->textureTypes_[index], 0);
            glBindTexture(glType, texture->GetGPUObjectName());
            impl_->textureTypes_[index] = glType;

            if (texture->GetParametersDirty())
                texture->UpdateParameters();
            if (texture->GetLevelsDirty())
                texture->RegenerateLevels();
        }
        else if (impl_->textureTypes_[index])
        {
            glBindTexture(impl_->textureTypes_[index], 0);
            impl_->textureTypes_[index] = 0;
        }

        textures_[index] = texture;
    }
    else
    {
        if (texture && (texture->GetParametersDirty() || texture->GetLevelsDirty()))
        {
            if (impl_->activeTexture_ != index)
            {
                glActiveTexture(GL_TEXTURE0 + index);
                impl_->activeTexture_ = index;
            }

            glBindTexture(texture->GetTarget(), texture->GetGPUObjectName());
            if (texture->GetParametersDirty())
                texture->UpdateParameters();
            if (texture->GetLevelsDirty())
                texture->RegenerateLevels();
        }
    }
    */
}

void Graphics::SetTextureForUpdate(Texture* texture)
{
    /*
    if (impl_->activeTexture_ != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        impl_->activeTexture_ = 0;
    }

    unsigned glType = texture->GetTarget();
    // Unbind old texture type if necessary
    if (impl_->textureTypes_[0] && impl_->textureTypes_[0] != glType)
        glBindTexture(impl_->textureTypes_[0], 0);
    glBindTexture(glType, texture->GetGPUObjectName());
    impl_->textureTypes_[0] = glType;
    textures_[0] = texture;
    */
}

void Graphics::SetDefaultTextureFilterMode(TextureFilterMode mode)
{
    if (mode != defaultTextureFilterMode_)
    {
        defaultTextureFilterMode_ = mode;
        SetTextureParametersDirty();
    }
}

void Graphics::SetDefaultTextureAnisotropy(unsigned level)
{
    level = Max(level, 1U);

    if (level != defaultTextureAnisotropy_)
    {
        defaultTextureAnisotropy_ = level;
        SetTextureParametersDirty();
    }
}

void Graphics::SetTextureParametersDirty()
{
    MutexLock lock(gpuObjectMutex_);

    for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
    {
        auto* texture = dynamic_cast<Texture*>(*i);
        if (texture)
            texture->SetParametersDirty();
    }
}

void Graphics::ResetRenderTargets()
{
    for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
        SetRenderTarget(i, (RenderSurface*)nullptr);
    SetDepthStencil((RenderSurface*)nullptr);
    SetViewport(IntRect(0, 0, width_, height_));
}

void Graphics::ResetRenderTarget(unsigned index)
{
    SetRenderTarget(index, (RenderSurface*)nullptr);
}

void Graphics::ResetDepthStencil()
{
    SetDepthStencil((RenderSurface*)nullptr);
}

void Graphics::SetRenderTarget(unsigned index, RenderSurface* renderTarget)
{
    if (index >= MAX_RENDERTARGETS)
        return;

    if (renderTarget != renderTargets_[index])
    {
        renderTargets_[index] = renderTarget;
        view_context_dirty_ = true;
        // If the rendertarget is also bound as a texture, replace with backup texture or null
//         if (renderTarget)
//         {
//             Texture* parentTexture = renderTarget->GetParentTexture();
// 
//             for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
//             {
//                 if (textures_[i] == parentTexture)
//                     SetTexture(i, textures_[i]->GetBackupTexture());
//             }
// 
//             // If multisampled, mark the texture & surface needing resolve
//             if (parentTexture->GetMultiSample() > 1 && parentTexture->GetAutoResolve())
//             {
//                 parentTexture->SetResolveDirty(true);
//                 renderTarget->SetResolveDirty(true);
//             }
// 
//             // If mipmapped, mark the levels needing regeneration
//             if (parentTexture->GetLevels() > 1)
//                 parentTexture->SetLevelsDirty();
//         }

        impl_->fboDirty_ = true;
    }
}

void Graphics::SetRenderTarget(unsigned index, Texture2D* texture)
{
    RenderSurface* renderTarget = nullptr;
    if (texture)
        renderTarget = texture->GetRenderSurface();

    SetRenderTarget(index, renderTarget);
}

void Graphics::SetDepthStencil(RenderSurface* depthStencil)
{
    // If we are using a rendertarget texture, it is required in OpenGL to also have an own depth-stencil
    // Create a new depth-stencil texture as necessary to be able to provide similar behaviour as Direct3D9
    // Only do this for non-multisampled rendertargets; when using multisampled target a similarly multisampled
    // depth-stencil should also be provided (backbuffer depth isn't compatible)
    if (renderTargets_[0] && renderTargets_[0]->GetMultiSample() == 1 && !depthStencil)
    {
        int width = renderTargets_[0]->GetWidth();
        int height = renderTargets_[0]->GetHeight();

        // Direct3D9 default depth-stencil can not be used when rendertarget is larger than the window.
        // Check size similarly
        if (width <= width_ && height <= height_)
        {
            unsigned searchKey = (width << 16u) | height;
            HashMap<unsigned, SharedPtr<Texture2D> >::Iterator i = impl_->depthTextures_.Find(searchKey);
            if (i != impl_->depthTextures_.End())
                depthStencil = i->second_->GetRenderSurface();
            else
            {
                SharedPtr<Texture2D> newDepthTexture(new Texture2D(context_));
                newDepthTexture->SetSize(width, height, GetDepthStencilFormat(), TEXTURE_DEPTHSTENCIL);
                impl_->depthTextures_[searchKey] = newDepthTexture;
                depthStencil = newDepthTexture->GetRenderSurface();
            }
        }
    }

    if (depthStencil != depthStencil_)
    {
        depthStencil_ = depthStencil;
        view_context_dirty_ = true;
        impl_->fboDirty_ = true;
    }
}

void Graphics::SetDepthStencil(Texture2D* texture)
{
    RenderSurface* depthStencil = nullptr;
    if (texture)
        depthStencil = texture->GetRenderSurface();

    SetDepthStencil(depthStencil);
}

void Graphics::SetViewport(const IntRect& rect)
{
//    PrepareDraw();

    IntVector2 rtSize = GetRenderTargetDimensions();

    IntRect rectCopy = rect;

    if (rectCopy.right_ <= rectCopy.left_)
        rectCopy.right_ = rectCopy.left_ + 1;
    if (rectCopy.bottom_ <= rectCopy.top_)
        rectCopy.bottom_ = rectCopy.top_ + 1;
    rectCopy.left_ = Clamp(rectCopy.left_, 0, rtSize.x_);
    rectCopy.top_ = Clamp(rectCopy.top_, 0, rtSize.y_);
    rectCopy.right_ = Clamp(rectCopy.right_, 0, rtSize.x_);
    rectCopy.bottom_ = Clamp(rectCopy.bottom_, 0, rtSize.y_);

    // Use Direct3D convention with the vertical coordinates ie. 0 is top
    //glViewport(rectCopy.left_, rtSize.y_ - rectCopy.bottom_, rectCopy.Width(), rectCopy.Height());
    if (viewport_ != rectCopy)
    {
        viewport_ = rectCopy;
        view_context_dirty_ = true;
    }

    // Disable scissor test, needs to be re-enabled by the user
    SetScissorTest(false);
}

void Graphics::SetBlendMode(BlendMode mode, bool alphaToCoverage)
{
    render_state_ &= ~BGFX_STATE_BLEND_MASK;
    render_state_ &= ~BGFX_STATE_BLEND_EQUATION_MASK;
    render_state_ &= ~BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
    render_state_ |= bgfxRSBend(mode, alphaToCoverage);

//     if (mode != blendMode_)
//     {
//         if (mode == BLEND_REPLACE)
//             glDisable(GL_BLEND);
//         else
//         {
//             glEnable(GL_BLEND);
//             glBlendFunc(glSrcBlend[mode], glDestBlend[mode]);
//             glBlendEquation(glBlendOp[mode]);
//         }
// 
//         blendMode_ = mode;
//     }
// 
//     if (alphaToCoverage != alphaToCoverage_)
//     {
//         if (alphaToCoverage)
//             glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
//         else
//             glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
// 
//         alphaToCoverage_ = alphaToCoverage;
//     }
}

void Graphics::SetBlendModeEx(uint64_t mode)
{
    render_state_ &= ~BGFX_STATE_BLEND_MASK;
    render_state_ &= ~BGFX_STATE_BLEND_EQUATION_MASK;
    render_state_ &= ~BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
    render_state_ |= mode;
}

void Graphics::SetColorWrite(bool enable)
{
    render_state_ &= ~(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    if (enable)
    {
        render_state_ |= (BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    }
//     if (enable != colorWrite_)
//     {
//         if (enable)
//             glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//         else
//             glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
// 
//         colorWrite_ = enable;
//     }
}

void Graphics::SetCullMode(CullMode mode)
{
    render_state_ &= ~BGFX_STATE_CULL_MASK;
    render_state_ |= bgfxRSCull(mode);
    
//     if (mode != cullMode_)
//     {
//         if (mode == CULL_NONE)
//             glDisable(GL_CULL_FACE);
//         else
//         {
//             // Use Direct3D convention, ie. clockwise vertices define a front face
//             glEnable(GL_CULL_FACE);
//             glCullFace(mode == CULL_CCW ? GL_FRONT : GL_BACK);
//         }
// 
//         cullMode_ = mode;
//     }
}

void Graphics::SetDepthBias(float constantBias, float slopeScaledBias)
{
    if (constantBias != constantDepthBias_ || slopeScaledBias != slopeScaledDepthBias_)
    {
// #ifndef GL_ES_VERSION_2_0
//         if (slopeScaledBias != 0.0f)
//         {
//             // OpenGL constant bias is unreliable and dependent on depth buffer bitdepth, apply in the projection matrix instead
//             glEnable(GL_POLYGON_OFFSET_FILL);
//             glPolygonOffset(slopeScaledBias, 0.0f);
//         }
//         else
//             glDisable(GL_POLYGON_OFFSET_FILL);
// #endif
// 
        constantDepthBias_ = constantBias;
        slopeScaledDepthBias_ = slopeScaledBias;
        // Force update of the projection matrix shader parameter
        ClearParameterSource(SP_CAMERA);
    }
    //SetShaderParameter(VSP_DEPTHBIAS, Vector4{ constantBias, slopeScaledDepthBias_, 0.0f, 0.0f });
}

void Graphics::SetDepthTest(CompareMode mode)
{
    render_state_ &= ~BGFX_STATE_DEPTH_TEST_MASK;
    render_state_ |= bgfxRSDepthCompare(mode);
//     if (mode != depthTestMode_)
//     {
//         glDepthFunc(glCmpFunc[mode]);
//         depthTestMode_ = mode;
//     }
}

void Graphics::SetDepthWrite(bool enable)
{
    render_state_ &= ~BGFX_STATE_WRITE_Z;
    if (enable)
    {
        render_state_ |= BGFX_STATE_WRITE_Z;
    }

//     if (enable != depthWrite_)
//     {
//         glDepthMask(enable ? GL_TRUE : GL_FALSE);
//         depthWrite_ = enable;
//     }
}

void Graphics::SetFillMode(FillMode mode)
{
// #ifndef GL_ES_VERSION_2_0
//     if (mode != fillMode_)
//     {
//         glPolygonMode(GL_FRONT_AND_BACK, glFillMode[mode]);
//         fillMode_ = mode;
//     }
// #endif
    
    //_render->m_debug |= BGFX_DEBUG_WIREFRAME;
}

void Graphics::SetLineAntiAlias(bool enable)
{
    render_state_ &= ~BGFX_STATE_LINEAA;
    if (enable)
    {
        render_state_ |= BGFX_STATE_LINEAA;
    }
// #ifndef GL_ES_VERSION_2_0
//     if (enable != lineAntiAlias_)
//     {
//         if (enable)
//             glEnable(GL_LINE_SMOOTH);
//         else
//             glDisable(GL_LINE_SMOOTH);
//         lineAntiAlias_ = enable;
//     }
// #endif
}

void Graphics::SetScissorTest(bool enable, const Rect& rect, bool borderInclusive)
{
    // During some light rendering loops, a full rect is toggled on/off repeatedly.
    // Disable scissor in that case to reduce state changes
    if (rect.min_.x_ <= 0.0f && rect.min_.y_ <= 0.0f && rect.max_.x_ >= 1.0f && rect.max_.y_ >= 1.0f)
        enable = false;

    if (enable)
    {
        IntVector2 rtSize(GetRenderTargetDimensions());
        IntVector2 viewSize(viewport_.Size());
        IntVector2 viewPos(viewport_.left_, viewport_.top_);
        IntRect intRect;
        int expand = borderInclusive ? 1 : 0;

        intRect.left_ = Clamp((int)((rect.min_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_, 0, rtSize.x_ - 1);
        intRect.top_ = Clamp((int)((-rect.max_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_, 0, rtSize.y_ - 1);
        intRect.right_ = Clamp((int)((rect.max_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_ + expand, 0, rtSize.x_);
        intRect.bottom_ = Clamp((int)((-rect.min_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_ + expand, 0, rtSize.y_);

        if (intRect.right_ == intRect.left_)
            intRect.right_++;
        if (intRect.bottom_ == intRect.top_)
            intRect.bottom_++;

        if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
            enable = false;

        if (enable && scissorRect_ != intRect)
        {
            scissorRect_ = intRect;
            view_context_dirty_ = true;
        }
    }
    else
    {
        if (scissorRect_ != IntRect::ZERO)
        {
            scissorRect_ = IntRect::ZERO;
            view_context_dirty_ = true;
        }
    }
}

void Graphics::SetScissorTest(bool enable, const IntRect& rect)
{
    IntVector2 rtSize(GetRenderTargetDimensions());
    IntVector2 viewPos(viewport_.left_, viewport_.top_);

    if (enable)
    {
        IntRect intRect;
        intRect.left_ = Clamp(rect.left_ + viewPos.x_, 0, rtSize.x_ - 1);
        intRect.top_ = Clamp(rect.top_ + viewPos.y_, 0, rtSize.y_ - 1);
        intRect.right_ = Clamp(rect.right_ + viewPos.x_, 0, rtSize.x_);
        intRect.bottom_ = Clamp(rect.bottom_ + viewPos.y_, 0, rtSize.y_);

        if (intRect.right_ == intRect.left_)
            intRect.right_++;
        if (intRect.bottom_ == intRect.top_)
            intRect.bottom_++;

        if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
            enable = false;

        if (enable && scissorRect_ != intRect)
        {
            scissorRect_ = intRect;
            view_context_dirty_ = true;
        }
    }
    else
    {
        if (scissorRect_ != IntRect::ZERO)
        {
            scissorRect_ = IntRect::ZERO;
            view_context_dirty_ = true;
        }
    }
}

void Graphics::SetClipPlane(bool enable, const Plane& clipPlane, const Matrix3x4& view, const Matrix4& projection)
{
// #ifndef GL_ES_VERSION_2_0
//     if (enable != useClipPlane_)
//     {
//         if (enable)
//             glEnable(GL_CLIP_PLANE0);
//         else
//             glDisable(GL_CLIP_PLANE0);
// 
//         useClipPlane_ = enable;
//     }
// 
//     if (enable)
//     {
//         Matrix4 viewProj = projection * view;
//         clipPlane_ = clipPlane.Transformed(viewProj).ToVector4();
// 
//         if (!gl3Support)
//         {
//             GLdouble planeData[4];
//             planeData[0] = clipPlane_.x_;
//             planeData[1] = clipPlane_.y_;
//             planeData[2] = clipPlane_.z_;
//             planeData[3] = clipPlane_.w_;
// 
//             glClipPlane(GL_CLIP_PLANE0, &planeData[0]);
//         }
//     }
// #endif
}

void Graphics::SetStencilTest(bool enable, CompareMode mode, StencilOp pass, StencilOp fail, StencilOp zFail, unsigned stencilRef,
    unsigned compareMask, unsigned writeMask)
{
    front_stencil_ = back_stencil_ = BGFX_STENCIL_NONE;
    auto& flag = front_stencil_;
    if (enable) {
        flag |= BGFX_STENCIL_FUNC_REF(stencilRef);
        flag |= BGFX_STENCIL_FUNC_RMASK(compareMask);
        //flag |= BGFX_STENCIL_FUNC_RMASK_MASK(writeMask);
        flag |= bgfxRSStencilCompare(mode);
        flag |= bgfxRSStencilFail(fail);
        flag |= bgfxRSDepthFail(zFail);
        flag |= bgfxRSDepthPass(pass);
    }
    back_stencil_ = front_stencil_;
    // #ifndef GL_ES_VERSION_2_0
//     if (enable != stencilTest_)
//     {
//         if (enable)
//             glEnable(GL_STENCIL_TEST);
//         else
//             glDisable(GL_STENCIL_TEST);
//         stencilTest_ = enable;
//     }
// 
//     if (enable)
//     {
//         if (mode != stencilTestMode_ || stencilRef != stencilRef_ || compareMask != stencilCompareMask_)
//         {
//             glStencilFunc(glCmpFunc[mode], stencilRef, compareMask);
//             stencilTestMode_ = mode;
//             stencilRef_ = stencilRef;
//             stencilCompareMask_ = compareMask;
//         }
//         if (writeMask != stencilWriteMask_)
//         {
//             glStencilMask(writeMask);
//             stencilWriteMask_ = writeMask;
//         }
//         if (pass != stencilPass_ || fail != stencilFail_ || zFail != stencilZFail_)
//         {
//             glStencilOp(glStencilOps[fail], glStencilOps[zFail], glStencilOps[pass]);
//             stencilPass_ = pass;
//             stencilFail_ = fail;
//             stencilZFail_ = zFail;
//         }
//     }
// #endif
}

bool Graphics::IsInitialized() const
{
    return window_;
}

bool Graphics::GetDither() const
{
    return /*glIsEnabled(GL_DITHER) ? true : */false;
}

bool Graphics::IsDeviceLost() const
{
    return false;
//    // On iOS and tvOS treat window minimization as device loss, as it is forbidden to access OpenGL when minimized
//#if defined(IOS) || defined(TVOS)
//    if (window_ && (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) != 0)
//        return true;
//#endif
//
//    return impl_->context_ == nullptr;
}

PODVector<int> Graphics::GetMultiSampleLevels() const
{
    PODVector<int> ret;
    // No multisampling always supported
    ret.Push(1);

// #ifndef GL_ES_VERSION_2_0
//     int maxSamples = 0;
//     glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
//     for (int i = 2; i <= maxSamples && i <= 16; i *= 2)
//         ret.Push(i);
// #endif

    return ret;
}

unsigned Graphics::GetFormat(CompressedFormat format) const
{
    return GetCompressedFormat(format);
//     switch (format)
//     {
//     case CF_RGBA:
//         return bgfx::TextureFormat::RGBA8; // GL_RGBA;
// 
//     case CF_DXT1:
//         return bgfx::TextureFormat::BC1; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : 0;
// 
// #if !defined(GL_ES_VERSION_2_0) || defined(__EMSCRIPTEN__)
//     case CF_DXT3:
//         return bgfx::TextureFormat::BC2; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT : 0;
// 
//     case CF_DXT5:
//         return bgfx::TextureFormat::BC3; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : 0;
// #endif
// #ifdef GL_ES_VERSION_2_0
//     case CF_ETC1:
//         return bgfx::TextureFormat::ETC1; // etcTextureSupport_ ? GL_ETC1_RGB8_OES : 0;
// 
//     case CF_ETC2_RGB:
//         return bgfx::TextureFormat::ETC2; // etc2TextureSupport_ ? GL_ETC2_RGB8_OES : 0;
// 
//     case CF_ETC2_RGBA:
//         return bgfx::TextureFormat::ETC2A; // etc2TextureSupport_ ? GL_ETC2_RGBA8_OES : 0;
// 
//     case CF_PVRTC_RGB_2BPP:
//         return bgfx::TextureFormat::PTC12; // pvrtcTextureSupport_ ? COMPRESSED_RGB_PVRTC_2BPPV1_IMG : 0;
// 
//     case CF_PVRTC_RGB_4BPP:
//         return bgfx::TextureFormat::PTC14; // pvrtcTextureSupport_ ? COMPRESSED_RGB_PVRTC_4BPPV1_IMG : 0;
// 
//     case CF_PVRTC_RGBA_2BPP:
//         return bgfx::TextureFormat::PTC12A; // pvrtcTextureSupport_ ? COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : 0;
// 
//     case CF_PVRTC_RGBA_4BPP:
//         return bgfx::TextureFormat::PTC14A; // pvrtcTextureSupport_ ? COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : 0;
// #endif
// 
//     default:
//         return bgfx::TextureFormat::Unknown;
//     }
}

unsigned Graphics::GetMaxBones()
{
#ifdef RPI
    // At the moment all RPI GPUs are low powered and only have limited number of uniforms
    return 32;
#else
    return gl3Support ? 128 : 64;
#endif
}

bool Graphics::GetGL3Support()
{
    return true; // gl3Support;
}

ShaderVariation* Graphics::GetShader(ShaderType type, const String& name, const String& defines) const
{
    return GetShader(type, name.CString(), defines.CString());
}

ShaderVariation* Graphics::GetShader(ShaderType type, const char* name, const char* defines) const
{
    if (lastShaderName_ != name || !lastShader_)
    {
        auto* cache = GetSubsystem<ResourceCache>();
        String realName;// = (type == VS) ? "vs_" : "fs_";
        realName += name;
        String fullShaderName = shaderPath_ + realName + shaderExtension_;
        // Try to reduce repeated error log prints because of missing shaders
        if (lastShaderName_ == realName && !cache->Exists(fullShaderName))
            return nullptr;

        lastShader_ = cache->GetResource<Shader>(fullShaderName);
        lastShaderName_ = realName;
    }

    return lastShader_ ? lastShader_->GetVariation(type, defines) : nullptr;
}

VertexBuffer* Graphics::GetVertexBuffer(unsigned index) const
{
    return index < MAX_VERTEX_STREAMS ? vertexBuffers_[index] : nullptr;
}

ShaderProgram* Graphics::GetShaderProgram() const
{
    return impl_->shaderProgram_;
}

TextureUnit Graphics::GetTextureUnit(const String& name)
{
    HashMap<String, TextureUnit>::Iterator i = textureUnits_.Find(name);
    if (i != textureUnits_.End())
        return i->second_;
    else
        return MAX_TEXTURE_UNITS;
}

const String& Graphics::GetTextureUnitName(TextureUnit unit)
{
    for (HashMap<String, TextureUnit>::Iterator i = textureUnits_.Begin(); i != textureUnits_.End(); ++i)
    {
        if (i->second_ == unit)
            return i->first_;
    }
    return String::EMPTY;
}

Texture* Graphics::GetTexture(unsigned index) const
{
    return index < MAX_TEXTURE_UNITS ? textures_[index].texture : nullptr;
}

RenderSurface* Graphics::GetRenderTarget(unsigned index) const
{
    return index < MAX_RENDERTARGETS ? renderTargets_[index] : nullptr;
}

IntVector2 Graphics::GetRenderTargetDimensions() const
{
    int width, height;

    if (renderTargets_[0])
    {
        width = renderTargets_[0]->GetWidth();
        height = renderTargets_[0]->GetHeight();
    }
    else if (depthStencil_)
    {
        width = depthStencil_->GetWidth();
        height = depthStencil_->GetHeight();
    }
    else
    {
        width = width_;
        height = height_;
    }

    return IntVector2(width, height);
}

void Graphics::OnWindowResized()
{
    if (!window_)
        return;

    int newWidth = GetSubsystem<Input>()->GetWidth();
    int newHeight = GetSubsystem<Input>()->GetHeight();
    //     SDL_GL_GetDrawableSize(window_, &newWidth, &newHeight);
    if (newWidth == width_ && newHeight == height_)
        return;

    width_ = newWidth;
    height_ = newHeight;

//     int logicalWidth, logicalHeight;
//     SDL_GetWindowSize(window_, &logicalWidth, &logicalHeight);
//     screenParams_.highDPI_ = (width_ != logicalWidth) || (height_ != logicalHeight);

    // Reset rendertargets and viewport for the new screen size. Also clean up any FBO's, as they may be screen size dependent
    CleanupFramebuffers();
    ResetRenderTargets();

    URHO3D_LOGDEBUGF("Window was resized to %dx%d", width_, height_);

#ifdef __EMSCRIPTEN__0
    EM_ASM({
        Module.SetRendererSize($0, $1);
    }, width_, height_);
#endif

    using namespace ScreenMode;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_WIDTH] = width_;
    eventData[P_HEIGHT] = height_;
    eventData[P_FULLSCREEN] = screenParams_.fullscreen_;
    eventData[P_RESIZABLE] = screenParams_.resizable_;
    eventData[P_BORDERLESS] = screenParams_.borderless_;
    eventData[P_HIGHDPI] = screenParams_.highDPI_;
    SendEvent(E_SCREENMODE, eventData);
}

void Graphics::OnWindowMoved()
{
    if (!window_ || screenParams_.fullscreen_)
        return;

//     int newX, newY;
// 
//     SDL_GetWindowPosition(window_, &newX, &newY);
//     if (newX == position_.x_ && newY == position_.y_)
//         return;
// 
//     position_.x_ = newX;
//     position_.y_ = newY;

    URHO3D_LOGTRACEF("Window was moved to %d,%d", position_.x_, position_.y_);

    using namespace WindowPos;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_X] = position_.x_;
    eventData[P_Y] = position_.y_;
    SendEvent(E_WINDOWPOS, eventData);
}

void Graphics::CleanupRenderSurface(RenderSurface* surface)
{
//     if (!surface)
//         return;
// 
//     // Flush pending FBO changes first if any
//     PrepareDraw();
// 
//     unsigned currentFBO = impl_->boundFBO_;
// 
//     // Go through all FBOs and clean up the surface from them
//     for (HashMap<unsigned long long, FrameBufferObject>::Iterator i = impl_->frameBuffers_.Begin();
//          i != impl_->frameBuffers_.End(); ++i)
//     {
//         for (unsigned j = 0; j < MAX_RENDERTARGETS; ++j)
//         {
//             if (i->second_.colorAttachments_[j] == surface)
//             {
//                 if (currentFBO != i->second_.fbo_)
//                 {
//                     BindFramebuffer(i->second_.fbo_);
//                     currentFBO = i->second_.fbo_;
//                 }
//                 BindColorAttachment(j, GL_TEXTURE_2D, 0, false);
//                 i->second_.colorAttachments_[j] = nullptr;
//                 // Mark drawbuffer bits to need recalculation
//                 i->second_.drawBuffers_ = M_MAX_UNSIGNED;
//             }
//         }
//         if (i->second_.depthAttachment_ == surface)
//         {
//             if (currentFBO != i->second_.fbo_)
//             {
//                 BindFramebuffer(i->second_.fbo_);
//                 currentFBO = i->second_.fbo_;
//             }
//             BindDepthAttachment(0, false);
//             BindStencilAttachment(0, false);
//             i->second_.depthAttachment_ = nullptr;
//         }
//     }
// 
//     // Restore previously bound FBO now if needed
//     if (currentFBO != impl_->boundFBO_)
//         BindFramebuffer(impl_->boundFBO_);
}

void Graphics::CleanupShaderPrograms(ShaderVariation* variation)
{
    for (ShaderProgramMap::Iterator i = impl_->shaderPrograms_.Begin(); i != impl_->shaderPrograms_.End();)
    {
        if (i->second_->GetVertexShader() == variation || i->second_->GetPixelShader() == variation)
            i = impl_->shaderPrograms_.Erase(i);
        else
            ++i;
    }

    if (vertexShader_ == variation || pixelShader_ == variation)
        impl_->shaderProgram_ = nullptr;
}

ConstantBuffer* Graphics::GetOrCreateConstantBuffer(ShaderType /*type*/,  unsigned index, unsigned size)
{
    // Note: shaderType parameter is not used on OpenGL, instead binding index should already use the PS range
    // for PS constant buffers

    unsigned key = (index << 16u) | size;
    HashMap<unsigned, SharedPtr<ConstantBuffer> >::Iterator i = impl_->allConstantBuffers_.Find(key);
    if (i == impl_->allConstantBuffers_.End())
    {
        i = impl_->allConstantBuffers_.Insert(MakePair(key, SharedPtr<ConstantBuffer>(new ConstantBuffer(context_))));
        i->second_->SetSize(size);
    }
    return i->second_.Get();
}

void Graphics::Release(bool clearGPUObjects, bool closeWindow)
{
    if (!window_)
        return;

    {
        MutexLock lock(gpuObjectMutex_);

        if (clearGPUObjects)
        {
            // Shutting down: release all GPU objects that still exist
            // Shader programs are also GPU objects; clear them first to avoid list modification during iteration
            impl_->shaderPrograms_.Clear();

            for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
                (*i)->Release();
            gpuObjects_.Clear();
        }
        else
        {
            // We are not shutting down, but recreating the context: mark GPU objects lost
            for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
                (*i)->OnDeviceLost();

            // In this case clear shader programs last so that they do not attempt to delete their OpenGL program
            // from a context that may no longer exist
            impl_->shaderPrograms_.Clear();

            SendEvent(E_DEVICELOST);
        }
    }

    CleanupFramebuffers();
    impl_->depthTextures_.Clear();

    // End fullscreen mode first to counteract transition and getting stuck problems on OS X
#if defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
//    if (closeWindow && screenParams_.fullscreen_ && !externalWindow_)
//        SDL_SetWindowFullscreen(window_, 0);
#endif

    if (impl_->context_)
    {
        // Do not log this message if we are exiting
        if (!clearGPUObjects)
            URHO3D_LOGINFO("OpenGL context lost");

        //SDL_GL_DeleteContext(impl_->context_);
        impl_->context_ = nullptr;
    }

    if (closeWindow)
    {
        //SDL_ShowCursor(SDL_TRUE);

        // Do not destroy external window except when shutting down
        if (!externalWindow_ || clearGPUObjects)
        {
            //SDL_DestroyWindow(window_);
            window_ = false;
        }
    }
}

void Graphics::Restore()
{
//     if (!window_)
//         return;
// 
// #ifdef __ANDROID__
//     // On Android the context may be lost behind the scenes as the application is minimized
//     if (impl_->context_ && !SDL_GL_GetCurrentContext())
//     {
//         impl_->context_ = 0;
//         // Mark GPU objects lost without a current context. In this case they just mark their internal state lost
//         // but do not perform OpenGL commands to delete the GL objects
//         Release(false, false);
//     }
// #endif
// 
//     // Ensure first that the context exists
//     if (!impl_->context_)
//     {
//         impl_->context_ = SDL_GL_CreateContext(window_);
// 
// #ifndef GL_ES_VERSION_2_0
//         // If we're trying to use OpenGL 3, but context creation fails, retry with 2
//         if (!forceGL2_ && !impl_->context_)
//         {
//             forceGL2_ = true;
//             SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//             SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//             SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
//             impl_->context_ = SDL_GL_CreateContext(window_);
//         }
// #endif
// 
// #if defined(IOS) || defined(TVOS)
//         glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&impl_->systemFBO_);
// #endif
// 
//         if (!impl_->context_)
//         {
//             URHO3D_LOGERRORF("Could not create OpenGL context, root cause '%s'", SDL_GetError());
//             return;
//         }
// 
//         // Clear cached extensions string from the previous context
//         extensions.Clear();
// 
//         // Initialize OpenGL extensions library (desktop only)
// #ifndef GL_ES_VERSION_2_0
//         GLenum err = glewInit();
//         if (GLEW_OK != err)
//         {
//             URHO3D_LOGERRORF("Could not initialize OpenGL extensions, root cause: '%s'", glewGetErrorString(err));
//             return;
//         }
// 
//         if (!forceGL2_ && GLEW_VERSION_3_2)
//         {
//             gl3Support = true;
//             apiName_ = "GL3";
// 
//             // Create and bind a vertex array object that will stay in use throughout
//             unsigned vertexArrayObject;
//             glGenVertexArrays(1, &vertexArrayObject);
//             glBindVertexArray(vertexArrayObject);
//         }
//         else if (GLEW_VERSION_2_0)
//         {
//             if (!GLEW_EXT_framebuffer_object || !GLEW_EXT_packed_depth_stencil)
//             {
//                 URHO3D_LOGERROR("EXT_framebuffer_object and EXT_packed_depth_stencil OpenGL extensions are required");
//                 return;
//             }
// 
//             gl3Support = false;
//             apiName_ = "GL2";
//         }
//         else
//         {
//             URHO3D_LOGERROR("OpenGL 2.0 is required");
//             return;
//         }
// 
//         // Enable seamless cubemap if possible
//         // Note: even though we check the extension, this can lead to software fallback on some old GPU's
//         // See https://github.com/urho3d/Urho3D/issues/1380 or
//         // http://distrustsimplicity.net/articles/gl_texture_cube_map_seamless-on-os-x/
//         // In case of trouble or for wanting maximum compatibility, simply remove the glEnable below.
//         if (gl3Support || GLEW_ARB_seamless_cube_map)
//             glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
// #endif
// 
//         // Set up texture data read/write alignment. It is important that this is done before uploading any texture data
//         glPixelStorei(GL_PACK_ALIGNMENT, 1);
//         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//         ResetCachedState();
//     }
// 
//     {
//         MutexLock lock(gpuObjectMutex_);
// 
//         for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
//             (*i)->OnDeviceReset();
//     }
// 
//     SendEvent(E_DEVICERESET);
}

void Graphics::MarkFBODirty()
{
    impl_->fboDirty_ = true;
}

void Graphics::SetVBO(unsigned object)
{

}

void Graphics::SetUBO(unsigned object)
{

}

unsigned Graphics::GetAlphaFormat()
{
    return bgfx::TextureFormat::A8;
    //bgfx::TextureFormat::R8;
}

unsigned Graphics::GetLuminanceFormat()
{
    return bgfx::TextureFormat::R8;
}

unsigned Graphics::GetLuminanceAlphaFormat()
{
    return bgfx::TextureFormat::RG8;
}

unsigned Graphics::GetRGBFormat()
{
    return bgfx::TextureFormat::RGB8;
}

unsigned Graphics::GetRGBAFormat()
{
    return bgfx::TextureFormat::RGBA8;
}

unsigned Graphics::GetRGBA16Format()
{
    return bgfx::TextureFormat::RGBA16;
}

unsigned Graphics::GetRGBAFloat16Format()
{
    return bgfx::TextureFormat::RGBA16F;
}

unsigned Graphics::GetRGBAFloat32Format()
{
    return bgfx::TextureFormat::RGBA32F;
}

unsigned Graphics::GetRG16Format()
{
    return bgfx::TextureFormat::RG16;
}

unsigned Graphics::GetRGFloat16Format()
{
    return bgfx::TextureFormat::RG16F;
}

unsigned Graphics::GetRGFloat32Format()
{
    return bgfx::TextureFormat::RG32F;
}

unsigned Graphics::GetFloat16Format()
{
    return bgfx::TextureFormat::R16F;
}

unsigned Graphics::GetFloat32Format()
{
    return bgfx::TextureFormat::R32F;
}

unsigned Graphics::GetLinearDepthFormat()
{
    return bgfx::TextureFormat::R32F;
}

unsigned Graphics::GetDepthStencilFormat()
{
    return bgfx::TextureFormat::D24S8;
}

unsigned Graphics::GetReadableDepthFormat()
{
    return bgfx::TextureFormat::D24;
}

unsigned Graphics::GetD32()
{
    return bgfx::TextureFormat::D32;
}

unsigned Graphics::GetBGRAFormat()
{
    return bgfx::TextureFormat::BGRA8;
}

unsigned Graphics::GetCompressedFormat(CompressedFormat format)
{
    switch (format)
    {
    case CF_RGBA:
        return bgfx::TextureFormat::RGBA8; // GL_RGBA;

    case CF_DXT1:
        return bgfx::TextureFormat::BC1; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : 0;

#if !defined(GL_ES_VERSION_2_0) || defined(__EMSCRIPTEN__)
    case CF_DXT3:
        return bgfx::TextureFormat::BC2; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT : 0;

    case CF_DXT5:
        return bgfx::TextureFormat::BC3; // dxtTextureSupport_ ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : 0;
#endif
#ifdef GL_ES_VERSION_2_0
    case CF_ETC1:
        return bgfx::TextureFormat::ETC1; // etcTextureSupport_ ? GL_ETC1_RGB8_OES : 0;

    case CF_ETC2_RGB:
        return bgfx::TextureFormat::ETC2; // etc2TextureSupport_ ? GL_ETC2_RGB8_OES : 0;

    case CF_ETC2_RGBA:
        return bgfx::TextureFormat::ETC2A; // etc2TextureSupport_ ? GL_ETC2_RGBA8_OES : 0;

    case CF_PVRTC_RGB_2BPP:
        return bgfx::TextureFormat::PTC12; // pvrtcTextureSupport_ ? COMPRESSED_RGB_PVRTC_2BPPV1_IMG : 0;

    case CF_PVRTC_RGB_4BPP:
        return bgfx::TextureFormat::PTC14; // pvrtcTextureSupport_ ? COMPRESSED_RGB_PVRTC_4BPPV1_IMG : 0;

    case CF_PVRTC_RGBA_2BPP:
        return bgfx::TextureFormat::PTC12A; // pvrtcTextureSupport_ ? COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : 0;

    case CF_PVRTC_RGBA_4BPP:
        return bgfx::TextureFormat::PTC14A; // pvrtcTextureSupport_ ? COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : 0;
#endif

    default:
        return bgfx::TextureFormat::Unknown;
    }
}

unsigned Graphics::GetFormat(const String& formatName)
{
    String nameLower = formatName.ToLower().Trimmed();

    if (nameLower == "a")
        return GetAlphaFormat();
    if (nameLower == "l")
        return GetLuminanceFormat();
    if (nameLower == "la")
        return GetLuminanceAlphaFormat();
    if (nameLower == "rgb")
        return GetRGBFormat();
    if (nameLower == "rgba")
        return GetRGBAFormat();
    if (nameLower == "rgba16")
        return GetRGBA16Format();
    if (nameLower == "rgba16f")
        return GetRGBAFloat16Format();
    if (nameLower == "rgba32f")
        return GetRGBAFloat32Format();
    if (nameLower == "rg16")
        return GetRG16Format();
    if (nameLower == "rg16f")
        return GetRGFloat16Format();
    if (nameLower == "rg32f")
        return GetRGFloat32Format();
    if (nameLower == "r16f")
        return GetFloat16Format();
    if (nameLower == "r32f" || nameLower == "float")
        return GetFloat32Format();
    if (nameLower == "lineardepth" || nameLower == "depth")
        return GetLinearDepthFormat();
    if (nameLower == "d24s8")
        return GetDepthStencilFormat();
    if (nameLower == "readabledepth" || nameLower == "hwdepth")
        return GetReadableDepthFormat();

    return GetRGBFormat();
}

void Graphics::CheckFeatureSupport()
{
    // Get renderer capabilities info.
    const bgfx::Caps* caps = bgfx::getCaps();

    // Check supported features: light pre-pass, deferred rendering and hardware depth texture
    lightPrepassSupport_ = false;
    deferredSupport_ = false;

    
#ifndef GL_ES_VERSION_2_0
    int numSupportedRTs = 1;
    if (true/*gl3Support*/)
    {
        // Work around GLEW failure to check extensions properly from a GL3 context
        instancingSupport_ = ((BGFX_CAPS_INSTANCING & caps->supported) != 0);//glDrawElementsInstanced != nullptr && glVertexAttribDivisor != nullptr;
        dxtTextureSupport_ = true;
        anisotropySupport_ = true;
        sRGBSupport_ = true;
        sRGBWriteSupport_ = true;
        numSupportedRTs = caps->limits.maxFBAttachments;
        //glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &numSupportedRTs);
    }
    else
    {
//         instancingSupport_ = GLEW_ARB_instanced_arrays != 0;
//         dxtTextureSupport_ = GLEW_EXT_texture_compression_s3tc != 0;
//         anisotropySupport_ = GLEW_EXT_texture_filter_anisotropic != 0;
//         sRGBSupport_ = GLEW_EXT_texture_sRGB != 0;
//         sRGBWriteSupport_ = GLEW_EXT_framebuffer_sRGB != 0;
// 
//         glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &numSupportedRTs);
    }

    // Must support 2 rendertargets for light pre-pass, and 4 for deferred
    if (numSupportedRTs >= 2)
        lightPrepassSupport_ = true;
    if (numSupportedRTs >= 4)
        deferredSupport_ = true;

#if defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
    // On macOS check for an Intel driver and use shadow map RGBA dummy color textures, because mixing
    // depth-only FBO rendering and backbuffer rendering will bug, resulting in a black screen in full
    // screen mode, and incomplete shadow maps in windowed mode
//    String renderer((const char*)glGetString(GL_RENDERER));
//    if (renderer.Contains("Intel", false))
//        dummyColorFormat_ = GetRGBAFormat();
#endif
#else
    // Check for supported compressed texture formats
#ifdef __EMSCRIPTEN__
    dxtTextureSupport_ = CheckExtension("WEBGL_compressed_texture_s3tc"); // https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc/
    etcTextureSupport_ = CheckExtension("WEBGL_compressed_texture_etc1"); // https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc1/
    pvrtcTextureSupport_ = CheckExtension("WEBGL_compressed_texture_pvrtc"); // https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_pvrtc/
    etc2TextureSupport_ = gl3Support || CheckExtension("WEBGL_compressed_texture_etc"); // https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc/
    // Instancing is in core in WebGL 2, so the extension may not be present anymore. In WebGL 1, find https://www.khronos.org/registry/webgl/extensions/ANGLE_instanced_arrays/
    // TODO: In the distant future, this may break if WebGL 3 is introduced, so either improve the GL_VERSION parsing here, or keep track of which WebGL version we attempted to initialize.
    instancingSupport_ = (strstr((const char *)glGetString(GL_VERSION), "WebGL 2.") != 0) || CheckExtension("ANGLE_instanced_arrays");
#else
    dxtTextureSupport_ = CheckExtension("EXT_texture_compression_dxt1");
    etcTextureSupport_ = CheckExtension("OES_compressed_ETC1_RGB8_texture");
    etc2TextureSupport_ = gl3Support || CheckExtension("OES_compressed_ETC2_RGBA8_texture");
    pvrtcTextureSupport_ = CheckExtension("IMG_texture_compression_pvrtc");
#endif

    // Check for best supported depth renderbuffer format for GLES2
    if (CheckExtension("GL_OES_depth24"))
        glesDepthStencilFormat = GL_DEPTH_COMPONENT24_OES;
    if (CheckExtension("GL_OES_packed_depth_stencil"))
        glesDepthStencilFormat = GL_DEPTH24_STENCIL8_OES;
    #ifdef __EMSCRIPTEN__
    if (!CheckExtension("WEBGL_depth_texture"))
#else
    if (!CheckExtension("GL_OES_depth_texture"))
#endif
    {
        shadowMapFormat_ = 0;
        hiresShadowMapFormat_ = 0;
        glesReadableDepthFormat = 0;
    }
    else
    {
#if defined(IOS) || defined(TVOS)
        // iOS hack: depth renderbuffer seems to fail, so use depth textures for everything if supported
        glesDepthStencilFormat = GL_DEPTH_COMPONENT;
#endif
        shadowMapFormat_ = GL_DEPTH_COMPONENT;
        hiresShadowMapFormat_ = 0;
        // WebGL shadow map rendering seems to be extremely slow without an attached dummy color texture
        #ifdef __EMSCRIPTEN__
        dummyColorFormat_ = GetRGBAFormat();
#endif
    }
#endif

    // Consider OpenGL shadows always hardware sampled, if supported at all
    hardwareShadowSupport_ = shadowMapFormat_ != 0;
}

void Graphics::PrepareDraw()
{
    if (view_context_dirty_)
    {
        view_context_dirty_ = false;
        current_view_id_++;
        if (ui_view_)
        {
            // TODO: no sort for ui
            bgfx::setViewMode(current_view_id_, bgfx::ViewMode::Sequential);
        }
    }
    if (last_view_id_ != current_view_id_)
    {
        last_view_id_ = current_view_id_;
        bgfx::setViewRect(current_view_id_, viewport_.left_, viewport_.top_, viewport_.Width(), viewport_.Height());
        bgfx::setViewScissor(current_view_id_, scissorRect_.left_, scissorRect_.top_, scissorRect_.Width(),
                             scissorRect_.Height());
//    }
//     if (impl_->fboDirty_)
//     {
//         impl_->fboDirty_ = false;
        // First check if no framebuffer is needed. In that case simply return to backbuffer rendering
        bool noFbo = !depthStencil_;
        if (noFbo)
        {
            for (auto& renderTarget : renderTargets_)
            {
                if (renderTarget)
                {
                    noFbo = false;
                    break;
                }
            }
        }

        if (noFbo)
        {
            if (impl_->boundFBO_ != impl_->systemFBO_)
            {
                //BindFramebuffer(impl_->systemFBO_);
                impl_->boundFBO_ = impl_->systemFBO_;
            }

#ifndef GL_ES_VERSION_2_0
            // Disable/enable sRGB write
            if (sRGBWriteSupport_)
            {
                bool sRGBWrite = sRGB_;
                if (sRGBWrite != impl_->sRGBWrite_)
                {
                    //                     if (sRGBWrite)
                    //                         glEnable(GL_FRAMEBUFFER_SRGB_EXT);
                    //                     else
                    //                         glDisable(GL_FRAMEBUFFER_SRGB_EXT);
                    impl_->sRGBWrite_ = sRGBWrite;
                }
            }
#endif
            bgfx::setViewFrameBuffer(current_view_id_, BGFX_INVALID_HANDLE);
        }
        else
        {
            if (renderTargets_[0])
            {
                if (renderTargets_[0]->GetFrameBufferHandle() == bgfx::kInvalidHandle)
                {
                    bgfx::TextureHandle fbt[2];
                    uint8_t count = 1;
                    fbt[0] = bgfx::TextureHandle{renderTargets_[0]->GetParentTexture()->GetGPUObjectHandle()};
                    if (depthStencil_)
                    {
                        count++;
                        fbt[1] = bgfx::TextureHandle{depthStencil_->GetParentTexture()->GetGPUObjectHandle()};
                    }
                    auto framebuffer = bgfx::createFrameBuffer(count, fbt);
                    renderTargets_[0]->SetFrameBufferHandle(framebuffer.idx);
                }
                bgfx::setViewFrameBuffer(current_view_id_,
                                         bgfx::FrameBufferHandle{renderTargets_[0]->GetFrameBufferHandle()});
            }
            else if (depthStencil_)
            {
                if (depthStencil_->GetFrameBufferHandle() == bgfx::kInvalidHandle)
                {
                    auto fbt = bgfx::TextureHandle{depthStencil_->GetParentTexture()->GetGPUObjectHandle()};
                    auto framebuffer = bgfx::createFrameBuffer(1, &fbt);
                    depthStencil_->SetFrameBufferHandle(framebuffer.idx);
                }
                bgfx::setViewFrameBuffer(current_view_id_,
                                         bgfx::FrameBufferHandle{depthStencil_->GetFrameBufferHandle()});
            }

#ifndef GL_ES_VERSION_2_0
            // Disable/enable sRGB write
            if (sRGBWriteSupport_)
            {
                bool sRGBWrite = renderTargets_[0] ? renderTargets_[0]->GetParentTexture()->GetSRGB() : sRGB_;
                if (sRGBWrite != impl_->sRGBWrite_)
                {
                    //                 if (sRGBWrite)
                    //                     glEnable(GL_FRAMEBUFFER_SRGB_EXT);
                    //                 else
                    //                     glDisable(GL_FRAMEBUFFER_SRGB_EXT);
                    impl_->sRGBWrite_ = sRGBWrite;
                }
            }
#endif
        }
    }
}

void Graphics::CleanupFramebuffers()
{
    if (!IsDeviceLost())
    {
        //BindFramebuffer(impl_->systemFBO_);
        impl_->boundFBO_ = impl_->systemFBO_;
        impl_->fboDirty_ = true;

        for (HashMap<unsigned long long, FrameBufferObject>::Iterator i = impl_->frameBuffers_.Begin();
             i != impl_->frameBuffers_.End(); ++i)
            DeleteFramebuffer(i->second_.fbo_);

        if (impl_->resolveSrcFBO_)
            DeleteFramebuffer(impl_->resolveSrcFBO_);
        if (impl_->resolveDestFBO_)
            DeleteFramebuffer(impl_->resolveDestFBO_);
    }
    else
        impl_->boundFBO_ = 0;

    impl_->resolveSrcFBO_ = 0;
    impl_->resolveDestFBO_ = 0;

    impl_->frameBuffers_.Clear();
}

void Graphics::ResetCachedState()
{
    for (auto& vertexBuffer : vertexBuffers_)
        vertexBuffer = nullptr;

    for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
    {
        textures_[i].texture = nullptr;
        textures_[i].flags = UINT32_MAX;
        impl_->textureTypes_[i] = 0;
    }

    for (auto& renderTarget : renderTargets_)
        renderTarget = nullptr;

    depthStencil_ = nullptr;
    viewport_ = IntRect(0, 0, 0, 0);
    indexBuffer_ = nullptr;
    vertexShader_ = nullptr;
    pixelShader_ = nullptr;
    blendMode_ = BLEND_REPLACE;
    alphaToCoverage_ = false;
    colorWrite_ = true;
    cullMode_ = CULL_NONE;
    constantDepthBias_ = 0.0f;
    slopeScaledDepthBias_ = 0.0f;
    depthTestMode_ = CMP_ALWAYS;
    depthWrite_ = false;
    lineAntiAlias_ = false;
    fillMode_ = FILL_SOLID;
    scissorTest_ = false;
    scissorRect_ = IntRect::ZERO;
    stencilTest_ = false;
    stencilTestMode_ = CMP_ALWAYS;
    stencilPass_ = OP_KEEP;
    stencilFail_ = OP_KEEP;
    stencilZFail_ = OP_KEEP;
    stencilRef_ = 0;
    stencilCompareMask_ = M_MAX_UNSIGNED;
    stencilWriteMask_ = M_MAX_UNSIGNED;
    useClipPlane_ = false;
    impl_->shaderProgram_ = nullptr;
    impl_->lastInstanceOffset_ = 0;
    impl_->activeTexture_ = 0;
    impl_->enabledVertexAttributes_ = 0;
    impl_->usedVertexAttributes_ = 0;
    impl_->instancingVertexAttributes_ = 0;
    impl_->boundFBO_ = impl_->systemFBO_;
    impl_->boundVBO_ = 0;
    impl_->boundUBO_ = 0;
    impl_->sRGBWrite_ = false;

    // Set initial state to match Direct3D
    if (impl_->context_)
    {
        //glEnable(GL_DEPTH_TEST);
        SetCullMode(CULL_CCW);
        SetDepthTest(CMP_LESSEQUAL);
        SetDepthWrite(true);
    }

    for (auto& constantBuffer : impl_->constantBuffers_)
        constantBuffer = nullptr;
    impl_->dirtyConstantBuffers_.Clear();
}

void Graphics::SetTextureUnitMappings()
{
    textureUnits_["DiffMap"] = TU_DIFFUSE;
    textureUnits_["DiffCubeMap"] = TU_DIFFUSE;
    textureUnits_["AlbedoBuffer"] = TU_ALBEDOBUFFER;
    textureUnits_["NormalMap"] = TU_NORMAL;
    textureUnits_["NormalBuffer"] = TU_NORMALBUFFER;
    textureUnits_["SpecMap"] = TU_SPECULAR;
    textureUnits_["EmissiveMap"] = TU_EMISSIVE;
    textureUnits_["EnvMap"] = TU_ENVIRONMENT;
    textureUnits_["EnvCubeMap"] = TU_ENVIRONMENT;
    textureUnits_["LightRampMap"] = TU_LIGHTRAMP;
    textureUnits_["LightSpotMap"] = TU_LIGHTSHAPE;
    textureUnits_["LightCubeMap"] = TU_LIGHTSHAPE;
    textureUnits_["ShadowMap"] = TU_SHADOWMAP;
//#ifndef GL_ES_VERSION_2_0
    textureUnits_["VolumeMap"] = TU_VOLUMEMAP;
    textureUnits_["FaceSelectCubeMap"] = TU_FACESELECT;
    textureUnits_["IndirectionCubeMap"] = TU_INDIRECTION;
    textureUnits_["DepthBuffer"] = TU_DEPTHBUFFER;
    textureUnits_["LightBuffer"] = TU_LIGHTBUFFER;
    textureUnits_["ZoneCubeMap"] = TU_ZONE;
    textureUnits_["ZoneVolumeMap"] = TU_ZONE;
//#endif
}

unsigned Graphics::CreateFramebuffer()
{
    unsigned newFbo = 0;
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//         glGenFramebuffersEXT(1, &newFbo);
//     else
// #endif
//         glGenFramebuffers(1, &newFbo);
    return newFbo;
}

void Graphics::DeleteFramebuffer(unsigned fbo)
{
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//         glDeleteFramebuffersEXT(1, &fbo);
//     else
// #endif
//         glDeleteFramebuffers(1, &fbo);
}

void Graphics::BindFramebuffer(unsigned fbo)
{
    return;
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
//     else
// #endif
//         glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Graphics::BindColorAttachment(unsigned index, unsigned target, unsigned object, bool isRenderBuffer)
{
    assert(false);
//     if (!object)
//         isRenderBuffer = false;
// 
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + index, target, object, 0);
//         else
//             glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + index, GL_RENDERBUFFER_EXT, object);
//     }
//     else
// #endif
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, object, 0);
//         else
//             glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, object);
//     }
}

void Graphics::BindDepthAttachment(unsigned object, bool isRenderBuffer)
{
    assert(false);
//     if (!object)
//         isRenderBuffer = false;
// 
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, object, 0);
//         else
//             glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, object);
//     }
//     else
// #endif
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, object, 0);
//         else
//             glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, object);
//     }
}

void Graphics::BindStencilAttachment(unsigned object, bool isRenderBuffer)
{
    assert(false);
//     if (!object)
//         isRenderBuffer = false;
// 
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, object, 0);
//         else
//             glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, object);
//     }
//     else
// #endif
//     {
//         if (!isRenderBuffer)
//             glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, object, 0);
//         else
//             glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, object);
//     }
}

bool Graphics::CheckFramebuffer()
{
    return false;
// #ifndef GL_ES_VERSION_2_0
//     if (!gl3Support)
//         return glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
//     else
// #endif
//         return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Graphics::SetVertexAttribDivisor(unsigned location, unsigned divisor)
{
    assert(false);
// #ifndef GL_ES_VERSION_2_0
//     if (gl3Support && instancingSupport_)
//         glVertexAttribDivisor(location, divisor);
//     else if (instancingSupport_)
//         glVertexAttribDivisorARB(location, divisor);
// #else
// #ifdef __EMSCRIPTEN__
//     if (instancingSupport_)
//         glVertexAttribDivisorANGLE(location, divisor);
// #endif
// #endif
}

void* Graphics::AllocInstanceDataBuffer(uint32_t numInstances, uint16_t instanceStride)
{
    static bgfx::InstanceDataBuffer idb;
    bgfx::allocInstanceDataBuffer(&idb, numInstances, instanceStride);
    return &idb;
}

void Graphics::WriteInstanceData(void* idb, uint32_t& pos, void* data, uint32_t len)
{
    memcpy(((bgfx::InstanceDataBuffer*)idb)->data + pos, data, len);
    pos += len;
}

String Graphics::GetCompiledShaderPath() const
{
    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Noop:
    case bgfx::RendererType::Direct3D9:
        return "compiled/dx9/";
        break;
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12:
        return "compiled/dx11/";
        break;
    case bgfx::RendererType::Gnm:
        return "compiled/pssl/";
        break;
    case bgfx::RendererType::Metal:
        return "compiled/metal/";
        break;
    case bgfx::RendererType::Nvn:
        return "compiled/nvn/";
        break;
    case bgfx::RendererType::OpenGL:
        return "compiled/glsl/";
        break;
    case bgfx::RendererType::OpenGLES:
        return "compiled/essl/";
        break;
    case bgfx::RendererType::Vulkan:
        return "compiled/spirv/";
        break;
    case bgfx::RendererType::WebGPU:
        return "compiled/spirv/";
        break;

    case bgfx::RendererType::Count:
        BX_ASSERT(false, "You should not be here!");
        break;
    }
}

bool IsRendererTypeOpendGL()
{
    return (bgfx::getRendererType() == bgfx::RendererType::OpenGL) || (bgfx::getRendererType() == bgfx::RendererType::OpenGLES);
}

uint64_t bgfxRSBend(Urho3D::BlendMode mode, bool alphaToCoverage)
{
    uint64_t flag = 0;
    switch (mode) {
    case Urho3D::BLEND_ADD:
        flag |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
        //flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_MULTIPLY:
        flag |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_SRC_COLOR, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE);
        //flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_ALPHA:
        flag |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
        //flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_ADDALPHA:
        flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_PREMULALPHA:
        flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_INVDESTALPHA:
        flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_INV_DST_ALPHA, BGFX_STATE_BLEND_DST_ALPHA);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_SUBTRACT:
        //flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
        flag |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE);
        //flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB);
        flag |= BGFX_STATE_BLEND_EQUATION_SEPARATE(BGFX_STATE_BLEND_EQUATION_REVSUB, BGFX_STATE_BLEND_EQUATION_ADD);
        break;
    case Urho3D::BLEND_SUBTRACTALPHA:
        flag |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
        flag |= BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB);
        break;
    default:
        break;
    }
    if (alphaToCoverage)
    {
        flag |= BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
    }
    return flag;
}

uint64_t bgfxRSCull(Urho3D::CullMode mode)
{
    return (mode == Urho3D::CULL_NONE) ? 0 : (((mode == Urho3D::CULL_CCW) ? BGFX_STATE_CULL_CW : BGFX_STATE_CULL_CCW));
}

uint64_t bgfxRSDepthCompare(Urho3D::CompareMode mode)
{
    uint64_t flag = 0;
    switch (mode) {
    case Urho3D::CMP_ALWAYS:
        flag = BGFX_STATE_DEPTH_TEST_ALWAYS;
        break;
    case Urho3D::CMP_EQUAL:
        flag = BGFX_STATE_DEPTH_TEST_EQUAL;
        break;
    case Urho3D::CMP_NOTEQUAL:
        flag = BGFX_STATE_DEPTH_TEST_NOTEQUAL;
        break;
    case Urho3D::CMP_LESS:
        flag = BGFX_STATE_DEPTH_TEST_LESS;
        break;
    case Urho3D::CMP_LESSEQUAL:
        flag = BGFX_STATE_DEPTH_TEST_LEQUAL;
        break;
    case Urho3D::CMP_GREATER:
        flag = BGFX_STATE_DEPTH_TEST_GREATER;
        break;
    case Urho3D::CMP_GREATEREQUAL:
        flag = BGFX_STATE_DEPTH_TEST_GEQUAL;
        break;
    case Urho3D::MAX_COMPAREMODES:
        flag = BGFX_STATE_DEPTH_TEST_NEVER;
    default:
        break;
    }
    return flag;
}

uint64_t bgfxRSStencilFail(Urho3D::StencilOp fail)
{
    switch (fail) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_FAIL_S_KEEP;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_FAIL_S_ZERO;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_FAIL_S_REPLACE;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_FAIL_S_INCR;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_FAIL_S_DECR;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthFail(Urho3D::StencilOp fail)
{
    switch (fail) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_FAIL_Z_KEEP;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_FAIL_Z_ZERO;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_FAIL_Z_REPLACE;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_FAIL_Z_INCR;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_FAIL_Z_DECR;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthPass(Urho3D::StencilOp pass)
{
    switch (pass) {
    case Urho3D::OP_KEEP:
        return BGFX_STENCIL_OP_PASS_Z_KEEP;
    case Urho3D::OP_ZERO:
        return BGFX_STENCIL_OP_PASS_Z_ZERO;
    case Urho3D::OP_REF:
        return BGFX_STENCIL_OP_PASS_Z_REPLACE;
    case Urho3D::OP_INCR:
        return BGFX_STENCIL_OP_PASS_Z_INCR;
    case Urho3D::OP_DECR:
        return BGFX_STENCIL_OP_PASS_Z_DECR;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSStencilCompare(Urho3D::CompareMode mode)
{
    switch (mode) {
    case Urho3D::CMP_ALWAYS:
        return BGFX_STENCIL_TEST_ALWAYS;
    case Urho3D::CMP_EQUAL:
        return BGFX_STENCIL_TEST_EQUAL;
    case Urho3D::CMP_NOTEQUAL:
        return BGFX_STENCIL_TEST_NOTEQUAL;
    case Urho3D::CMP_LESS:
        return BGFX_STENCIL_TEST_LESS;
    case Urho3D::CMP_LESSEQUAL:
        return BGFX_STENCIL_TEST_LEQUAL;
    case Urho3D::CMP_GREATER:
        return BGFX_STENCIL_TEST_GREATER;
    case Urho3D::CMP_GREATEREQUAL:
        return BGFX_STENCIL_TEST_GEQUAL;
    case Urho3D::MAX_COMPAREMODES:
        return BGFX_STENCIL_TEST_NEVER;
    default:
        assert(0);
        break;
    }
}

uint64_t bgfxRSDepthWrite(bool enable)
{
    return enable ? BGFX_STATE_WRITE_Z : 0;
}

uint64_t bgfxRSAlphaToCoverage(bool enable)
{
    return enable ? BGFX_STATE_BLEND_ALPHA_TO_COVERAGE : 0;
}

uint8_t bgfxCubeMapSide(Urho3D::CubeMapFace cubeMapFace)
{
    switch (cubeMapFace) {
    case Urho3D::FACE_POSITIVE_X:
        return BGFX_CUBE_MAP_POSITIVE_X;
    case Urho3D::FACE_NEGATIVE_X:
        return BGFX_CUBE_MAP_NEGATIVE_X;
    case Urho3D::FACE_POSITIVE_Y:
        return BGFX_CUBE_MAP_POSITIVE_Y;
    case Urho3D::FACE_NEGATIVE_Y:
        return BGFX_CUBE_MAP_NEGATIVE_Y;
    case Urho3D::FACE_POSITIVE_Z:
        return BGFX_CUBE_MAP_POSITIVE_Z;
    case Urho3D::FACE_NEGATIVE_Z:
        return BGFX_CUBE_MAP_NEGATIVE_Z;
    default:
        break;
    }
    return 0xff;
}

uint64_t bgfxRSPrimitiveType(Urho3D::PrimitiveType type)
{
    switch (type)
    {
    case Urho3D::TRIANGLE_LIST:
        return 0;
    case Urho3D::LINE_LIST:
        return BGFX_STATE_PT_LINES;
    case Urho3D::POINT_LIST:
        return BGFX_STATE_PT_POINTS;
    case Urho3D::TRIANGLE_STRIP:
        return BGFX_STATE_PT_TRISTRIP;
    case Urho3D::LINE_STRIP:
        return BGFX_STATE_PT_LINESTRIP;
//     case Urho3D::TRIANGLE_FAN:
//         return BGFX_STATE_PT_TRISTRIP;
    default:
        break;
    }
    return 0;
}

uint32_t bgfxAddressMode(Urho3D::TextureCoordinate coord, Urho3D::TextureAddressMode mode)
{
    if (coord == COORD_U) {
        if (mode == ADDRESS_MIRROR) {
            return BGFX_SAMPLER_U_MIRROR;
        } else if (mode == ADDRESS_CLAMP) {
            return BGFX_SAMPLER_U_CLAMP;
        } else if (mode == ADDRESS_BORDER) {
            return BGFX_SAMPLER_U_BORDER;
        }
    } else if (coord == COORD_V) {
        if (mode == ADDRESS_MIRROR) {
            return BGFX_SAMPLER_V_MIRROR;
        } else if (mode == ADDRESS_CLAMP) {
            return BGFX_SAMPLER_V_CLAMP;
        } else if (mode == ADDRESS_BORDER) {
            return BGFX_SAMPLER_V_BORDER;
        }
    } else if (coord == COORD_W) {
        if (mode == ADDRESS_MIRROR) {
            return BGFX_SAMPLER_W_MIRROR;
        } else if (mode == ADDRESS_CLAMP) {
            return BGFX_SAMPLER_W_CLAMP;
        } else if (mode == ADDRESS_BORDER) {
            return BGFX_SAMPLER_W_BORDER;
        }
    }
    return 0;
}

uint32_t bgfxFilterMode(Urho3D::TextureFilterMode filter, int32_t levels)
{
    uint32_t flags = 0;
    switch (filter) {
    case FILTER_NEAREST:
        flags |= BGFX_SAMPLER_MIN_POINT;
        flags |= BGFX_SAMPLER_MAG_POINT;
        if (levels > 1) {
            flags |= BGFX_SAMPLER_MIP_POINT;
        }
        break;
    case FILTER_BILINEAR:
        if (levels > 1) {
            flags = BGFX_SAMPLER_MIP_POINT;
        }
        break;
    case FILTER_ANISOTROPIC:
    case FILTER_TRILINEAR:
        flags |= BGFX_SAMPLER_MIN_ANISOTROPIC;
        flags |= BGFX_SAMPLER_MAG_ANISOTROPIC;
        break;
    case FILTER_NEAREST_ANISOTROPIC:
        flags |= BGFX_SAMPLER_MIN_POINT;
        flags |= BGFX_SAMPLER_MAG_POINT;
        break;
    default:
        break;
    }
    return flags;
}
}
