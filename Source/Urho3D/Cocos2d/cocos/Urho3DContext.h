#pragma once
#include "Urho3D.h"
namespace Urho3D {
	class Context;
}
URHO3D_API void SetUrho3DContext(Urho3D::Context* context);
URHO3D_API Urho3D::Context* GetUrho3DContext();
