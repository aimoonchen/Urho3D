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
#include "../../IO/File.h"
#include "../../IO/FileSystem.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/Shader.h"
#include "../../Graphics/ShaderProgram.h"
#include "../../Graphics/ShaderVariation.h"
#include "../../IO/Log.h"

#include "bgfx/bgfx.h"

#if _WIN32
#include "shaderc/shaderc.h"
#endif

#include "../../DebugNew.h"

namespace Urho3D
{

const char* ShaderVariation::elementSemanticNames[] =
{
    "POS",
    "NORMAL",
    "BINORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "BLENDWEIGHT",
    "BLENDINDICES",
    "OBJECTINDEX"
};

void ShaderVariation::OnDeviceLost()
{

}

void ShaderVariation::Release()
{
    if (object_.handle_ != bgfx::kInvalidHandle)
    {
        if (graphics_)
        {
            if (type_ == VS)
            {
                if (graphics_->GetVertexShader() == this)
                    graphics_->SetShaders(nullptr, nullptr);
            }
            else
            {
                if (graphics_->GetPixelShader() == this)
                    graphics_->SetShaders(nullptr, nullptr);
            }
        }
        bgfx::destroy(bgfx::ShaderHandle{object_.handle_});
        object_.handle_ = bgfx::kInvalidHandle;
        graphics_->CleanupShaderPrograms(this);
    }

    compilerOutput_.Clear();
}

bool ShaderVariation::Create()
{
    Release();

    if (!owner_)
    {
        compilerOutput_ = "Owner shader has expired";
        return false;
    }

    const String& originalShaderCode = owner_->GetSourceCode(type_);
    String shaderCode;
    
    // Prepend the defines to the shader code
    Vector<String> defineVec = defines_.Split(' ');
    for (unsigned i = 0; i < defineVec.Size(); ++i)
    {
        // Add extra space for the checking code below
        String defineString = "#define " + defineVec[i].Replaced('=', ' ') + " \n";
        shaderCode += defineString;

        // In debug mode, check that all defines are referenced by the shader code
#ifdef _DEBUG
        String defineCheck = defineString.Substring(8, defineString.Find(' ', 8) - 8);
        if (originalShaderCode.Find(defineCheck) == String::NPOS)
            URHO3D_LOGWARNING("Shader " + GetFullName() + " does not use the define " + defineCheck);
#endif
    }

#ifdef RPI
    if (type_ == VS)
        shaderCode += "#define RPI\n";
#endif
#ifdef __EMSCRIPTEN__
    shaderCode += "#define WEBGL\n";
#endif

    const auto& shaderPath = owner_->GetShaderPath();
    StringHash filenameHash(shaderPath.Substring(13, shaderPath.FindLast('/') - 12) + GetName() +
                           ((type_ == VS) ? "vs" : "fs") + String::Joined(defineVec, " "));
    String binFilename = String(StringHash(filenameHash).Value()) + ".bin";

    auto graphics = owner_->GetContext()->GetSubsystem<Graphics>();
    String fullBinName = "Shaders/BGFX/" + graphics->GetCompiledShaderPath() + binFilename;
    auto cache = owner_->GetContext()->GetSubsystem<ResourceCache>();
    if (!cache->Exists(fullBinName)) {
#if _WIN32
        String localPath("C:/GitProjects/");
        //String localPath("D:/Github/");
        auto compile_shader = [this, &localPath, &shaderPath, &binFilename, &defineVec](const String& platform) {
            Vector<String> defines;
            defines.Push((type_ == VS) ? "COMPILEVS" : "COMPILEPS");
            defines.Push("MAXBONES=" + String(Graphics::GetMaxBones()));
            for (const auto& def : defineVec)
            {
                defines.Push(def);
            }
            String shader_command = localPath + "Urho3D/bin/CoreData/Shaders/BGFX/shaderc.exe";
            shader_command += " -i " + localPath + "Urho3D/bin/CoreData/Shaders/BGFX";
            shader_command += " -f " + localPath + "Urho3D/bin/CoreData/" + shaderPath;
            shader_command += " --define " + String::Joined(defines, ";");
            String typestr = (type_ == VS) ? "v" : "f";
            shader_command += " --type " + typestr;
            shader_command += " --varyingdef " + localPath + "Urho3D/bin/CoreData/" +
                              shaderPath.Substring(0, shaderPath.FindLast('/')) + "/varying.def.sc";
            if (platform == "osx")
            {
                shader_command += " --platform osx -p metal";
                shader_command += " -o " + localPath + "Urho3D/bin/CoreData/Shaders/BGFX/compiled/metal/" + binFilename;
            }
            else if (platform == "android")
            {
                shader_command += " --platform android";
                shader_command += " -o " + localPath + "Urho3D/bin/CoreData/Shaders/BGFX/compiled/essl/" + binFilename;
            }
            else if (platform == "windows")
            {
                shader_command += " --platform windows -p 150";
                shader_command += " -o " + localPath + "Urho3D/bin/CoreData/Shaders/BGFX/compiled/glsl/" + binFilename;
            }
            return system(shader_command.CString());
        };
        auto ret = compile_shader("windows");
        if (ret != 0)
        {
            ;
        }
        ret = compile_shader("osx");
        if (ret != 0)
        {
            ;
        }
//         ret = compile_shader("android");
//         if (ret != 0)
//         {
//             ;
//         }
#else
        URHO3D_LOGERRORF("Can't found file : %s", fullBinName.CString());
        return false;
#endif
    }
    if (cache->Exists(fullBinName)) {
        auto binfile = cache->GetFile(fullBinName);
        auto dataSize = binfile->GetSize();
        const bgfx::Memory* bgfxmem = bgfx::alloc(dataSize);
        if (binfile->Read(bgfxmem->data, dataSize) != dataSize)
        {
            URHO3D_LOGERRORF("Read file error : %s", fullBinName.CString());
            return false;
        }
        auto shaderHandle = bgfx::createShader(bgfxmem);
        object_.handle_ = shaderHandle.idx;
        return bgfx::isValid(shaderHandle);
    } else {
        URHO3D_LOGERRORF("Can't found file : %s", fullBinName.CString());
        return false;
    }
}

void ShaderVariation::SetDefines(const String& defines)
{
    defines_ = defines;
}

// These methods are no-ops for bgfx
bool ShaderVariation::LoadByteCode(const String& binaryShaderName) { return false; }
bool ShaderVariation::Compile() { return false; }
void ShaderVariation::ParseParameters(unsigned char* bufData, unsigned bufSize) {}
void ShaderVariation::SaveByteCode(const String& binaryShaderName) {}
void ShaderVariation::CalculateConstantBufferSizes() {}

}
