#include "EditorCamera.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Input/Input.h"

using namespace Urho3D;


class CameraSmoothInterpolate
{
public:
	Camera* camera{ nullptr };
	Node* cameraLookAtNode{ nullptr };
	Node* cameraNode{ nullptr };

	Vector3 lookAtNodeBeginPos;
	Vector3 cameraNodeBeginPos;

	Vector3 lookAtNodeEndPos;
	Vector3 cameraNodeEndPos;

	Quaternion cameraNodeBeginRot;
	Quaternion cameraNodeEndRot;

	float cameraBeginZoom;
	float cameraEndZoom;

	bool isRunning = false;
	float duration = 0.0f;
	float elapsedTime = 0.0f;

	bool interpLookAtNodePos = false;
	bool interpCameraNodePos = false;
	bool interpCameraRot = false;
	bool interpCameraZoom = false;

	CameraSmoothInterpolate()
	{
	}

	void SetLookAtNodePosition(Vector3 lookAtBeginPos, Vector3 lookAtEndPos);
	void SetCameraNodePosition(Vector3 cameraBeginPos, Vector3 cameraEndPos);
	void SetCameraNodeRotation(Quaternion cameraBeginRot, Quaternion cameraEndRot);
	void SetCameraZoom(float beginZoom, float endZoom);
	void Start(float duration_);
	void Stop();
	void Finish();
	bool IsRunning() const { return isRunning; }
	// Cubic easing out
	// http://robertpenner.com/easing/
	float EaseOut(float t, float b, float c, float d)
	{
		return c * ((t = t / d - 1) * t * t + 1) + b;
	}

	void Update(float timeStep);
};

void CameraSmoothInterpolate::SetLookAtNodePosition(Vector3 lookAtBeginPos, Vector3 lookAtEndPos)
{
	lookAtNodeBeginPos = lookAtBeginPos;
	lookAtNodeEndPos = lookAtEndPos;
	interpLookAtNodePos = true;
}

void CameraSmoothInterpolate::SetCameraNodePosition(Vector3 cameraBeginPos, Vector3 cameraEndPos)
{
	cameraNodeBeginPos = cameraBeginPos;
	cameraNodeEndPos = cameraEndPos;
	interpCameraNodePos = true;
}

void CameraSmoothInterpolate::SetCameraNodeRotation(Quaternion cameraBeginRot, Quaternion cameraEndRot)
{
	cameraNodeBeginRot = cameraBeginRot;
	cameraNodeEndRot = cameraEndRot;
	interpCameraRot = true;
}

void CameraSmoothInterpolate::SetCameraZoom(float beginZoom, float endZoom)
{
	cameraBeginZoom = beginZoom;
	cameraEndZoom = endZoom;
	interpCameraZoom = true;
}

void CameraSmoothInterpolate::Start(float duration_)
{
	if (!cameraLookAtNode || !cameraNode || !camera)
		return;

	duration = duration_;
	elapsedTime = 0.0f;
	isRunning = true;
}

void CameraSmoothInterpolate::Stop()
{
	interpLookAtNodePos = false;
	interpCameraNodePos = false;
	interpCameraRot = false;
	interpCameraZoom = false;

	isRunning = false;
}

void CameraSmoothInterpolate::Finish()
{
	if (!isRunning)
		return;

	if (!cameraLookAtNode || !cameraNode || !camera)
		return;

	if (interpLookAtNodePos)
		cameraLookAtNode->SetWorldPosition(lookAtNodeEndPos);

	if (interpCameraNodePos)
		cameraNode->SetPosition(cameraNodeEndPos);

	if (interpCameraRot)
	{
		cameraNode->SetRotation(cameraNodeEndRot);
		ReacquireCameraYawPitch();
	}

	if (interpCameraZoom)
	{
		orthoCameraZoom = cameraEndZoom;
		camera->SetZoom(cameraEndZoom);
	}

	interpLookAtNodePos = false;
	interpCameraNodePos = false;
	interpCameraRot = false;
	interpCameraZoom = false;

	isRunning = false;
}
void CameraSmoothInterpolate::Update(float timeStep)
{
	if (!isRunning)
		return;

	if (!cameraLookAtNode || !cameraNode || !camera)
		return;

	elapsedTime += timeStep;

	if (elapsedTime <= duration)
	{
		float factor = EaseOut(elapsedTime, 0.0f, 1.0f, duration);

		if (interpLookAtNodePos)
			cameraLookAtNode->SetWorldPosition(lookAtNodeBeginPos + (lookAtNodeEndPos - lookAtNodeBeginPos) * factor);

		if (interpCameraNodePos)
			cameraNode->SetPosition(cameraNodeBeginPos + (cameraNodeEndPos - cameraNodeBeginPos) * factor);

		if (interpCameraRot)
		{
			cameraNode->SetRotation(cameraNodeBeginRot.Slerp(cameraNodeEndRot, factor));
			ReacquireCameraYawPitch();
		}

		if (interpCameraZoom)
		{
			orthoCameraZoom = cameraBeginZoom + (cameraEndZoom - cameraBeginZoom) * factor;
			camera->SetZoom(orthoCameraZoom);
		}
	}
	else
	{
		Finish();
	}
}

EditorCamera::EditorCamera(Context* context)
	: LogicComponent(context)
{
	SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);
}

void EditorCamera::RegisterObject(Urho3D::Context* context)
{
	context->RegisterFactory<EditorCamera>();
}

void EditorCamera::Start()
{
	cameraLookAtNode = GetNode()->CreateChild("cameraLookAtNode");
	cameraNode = cameraLookAtNode->CreateChild("cameraNode");
	camera = cameraNode->CreateComponent<Camera>();
	camera->SetFillMode(fillMode);
	camera->SetViewMask(0xffffffff);
	orthoCameraZoom = camera->GetZoom();
	cameraSmoothInterpolate = std::make_unique<CameraSmoothInterpolate>();
}

void EditorCamera::Update(float timeStep)
{

}

void EditorCamera::FixedUpdate(float timeStep)
{

}
void EditorCamera::ResetCamera()
{
	cameraSmoothInterpolate->Stop();

	cameraLookAtNode->SetPosition(Vector3(0, 0, 0));
	cameraLookAtNode->SetRotation(Quaternion());

	cameraNode->SetPosition(Vector3(0, 5, -10));
	// Look at the origin so user can see the scene.
	cameraNode->SetRotation(Quaternion(Vector3(0, 0, 1), -cameraNode->GetPosition()));
	ReacquireCameraYawPitch();
	//UpdateSettingsUI();
}

void EditorCamera::ToggleOrthographic()
{
	SetOrthographic(!camera->IsOrthographic());
}

void EditorCamera::SetOrthographic(bool orthographic)
{
	camera->SetOrthographic(orthographic);
	if (camera->IsOrthographic())
		camera->SetZoom(orthoCameraZoom);
	else
		camera->SetZoom(1.0f);

	//UpdateSettingsUI();
}

void EditorCamera::UpdateEx(float timeStep)
{
	// Update camera smooth move
	if (cameraSmoothInterpolate->IsRunning())
	{
		cameraSmoothInterpolate->Update(timeStep);
	}

// 	Vector3 cameraPos = cameraNode->GetPosition();
// 	String xText(cameraPos.x_);
// 	String yText(cameraPos.y_);
// 	String zText(cameraPos.z_);
// 	xText.Resize(8);
// 	yText.Resize(8);
// 	zText.Resize(8);
// 
// 	cameraPosText->SetText(String(
// 		"Pos: " + xText + " " + yText + " " + zText +
// 		" Zoom: " + std::to_string(camera->GetZoom()).c_str()));
// 
// 	cameraPosText->SetSize(cameraPosText->GetMinSize());
}

void EditorCamera::CameraPan(Vector3 trans)
{
	cameraSmoothInterpolate->Stop();

	cameraLookAtNode->Translate(trans);
}

void EditorCamera::CameraMoveForward(Vector3 trans)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->Translate(trans, TS_PARENT);
}

void EditorCamera::CameraRotateAroundLookAt(Quaternion rot)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->SetRotation(rot);

	Vector3 dir = cameraNode->GetDirection();
	dir.Normalize();

	float dist = cameraNode->GetPosition().Length();

	cameraNode->SetPosition(-dir * dist);
}

void EditorCamera::CameraRotateAroundCenter(Quaternion rot)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->SetRotation(rot);

	Vector3 oldPos = cameraNode->GetWorldPosition();

	Vector3 dir = cameraNode->GetWorldDirection();
	dir.Normalize();

	float dist = cameraNode->GetPosition().Length();

	cameraLookAtNode->SetWorldPosition(cameraNode->GetWorldPosition() + dir * dist);
	cameraNode->SetWorldPosition(oldPos);
}

void EditorCamera::CameraRotateAroundSelect(Quaternion rot)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->SetRotation(rot);

	Vector3 dir = cameraNode->GetDirection();
	dir.Normalize();

	float dist = cameraNode->GetPosition().Length();

	cameraNode->SetPosition(-dir * dist);

	Vector3 centerPoint;
	if ((selectedNodes.length > 0 || selectedComponents.length > 0))
		centerPoint = SelectedNodesCenterPoint();
	else
		centerPoint = lastSelectedNodesCenterPoint;

	// legacy way, camera look-at will jump to the selection
	cameraLookAtNode->SetWorldPosition(centerPoint);
}

void EditorCamera::CameraZoom(float zoom)
{
	cameraSmoothInterpolate->Stop();

	camera->SetZoom(Clamp(zoom, 0.1f, 30.0f));
}

void EditorCamera::HandleStandardUserInput(float timeStep)
{
	// Speedup camera move if Shift key is down
	float speedMultiplier = 1.0;
	if (input->GetKeyDown(KEY_LSHIFT])
		speedMultiplier = cameraShiftSpeedMultiplier;

	// Handle FPS mode
	if (!input->GetKeyDown(KEY_LCTRL] && !input->GetKeyDown(KEY_LALT])
	{
		if (input->GetKeyDown(KEY_W] || input->GetKeyDown(KEY_UP])
		{
			Vector3 dir = cameraNode.direction;
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
		if (input->GetKeyDown(KEY_S] || input->GetKeyDown(KEY_DOWN])
		{
			Vector3 dir = cameraNode.direction;
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
		if (input->GetKeyDown(KEY_A] || input->GetKeyDown(KEY_LEFT])
		{
			Vector3 dir = cameraNode.right;
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
		if (input->GetKeyDown(KEY_D] || input->GetKeyDown(KEY_RIGHT])
		{
			Vector3 dir = cameraNode.right;
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
		if (input->GetKeyDown(KEY_E] || input->GetKeyDown(KEY_PAGEUP])
		{
			Vector3 dir = cameraNode.up;
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
		if (input->GetKeyDown(KEY_Q] || input->GetKeyDown(KEY_PAGEDOWN])
		{
			Vector3 dir = cameraNode.up;
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			FadeUI();
		}
	}

	// Zoom in/out
	if (input.mouseMoveWheel != 0 && ui.GetElementAt(ui.cursor.position) is null)
	{
		float distance = cameraNode.position.length;
		float ratio = distance / 40.0f;
		float factor = ratio < 1.0f ? ratio : 1.0f;

		if (!camera.orthographic)
		{
			Vector3 dir = cameraNode.direction;
			dir.Normalize();
			dir *= input.mouseMoveWheel * 40 * timeStep * cameraBaseSpeed * speedMultiplier * factor;

			CameraMoveForward(dir);
		}
		else
		{
			float zoom = camera.zoom + input.mouseMoveWheel * speedMultiplier * factor;

			CameraZoom(zoom);
		}
	}


	// Rotate/orbit/pan camera
	bool changeCamViewButton = false;

	changeCamViewButton = input.mouseButtonDown[MOUSEB_RIGHT] || input.mouseButtonDown[MOUSEB_MIDDLE];

	if (changeCamViewButton)
	{
		SetMouseLock();

		IntVector2 mouseMove = input.mouseMove;
		if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
		{
			bool panTheCamera = false;

			if (input.mouseButtonDown[MOUSEB_MIDDLE])
			{
				if (mmbPanMode)
					panTheCamera = !input->GetKeyDown(KEY_LSHIFT];
				else
					panTheCamera = input->GetKeyDown(KEY_LSHIFT];
			}

			// Pan the camera
			if (panTheCamera)
			{
				Vector3 right = -cameraNode->GetWorldRight();
				right.Normalize();
				right *= mouseMove.x_;
				Vector3 up = cameraNode->GetWorldUp();
				up.Normalize();
				up *= mouseMove.y_;

				Vector3 trans = (right + up) * timeStep * cameraBaseSpeed * 0.5;

				CameraPan(trans);
			}
			else // Rotate the camera
			{
				activeViewport->cameraYaw += mouseMove.x_ * cameraBaseRotationSpeed;
				activeViewport->cameraPitch += mouseMove.y_ * cameraBaseRotationSpeed;

				if (limitRotation)
					activeViewport->cameraPitch = Clamp(activeViewport->cameraPitch, -90.0f, 90.0f);

				Quaternion rot = Quaternion(activeViewport->cameraPitch, activeViewport->cameraYaw, 0);

				if (input.mouseButtonDown[MOUSEB_MIDDLE]) // Rotate around the camera center
				{
					if (rotateAroundSelect)
						CameraRotateAroundSelect(rot);
					else
						CameraRotateAroundLookAt(rot);

					orbiting = true;
				}
				else // Rotate around the look-at
				{
					CameraRotateAroundCenter(rot);

					orbiting = true;
				}
			}
		}
	}
	else
		ReleaseMouseLock();

	if (orbiting && !input.mouseButtonDown[MOUSEB_MIDDLE])
		orbiting = false;
}

void EditorCamera::HandleBlenderUserInput(float timeStep)
{
// 	if (ui.HasModalElement() || ui.focusElement !is null)
// 	{
// 		ReleaseMouseLock();
// 		return;
// 	}
	Input* input = GetSubsystem<Input>();
	// Check for camara fly mode
	if (input->GetKeyDown(KEY_LSHIFT) && input.keyPress[KEY_F])
	{
		cameraFlyMode = !cameraFlyMode;
	}

	// Speedup camera move if Shift key is down
	float speedMultiplier = 1.0;
	if (input->GetKeyDown(KEY_LSHIFT))
		speedMultiplier = cameraShiftSpeedMultiplier;

	// Handle FPS mode
	if (!input->GetKeyDown(KEY_LCTRL) && !input->GetKeyDown(KEY_LALT))
	{
		if (cameraFlyMode /*&& !input->GetKeyDown(KEY_LSHIFT]*/)
		{
			if (input->GetKeyDown(KEY_W) || input->GetKeyDown(KEY_UP))
			{
				Vector3 dir = cameraNode->GetDirection();
				dir.Normalize();

				CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
			if (input->GetKeyDown(KEY_S) || input->GetKeyDown(KEY_DOWN))
			{
				Vector3 dir = cameraNode->GetDirection();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
			if (input->GetKeyDown(KEY_A) || input->GetKeyDown(KEY_LEFT))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
			if (input->GetKeyDown(KEY_D) || input->GetKeyDown(KEY_RIGHT))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
			if (input->GetKeyDown(KEY_E) || input->GetKeyDown(KEY_PAGEUP))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
			if (input->GetKeyDown(KEY_Q) || input->GetKeyDown(KEY_PAGEDOWN))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				FadeUI();
			}
		}
	}

	if (input.mouseMoveWheel != 0 && ui.GetElementAt(ui.cursor.position) is null)
	{
		if (!camera.orthographic)
		{
			if (input->GetKeyDown(KEY_LSHIFT))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * input.mouseMoveWheel * 5 * timeStep * cameraBaseSpeed * speedMultiplier);
			}
			else if (input->GetKeyDown(KEY_LCTRL))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * input.mouseMoveWheel * 5 * timeStep * cameraBaseSpeed * speedMultiplier);
			}
			else // Zoom in/out
			{
				float distance = cameraNode->GetPosition().Length();
				float ratio = distance / 40.0f;
				float factor = ratio < 1.0f ? ratio : 1.0f;

				Vector3 dir = cameraNode->GetDirection();
				dir.Normalize();
				dir *= input.mouseMoveWheel * 40 * timeStep * cameraBaseSpeed * speedMultiplier * factor;

				CameraMoveForward(dir);
			}
		}
		else
		{
			if (input->GetKeyDown(KEY_LSHIFT))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * input.mouseMoveWheel * timeStep * cameraBaseSpeed * speedMultiplier * 4.0f);
			}
			else if (input->GetKeyDown(KEY_LCTRL))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * input.mouseMoveWheel * timeStep * cameraBaseSpeed * speedMultiplier * 4.0f);
			}
			else
			{
				float zoom = camera->GetZoom() + input.mouseMoveWheel * speedMultiplier * 0.5f;

				CameraZoom(zoom);
			}
		}
	}

	// Rotate/orbit/pan camera
	bool changeCamViewButton = input.mouseButtonDown[MOUSEB_MIDDLE] || cameraFlyMode;

	if (input.mouseButtonPress[MOUSEB_RIGHT] || input->GetKeyDown(KEY_ESCAPE])
		cameraFlyMode = false;

	if (changeCamViewButton)
	{
		SetMouseLock();

		IntVector2 mouseMove = input.mouseMove;
		if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
		{
			bool panTheCamera = false;

			if (!cameraFlyMode)
				panTheCamera = input->GetKeyDown(KEY_LSHIFT);

			if (panTheCamera)
			{
				Vector3 right = -cameraNode->GetWorldRight();
				right.Normalize();
				right *= mouseMove.x_;
				Vector3 up = cameraNode->GetWorldUp();
				up.Normalize();
				up *= mouseMove.y_;

				Vector3 trans = (right + up) * timeStep * cameraBaseSpeed * 0.5;

				CameraPan(trans);
			}
			else
			{
				activeViewport->cameraYaw += mouseMove.x_ * cameraBaseRotationSpeed;
				activeViewport->cameraPitch += mouseMove.y_ * cameraBaseRotationSpeed;

				if (limitRotation)
					activeViewport->cameraPitch = Clamp(activeViewport->cameraPitch, -90.0f, 90.0f);

				Quaternion rot = Quaternion(activeViewport->cameraPitch, activeViewport->cameraYaw, 0);

				if (cameraFlyMode)
				{
					CameraRotateAroundCenter(rot);
					orbiting = true;
				}
				else if (input.mouseButtonDown[MOUSEB_MIDDLE])
				{
					if (rotateAroundSelect)
						CameraRotateAroundSelect(rot);
					else
						CameraRotateAroundLookAt(rot);

					orbiting = true;
				}
			}
		}
	}
	else
		ReleaseMouseLock();

	if (orbiting && !input.mouseButtonDown[MOUSEB_MIDDLE])
		orbiting = false;

	// force to select component node for manipulation if selected only component and not his node
	if ((editMode != EDIT_SELECT && editNodes.empty) && lastSelectedComponent.Get() !is null)
	{
		if (lastSelectedComponent.Get() !is null)
		{
			Component@ component = lastSelectedComponent.Get();
			SelectNode(component.node, false);
		}
	}
}

void EditorCamera::ReacquireCameraYawPitch()
{
	const auto& rotation = cameraNode->GetRotation();
	cameraYaw = rotation.YawAngle();
	cameraPitch = rotation.PitchAngle();
}
