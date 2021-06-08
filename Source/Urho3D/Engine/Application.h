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

/// \file
/// @nobindfile

#pragma once
#include <memory>
#include "../Core/Context.h"
#include "../Core/Main.h"
#include "../Engine/Engine.h"
#include "../Core/entry/entry.h"

namespace Urho3D
{
template<typename Impl>
class bgfxApp : public entry::AppI
{
public:
    bgfxApp(const char* _name, const char* _description, const char* _url)
        : entry::AppI(_name, _description, _url)
    {
    }

    virtual ~bgfxApp()
    {
    }

    void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
    {
        AppI::init(_argc, _argv, _width, _height);
        argc_ = _argc;
        argv_ = _argv;
    }

    bool update() override
    {
#if defined(_WIN32) && !defined(URHO3D_WIN32_CONSOLE)
        Urho3D::ParseArguments(GetCommandLineW());
#elif defined(__ANDROID__) || defined(IOS) || defined(TVOS)
        Urho3D::ParseArguments(argc_, (char**)argv_);
#endif
        std::shared_ptr<Urho3D::Context> context_ = std::make_shared<Urho3D::Context>();
        std::shared_ptr<Impl> urho3d_app_ = std::make_shared<Impl>(context_.get());
        urho3d_app_->Run();
        return false;
    }
private:
    int32_t argc_;
    const char* const* argv_;
};

class Engine;

/// Base class for creating applications which initialize the Urho3D engine and run a main loop until exited.
class URHO3D_API Application : public Object
{
    URHO3D_OBJECT(Application, Object);

public:
    /// Construct. Parse default engine parameters from the command line, and create the engine in an uninitialized state.
    explicit Application(Context* context);

    /// Setup before engine initialization. This is a chance to eg. modify the engine parameters. Call ErrorExit() to terminate without initializing the engine. Called by Application.
    virtual void Setup() { }

    /// Setup after engine initialization and before running the main loop. Call ErrorExit() to terminate without running the main loop. Called by Application.
    virtual void Start() { }

    /// Cleanup after the main loop. Called by Application.
    virtual void Stop() { }

    /// Initialize the engine and run the main loop, then return the application exit code. Catch out-of-memory exceptions while running.
    int Run();
    /// Show an error message (last log message if empty), terminate the main loop, and set failure exit code.
    void ErrorExit(const String& message = String::EMPTY);

protected:
    /// Handle log message.
    void HandleLogMessage(StringHash eventType, VariantMap& eventData);

    /// Urho3D engine.
    SharedPtr<Engine> engine_;
    /// Engine parameters map.
    VariantMap engineParameters_;
    /// Collected startup error log messages.
    String startupErrors_;
    /// Application exit code.
    int exitCode_;
};

// Macro for defining a main function which creates a Context and the application, then runs it
#if !defined(IOS) && !defined(TVOS)
// #define URHO3D_DEFINE_APPLICATION_MAIN(className) \
// int RunApplication() \
// { \
//     Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context()); \
//     Urho3D::SharedPtr<className> application(new className(context)); \
//     return application->Run(); \
// } \
// URHO3D_DEFINE_MAIN(RunApplication())
#else
// On iOS/tvOS we will let this function exit, so do not hold the context and application in SharedPtr's
// #define URHO3D_DEFINE_APPLICATION_MAIN(className) \
// int RunApplication() \
// { \
//     Urho3D::Context* context = new Urho3D::Context(); \
//     className* application = new className(context); \
//     return application->Run(); \
// } \
// URHO3D_DEFINE_MAIN(RunApplication());
#endif

} // namespace Urho3D

#if defined(__ANDROID__)
#define URHO3D_DEFINE_APPLICATION_MAIN(className, ...)  \
int _main_(int _argc, char** _argv) {                   \
        bgfxApp<className> app(__VA_ARGS__);            \
        return entry::runApp(&app, _argc, _argv);       \
    }                                                   \
extern "C" __attribute__((visibility("default"))) int SDL_main(int argc, char** argv); \
int SDL_main(int _argc, char** _argv) {                 \
	return entry::do_main(_argc, _argv);                \
}
#elif defined(IOS) || defined(TVOS)
#define URHO3D_DEFINE_APPLICATION_MAIN(className, ...)  \
int _main_(int _argc, char** _argv) {                   \
        bgfxApp<className> app(__VA_ARGS__);            \
        return entry::runApp(&app, _argc, _argv);       \
    }
#else
#define URHO3D_DEFINE_APPLICATION_MAIN(className, ...)  \
int _main_(int _argc, char** _argv) {                   \
        bgfxApp<className> app(__VA_ARGS__);            \
        return entry::runApp(&app, _argc, _argv);       \
    }                                                   \
int main(int _argc, char** _argv) {                     \
	return entry::do_main(_argc, _argv);                \
}
#endif
