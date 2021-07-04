/**
 Copyright 2013 BlackBerry Inc.
 Copyright (c) 2014-2017 Chukong Technologies
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Original file from GamePlay3D: http://gameplay3d.org

 This file was modified to fit the cocos2d-x project
 */

#ifndef QUATERNION_H_
#define QUATERNION_H_

#include "math/Vec3.h"
#include "math/Mat4.h"
//#include "Plane.h"

/**
 * @addtogroup base
 * @{
 */

NS_CC_MATH_BEGIN

class Mat4;

/**
 * Defines a 4-element quaternion that represents the orientation of an object in space.
 *
 * Quaternions are typically used as a replacement for euler angles and rotation matrices as a way to achieve smooth interpolation and avoid gimbal lock.
 *
 * Note that this quaternion class does not automatically keep the quaternion normalized. Therefore, care must be taken to normalize the quaternion when necessary, by calling the normalize method.
 * This class provides three methods for doing quaternion interpolation: lerp, slerp, and squad.
 *
 * lerp (linear interpolation): the interpolation curve gives a straight line in quaternion space. It is simple and fast to compute. The only problem is that it does not provide constant angular velocity. Note that a constant velocity is not necessarily a requirement for a curve;
 * slerp (spherical linear interpolation): the interpolation curve forms a great arc on the quaternion unit sphere. Slerp provides constant angular velocity;
 * squad (spherical spline interpolation): interpolating between a series of rotations using slerp leads to the following problems:
 * - the curve is not smooth at the control points;
 * - the angular velocity is not constant;
 * - the angular velocity is not continuous at the control points.
 *
 * Since squad is continuously differentiable, it remedies the first and third problems mentioned above.
 * The slerp method provided here is intended for interpolation of principal rotations. It treats +q and -q as the same principal rotation and is at liberty to use the negative of either input. The resulting path is always the shorter arc.
 *
 * The lerp method provided here interpolates strictly in quaternion space. Note that the resulting path may pass through the origin if interpolating between a quaternion and its exact negative.
 *
 * As an example, consider the following quaternions:
 *
 * q1 = (0.6, 0.8, 0.0, 0.0),
 * q2 = (0.0, 0.6, 0.8, 0.0),
 * q3 = (0.6, 0.0, 0.8, 0.0), and
 * q4 = (-0.8, 0.0, -0.6, 0.0).
 * For the point p = (1.0, 1.0, 1.0), the following figures show the trajectories of p using lerp, slerp, and squad.
 */
class CC_DLL Quat
{
    friend class Curve;
    friend class Transform;

public:

    /**
     * The x-value of the quaternion's vector component.
     */
    float x;
    /**
     * The y-value of the quaternion's vector component.
     */
    float y;
    /**
     * The z-value of the quaternion's vector component.
     */
    float z;
    /**
     * The scalar component of the quaternion.
     */
    float w;

    /**
     * Constructs a quaternion initialized to (0, 0, 0, 1).
     */
    Quat();

    /**
     * Constructs a quaternion initialized to (0, 0, 0, 1).
     *
     * @param xx The x component of the quaternion.
     * @param yy The y component of the quaternion.
     * @param zz The z component of the quaternion.
     * @param ww The w component of the quaternion.
     */
    Quat(float xx, float yy, float zz, float ww);

    /**
     * Constructs a new quaternion from the values in the specified array.
     *
     * @param array The values for the new quaternion.
     */
    Quat(float* array);

    /**
     * Constructs a quaternion equal to the rotational part of the specified matrix.
     *
     * @param m The matrix.
     */
    Quat(const Mat4& m);

    /**
     * Constructs a quaternion equal to the rotation from the specified axis and angle.
     *
     * @param axis A vector describing the axis of rotation.
     * @param angle The angle of rotation (in radians).
     */
    Quat(const Vec3& axis, float angle);

    /**
     * Constructs a new quaternion that is a copy of the specified one.
     *
     * @param copy The quaternion to copy.
     */
    Quat(const Quat& copy);

    /**
     * Destructor.
     */
    ~Quat();

    /**
     * Returns the identity quaternion.
     *
     * @return The identity quaternion.
     */
    static const Quat& identity();

    /**
     * Returns the quaternion with all zeros.
     *
     * @return The quaternion.
     */
    static const Quat& zero();

    /**
     * Determines if this quaternion is equal to the identity quaternion.
     *
     * @return true if it is the identity quaternion, false otherwise.
     */
    bool isIdentity() const;

    /**
     * Determines if this quaternion is all zeros.
     *
     * @return true if this quaternion is all zeros, false otherwise.
     */
    bool isZero() const;

    /**
     * Creates a quaternion equal to the rotational part of the specified matrix
     * and stores the result in dst.
     *
     * @param m The matrix.
     * @param dst A quaternion to store the conjugate in.
     */
    static void createFromRotationMatrix(const Mat4& m, Quat* dst);

    /**
     * Creates this quaternion equal to the rotation from the specified axis and angle
     * and stores the result in dst.
     *
     * @param axis A vector describing the axis of rotation.
     * @param angle The angle of rotation (in radians).
     * @param dst A quaternion to store the conjugate in.
     */
    static void createFromAxisAngle(const Vec3& axis, float angle, Quat* dst);

    /**
     * Sets this quaternion to the conjugate of itself.
     */
    void conjugate();

    /**
     * Gets the conjugate of this quaternion.
     *
     */
    Quat getConjugated() const;

    /**
     * Sets this quaternion to the inverse of itself.
     *
     * Note that the inverse of a quaternion is equal to its conjugate
     * when the quaternion is unit-length. For this reason, it is more
     * efficient to use the conjugate method directly when you know your
     * quaternion is already unit-length.
     *
     * @return true if the inverse can be computed, false otherwise.
     */
    bool inverse();

    /**
     * Gets the inverse of this quaternion.
     *
     * Note that the inverse of a quaternion is equal to its conjugate
     * when the quaternion is unit-length. For this reason, it is more
     * efficient to use the conjugate method directly when you know your
     * quaternion is already unit-length.
     */
    Quat getInversed() const;

    /**
     * Multiplies this quaternion by the specified one and stores the result in this quaternion.
     *
     * @param q The quaternion to multiply.
     */
    void multiply(const Quat& q);

    /**
     * Multiplies the specified quaternions and stores the result in dst.
     *
     * @param q1 The first quaternion.
     * @param q2 The second quaternion.
     * @param dst A quaternion to store the result in.
     */
    static void multiply(const Quat& q1, const Quat& q2, Quat* dst);

    /**
     * Normalizes this quaternion to have unit length.
     *
     * If the quaternion already has unit length or if the length
     * of the quaternion is zero, this method does nothing.
     */
    void normalize();

    /**
     * Get the normalized quaternion.
     *
     * If the quaternion already has unit length or if the length
     * of the quaternion is zero, this method simply copies
     * this vector.
     */
    Quat getNormalized() const;

    /**
     * Sets the elements of the quaternion to the specified values.
     *
     * @param xx The new x-value.
     * @param yy The new y-value.
     * @param zz The new z-value.
     * @param ww The new w-value.
     */
    void set(float xx, float yy, float zz, float ww);

    /**
     * Sets the elements of the quaternion from the values in the specified array.
     *
     * @param array An array containing the elements of the quaternion in the order x, y, z, w.
     */
    void set(float* array);

    /**
     * Sets the quaternion equal to the rotational part of the specified matrix.
     *
     * @param m The matrix.
     */
    void set(const Mat4& m);

    /**
     * Sets the quaternion equal to the rotation from the specified axis and angle.
     * 
     * @param axis The axis of rotation.
     * @param angle The angle of rotation (in radians).
     */
    void set(const Vec3& axis, float angle);

    /**
     * Sets the elements of this quaternion to a copy of the specified quaternion.
     *
     * @param q The quaternion to copy.
     */
    void set(const Quat& q);

    /**
     * Sets this quaternion to be equal to the identity quaternion.
     */
    void setIdentity();

    /**
     * Converts this Quaternion4f to axis-angle notation. The axis is normalized.
     *
     * @param e The Vec3f which stores the axis.
     * 
     * @return The angle (in radians).
     */
    float toAxisAngle(Vec3* e) const;

    /**
     * Interpolates between two quaternions using linear interpolation.
     *
     * The interpolation curve for linear interpolation between
     * quaternions gives a straight line in quaternion space.
     *
     * @param q1 The first quaternion.
     * @param q2 The second quaternion.
     * @param t The interpolation coefficient.
     * @param dst A quaternion to store the result in.
     */
    static void lerp(const Quat& q1, const Quat& q2, float t, Quat* dst);
    
    /**
     * Interpolates between two quaternions using spherical linear interpolation.
     *
     * Spherical linear interpolation provides smooth transitions between different
     * orientations and is often useful for animating models or cameras in 3D.
     *
     * Note: For accurate interpolation, the input quaternions must be at (or close to) unit length.
     * This method does not automatically normalize the input quaternions, so it is up to the
     * caller to ensure they call normalize beforehand, if necessary.
     *
     * @param q1 The first quaternion.
     * @param q2 The second quaternion.
     * @param t The interpolation coefficient.
     * @param dst A quaternion to store the result in.
     */
    static void slerp(const Quat& q1, const Quat& q2, float t, Quat* dst);
    
    /**
     * Interpolates over a series of quaternions using spherical spline interpolation.
     *
     * Spherical spline interpolation provides smooth transitions between different
     * orientations and is often useful for animating models or cameras in 3D.
     *
     * Note: For accurate interpolation, the input quaternions must be unit.
     * This method does not automatically normalize the input quaternions,
     * so it is up to the caller to ensure they call normalize beforehand, if necessary.
     *
     * @param q1 The first quaternion.
     * @param q2 The second quaternion.
     * @param s1 The first control point.
     * @param s2 The second control point.
     * @param t The interpolation coefficient.
     * @param dst A quaternion to store the result in.
     */
    static void squad(const Quat& q1, const Quat& q2, const Quat& s1, const Quat& s2, float t, Quat* dst);

    /**
     * Calculates the quaternion product of this quaternion with the given quaternion.
     * 
     * Note: this does not modify this quaternion.
     * 
     * @param q The quaternion to multiply.
     * @return The quaternion product.
     */
    inline Quat operator*(const Quat& q) const;

    /**
     * Calculates the quaternion product of this quaternion with the given vec3.
     * @param v The vec3 to multiply.
     * @return The vec3 product.
     */
    inline Vec3 operator*(const Vec3& v) const;

    /**
     * Multiplies this quaternion with the given quaternion.
     * 
     * @param q The quaternion to multiply.
     * @return This quaternion, after the multiplication occurs.
     */
    inline Quat& operator*=(const Quat& q);
    
    /** equals to Quat(0,0,0, 0) */
    static const Quat ZERO;

private:

    /**
     * Interpolates between two quaternions using spherical linear interpolation.
     *
     * Spherical linear interpolation provides smooth transitions between different
     * orientations and is often useful for animating models or cameras in 3D.
     *
     * Note: For accurate interpolation, the input quaternions must be at (or close to) unit length.
     * This method does not automatically normalize the input quaternions, so it is up to the
     * caller to ensure they call normalize beforehand, if necessary.
     *
     * @param q1x The x component of the first quaternion.
     * @param q1y The y component of the first quaternion.
     * @param q1z The z component of the first quaternion.
     * @param q1w The w component of the first quaternion.
     * @param q2x The x component of the second quaternion.
     * @param q2y The y component of the second quaternion.
     * @param q2z The z component of the second quaternion.
     * @param q2w The w component of the second quaternion.
     * @param t The interpolation coefficient.
     * @param dstx A pointer to store the x component of the slerp in.
     * @param dsty A pointer to store the y component of the slerp in.
     * @param dstz A pointer to store the z component of the slerp in.
     * @param dstw A pointer to store the w component of the slerp in.
     */
    static void slerp(float q1x, float q1y, float q1z, float q1w, float q2x, float q2y, float q2z, float q2w, float t, float* dstx, float* dsty, float* dstz, float* dstw);

    static void slerpForSquad(const Quat& q1, const Quat& q2, float t, Quat* dst);
};

inline Quat Quat::operator*(const Quat& q) const
{
    Quat result(*this);
    result.multiply(q);
    return result;
}

inline Quat& Quat::operator*=(const Quat& q)
{
    multiply(q);
    return *this;
}

inline Vec3 Quat::operator*(const Vec3& v) const
{
    Vec3 uv, uuv;
    Vec3 qvec(x, y, z);
    Vec3::cross(qvec, v, &uv);
    Vec3::cross(qvec, uv, &uuv);

    uv *= (2.0f * w);
    uuv *= 2.0f;

    return v + uv + uuv;
}

NS_CC_MATH_END
/**
 end of base group
 @}
 */

#endif
