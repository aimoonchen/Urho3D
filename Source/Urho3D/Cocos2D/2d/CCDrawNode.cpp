/* Copyright (c) 2012 Scott Lembcke and Howling Moon Software
 * Copyright (c) 2012 cocos2d-x.org
 * Copyright (c) 2013-2016 Chukong Technologies Inc.
 * Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "2d/CCDrawNode.h"
#include "base/CCEventType.h"
#include "base/CCConfiguration.h"
#include "renderer/CCRenderer.h"
// #include "renderer/ccGLStateCache.h"
// #include "renderer/CCGLProgramState.h"
// #include "renderer/CCGLProgramCache.h"
#include "base/CCDirector.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "2d/CCActionCatmullRom.h"
#include "platform/CCGL.h"
#include "Urho3DContext.h"
#include "../../Core/Context.h"
#include "../../Core/Timer.h"
#include "../../Graphics/Graphics.h"
#include "../../Graphics/VertexBuffer.h"
#ifndef M_PI
#define M_PI 3.14159265358
#endif
NS_CC_BEGIN

// Vec2 == CGPoint in 32-bits, but not in 64-bits (OS X)
// that's why the "v2f" functions are needed
static Vec2 v2fzero(0.0f,0.0f);

static inline Vec2 v2f(float x, float y)
{
    Vec2 ret(x, y);
    return ret;
}

static inline Vec2 v2fadd(const Vec2 &v0, const Vec2 &v1)
{
    return v2f(v0.x+v1.x, v0.y+v1.y);
}

static inline Vec2 v2fsub(const Vec2 &v0, const Vec2 &v1)
{
    return v2f(v0.x-v1.x, v0.y-v1.y);
}

static inline Vec2 v2fmult(const Vec2 &v, float s)
{
    return v2f(v.x * s, v.y * s);
}

static inline Vec2 v2fperp(const Vec2 &p0)
{
    return v2f(-p0.y, p0.x);
}

static inline Vec2 v2fneg(const Vec2 &p0)
{
    return v2f(-p0.x, - p0.y);
}

static inline float v2fdot(const Vec2 &p0, const Vec2 &p1)
{
    return  p0.x * p1.x + p0.y * p1.y;
}

static inline Vec2 v2fnormalize(const Vec2 &p)
{
    Vec2 r(p.x, p.y);
    r.normalize();
    return v2f(r.x, r.y);
}

static inline Vec2 __v2f(const Vec2 &v)
{
//#ifdef __LP64__
    return v2f(v.x, v.y);
// #else
//     return * ((Vec2*) &v);
// #endif
}

static inline Tex2F __t(const Vec2 &v)
{
    return *(Tex2F*)&v;
}

// implementation of DrawNode

DrawNode::DrawNode(GLfloat lineWidth)
: _lineWidth(lineWidth)
, _defaultLineWidth(lineWidth)
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
#if CC_ENABLE_CACHE_TEXTURE_DATA
    // Need to listen the event only when not use batchnode, because it will use VBO
    auto listener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
        /** listen the event that renderer was recreated on Android/WP8 */
        this->setupBuffer();
    });

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
    vertexBuffer_ = new Urho3D::VertexBuffer(GetUrho3DContext());
    vertexBufferPoint_ = new Urho3D::VertexBuffer(GetUrho3DContext());
    vertexBufferLine_ = new Urho3D::VertexBuffer(GetUrho3DContext());
}

DrawNode::~DrawNode()
{
    delete vertexBuffer_;
    delete vertexBufferPoint_;
    delete vertexBufferLine_;
    free(_buffer);
    _buffer = nullptr;
//     free(_bufferGLPoint);
//     _bufferGLPoint = nullptr;
//     free(_bufferGLLine);
//     _bufferGLLine = nullptr;
//     
//     glDeleteBuffers(1, &_vbo);
//     glDeleteBuffers(1, &_vboGLLine);
//     glDeleteBuffers(1, &_vboGLPoint);
//     _vbo = 0;
//     _vboGLPoint = 0;
//     _vboGLLine = 0;
//     
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(0);
//         glDeleteVertexArrays(1, &_vao);
//         glDeleteVertexArrays(1, &_vaoGLLine);
//         glDeleteVertexArrays(1, &_vaoGLPoint);
//         _vao = _vaoGLLine = _vaoGLPoint = 0;
//     }
}

DrawNode* DrawNode::create(GLfloat defaultLineWidth)
{
    DrawNode* ret = new (std::nothrow) DrawNode(defaultLineWidth);
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

void DrawNode::ensureCapacity(int count)
{
    CCASSERT(count>=0, "capacity must be >= 0");
    
    if(_bufferCount + count > _bufferCapacity)
    {
        _bufferCapacity += MAX(_bufferCapacity, count);
        _buffer = (V3F_C4B_T2F*)realloc(_buffer, _bufferCapacity*sizeof(V3F_C4B_T2F));
    }
}

void DrawNode::ensureCapacityGLPoint(int count)
{
    CCASSERT(count>=0, "capacity must be >= 0");
    
    if(_bufferCountGLPoint + count > _bufferCapacityGLPoint)
    {
        _bufferCapacityGLPoint += MAX(_bufferCapacityGLPoint, count);
        _bufferGLPoint = (V3F_C4B_T2F*)realloc(_bufferGLPoint, _bufferCapacityGLPoint*sizeof(V3F_C4B_T2F));
    }
}

void DrawNode::ensureCapacityGLLine(int count)
{
    CCASSERT(count>=0, "capacity must be >= 0");
    
    if(_bufferCountGLLine + count > _bufferCapacityGLLine)
    {
        _bufferCapacityGLLine += MAX(_bufferCapacityGLLine, count);
        _bufferGLLine = (V3F_C4B_T2F*)realloc(_bufferGLLine, _bufferCapacityGLLine*sizeof(V3F_C4B_T2F));
    }
}

void DrawNode::setupBuffer()
{
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         glGenVertexArrays(1, &_vao);
//         GL::bindVAO(_vao);
//         glGenBuffers(1, &_vbo);
//         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)* _bufferCapacity, _buffer, GL_STREAM_DRAW);
//         // vertex
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         // color
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         // texcoord
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
// 
//         glGenVertexArrays(1, &_vaoGLLine);
//         GL::bindVAO(_vaoGLLine);
//         glGenBuffers(1, &_vboGLLine);
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
//         // vertex
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         // color
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         // texcoord
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
// 
//         glGenVertexArrays(1, &_vaoGLPoint);
//         GL::bindVAO(_vaoGLPoint);
//         glGenBuffers(1, &_vboGLPoint);
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);
//         // vertex
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         // color
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         // Texture coord as pointsize
//         glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
// 
//         GL::bindVAO(0);
//         glBindBuffer(GL_ARRAY_BUFFER, 0);
// 
//     }
//     else
//     {
//         glGenBuffers(1, &_vbo);
//         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)* _bufferCapacity, _buffer, GL_STREAM_DRAW);
// 
//         glGenBuffers(1, &_vboGLLine);
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
// 
//         glGenBuffers(1, &_vboGLPoint);
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);
// 
//         glBindBuffer(GL_ARRAY_BUFFER, 0);
//     }
// 
//     CHECK_GL_ERROR_DEBUG();
}

bool DrawNode::init()
{
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    //setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR));
    
    ensureCapacity(512);
    ensureCapacityGLPoint(64);
    ensureCapacityGLLine(256);
    
    setupBuffer();
    
    _dirty = true;
    _dirtyGLLine = true;
    _dirtyGLPoint = true; 
    return true;
}

void DrawNode::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    if(_bufferCount)
    {
        _customCommand.init(_globalZOrder, transform, flags);
        _customCommand.func = CC_CALLBACK_0(DrawNode::onDraw, this, transform, flags);
        renderer->addCommand(&_customCommand);
    }
    
    if(_bufferCountGLPoint)
    {
        _customCommandGLPoint.init(_globalZOrder, transform, flags);
        _customCommandGLPoint.func = CC_CALLBACK_0(DrawNode::onDrawGLPoint, this, transform, flags);
        renderer->addCommand(&_customCommandGLPoint);
    }
    
    if(_bufferCountGLLine)
    {
        _customCommandGLLine.init(_globalZOrder, transform, flags);
        _customCommandGLLine.func = CC_CALLBACK_0(DrawNode::onDrawGLLine, this, transform, flags);
        renderer->addCommand(&_customCommandGLLine);
    }
}

void DrawNode::onDraw(const Mat4 &transform, uint32_t /*flags*/)
{
//     getGLProgramState()->apply(transform);
//     auto glProgram = this->getGLProgram();
//     glProgram->setUniformLocationWith1f(glProgram->getUniformLocation("u_alpha"), _displayedOpacity / 255.0);
//     GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirty)
    {
//         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacity, _buffer, GL_STREAM_DRAW);
        unsigned numVertices = _bufferCount; // vertexData.Size() / UI_VERTEX_SIZE;
        if (vertexBuffer_->GetVertexCount() < numVertices || vertexBuffer_->GetVertexCount() > numVertices * 2)
            vertexBuffer_->SetSize(numVertices, Urho3D::MASK_POSITION | Urho3D::MASK_COLOR | Urho3D::MASK_TEXCOORD1,
                                   true);

        vertexBuffer_->SetData(_buffer);
        _dirty = false;
    }

    		// Engine does not render when window is closed or device is lost
    //	assert(graphics_ && graphics_->IsInitialized() && !graphics_->IsDeviceLost());
    auto graphics_ = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>();
    // 	if (batches.Empty())
    // 		return;
    float uiScale_ = 1.0f;
    unsigned alphaFormat = Urho3D::Graphics::GetAlphaFormat();
    Urho3D::RenderSurface* surface = graphics_->GetRenderTarget(0);
    Urho3D::IntVector2 viewSize = graphics_->GetViewport().Size();
    // 	cocos2d::Size design = Director::getInstance()->getOpenGLView()->getDesignResolutionSize();
    // 	Urho3D::IntVector2 viewSize{ (int)design.width, (int)design.height };
    // 	//auto vp = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>()->GetViewport();
    // 	//Urho3D::IntVector2 viewSize{ vp.Width(), vp.Height() };
    Urho3D::Vector2 invScreenSize(1.0f / (float)viewSize.x_, 1.0f / (float)viewSize.y_);
    //     Urho3D::Vector2 scale(2.0f * invScreenSize.x_, -2.0f * invScreenSize.y_);
    //     Urho3D::Vector2 offset(-1.0f, 1.0f);
    Urho3D::Vector2 scale(2.0f * invScreenSize.x_, 2.0f * invScreenSize.y_);
    Urho3D::Vector2 offset(-1.0f, -1.0f);
    if (surface)
    {
#ifdef URHO3D_OPENGL
        // On OpenGL, flip the projection if rendering to a texture so that the texture can be addressed in the
        // same way as a render texture produced on Direct3D.
        offset.y_ = -offset.y_;
        scale.y_ = -scale.y_;
#endif
    }
    const auto& matrixP = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    Mat4 matrixMVP = matrixP * transform;
    matrixMVP.transpose();
    Urho3D::Matrix4 projection(matrixMVP.m);
    // 	Urho3D::Matrix4 projection(Urho3D::Matrix4::IDENTITY);
    // 	projection.m00_ = scale.x_ * uiScale_;
    // 	projection.m03_ = offset.x_;
    // 	projection.m11_ = scale.y_ * uiScale_;
    // 	projection.m13_ = offset.y_;
    // 	projection.m22_ = 1.0f;
    // 	projection.m23_ = 0.0f;
    // 	projection.m33_ = 1.0f;

    graphics_->ClearParameterSources();
    graphics_->SetColorWrite(true);
#ifdef URHO3D_OPENGL
    // Reverse winding if rendering to texture on OpenGL
    if (surface)
        graphics_->SetCullMode(Urho3D::CULL_CW);
    else
#endif
        // graphics_->SetCullMode(Urho3D::CULL_CCW);
        // graphics_->SetCullMode(Urho3D::CULL_CW);
        graphics_->SetCullMode(Urho3D::CULL_NONE);
    graphics_->SetDepthTest(Urho3D::CMP_ALWAYS);
    graphics_->SetDepthWrite(false);
    graphics_->SetFillMode(Urho3D::FILL_SOLID);
    graphics_->SetStencilTest(false);
    graphics_->SetVertexBuffer(vertexBuffer_);
    // graphics_->SetIndexBuffer(indexBuffer_);

    Urho3D::ShaderVariation* noTextureVS = graphics_->GetShader(Urho3D::VS, "Basic", "VERTEXCOLOR");
    Urho3D::ShaderVariation* diffTextureVS = graphics_->GetShader(Urho3D::VS, "Basic", "DIFFMAP VERTEXCOLOR");
    Urho3D::ShaderVariation* noTexturePS = graphics_->GetShader(Urho3D::PS, "Basic", "VERTEXCOLOR");
    Urho3D::ShaderVariation* diffTexturePS = graphics_->GetShader(Urho3D::PS, "Basic", "DIFFMAP VERTEXCOLOR");
    Urho3D::ShaderVariation* diffMaskTexturePS =
        graphics_->GetShader(Urho3D::PS, "Basic", "DIFFMAP ALPHAMASK VERTEXCOLOR");
    Urho3D::ShaderVariation* alphaTexturePS = graphics_->GetShader(Urho3D::PS, "Basic", "ALPHAMAP VERTEXCOLOR");

    // for (unsigned i = batchStart; i < batchEnd; ++i)
    for (int i = 0; i < 1 /*batchesTotal*/; ++i)
    {
        // 		const UIBatch& batch = batches[i];
        // 		if (batch.vertexStart_ == batch.vertexEnd_)
        // 			continue;

        Urho3D::ShaderVariation* ps;
        Urho3D::ShaderVariation* vs;

        auto texture = nullptr;      // _triBatchesToDraw[i].cmd->GetTexture();
        auto blendType = _blendFunc; // _triBatchesToDraw[i].cmd->getBlendType();
        Urho3D::BlendMode blendMode = Urho3D::BLEND_REPLACE;
        if (blendType == BlendFunc::ALPHA_PREMULTIPLIED)
        {
            blendMode = Urho3D::BLEND_PREMULALPHA;
        }
        else if (blendType == BlendFunc::ALPHA_NON_PREMULTIPLIED)
        {
            blendMode = Urho3D::BLEND_ALPHA;
        }
        else if (blendType == BlendFunc::ADDITIVE)
        {
            blendMode = Urho3D::BLEND_ADD;
        }

        if (!texture)
        {
            ps = noTexturePS;
            vs = noTextureVS;
        }
        // 		else
        // 		{
        // 			// If texture contains only an alpha channel, use alpha shader (for fonts)
        // 			vs = diffTextureVS;
        //
        // 			if (texture->GetFormat() == alphaFormat)
        // 				ps = alphaTexturePS;
        // 			// 			else if (batch.blendMode_ != Urho3D::BLEND_ALPHA && batch.blendMode_ != Urho3D::BLEND_ADDALPHA
        // && batch.blendMode_ != Urho3D::BLEND_PREMULALPHA)
        // 			// 				ps = diffMaskTexturePS;
        // 			else
        // 				ps = diffTexturePS;
        // 		}

        graphics_->SetShaders(vs, ps);
        if (graphics_->NeedParameterUpdate(Urho3D::SP_OBJECT, this))
            graphics_->SetShaderParameter(Urho3D::VSP_MODEL, Urho3D::Matrix3x4::IDENTITY);
        if (graphics_->NeedParameterUpdate(Urho3D::SP_CAMERA, this))
            graphics_->SetShaderParameter(Urho3D::VSP_VIEWPROJ, projection);
        if (graphics_->NeedParameterUpdate(Urho3D::SP_MATERIAL, this))
            graphics_->SetShaderParameter(Urho3D::PSP_MATDIFFCOLOR, Urho3D::Color(1.0f, 1.0f, 1.0f, 1.0f));

        float elapsedTime = GetUrho3DContext()->GetSubsystem<Urho3D::Time>()->GetElapsedTime();
        graphics_->SetShaderParameter(Urho3D::VSP_ELAPSEDTIME, elapsedTime);
        graphics_->SetShaderParameter(Urho3D::PSP_ELAPSEDTIME, elapsedTime);

        //         Urho3D::IntRect scissor = batch.scissor_;
        // 		scissor.left_ = (int)(scissor.left_ * uiScale_);
        // 		scissor.top_ = (int)(scissor.top_ * uiScale_);
        // 		scissor.right_ = (int)(scissor.right_ * uiScale_);
        // 		scissor.bottom_ = (int)(scissor.bottom_ * uiScale_);

        // Flip scissor vertically if using OpenGL texture rendering
#ifdef URHO3D_OPENGL
        if (surface)
        {
            // 			int top = scissor.top_;
            // 			int bottom = scissor.bottom_;
            // 			scissor.top_ = viewSize.y_ - bottom;
            // 			scissor.bottom_ = viewSize.y_ - top;
        }
#endif

        graphics_->SetBlendMode(blendMode);
        //		graphics_->SetScissorTest(true, scissor);
        // graphics_->SetTexture(0, texture);
        // 		graphics_->Draw(Urho3D::TRIANGLE_LIST, batch.vertexStart_ / UI_VERTEX_SIZE,
        // 			(batch.vertexEnd_ - batch.vertexStart_) / UI_VERTEX_SIZE);
        graphics_->Draw(Urho3D::TRIANGLE_LIST, 0, _bufferCount);
        // graphics_->Draw(Urho3D::TRIANGLE_LIST, _triBatchesToDraw[i].offset, _triBatchesToDraw[i].indicesToDraw, 0,
        // 0);
    }

    //     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(_vao);
//     }
//     else
//     {
//         GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
// 
//         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
//         // vertex
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         // color
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         // texcoord
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
//     }
// 
//     glDrawArrays(GL_TRIANGLES, 0, _bufferCount);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(0);
//     }
//     
//     CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCount);
//     CHECK_GL_ERROR_DEBUG();
}

void DrawNode::onDrawGLLine(const Mat4 &transform, uint32_t /*flags*/)
{
//     auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR);
//     glProgram->use();
//     glProgram->setUniformsForBuiltins(transform);
//     glProgram->setUniformLocationWith1f(glProgram->getUniformLocation("u_alpha"), _displayedOpacity / 255.0);
// 
//     GL::blendFunc(_blendFunc.src, _blendFunc.dst);

    if (_dirtyGLLine)
    {
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
        unsigned numVertices = _bufferCountGLLine; // vertexData.Size() / UI_VERTEX_SIZE;
        if (vertexBufferLine_->GetVertexCount() < numVertices || vertexBufferLine_->GetVertexCount() > numVertices * 2)
            vertexBufferLine_->SetSize(numVertices, Urho3D::MASK_POSITION | Urho3D::MASK_COLOR | Urho3D::MASK_TEXCOORD1,
                                       true);

        vertexBufferLine_->SetData(_bufferGLLine);
        _dirtyGLLine = false;
    }

    	const auto& matrixP = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    Mat4 matrixMVP = matrixP * transform;
    matrixMVP.transpose();
    Urho3D::Matrix4 projection(matrixMVP.m);
    auto graphics_ = GetUrho3DContext()->GetSubsystem<Urho3D::Graphics>();
    graphics_->ClearParameterSources();
    graphics_->SetColorWrite(true);
#ifdef URHO3D_OPENGL
    // Reverse winding if rendering to texture on OpenGL
    if (false /*surface*/)
        graphics_->SetCullMode(Urho3D::CULL_CW);
    else
#endif
        // graphics_->SetCullMode(Urho3D::CULL_CCW);
        // graphics_->SetCullMode(Urho3D::CULL_CW);
        graphics_->SetCullMode(Urho3D::CULL_NONE);
    graphics_->SetDepthTest(Urho3D::CMP_ALWAYS);
    graphics_->SetDepthWrite(false);
    graphics_->SetFillMode(Urho3D::FILL_SOLID);
    graphics_->SetStencilTest(false);
    graphics_->SetVertexBuffer(vertexBufferLine_);

    auto blendType = _blendFunc; // _triBatchesToDraw[i].cmd->getBlendType();
    Urho3D::BlendMode blendMode = Urho3D::BLEND_REPLACE;
    if (blendType == BlendFunc::ALPHA_PREMULTIPLIED)
    {
        blendMode = Urho3D::BLEND_PREMULALPHA;
    }
    else if (blendType == BlendFunc::ALPHA_NON_PREMULTIPLIED)
    {
        blendMode = Urho3D::BLEND_ALPHA;
    }
    else if (blendType == BlendFunc::ADDITIVE)
    {
        blendMode = Urho3D::BLEND_ADD;
    }

    graphics_->SetShaders(graphics_->GetShader(Urho3D::VS, "Basic", "VERTEXCOLOR"),
                          graphics_->GetShader(Urho3D::PS, "Basic", "VERTEXCOLOR"));
    if (graphics_->NeedParameterUpdate(Urho3D::SP_OBJECT, this))
        graphics_->SetShaderParameter(Urho3D::VSP_MODEL, Urho3D::Matrix3x4::IDENTITY);
    if (graphics_->NeedParameterUpdate(Urho3D::SP_CAMERA, this))
        graphics_->SetShaderParameter(Urho3D::VSP_VIEWPROJ, projection);
    if (graphics_->NeedParameterUpdate(Urho3D::SP_MATERIAL, this))
        graphics_->SetShaderParameter(Urho3D::PSP_MATDIFFCOLOR, Urho3D::Color(1.0f, 1.0f, 1.0f, 1.0f));

    float elapsedTime = GetUrho3DContext()->GetSubsystem<Urho3D::Time>()->GetElapsedTime();
    graphics_->SetShaderParameter(Urho3D::VSP_ELAPSEDTIME, elapsedTime);
    graphics_->SetShaderParameter(Urho3D::PSP_ELAPSEDTIME, elapsedTime);

    //         Urho3D::IntRect scissor = batch.scissor_;
    // 		scissor.left_ = (int)(scissor.left_ * uiScale_);
    // 		scissor.top_ = (int)(scissor.top_ * uiScale_);
    // 		scissor.right_ = (int)(scissor.right_ * uiScale_);
    // 		scissor.bottom_ = (int)(scissor.bottom_ * uiScale_);

    // Flip scissor vertically if using OpenGL texture rendering
#ifdef URHO3D_OPENGL
// 		if (surface)
// 		{
// 			int top = scissor.top_;
// 			int bottom = scissor.bottom_;
// 			scissor.top_ = viewSize.y_ - bottom;
// 			scissor.bottom_ = viewSize.y_ - top;
// 		}
#endif

    graphics_->SetBlendMode(blendMode);
    //		graphics_->SetScissorTest(true, scissor);
    //glLineWidth(_lineWidth);

    graphics_->Draw(Urho3D::LINE_LIST, 0, _bufferCountGLLine);

//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(_vaoGLLine);
//     }
//     else
//     {
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
//         GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
//         // vertex
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         // color
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         // texcoord
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
//     }
// 
//     glLineWidth(_lineWidth);
//     glDrawArrays(GL_LINES, 0, _bufferCountGLLine);
//     
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(0);
//     }
//     
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCountGLLine);
// 
//     CHECK_GL_ERROR_DEBUG();
}

void DrawNode::onDrawGLPoint(const Mat4 &transform, uint32_t /*flags*/)
{
//     auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE);
//     glProgram->use();
//     glProgram->setUniformsForBuiltins(transform);
//     glProgram->setUniformLocationWith1f(glProgram->getUniformLocation("u_alpha"), _displayedOpacity / 255.0);
// 
//     GL::blendFunc(_blendFunc.src, _blendFunc.dst);
// 
//     if (_dirtyGLPoint)
//     {
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);
//         
//         _dirtyGLPoint = false;
//     }
//     
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(_vaoGLPoint);
//     }
//     else
//     {
//         glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
//         GL::enableVertexAttribs( GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, vertices));
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, colors));
//         glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid *)offsetof(V3F_C4B_T2F, texCoords));
//     }
//     
//     glDrawArrays(GL_POINTS, 0, _bufferCountGLPoint);
//     
//     if (Configuration::getInstance()->supportsShareableVAO())
//     {
//         GL::bindVAO(0);
//     }
//     
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     
//     CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCountGLPoint);
//     CHECK_GL_ERROR_DEBUG();
}

void DrawNode::drawPoint(const Vec2& position, const float pointSize, const Color4F &color)
{
    ensureCapacityGLPoint(1);
    
    V3F_C4B_T2F *point = (V3F_C4B_T2F*)(_bufferGLPoint + _bufferCountGLPoint);
    V3F_C4B_T2F a = {{position.x, position.y, 0.0f}, Color4B(color), Tex2F(pointSize, 0)};
    *point = a;
    
    _bufferCountGLPoint += 1;
    _dirtyGLPoint = true;
}

void DrawNode::drawPoints(const Vec2 *position, unsigned int numberOfPoints, const Color4F &color)
{
    drawPoints(position, numberOfPoints, 1.0, color);
}

void DrawNode::drawPoints(const Vec2 *position, unsigned int numberOfPoints, const float pointSize, const Color4F &color)
{
    ensureCapacityGLPoint(numberOfPoints);
    
    V3F_C4B_T2F *point = (V3F_C4B_T2F*)(_bufferGLPoint + _bufferCountGLPoint);
    
    for(unsigned int i=0; i < numberOfPoints; i++,point++)
    {
        V3F_C4B_T2F a = {{position[i].x, position[i].y, 0.0f}, Color4B(color), Tex2F(pointSize, 0)};
        *point = a;
    }
    
    _bufferCountGLPoint += numberOfPoints;
    _dirtyGLPoint = true;
}

void DrawNode::drawLine(const Vec2 &origin, const Vec2 &destination, const Color4F &color)
{
    ensureCapacityGLLine(2);
    
    V3F_C4B_T2F *point = (V3F_C4B_T2F*)(_bufferGLLine + _bufferCountGLLine);
    
    V3F_C4B_T2F a = {{origin.x, origin.y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
    V3F_C4B_T2F b = {{destination.x, destination.y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
    
    *point = a;
    *(point+1) = b;
    
    _bufferCountGLLine += 2;
    _dirtyGLLine = true;
}

void DrawNode::drawRect(const Vec2 &origin, const Vec2 &destination, const Color4F &color)
{
    drawLine(Vec2(origin.x, origin.y), Vec2(destination.x, origin.y), color);
    drawLine(Vec2(destination.x, origin.y), Vec2(destination.x, destination.y), color);
    drawLine(Vec2(destination.x, destination.y), Vec2(origin.x, destination.y), color);
    drawLine(Vec2(origin.x, destination.y), Vec2(origin.x, origin.y), color);
}

void DrawNode::drawPoly(const Vec2 *poli, unsigned int numberOfPoints, bool closePolygon, const Color4F &color)
{
    unsigned int vertex_count;
    if(closePolygon)
    {
        vertex_count = 2 * numberOfPoints;
        ensureCapacityGLLine(vertex_count);
    }
    else
    {
        vertex_count = 2 * (numberOfPoints - 1);
        ensureCapacityGLLine(vertex_count);
    }
    
    V3F_C4B_T2F *point = (V3F_C4B_T2F*)(_bufferGLLine + _bufferCountGLLine);
 
    unsigned int i = 0;
    for(; i<numberOfPoints-1; i++)
    {
        V3F_C4B_T2F a = {{poli[i].x, poli[i].y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
        V3F_C4B_T2F b = {{poli[i + 1].x, poli[i + 1].y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
        
        *point = a;
        *(point+1) = b;
        point += 2;
    }
    if(closePolygon)
    {
        V3F_C4B_T2F a = {{poli[i].x, poli[i].y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
        V3F_C4B_T2F b = {{poli[0].x, poli[0].y, 0.0f}, Color4B(color), Tex2F(0.0, 0.0)};
        *point = a;
        *(point+1) = b;
    }
    
    _bufferCountGLLine += vertex_count;
}

void DrawNode::drawCircle(const Vec2& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY, const Color4F &color)
{
    const float coef = 2.0f * (float)M_PI / segments;
    
    Vec2 *vertices = new (std::nothrow) Vec2[segments+2];
    if( ! vertices )
        return;
    
    for(unsigned int i = 0;i <= segments; i++) {
        float rads = i*coef;
        GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
        GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;
        
        vertices[i].x = j;
        vertices[i].y = k;
    }
    if(drawLineToCenter)
    {
        vertices[segments+1].x = center.x;
        vertices[segments+1].y = center.y;
        drawPoly(vertices, segments+2, true, color);
    }
    else
        drawPoly(vertices, segments+1, true, color);
    
    CC_SAFE_DELETE_ARRAY(vertices);
}

void DrawNode::drawCircle(const Vec2 &center, float radius, float angle, unsigned int segments, bool drawLineToCenter, const Color4F &color)
{
    drawCircle(center, radius, angle, segments, drawLineToCenter, 1.0f, 1.0f, color);
}

void DrawNode::drawQuadBezier(const Vec2 &origin, const Vec2 &control, const Vec2 &destination, unsigned int segments, const Color4F &color)
{
    Vec2* vertices = new (std::nothrow) Vec2[segments + 1];
    if( ! vertices )
        return;
    
    float t = 0.0f;
    for(unsigned int i = 0; i < segments; i++)
    {
        vertices[i].x = powf(1 - t, 2) * origin.x + 2.0f * (1 - t) * t * control.x + t * t * destination.x;
        vertices[i].y = powf(1 - t, 2) * origin.y + 2.0f * (1 - t) * t * control.y + t * t * destination.y;
        t += 1.0f / segments;
    }
    vertices[segments].x = destination.x;
    vertices[segments].y = destination.y;
    
    drawPoly(vertices, segments+1, false, color);

    CC_SAFE_DELETE_ARRAY(vertices);
}

void DrawNode::drawCubicBezier(const Vec2 &origin, const Vec2 &control1, const Vec2 &control2, const Vec2 &destination, unsigned int segments, const Color4F &color)
{
    Vec2* vertices = new (std::nothrow) Vec2[segments + 1];
    if( ! vertices )
        return;
    
    float t = 0;
    for (unsigned int i = 0; i < segments; i++)
    {
        vertices[i].x = powf(1 - t, 3) * origin.x + 3.0f * powf(1 - t, 2) * t * control1.x + 3.0f * (1 - t) * t * t * control2.x + t * t * t * destination.x;
        vertices[i].y = powf(1 - t, 3) * origin.y + 3.0f * powf(1 - t, 2) * t * control1.y + 3.0f * (1 - t) * t * t * control2.y + t * t * t * destination.y;
        t += 1.0f / segments;
    }
    vertices[segments].x = destination.x;
    vertices[segments].y = destination.y;
    
    drawPoly(vertices, segments+1, false, color);

    CC_SAFE_DELETE_ARRAY(vertices);
}

void DrawNode::drawCardinalSpline(PointArray *config, float tension,  unsigned int segments, const Color4F &color)
{
    Vec2* vertices = new (std::nothrow) Vec2[segments + 1];
    if( ! vertices )
        return;
    
    ssize_t p;
    float lt;
    float deltaT = 1.0f / config->count();
    
    for( unsigned int i=0; i < segments+1;i++) {
        
        float dt = (float)i / segments;
        
        // border
        if( dt == 1 ) {
            p = config->count() - 1;
            lt = 1;
        } else {
            p = dt / deltaT;
            lt = (dt - deltaT * (float)p) / deltaT;
        }
        
        // Interpolate
        Vec2 pp0 = config->getControlPointAtIndex(p-1);
        Vec2 pp1 = config->getControlPointAtIndex(p+0);
        Vec2 pp2 = config->getControlPointAtIndex(p+1);
        Vec2 pp3 = config->getControlPointAtIndex(p+2);
        
        Vec2 newPos = ccCardinalSplineAt( pp0, pp1, pp2, pp3, tension, lt);
        vertices[i].x = newPos.x;
        vertices[i].y = newPos.y;
    }
    
    drawPoly(vertices, segments+1, false, color);
    
    CC_SAFE_DELETE_ARRAY(vertices);
}

void DrawNode::drawCatmullRom(PointArray *points, unsigned int segments, const Color4F &color)
{
    drawCardinalSpline( points, 0.5f, segments, color);
}

void DrawNode::drawDot(const Vec2 &pos, float radius, const Color4F &color)
{
    unsigned int vertex_count = 2*3;
    ensureCapacity(vertex_count);
    
    V3F_C4B_T2F a = {Vec3(pos.x - radius, pos.y - radius, 0.0f), Color4B(color), Tex2F(-1.0, -1.0) };
    V3F_C4B_T2F b = {Vec3(pos.x - radius, pos.y + radius, 0.0f), Color4B(color), Tex2F(-1.0, 1.0)};
    V3F_C4B_T2F c = {Vec3(pos.x + radius, pos.y + radius, 0.0f), Color4B(color), Tex2F(1.0, 1.0)};
    V3F_C4B_T2F d = {Vec3(pos.x + radius, pos.y - radius, 0.0f), Color4B(color), Tex2F(1.0, -1.0)};
    
    V3F_C4B_T2F_Triangle *triangles = (V3F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
    V3F_C4B_T2F_Triangle triangle0 = {a, b, c};
    V3F_C4B_T2F_Triangle triangle1 = {a, c, d};
    triangles[0] = triangle0;
    triangles[1] = triangle1;
    
    _bufferCount += vertex_count;
    
    _dirty = true;
}

void DrawNode::drawRect(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2& p4, const Color4F &color)
{
    drawLine(Vec2(p1.x, p1.y), Vec2(p2.x, p2.y), color);
    drawLine(Vec2(p2.x, p2.y), Vec2(p3.x, p3.y), color);
    drawLine(Vec2(p3.x, p3.y), Vec2(p4.x, p4.y), color);
    drawLine(Vec2(p4.x, p4.y), Vec2(p1.x, p1.y), color);
}

void DrawNode::drawSegment(const Vec2 &from, const Vec2 &to, float radius, const Color4F &color)
{
    unsigned int vertex_count = 6*3;
    ensureCapacity(vertex_count);
    
    Vec2 a = __v2f(from);
    Vec2 b = __v2f(to);
    
    
    Vec2 n = v2fnormalize(v2fperp(v2fsub(b, a)));
    Vec2 t = v2fperp(n);
    
    Vec2 nw = v2fmult(n, radius);
    Vec2 tw = v2fmult(t, radius);
    Vec2 v0 = v2fsub(b, v2fadd(nw, tw));
    Vec2 v1 = v2fadd(b, v2fsub(nw, tw));
    Vec2 v2 = v2fsub(b, nw);
    Vec2 v3 = v2fadd(b, nw);
    Vec2 v4 = v2fsub(a, nw);
    Vec2 v5 = v2fadd(a, nw);
    Vec2 v6 = v2fsub(a, v2fsub(nw, tw));
    Vec2 v7 = v2fadd(a, v2fadd(nw, tw));
    
    
    V3F_C4B_T2F_Triangle *triangles = (V3F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
    
    V3F_C4B_T2F_Triangle triangles0 = {
        {{v0.x, v0.y, 0.0f}, Color4B(color), __t(v2fneg(v2fadd(n, t)))},
        {{v1.x, v1.y, 0.0f}, Color4B(color), __t(v2fsub(n, t))},
        {{v2.x, v2.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
    };
    triangles[0] = triangles0;
    
    V3F_C4B_T2F_Triangle triangles1 = {
        {{v3.x, v3.y, 0.0f}, Color4B(color), __t(n)},
        {{v1.x, v1.y, 0.0f}, Color4B(color), __t(v2fsub(n, t))},
        {{v2.x, v2.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
    };
    triangles[1] = triangles1;
    
    V3F_C4B_T2F_Triangle triangles2 = {
        {{v3.x, v3.y, 0.0f}, Color4B(color), __t(n)},
        {{v4.x, v4.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
        {{v2.x, v2.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
    };
    triangles[2] = triangles2;

    V3F_C4B_T2F_Triangle triangles3 = {
        {{v3.x, v3.y, 0.0f}, Color4B(color), __t(n)},
        {{v4.x, v4.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
        {{v5.x, v5.y, 0.0f}, Color4B(color), __t(n)},
    };
    triangles[3] = triangles3;

    V3F_C4B_T2F_Triangle triangles4 = {
        {{v6.x, v6.y, 0.0f}, Color4B(color), __t(v2fsub(t, n))},
        {{v4.x, v4.y, 0.0f}, Color4B(color), __t(v2fneg(n))},
        {{v5.x, v5.y, 0.0f}, Color4B(color), __t(n)},
    };
    triangles[4] = triangles4;

    V3F_C4B_T2F_Triangle triangles5 = {
        {{v6.x, v6.y, 0.0f}, Color4B(color), __t(v2fsub(t, n))},
        {{v7.x, v7.y, 0.0f}, Color4B(color), __t(v2fadd(n, t))},
        {{v5.x, v5.y, 0.0f}, Color4B(color), __t(n)},
    };
    triangles[5] = triangles5;
    
    _bufferCount += vertex_count;
    
    _dirty = true;
}

void DrawNode::drawPolygon(const Vec2 *verts, int count, const Color4F &fillColor, float borderWidth, const Color4F &borderColor)
{
    CCASSERT(count >= 0, "invalid count value");
    
    bool outline = (borderColor.a > 0.0f && borderWidth > 0.0f);
    
    auto  triangle_count = outline ? (3*count - 2) : (count - 2);
    auto vertex_count = 3*triangle_count;
    ensureCapacity(vertex_count);
    
    V3F_C4B_T2F_Triangle *triangles = (V3F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
    V3F_C4B_T2F_Triangle *cursor = triangles;
    
    for (int i = 0; i < count-2; i++)
    {
        V3F_C4B_T2F_Triangle tmp = {
            {{verts[0].x, verts[0].y, 0.0f}, Color4B(fillColor), __t(v2fzero)},
            {{verts[i + 1].x, verts[i + 1].y, 0.0f}, Color4B(fillColor), __t(v2fzero)},
            {{verts[i + 2].x, verts[i + 2].y, 0.0f}, Color4B(fillColor), __t(v2fzero)},
        };
        
        *cursor++ = tmp;
    }
    
    if(outline)
    {
        struct ExtrudeVerts {Vec2 offset, n;};
        struct ExtrudeVerts* extrude = (struct ExtrudeVerts*)malloc(sizeof(struct ExtrudeVerts)*count);
        memset(extrude, 0, sizeof(struct ExtrudeVerts)*count);
        
        for (int i = 0; i < count; i++)
        {
            Vec2 v0 = __v2f(verts[(i-1+count)%count]);
            Vec2 v1 = __v2f(verts[i]);
            Vec2 v2 = __v2f(verts[(i+1)%count]);
            
            Vec2 n1 = v2fnormalize(v2fperp(v2fsub(v1, v0)));
            Vec2 n2 = v2fnormalize(v2fperp(v2fsub(v2, v1)));
            
            Vec2 offset = v2fmult(v2fadd(n1, n2), 1.0f / (v2fdot(n1, n2) + 1.0f));
            struct ExtrudeVerts tmp = {offset, n2};
            extrude[i] = tmp;
        }
        
        for(int i = 0; i < count; i++)
        {
            int j = (i+1)%count;
            Vec2 v0 = __v2f(verts[i]);
            Vec2 v1 = __v2f(verts[j]);
            
            Vec2 n0 = extrude[i].n;
            
            Vec2 offset0 = extrude[i].offset;
            Vec2 offset1 = extrude[j].offset;
            
            Vec2 inner0 = v2fsub(v0, v2fmult(offset0, borderWidth));
            Vec2 inner1 = v2fsub(v1, v2fmult(offset1, borderWidth));
            Vec2 outer0 = v2fadd(v0, v2fmult(offset0, borderWidth));
            Vec2 outer1 = v2fadd(v1, v2fmult(offset1, borderWidth));
            
            V3F_C4B_T2F_Triangle tmp1 = {
                {{inner0.x, inner0.y, 0.0f}, Color4B(borderColor), __t(v2fneg(n0))},
                {{inner1.x, inner1.y, 0.0f}, Color4B(borderColor), __t(v2fneg(n0))},
                {{outer1.x, outer1.y, 0.0f}, Color4B(borderColor), __t(n0)}
            };
            *cursor++ = tmp1;
            
            V3F_C4B_T2F_Triangle tmp2 = {
                {{inner0.x, inner0.y, 0.0f}, Color4B(borderColor), __t(v2fneg(n0))},
                {{outer0.x, outer0.y, 0.0f}, Color4B(borderColor), __t(n0)},
                {{outer1.x, outer1.y, 0.0f}, Color4B(borderColor), __t(n0)}
            };
            *cursor++ = tmp2;
        }
        
        free(extrude);
    }
    
    _bufferCount += vertex_count;
    
    _dirty = true;
}

void DrawNode::drawSolidRect(const Vec2 &origin, const Vec2 &destination, const Color4F &color)
{
    Vec2 vertices[] = {
        origin,
        Vec2(destination.x, origin.y),
        destination,
        Vec2(origin.x, destination.y)
    };
    
    drawSolidPoly(vertices, 4, color );
}

void DrawNode::drawSolidPoly(const Vec2 *poli, unsigned int numberOfPoints, const Color4F &color)
{
    drawPolygon(poli, numberOfPoints, color, 0.0, Color4F(0.0, 0.0, 0.0, 0.0));
}

void DrawNode::drawSolidCircle(const Vec2& center, float radius, float angle, unsigned int segments, float scaleX, float scaleY, const Color4F &color)
{
    const float coef = 2.0f * (float)M_PI / segments;
    
    Vec2 *vertices = new (std::nothrow) Vec2[segments];
    if( ! vertices )
        return;
    
    for(unsigned int i = 0;i < segments; i++)
    {
        float rads = i*coef;
        GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
        GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;
        
        vertices[i].x = j;
        vertices[i].y = k;
    }
    
    drawSolidPoly(vertices, segments, color);
    
    CC_SAFE_DELETE_ARRAY(vertices);
}

void DrawNode::drawSolidCircle( const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
    drawSolidCircle(center, radius, angle, segments, 1.0f, 1.0f, color);
}

void DrawNode::drawTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Color4F &color)
{
    unsigned int vertex_count = 3;
    ensureCapacity(vertex_count);

    Color4B col = Color4B(color);
    V3F_C4B_T2F a = {Vec3(p1.x, p1.y, 0.0f), col, Tex2F(0.0, 0.0) };
    V3F_C4B_T2F b = {Vec3(p2.x, p2.y, 0.0f), col, Tex2F(0.0, 0.0)};
    V3F_C4B_T2F c = {Vec3(p3.x, p3.y, 0.0f), col, Tex2F(0.0, 0.0)};

    V3F_C4B_T2F_Triangle *triangles = (V3F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
    V3F_C4B_T2F_Triangle triangle = {a, b, c};
    triangles[0] = triangle;

    _bufferCount += vertex_count;
    _dirty = true;
}

void DrawNode::drawQuadraticBezier(const Vec2& from, const Vec2& control, const Vec2& to, unsigned int segments, const Color4F &color)
{
    drawQuadBezier(from, control, to, segments, color);
}

void DrawNode::clear()
{
    _bufferCount = 0;
    _dirty = true;
    _bufferCountGLLine = 0;
    _dirtyGLLine = true;
    _bufferCountGLPoint = 0;
    _dirtyGLPoint = true;
    _lineWidth = _defaultLineWidth;
}

const BlendFunc& DrawNode::getBlendFunc() const
{
    return _blendFunc;
}

void DrawNode::setBlendFunc(const BlendFunc &blendFunc)
{
    _blendFunc = blendFunc;
}

void DrawNode::setLineWidth(GLfloat lineWidth)
{
    _lineWidth = lineWidth;
}

GLfloat DrawNode::getLineWidth()
{
    return this->_lineWidth;
}

void DrawNode::visit(Renderer* renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
    if (_isolated)
    {
        //ignore `parentTransform` from parent
        Node::visit(renderer, Mat4::IDENTITY, parentFlags);
    }
    else
    {
        Node::visit(renderer, parentTransform, parentFlags);
    }
}

NS_CC_END
