#pragma once
#include <memory>
#include "Urho3D/Scene/LogicComponent.h"
#include "Urho3D/Graphics/GraphicsDefs.h"

namespace Urho3D {
	class Camera;
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
	
	void CameraPan(Vector3 trans);
	void CameraMoveForward(Vector3 trans);
	void CameraRotateAroundLookAt(Quaternion rot);
	void CameraRotateAroundCenter(Quaternion rot);
	void CameraRotateAroundSelect(Quaternion rot);
	void CameraZoom(float zoom);
	void HandleStandardUserInput(float timeStep);
	void HandleBlenderUserInput(float timeStep);
protected:
private:
	SharedPtr<Camera> camera;
	SharedPtr<Node> cameraLookAtNode;
	SharedPtr<Node> cameraNode;
	FillMode fillMode = FILL_SOLID;
	float orthoCameraZoom = 1.0f;
	float cameraYaw = 0;
	float cameraPitch = 0;
	std::unique_ptr<CameraSmoothInterpolate> cameraSmoothInterpolate; // Camera smooth interpolation control
};