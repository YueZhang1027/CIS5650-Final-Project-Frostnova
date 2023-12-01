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

#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <cmath>
#include <vector>

#ifdef DARWIN
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <openvdb/openvdb.h>

/// @file Utilities.h
/// @brief File containing generic utility functions used throughout the
/// application
/// @author Callum James
/// @version 1.0
/// @date 12/02/2014
/// Revision History:
/// Initial Version 05/01/2014

/// @brief 2 times PI
const static float ud_TWO_PI = float(2 * M_PI);
/// @brief Value of PI
const static float ud_PI = float(M_PI);
/// @brief PI divided by 2
const static float ud_PI2 = float(M_PI / 2.0);
/// @brief PI divided by 4
const static float ud_PI4 = float(M_PI / 4.0);

/// @namespace Utilities
namespace Utilities {
/// @brief Method to convert degrees to radians - returns float
/// @param [in] _deg const float - degrees passed in
float u_radians(const float _deg);
/// @brief Method to convert radians into degrees - returns float
/// @param [in] _rad const float - radians passed in
float u_degrees(const float _rad);

/// @brief Method to convert a GLubyte to a string - returns std::string
/// @param [in] _in const GLubyte* - values passed in
std::string glubyteToStdString(const GLubyte *_in);
/// @brief Method to convert an openvdb::Coord to a string - returns std::string
/// @param [in] _in const openvdb::Coord - coord to convert
std::string vdbCoordToStdString(const openvdb::Coord _in);
/// @brief convert a level to a colour value - returns openvdb::Vec3f
/// @param [in] _level int - level to return as colour
openvdb::Vec3f getColourFromLevel(int _level);

/// @brief Check for a GLError - returns GLenum
GLenum checkGLError();
/// @brief Expand Matrix3x3 to a float array - returns std::vector<GLfloat>
/// @param [in] _in openvdb::Mat3R - matrix to flatten
std::vector<GLfloat> u_Mat3ToFloatArray(openvdb::Mat3R _in);
/// @brief Expand Matrix4x4 to a float array - returns std::vector<GLfloat>
/// @param [in] _in openvdb::Mat4s - matrix to flatten
std::vector<GLfloat> u_Mat4ToFloatArray(openvdb::Mat4s _in);
/// @brief Report that not external GPU device is attached

inline void printNoExtGPU() {
  std::cout << "No external GPU connected - current available GPU memory is "
               "not available"
            << std::endl;
}
/// @brief Report that AMD is not currently supported in this application for
/// memory querying
inline void printAMDNotSupported() {
  std::cout << "AMD graphic cards currently not supported" << std::endl;
}
/// @brief Modify file paths if necessary due to platform being built on
/// @param [in] _path std::string - path to modify
/// @returns std::string
std::string PLATFORM_FILE_PATH(std::string _path);
}  // namespace Utilities

#endif /* __UTILITIES_H__ */
