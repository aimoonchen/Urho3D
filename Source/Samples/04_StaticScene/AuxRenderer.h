#pragma once
#include <vector>
#include "Urho3D/Math/Color.h"
#include "Urho3D/Math/Frustum.h"
#include "Urho3D/Scene/Component.h"
namespace Urho3D
{
class BoundingBox;
class Camera;
class Polyhedron;
class Drawable;
class Light;
class Matrix3x4;
class Renderer;
class Skeleton;
class Sphere;
class IndexBuffer;
class VertexBuffer;
class Zone;
} // namespace Urho3D
namespace editor
{
/// Debug rendering line.
struct DebugLine
{
    /// Construct undefined.
    DebugLine() = default;

    /// Construct with start and end positions and color.
    DebugLine(const Urho3D::Vector3& start, const Urho3D::Vector3& end, unsigned color)
        : start_(start)
        , end_(end)
        , color_(color)
    {
    }

    /// Start position.
    Urho3D::Vector3 start_;
    /// End position.
    Urho3D::Vector3 end_;
    /// Color.
    unsigned color_{};
};

/// Debug render triangle.
struct DebugTriangle
{
    /// Construct undefined.
    DebugTriangle() = default;

    /// Construct with start and end positions and color.
    DebugTriangle(const Urho3D::Vector3& v1, const Urho3D::Vector3& v2, const Urho3D::Vector3& v3, unsigned color)
        : v1_(v1)
        , v2_(v2)
        , v3_(v3)
        , color_(color)
    {
    }

    /// Vertex a.
    Urho3D::Vector3 v1_;
    /// Vertex b.
    Urho3D::Vector3 v2_;
    /// Vertex c.
    Urho3D::Vector3 v3_;
    /// Color.
    unsigned color_{};
};

struct AuxObjVertex {
    AuxObjVertex(const Urho3D::Vector3& p, const Urho3D::Vector3& n)
        : pos{p}
        , normal{n}
    {
    }
    Urho3D::Vector3 pos;
    Urho3D::Vector3 normal;
};

/// Debug geometry rendering component. Should be added only to the root scene node.
class /*URHO3D_API*/ AuxRenderer : public Urho3D::Component
{
    URHO3D_OBJECT(AuxRenderer, Urho3D::Component);

public:
    /// Construct.
    explicit AuxRenderer(Urho3D::Context* context);
    /// Destruct.
    ~AuxRenderer() override;
    /// Register object factory.
    static void RegisterObject(Urho3D::Context* context);

    /// Set line antialiasing on/off. Default false.
    /// @property
    void SetLineAntiAlias(bool enable);
    /// Set the camera viewpoint. Call before rendering, or before adding geometry if you want to use culling.
    void SetView(Urho3D::Camera* camera);
    /// Add a line.
    void AddLine(const Urho3D::Vector3& start, const Urho3D::Vector3& end, const Urho3D::Color& color,
                 bool depthTest = true);
    /// Add a line with color already converted to unsigned.
    void AddLine(const Urho3D::Vector3& start, const Urho3D::Vector3& end, unsigned color, bool depthTest = true);
    /// Add a solid triangle.
    void AddTriangle(const Urho3D::Vector3& v1, const Urho3D::Vector3& v2, const Urho3D::Vector3& v3,
                     const Urho3D::Color& color,
                     bool depthTest = true);
    /// Add a solid triangle with color already converted to unsigned.
    void AddTriangle(const Urho3D::Vector3& v1, const Urho3D::Vector3& v2, const Urho3D::Vector3& v3, unsigned color, bool depthTest = true);
    /// Add a solid quadrangular polygon.
    void AddPolygon(const Urho3D::Vector3& v1, const Urho3D::Vector3& v2, const Urho3D::Vector3& v3,
                    const Urho3D::Vector3& v4, const Urho3D::Color& color,
                    bool depthTest = true);
    /// Add a solid quadrangular polygon with color already converted to unsigned.
    void AddPolygon(const Urho3D::Vector3& v1, const Urho3D::Vector3& v2, const Urho3D::Vector3& v3, const Urho3D::Vector3& v4, unsigned color,
                    bool depthTest = true);
    /// Add a scene node represented as its coordinate axes.
    void AddNode(Urho3D::Node* node, float scale = 1.0f, bool depthTest = true);
    /// Add a bounding box.
    void AddBoundingBox(const Urho3D::BoundingBox& box, const Urho3D::Color& color, bool depthTest = true,
                        bool solid = false);
    /// Add a bounding box with transform.
    void AddBoundingBox(const Urho3D::BoundingBox& box, const Urho3D::Matrix3x4& transform, const Urho3D::Color& color,
                        bool depthTest = true,
                        bool solid = false);
    /// Add a frustum.
    void AddFrustum(const Urho3D::Frustum& frustum, const Urho3D::Color& color, bool depthTest = true);
    /// Add a polyhedron.
    void AddPolyhedron(const Urho3D::Polyhedron& poly, const Urho3D::Color& color, bool depthTest = true);
    /// Add a sphere.
    void AddSphere(const Urho3D::Sphere& sphere, const Urho3D::Color& color, bool depthTest = true);
    /// Add a sphere sector. Angle ranges from 0 to 360. Identity Quaternion yields the filled portion of the sector
    /// upwards.
    void AddSphereSector(const Urho3D::Sphere& sphere, const Urho3D::Quaternion& rotation, float angle, bool drawLines,
                         const Urho3D::Color& color, bool depthTest = true);
    /// Add a cylinder.
    void AddCylinder(const Urho3D::Vector3& position, float radius, float height, const Urho3D::Color& color,
                     bool depthTest = true);
    /// Add a skeleton.
    void AddSkeleton(const Urho3D::Skeleton& skeleton, const Urho3D::Color& color, bool depthTest = true);
    /// Add a triangle mesh.
    void AddTriangleMesh(const void* vertexData, unsigned vertexSize, const void* indexData, unsigned indexSize,
                         unsigned indexStart, unsigned indexCount, const Urho3D::Matrix3x4& transform,
                         const Urho3D::Color& color,
                         bool depthTest = true);
    /// Add a triangle mesh.
    void AddTriangleMesh(const void* vertexData, unsigned vertexSize, unsigned vertexStart, const void* indexData,
                         unsigned indexSize, unsigned indexStart, unsigned indexCount,
                         const Urho3D::Matrix3x4& transform,
                         const Urho3D::Color& color, bool depthTest = true);
    /// Add a circle.
    void AddCircle(const Urho3D::Vector3& center, const Urho3D::Vector3& normal, float radius,
                   const Urho3D::Color& color, int steps = 64,
                   bool depthTest = true);
    /// Add a cross.
    void AddCross(const Urho3D::Vector3& center, float size, const Urho3D::Color& color, bool depthTest = true);
    /// Add a quad on the XZ plane.
    void AddQuad(const Urho3D::Vector3& center, float width, float height, const Urho3D::Color& color,
                 bool depthTest = true);

    /// Update vertex buffer and render all debug lines. The viewport and rendertarget should be set before.
    void Render();

    /// Return whether line antialiasing is enabled.
    /// @property
    bool GetLineAntiAlias() const { return lineAntiAlias_; }

    /// Return the view transform.
    const Urho3D::Matrix3x4& GetView() const { return view_; }

    /// Return the projection transform.
    const Urho3D::Matrix4& GetProjection() const { return projection_; }

    /// Return the view frustum.
    const Urho3D::Frustum& GetFrustum() const { return frustum_; }

    /// Check whether a bounding box is inside the view frustum.
    bool IsInside(const Urho3D::BoundingBox& box) const;
    /// Return whether has something to render.
    bool HasContent() const;
    
    //
    void SetLight(Urho3D::Node* lightNode);
    Urho3D::Camera* GetCamera() const { return camera_; }
    void SetCamera(Urho3D::Camera* camera);
    void SetRenderer(Urho3D::Renderer* renderer);
    void AddDrawObject(Urho3D::Matrix4 worldMat,
        Urho3D::SharedPtr<Urho3D::VertexBuffer> vertexBuffer,
        Urho3D::SharedPtr<Urho3D::IndexBuffer> indexBuffer,
        Urho3D::Color color,
        bool depthTest = true);
    void AddObject(std::vector<AuxObjVertex> object);
    void AddObject(std::vector<unsigned int> indices, std::vector<AuxObjVertex> object);
private:
    /// Handle end of frame. Clear debug geometry.
    void HandleEndFrame(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleRenderPathEvent(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Lines rendered with depth test.
    Urho3D::PODVector<DebugLine> lines_;
    /// Lines rendered without depth test.
    Urho3D::PODVector<DebugLine> noDepthLines_;
    /// Triangles rendered with depth test.
    Urho3D::PODVector<DebugTriangle> triangles_;
    /// Triangles rendered without depth test.
    Urho3D::PODVector<DebugTriangle> noDepthTriangles_;
    Urho3D::Camera* camera_{ nullptr };
    Urho3D::Node* light_node_{ nullptr };
    Urho3D::Zone* zone_{};
    /// View transform.
    Urho3D::Matrix3x4 view_;
    /// Projection transform.
    Urho3D::Matrix4 projection_;
    /// Projection transform in API-specific format.
    Urho3D::Matrix4 gpuProjection_;
    /// View frustum.
    Urho3D::Frustum frustum_;
    /// Vertex buffer.
    Urho3D::SharedPtr<Urho3D::VertexBuffer> vertexBuffer_;
    struct DrawObject {
		DrawObject(const Urho3D::Matrix4& worldMat,
			Urho3D::SharedPtr<Urho3D::VertexBuffer> vertexBuffer,
			Urho3D::SharedPtr<Urho3D::IndexBuffer> indexBuffer,
			Urho3D::Color color,
			bool depthTest)
            : world_mat_{ worldMat }
            , color_{ color }
            , depth_test_{ depthTest }
            , vertex_buffer_{ vertexBuffer }
            , index_buffer_{ indexBuffer }
        {

        }
        Urho3D::Matrix4 world_mat_;
        Urho3D::Color   color_;
        bool            depth_test_;
		Urho3D::SharedPtr<Urho3D::VertexBuffer> vertex_buffer_;
		Urho3D::SharedPtr<Urho3D::IndexBuffer> index_buffer_;
    };
    std::vector<DrawObject> draw_objects_;
    //
    std::vector<std::vector<AuxObjVertex>> obj_vertex_;
    std::vector<std::vector<unsigned int>> obj_index_;
    Urho3D::SharedPtr<Urho3D::VertexBuffer> obj_vertex_buffer_;
    Urho3D::SharedPtr<Urho3D::IndexBuffer> obj_index_buffer_;
    /// Line antialiasing flag.
    bool lineAntiAlias_;
};
}