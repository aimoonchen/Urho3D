#include "EditorCamera.h"
#include "Urho3D/Math/Vector3.h"
#include "Urho3D/Math/Quaternion.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/UI/UI.h"

using namespace Urho3D;

float cameraBaseSpeed = 3.0f;
float cameraBaseRotationSpeed = 0.2f;
float cameraShiftSpeedMultiplier = 5.0f;
float moveStep = 0.5f;
float rotateStep = 5.0f;
float scaleStep = 0.1f;
bool toggledMouseLock = false;
enum MouseOrbitMode
{
	ORBIT_RELATIVE = 0,
	ORBIT_WRAP
};
int mouseOrbitMode = ORBIT_RELATIVE;
bool mmbPanMode = false;
bool rotateAroundSelect = false;
bool orbiting = false;
bool limitRotation = false;
bool cameraFlyMode = true;
Vector3 lastSelectedNodesCenterPoint = Vector3(0.0f, 0.0f, 0.0f);
float orthoCameraZoom = 1.0f;

class CameraSmoothInterpolate
{
public:
	EditorCamera* editorCamera{nullptr};
	Vector3 lookAtNodeBeginPos;
	Vector3 cameraNodeBeginPos;

	Vector3 lookAtNodeEndPos;
	Vector3 cameraNodeEndPos;

	Quaternion cameraNodeBeginRot;
	Quaternion cameraNodeEndRot;

	float cameraBeginZoom = 1.0f;
	float cameraEndZoom = 1.0f;

	bool isRunning = false;
	float duration = 0.0f;
	float elapsedTime = 0.0f;

	bool interpLookAtNodePos = false;
	bool interpCameraNodePos = false;
	bool interpCameraRot = false;
	bool interpCameraZoom = false;

	CameraSmoothInterpolate(EditorCamera* ecamera)
		: editorCamera{ ecamera }
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
	if (!editorCamera)
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

	if (!editorCamera)
		return;

	if (interpLookAtNodePos)
		editorCamera->cameraLookAtNode->SetWorldPosition(lookAtNodeEndPos);

	if (interpCameraNodePos)
		editorCamera->cameraNode->SetPosition(cameraNodeEndPos);

	if (interpCameraRot)
	{
		editorCamera->cameraNode->SetRotation(cameraNodeEndRot);
		editorCamera->ReacquireCameraYawPitch();
	}

	if (interpCameraZoom)
	{
		orthoCameraZoom = cameraEndZoom;
		editorCamera->camera->SetZoom(cameraEndZoom);
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

	if (!editorCamera)
		return;

	elapsedTime += timeStep;

	if (elapsedTime <= duration)
	{
		float factor = EaseOut(elapsedTime, 0.0f, 1.0f, duration);

		if (interpLookAtNodePos)
			editorCamera->cameraLookAtNode->SetWorldPosition(lookAtNodeBeginPos + (lookAtNodeEndPos - lookAtNodeBeginPos) * factor);

		if (interpCameraNodePos)
			editorCamera->cameraNode->SetPosition(cameraNodeBeginPos + (cameraNodeEndPos - cameraNodeBeginPos) * factor);

		if (interpCameraRot)
		{
			editorCamera->cameraNode->SetRotation(cameraNodeBeginRot.Slerp(cameraNodeEndRot, factor));
			editorCamera->ReacquireCameraYawPitch();
		}

		if (interpCameraZoom)
		{
			orthoCameraZoom = cameraBeginZoom + (cameraEndZoom - cameraBeginZoom) * factor;
			editorCamera->camera->SetZoom(orthoCameraZoom);
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
	cameraSmoothInterpolate = std::make_unique<CameraSmoothInterpolate>(this);
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

void EditorCamera::CameraPan(const Vector3& trans)
{
	cameraSmoothInterpolate->Stop();

	cameraLookAtNode->Translate(trans);
}

void EditorCamera::CameraMoveForward(const Vector3& trans)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->Translate(trans, TS_PARENT);
}

void EditorCamera::CameraRotateAroundLookAt(const Quaternion& rot)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->SetRotation(rot);

	Vector3 dir = cameraNode->GetDirection();
	dir.Normalize();

	float dist = cameraNode->GetPosition().Length();

	cameraNode->SetPosition(-dir * dist);
}

void EditorCamera::CameraRotateAroundCenter(const Quaternion& rot)
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

void EditorCamera::CameraRotateAroundSelect(const Quaternion& rot)
{
	cameraSmoothInterpolate->Stop();

	cameraNode->SetRotation(rot);

	Vector3 dir = cameraNode->GetDirection();
	dir.Normalize();

	float dist = cameraNode->GetPosition().Length();

	cameraNode->SetPosition(-dir * dist);

	Vector3 centerPoint;
// 	if ((selectedNodes.length > 0 || selectedComponents.length > 0))
// 		centerPoint = SelectedNodesCenterPoint();
// 	else
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
	auto input = GetSubsystem<Input>();
	if (input->GetKeyDown(KEY_LSHIFT))
		speedMultiplier = cameraShiftSpeedMultiplier;

	// Handle FPS mode
	if (!input->GetKeyDown(KEY_LCTRL) && !input->GetKeyDown(KEY_LALT))
	{
		if (input->GetKeyDown(KEY_W) || input->GetKeyDown(KEY_UP))
		{
			Vector3 dir = cameraNode->GetDirection();
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
		if (input->GetKeyDown(KEY_S) || input->GetKeyDown(KEY_DOWN))
		{
			Vector3 dir = cameraNode->GetDirection();
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
		if (input->GetKeyDown(KEY_A) || input->GetKeyDown(KEY_LEFT))
		{
			Vector3 dir = cameraNode->GetRight();
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
		if (input->GetKeyDown(KEY_D) || input->GetKeyDown(KEY_RIGHT))
		{
			Vector3 dir = cameraNode->GetRight();
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
		if (input->GetKeyDown(KEY_E) || input->GetKeyDown(KEY_PAGEUP))
		{
			Vector3 dir = cameraNode->GetUp();
			dir.Normalize();

			CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
		if (input->GetKeyDown(KEY_Q) || input->GetKeyDown(KEY_PAGEDOWN))
		{
			Vector3 dir = cameraNode->GetUp();
			dir.Normalize();

			CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
			//FadeUI();
		}
	}

	// Zoom in/out
	auto ui = GetSubsystem<UI>();
	auto mouseMoveWheel = input->GetMouseMoveWheel();
	if (mouseMoveWheel != 0 && ui->GetElementAt(ui->GetCursor()->GetPosition(), true))
	{
		float distance = cameraNode->GetPosition().Length();
		float ratio = distance / 40.0f;
		float factor = ratio < 1.0f ? ratio : 1.0f;

		if (!camera->IsOrthographic())
		{
			Vector3 dir = cameraNode->GetDirection();
			dir.Normalize();
			dir *= mouseMoveWheel * 40 * timeStep * cameraBaseSpeed * speedMultiplier * factor;

			CameraMoveForward(dir);
		}
		else
		{
			float zoom = camera->GetZoom() + mouseMoveWheel * speedMultiplier * factor;

			CameraZoom(zoom);
		}
	}


	// Rotate/orbit/pan camera
	bool changeCamViewButton = false;

	changeCamViewButton = input->GetMouseButtonDown(MOUSEB_RIGHT) || input->GetMouseButtonDown(MOUSEB_MIDDLE);

	if (changeCamViewButton)
	{
		SetMouseLock();

		IntVector2 mouseMove = input->GetMouseMove();
		if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
		{
			bool panTheCamera = false;

			if (input->GetMouseButtonDown(MOUSEB_MIDDLE))
			{
				if (mmbPanMode)
					panTheCamera = !input->GetKeyDown(KEY_LSHIFT);
				else
					panTheCamera = input->GetKeyDown(KEY_LSHIFT);
			}

			// Pan the camera
			if (panTheCamera)
			{
				Vector3 right = -cameraNode->GetWorldRight();
				right.Normalize();
				right *= (float)mouseMove.x_;
				Vector3 up = cameraNode->GetWorldUp();
				up.Normalize();
				up *= (float)mouseMove.y_;

				Vector3 trans = (right + up) * timeStep * cameraBaseSpeed * 0.5;

				CameraPan(trans);
			}
			else // Rotate the camera
			{
				cameraYaw += mouseMove.x_ * cameraBaseRotationSpeed;
				cameraPitch += mouseMove.y_ * cameraBaseRotationSpeed;

				if (limitRotation)
					cameraPitch = Clamp(cameraPitch, -90.0f, 90.0f);

				Quaternion rot = Quaternion(cameraPitch, cameraYaw, 0);

				if (input->GetMouseButtonDown(MOUSEB_MIDDLE)) // Rotate around the camera center
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

	if (orbiting && !input->GetMouseButtonDown(MOUSEB_MIDDLE))
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
	if (input->GetKeyDown(KEY_LSHIFT) && input->GetKeyPress(KEY_F))
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
				//FadeUI();
			}
			if (input->GetKeyDown(KEY_S) || input->GetKeyDown(KEY_DOWN))
			{
				Vector3 dir = cameraNode->GetDirection();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				//FadeUI();
			}
			if (input->GetKeyDown(KEY_A) || input->GetKeyDown(KEY_LEFT))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				//FadeUI();
			}
			if (input->GetKeyDown(KEY_D) || input->GetKeyDown(KEY_RIGHT))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
				//FadeUI();
			}
			if (input->GetKeyDown(KEY_E) || input->GetKeyDown(KEY_PAGEUP))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * timeStep * cameraBaseSpeed * speedMultiplier);
				//FadeUI();
			}
			if (input->GetKeyDown(KEY_Q) || input->GetKeyDown(KEY_PAGEDOWN))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(-dir * timeStep * cameraBaseSpeed * speedMultiplier);
				//FadeUI();
			}
		}
	}
	auto mouseMoveWheel = input->GetMouseMoveWheel();
	auto ui = GetSubsystem<UI>();
	if (mouseMoveWheel != 0 && ui->GetElementAt(ui->GetCursor()->GetPosition(), true))
	{
		if (!camera->IsOrthographic())
		{
			if (input->GetKeyDown(KEY_LSHIFT))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * (float)mouseMoveWheel * 5.0f * timeStep * cameraBaseSpeed * speedMultiplier);
			}
			else if (input->GetKeyDown(KEY_LCTRL))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * (float)mouseMoveWheel * 5.0f * timeStep * cameraBaseSpeed * speedMultiplier);
			}
			else // Zoom in/out
			{
				float distance = cameraNode->GetPosition().Length();
				float ratio = distance / 40.0f;
				float factor = ratio < 1.0f ? ratio : 1.0f;

				Vector3 dir = cameraNode->GetDirection();
				dir.Normalize();
				dir *= mouseMoveWheel * 40 * timeStep * cameraBaseSpeed * speedMultiplier * factor;

				CameraMoveForward(dir);
			}
		}
		else
		{
			if (input->GetKeyDown(KEY_LSHIFT))
			{
				Vector3 dir = cameraNode->GetUp();
				dir.Normalize();

				CameraPan(dir * (float)mouseMoveWheel * timeStep * cameraBaseSpeed * speedMultiplier * 4.0f);
			}
			else if (input->GetKeyDown(KEY_LCTRL))
			{
				Vector3 dir = cameraNode->GetRight();
				dir.Normalize();

				CameraPan(dir * (float)mouseMoveWheel * timeStep * cameraBaseSpeed * speedMultiplier * 4.0f);
			}
			else
			{
				float zoom = camera->GetZoom() + mouseMoveWheel * speedMultiplier * 0.5f;

				CameraZoom(zoom);
			}
		}
	}

	// Rotate/orbit/pan camera
	bool changeCamViewButton = input->GetMouseButtonDown(MOUSEB_MIDDLE) || cameraFlyMode;

	if (input->GetMouseButtonPress(MOUSEB_RIGHT) || input->GetKeyDown(KEY_ESCAPE))
		cameraFlyMode = false;

	if (changeCamViewButton)
	{
		SetMouseLock();

		IntVector2 mouseMove = input->GetMouseMove();
		if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
		{
			bool panTheCamera = false;

			if (!cameraFlyMode)
				panTheCamera = input->GetKeyDown(KEY_LSHIFT);

			if (panTheCamera)
			{
				Vector3 right = -cameraNode->GetWorldRight();
				right.Normalize();
				right *= (float)mouseMove.x_;
				Vector3 up = cameraNode->GetWorldUp();
				up.Normalize();
				up *= (float)mouseMove.y_;

				Vector3 trans = (right + up) * timeStep * cameraBaseSpeed * 0.5;

				CameraPan(trans);
			}
			else
			{
				cameraYaw += mouseMove.x_ * cameraBaseRotationSpeed;
				cameraPitch += mouseMove.y_ * cameraBaseRotationSpeed;

				if (limitRotation)
					cameraPitch = Clamp(cameraPitch, -90.0f, 90.0f);

				Quaternion rot = Quaternion(cameraPitch, cameraYaw, 0);

				if (cameraFlyMode)
				{
					CameraRotateAroundCenter(rot);
					orbiting = true;
				}
				else if (input->GetMouseButtonDown(MOUSEB_MIDDLE))
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

	if (orbiting && !input->GetMouseButtonDown(MOUSEB_MIDDLE))
		orbiting = false;

	// force to select component node for manipulation if selected only component and not his node
// 	if ((editMode != EDIT_SELECT && editNodes.empty) && lastSelectedComponent.Get() !is null)
// 	{
// 		if (lastSelectedComponent.Get() !is null)
// 		{
// 			Component@ component = lastSelectedComponent.Get();
// 			SelectNode(component.node, false);
// 		}
// 	}
}

void EditorCamera::ReacquireCameraYawPitch()
{
	const auto& rotation = cameraNode->GetRotation();
	cameraYaw = rotation.YawAngle();
	cameraPitch = rotation.PitchAngle();
}


void EditorCamera::SetMouseMode(bool enable)
{
	auto input = GetSubsystem<Input>();
	auto ui = GetSubsystem<UI>();
	if (enable)
	{
		if (mouseOrbitMode == ORBIT_RELATIVE)
		{
			input->SetMouseMode(MM_RELATIVE);
			ui->GetCursor()->SetVisible(false);
		}
		else if (mouseOrbitMode == ORBIT_WRAP)
			input->SetMouseMode(MM_WRAP);
	}
	else
	{
		input->SetMouseMode(MM_ABSOLUTE);
		ui->GetCursor()->SetVisible(true);
	}
}

void EditorCamera::SetMouseLock()
{
	toggledMouseLock = true;
	SetMouseMode(true);
	//FadeUI();
}

void EditorCamera::ReleaseMouseLock()
{
	if (toggledMouseLock)
	{
		toggledMouseLock = false;
		SetMouseMode(false);
	}
}
