#pragma once
#include <memory>
#include "Urho3D/Container/Ptr.h"
#include "Urho3D/Scene/LogicComponent.h"
#include "Urho3D/Graphics/GraphicsDefs.h"

namespace Urho3D {
	class Camera;
	class Vector3;
	class Quaternion;
}
class CameraSmoothInterpolate;
class EditorCamera : public Urho3D::LogicComponent
{
	URHO3D_OBJECT(EditorCamera, LogicComponent);

public:
	explicit EditorCamera(Urho3D::Context* context);
	static void RegisterObject(Urho3D::Context* context);
	
	void Start() override;
	void Update(float timeStep) override;
	void FixedUpdate(float timeStep) override;
	
	void ResetCamera();
	void ReacquireCameraYawPitch();
	void UpdateEx(float timeStep);
	void SetOrthographic(bool orthographic);
	void ToggleOrthographic();
	
	void CameraPan(const Urho3D::Vector3& trans);
	void CameraMoveForward(const Urho3D::Vector3& trans);
	void CameraRotateAroundLookAt(const Urho3D::Quaternion& rot);
	void CameraRotateAroundCenter(const Urho3D::Quaternion& rot);
	void CameraRotateAroundSelect(const Urho3D::Quaternion& rot);
	void CameraZoom(float zoom);
	void HandleStandardUserInput(float timeStep);
	void HandleBlenderUserInput(float timeStep);
protected:
private:
	void SetMouseMode(bool enable);
	void SetMouseLock();
	void ReleaseMouseLock();
private:
	Urho3D::SharedPtr<Urho3D::Node> cameraLookAtNode;
	Urho3D::SharedPtr<Urho3D::Node> cameraNode;
	Urho3D::SharedPtr<Urho3D::Camera> camera;
	Urho3D::FillMode fillMode = Urho3D::FILL_SOLID;
	float orthoCameraZoom = 1.0f;
	float cameraYaw = 0;
	float cameraPitch = 0;
	std::unique_ptr<CameraSmoothInterpolate> cameraSmoothInterpolate; // Camera smooth interpolation control
	friend class CameraSmoothInterpolate;
};