#pragma once
/*
  Copyright (C) 2014 Callum James

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PLANE_H__
#define __PLANE_H__

#include <openvdb/openvdb.h>

/// @file Plane.h
/// @brief Simple class for a plane used within the camera class. Based on Jon
/// Maceys NGL Plane class
/// @author Jon Macey and modified by Callum James
/// @version 1.0
/// @date 12/02/2014 (of modifications)
/// Revision History:
/// Initial Version 21/03/2011 (Jon Maceys initial version)
/// @class Plane
/// @brief Plane class hevaily based on Jon Maceys NGL Camera class
/// (http://nccastaff.bmth.ac.uk/jmacey/GraphicsLib). Modified to rely on the
/// OpenVDB math library and remove some features not needed. Used only within
/// the Camera class for the frustrum.
class Plane {
public:
  /// @brief Constructor of the Plane class
  Plane();
  /// @brief Constructor of the Plane class
  /// @param [in] _v1 const openvdb::Vec3f - vector 1
  /// @param [in] _v2 const openvdb::Vec3f - vector 2
  /// @param [in] _v3 const openvdb::Vec3f - vector 3
  Plane(const openvdb::Vec3f &_v1, const openvdb::Vec3f &_v2,
        const openvdb::Vec3f &_v3);

  /// @brief Destructor of the Plane class
  ~Plane();

  /// @brief Set the plane
  /// @param [in] _v1 const openvdb::Vec3f - vector 1
  /// @param [in] _v2 const openvdb::Vec3f - vector 2
  /// @param [in] _v3 const openvdb::Vec3f - vector 3
  void set(const openvdb::Vec3f &_v1, const openvdb::Vec3f &_v2,
           const openvdb::Vec3f &_v3);

  /// @brief Set normal of the point on the plane
  /// @param [in] _normal const openvdb::Vec3f - normal to set to
  /// @param [in] _point const openvdb::Vec3f - point to set the normal on
  void setNormal(const openvdb::Vec3f &_normal, const openvdb::Vec3f &_point);

  /// @brief Method to set the plane floats
  /// @param [in] _a float - float a
  /// @param [in] _b float - float b
  /// @param [in] _c float - float c
  /// @param [in] _d float - float d
  void setFloats(float _a, float _b, float _c, float _d);

  /// @brief Get the distance between this plane and a point - returns float
  /// @param _p const openvdb::Vec3f - point to calculate distance to
  float distance(const openvdb::Vec3f &_p);

  /// @brief Method to get the normal of the plane - return openvdb::Vec3f
  inline openvdb::Vec3f getNormal() { return m_normal; }
  /// @brief Method to get the point - returns openvdb::Vec3f
  inline openvdb::Vec3f getPoint() { return m_point; }

  /// @brief Return the value D - returns float
  inline float getD() { return m_d; }

private:
  /// @brief Normal of the plane
  openvdb::Vec3f m_normal;
  /// @brief Point on the plane
  openvdb::Vec3f m_point;
  /// @brief Value D
  float m_d;
};

#endif /* __PLANE_H__ */
