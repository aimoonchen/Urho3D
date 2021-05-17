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

//     object_.name_ = glCreateShader(type_ == VS ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
//     if (!object_.name_)
//     {
//         compilerOutput_ = "Could not create shader object";
//         return false;
//     }

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

    const bgfx::Memory* bgfxmem = nullptr;
    
    StringHash definesHash(String::Joined(defineVec, " "));
    const auto& shaderPath = owner_->GetShaderPath();
    String binFilename = shaderPath.Substring(13, shaderPath.FindLast('/') - 12) + GetName() +
                         ((type_ == VS) ? "vs" : "fs") + String(definesHash.Value()) + ".bin";

    auto graphics = owner_->GetContext()->GetSubsystem<Graphics>();
    String fullBinName = "Shaders/BGFX/" + graphics->GetCompiledShaderPath() + binFilename;
    auto cache = owner_->GetContext()->GetSubsystem<ResourceCache>();
    if (cache->Exists(fullBinName)) {
        auto binfile = cache->GetFile(fullBinName);
        auto dataSize = binfile->GetSize();
        bgfxmem = bgfx::alloc(dataSize);
        if (binfile->Read(bgfxmem->data, dataSize) != dataSize) {
            URHO3D_LOGERRORF("Read file error : %s", fullBinName.CString());
            return false;
        }
    } else {
#if _WIN32
        String localPath("C:/GitProjects/");
        //String localPath("D:/Github/");
        String filePath = GetName() + ".sc";
        //
        bgfx::Options options;
        options.inputFilePath = filePath.CString();   // filePath;
        options.outputFilePath = ""; // outFilePath;
        options.shaderType = (type_ == VS) ? 'v':'f'; // bx::toLower(type[0]);

        options.disasm = false; // cmdLine.hasArg('\0', "disasm");
        options.platform = "windows"; // platform;

        options.raw = false; // cmdLine.hasArg('\0', "raw");

        options.profile = "150";//
        options.debugInformation = false; // cmdLine.hasArg('\0', "debug");
        options.avoidFlowControl = false; // cmdLine.hasArg('\0', "avoid-flow-control");
        options.noPreshader = false;      // cmdLine.hasArg('\0', "no-preshader");
        options.partialPrecision = false; // cmdLine.hasArg('\0', "partial-precision");
        options.preferFlowControl = false; // cmdLine.hasArg('\0', "prefer-flow-control");
        options.backwardsCompatibility = false; // cmdLine.hasArg('\0', "backwards-compatibility");
        options.warningsAreErrors = false;      // cmdLine.hasArg('\0', "Werror");
        options.keepIntermediate = false;       // cmdLine.hasArg('\0', "keep-intermediate");
        uint32_t optimization = 3;
        if (true/*cmdLine.hasArg(optimization, 'O')*/)
        {
            options.optimize = true;
            options.optimizationLevel = optimization;
        }
        options.depends = false; // cmdLine.hasArg("depends");
        options.preprocessOnly = false; // cmdLine.hasArg("preprocess");

        options.includeDirs.push_back((localPath + "Urho3D/bin/CoreData/Shaders/BGFX").CString());
        options.defines.push_back((type_ == VS) ? "COMPILEVS" : "COMPILEPS");
        auto maxbone = "MAXBONES=" + String(Graphics::GetMaxBones());
        options.defines.push_back(maxbone.CString());
        
        for (const auto& def : defineVec)
        {
            options.defines.push_back(def.CString());
        }
        const char* varying = NULL;
        bgfx::File attribdef;
        if ('c' != options.shaderType) {
            std::string defaultVarying = (localPath + "Urho3D/bin/CoreData/" +
                                          shaderPath.Substring(0, shaderPath.FindLast('/')) + "/varying.def.sc").CString();
            const char* varyingdef = defaultVarying.c_str(); // cmdLine.findOption("varyingdef", defaultVarying.c_str());
            attribdef.load(varyingdef);
            varying = attribdef.getData();
            if (NULL != varying && *varying != '\0') {
                options.dependencies.push_back(varyingdef);
            } else {
                bx::printf(
                    "ERROR: Failed to parse varying def file: \"%s\" No input/output semantics will be generated in "
                    "the code!\n",
                    varyingdef);
            }
        }
        const size_t padding = 16384;
        uint32_t size = originalShaderCode.Length();// (uint32_t)bx::getSize(&reader);
        char* data = new char[size + padding + 1];
        char* dataMetal = new char[size + padding + 1];
        //size = (uint32_t)bx::read(&reader, data, size);
        memcpy(data, originalShaderCode.CString(), size);

        if (data[0] == '\xef' && data[1] == '\xbb' && data[2] == '\xbf')
        {
            bx::memMove(data, &data[3], size - 3);
            size -= 3;
        }

        // Compiler generates "error X3000: syntax error: unexpected end of file"
        // if input doesn't have empty line at EOF.
        data[size] = '\n';
        bx::memSet(&data[size + 1], 0, padding);
        memcpy(dataMetal, data, size + padding + 1);

        bgfx::memory_writer mwriter;
        auto compiled = compileShader(varying, "" /*commandLineComment.c_str()*/, data, size, options, &mwriter);
        if (!compiled) {
            URHO3D_LOGERRORF("CompileShader(glsl) %s Failed.", filePath.CString());
        }
        bgfxmem = bgfx::copy(&mwriter.memory_[0], mwriter.current_size_);
        
        auto outFilePath0 = localPath + "Urho3D/bin/CoreData/Shaders/BGFX/compiled/glsl/" + binFilename;
        File file0(owner_->GetContext(), outFilePath0, FileMode::FILE_WRITE);
        file0.Write(&mwriter.memory_[0], mwriter.current_size_);
        //
        mwriter.current_size_ = 0;
        options.platform = "osx";
        options.profile = "metal";
        compiled = compileShader(varying, "" /*commandLineComment.c_str()*/, dataMetal, size, options, &mwriter);
        if (!compiled) {
            URHO3D_LOGERRORF("CompileShader(metal) %s Failed", filePath.CString());
        }
        auto outFilePath1 = localPath + "Urho3D/bin/CoreData/Shaders/BGFX/compiled/metal/" + binFilename;
        File file1(owner_->GetContext(), outFilePath1, FileMode::FILE_WRITE);
        file1.Write(&mwriter.memory_[0], mwriter.current_size_);
#else
        URHO3D_LOGERRORF("Can't found file : %s", fullBinName.CString());
        return false;
#endif
    }
    auto shaderHandle = bgfx::createShader(bgfxmem);
    object_.handle_ = shaderHandle.idx;
    return bgfx::isValid(shaderHandle);
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
