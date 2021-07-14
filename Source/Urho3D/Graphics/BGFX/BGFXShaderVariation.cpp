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
enum shader_target {
    HLSL = 0,
    ESSL,
    GLSL,
    METAL,
    SPIRV,
    MAX_TARGET
};

struct shader_flags {
    const char* VS_FLAGS;
    const char* FS_FLAGS;
    const char* CS_FLAGS;
    const char* SHADER_PATH;
} compile_flags[MAX_TARGET] = {
    {
        " --platform windows -p vs_5_0 -O 3 --type vertex",
        " --platform windows -p ps_5_0 -O 3 --type fragment",
        " --platform windows -p cs_5_0 -O 1 --type compute",
        "Shaders/BGFX/compiled/dx11/"
    },
    {
        " --platform android --type vertex",
        " --platform android --type fragment",
        " --platform android --type compute",
        "Shaders/BGFX/compiled/essl/"
    },
    {
        " --platform linux -p 150 --type vertex",
        " --platform linux -p 150 --type fragment",
        " --platform linux -p 430 --type compute",
        "Shaders/BGFX/compiled/glsl/"
    },
    {
        " --platform osx -p metal -O 3 --type vertex",
        " --platform osx -p metal -O 3 --type fragment",
        " --platform osx -p metal -O 3 --type compute",
        "Shaders/BGFX/compiled/metal/"
    },
    {
        " --platform linux -p spirv --type vertex",
        " --platform linux -p spirv --type fragment",
        " --platform linux -p spirv --type compute",
        "Shaders/BGFX/compiled/spirv/"
    }
};

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
    if (shaderPath.Substring(0, shaderPath.FindLast('/')) == "Shaders/BGFX")
    {
        isUrho3DType_ = true;
    }
    StringHash filenameHash(shaderPath.Substring(13, shaderPath.FindLast('/') - 12) + GetName() +
                           ((type_ == VS) ? "vs" : "fs") + String::Joined(defineVec, " "));
    String binFilename = String(StringHash(filenameHash).Value()) + ".bin";

    auto graphics = owner_->GetContext()->GetSubsystem<Graphics>();
    String fullBinName = "Shaders/BGFX/" + graphics->GetCompiledShaderPath() + binFilename;
    auto cache = owner_->GetContext()->GetSubsystem<ResourceCache>();
    if (!cache->Exists(fullBinName)) {
#if defined(_WIN32)// || defined(__APPLE__)
#ifdef __APPLE__
        String localPath("/Users/simonchen/Development/");
#else
        //String localPath("C:/GitProjects/");
        String localPath("D:/Github/");
#endif
        auto compile_shader = [this, &localPath, &shaderPath, &binFilename, &defineVec](shader_target target) {
            Vector<String> defines;
            defines.Push((type_ == VS) ? "COMPILEVS" : "COMPILEPS");
            // TODO: remove this
            if (target == GLSL || target == ESSL) {
                defines.Push("HOMOGENEOUS_NDC");
            }
            defines.Push("MAXBONES=" + String(Graphics::GetMaxBones()));
            for (const auto& def : defineVec) {
                defines.Push(def);
            }
#ifdef __APPLE__
            String shader_command = localPath + "Urho3D/3rd/bgfx/.build/osx-x64/bin/shadercRelease";
#else
            String shader_command = localPath + "Urho3D/3rd/bgfx/.build/win64_vs2019/bin/shadercRelease.exe";
#endif
            shader_command += (type_ == VS) ? compile_flags[target].VS_FLAGS : compile_flags[target].FS_FLAGS;
            shader_command += " --define " + String::Joined(defines, ";");
            shader_command += " --varyingdef " + localPath + "Urho3D/bin/CoreData/" + shaderPath.Substring(0, shaderPath.FindLast('/')) + "/varying.def.sc";
            shader_command += " -f " + localPath + "Urho3D/bin/CoreData/" + shaderPath;
            shader_command += " -o " + localPath + "Urho3D/bin/CoreData/" + compile_flags[target].SHADER_PATH + binFilename;
            //shader_command += " --debug";
            //shader_command += " --bin2c";
            return system(shader_command.CString());
        };
//         auto ret = compile_shader(HLSL);
//         if (ret != 0)
//         {
//             ;
//         }
        auto ret = compile_shader(GLSL);
        if (ret != 0)
        {
            ;
        }
        ret = compile_shader(METAL);
        if (ret != 0)
        {
            ;
        }
        ret = compile_shader(ESSL);
        if (ret != 0)
        {
            ;
        }
//         auto ret = compile_shader(SPIRV);
//         if (ret != 0)
//         {
//             ;
//         }
#else
        URHO3D_LOGERRORF("Can't found shader file : %s, source file : %s, defines : %s", fullBinName.CString(), shaderPath.CString(), defines_.CString());
        return false;
#endif
    }
    if (cache->Exists(fullBinName)) {
        auto binfile = cache->GetFile(fullBinName);
        auto dataSize = binfile->GetSize();
        const bgfx::Memory* bgfxmem = bgfx::alloc(dataSize);
        if (binfile->Read(bgfxmem->data, dataSize) != dataSize)
        {
            URHO3D_LOGERRORF("Read shader file error : %s", fullBinName.CString());
            return false;
        }
        auto shaderHandle = bgfx::createShader(bgfxmem);
        object_.handle_ = shaderHandle.idx;
        return bgfx::isValid(shaderHandle);
    } else {
        URHO3D_LOGERRORF("Can't found shader file : %s, source file : %s, defines : %s", fullBinName.CString(), shaderPath.CString(), defines_.CString());
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
