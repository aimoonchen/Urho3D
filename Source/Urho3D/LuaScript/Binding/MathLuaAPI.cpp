#include <sol/sol.hpp>
#include "../../Math/Vector2.h"
#include "../../Math/Vector3.h"
#include "../../Math/Quaternion.h"
#include "../../Math/Color.h"
#include "../../Math/StringHash.h"
using namespace Urho3D;

int sol2_MathLuaAPI_open(sol::state* lua)
{
	//lua->script("print('sol2_MathLuaAPI_open')");
	lua->new_usertype<StringHash>("StringHash", sol::constructors<StringHash(const char* str)>());
    lua->new_usertype<IntVector2>("IntVector2",
        sol::call_constructor, sol::factories([]() { return IntVector2(); }, [](int x, int y) { return IntVector2(x, y); }),
		"x_", &IntVector2::x_,
		"y_", &IntVector2::y_
        );
    lua->new_usertype<Vector2>("Vector2",
        sol::call_constructor, sol::factories([]() { return Vector2(); }, [](float x, float y) { return Vector2(x, y); }),
		"x_", &Vector2::x_,
		"y_", &Vector2::y_
        );
	lua->new_usertype<Vector3>("Vector3",
		sol::call_constructor, sol::factories([]() { return Vector3(); }, [](float x, float y) { return Vector3(x, y); }, [](float x, float y, float z) { return Vector3(x, y, z); }),
		"Normalize",			&Vector3::Normalize,
		"Length",				&Vector3::Length,
		"LengthSquared",		&Vector3::LengthSquared,
		"DotProduct",			&Vector3::DotProduct,
		"AbsDotProduct",		&Vector3::AbsDotProduct,
		"ProjectOntoAxis",		&Vector3::ProjectOntoAxis,
		"ProjectOntoPlane",		&Vector3::ProjectOntoPlane,
		"ProjectOntoLine",		&Vector3::ProjectOntoLine,
		"DistanceToPoint",		&Vector3::DistanceToPoint,
		"DistanceToPlane",		&Vector3::DistanceToPlane,
		"Orthogonalize",		&Vector3::Orthogonalize,
		"CrossProduct",			&Vector3::CrossProduct,
		"Abs",					&Vector3::Abs,
		"Lerp",					&Vector3::Lerp,
		"Equals",				&Vector3::Equals,
		"Angle",				&Vector3::Angle,
		"IsNaN",				&Vector3::IsNaN,
		"IsInf",				&Vector3::IsInf,
		"Normalized",			&Vector3::Normalized,
		"NormalizedOrDefault",	&Vector3::NormalizedOrDefault,
		"ReNormalized",			&Vector3::ReNormalized,
		//"ToString",				&Vector3::ToString,
		"ToHash",				&Vector3::ToHash,
		"x_",					&Vector3::x_,
		"y_",					&Vector3::y_,
		"z_",					&Vector3::z_);
    lua->new_usertype<Quaternion>("Quaternion",
		sol::call_constructor, sol::factories(
			[]() { return Quaternion(); },
			[](float w, float x, float y, float z) { return Quaternion(w, x, y, z); },
			[](float angle, const Vector3& axis) { return Quaternion(angle, axis); },
			[](float angle) { return Quaternion(angle); },
			[](float ex, float ey, float ez) { return Quaternion(ex, ey, ez); },
			[](const Vector3& angles) { return Quaternion(angles); },
			[](const Vector3& start, const Vector3& end) { return Quaternion(start, end); },
			[](const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis) { return Quaternion(xAxis, yAxis, zAxis); }
		),
		"w_", &Quaternion::w_,
		"x_", &Quaternion::x_,
        "y_", &Quaternion::y_,
		"z_", &Quaternion::z_);
	lua->new_usertype<Color>("Color",
		sol::call_constructor, sol::factories([]() { return Color(); }, [](float r, float g, float b) { return Color(r, g, b); }, [](float r, float g, float b, float a) { return Color(r, g, b, a); }),
		"r_", &Color::r_,
		"g_", &Color::g_,
		"b_", &Color::b_,
		"a_", &Color::a_
		);
	(*lua)["Random"] = sol::overload(
		[]() { return Random(); },
		[](float range) { return Random(range); },
		[](float min, float max) { return Random(min, max); },
		[](int range) { return Random(range); },
		[](int min, int max) { return Random(min, max); }
	);
	(*lua)["RandomNormal"] = RandomNormal;
	return 0;
}