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

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef DARWIN
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#endif

#ifdef __linux__
#include <GL/gl.h>
#endif

#include <openvdb/openvdb.h>

#include <iostream>

/// @file Types.h
/// @brief Header file containing definitions of type structures used throughout
/// the application
/// @author Callum James
/// @version 1.0
/// @date 12/02/2014
/// Revision History:
/// Initial Version 05/01/2014

/// @struct vDat
/// @brief Structure taken from Jon Maceys NGL to allow for storage of data to
/// upload to the graphics card in OpenGL
/// (http://nccastaff.bmth.ac.uk/jmacey/GraphicsLib)
// structure taken from Jon Maceys NGL
// http://nccastaff.bmth.ac.uk/jmacey/GraphicsLib
struct vDat {
  GLfloat u;
  GLfloat v;
  GLfloat nx;
  GLfloat ny;
  GLfloat nz;
  GLfloat x;
  GLfloat y;
  GLfloat z;

  GLfloat vx;
  GLfloat vy;
  GLfloat vz;

  GLfloat cx;
  GLfloat cy;
  GLfloat cz;

  GLfloat d;

  GLfloat temp;
};

/// @struct BBoxBare
/// @brief Structure to simply hold min and max values of a bounding box and
/// nothing else
struct BBoxBare {
  float minx;
  float miny;
  float minz;
  float maxx;
  float maxy;
  float maxz;
};

/// @struct Cull
/// @brief Structure to store all information about a single Cull and can be
/// used to upload to the graphics card and shaders
struct Cull {
  bool _active;
  int _channelOffset;
  int _channelType;
  int _cullType;
  float _floatULimit;
  float _floatLLimit;
  openvdb::Vec3s _vecULimit;
  openvdb::Vec3s _vecLLimit;
};

/// @struct TempVec3
/// @brief Very simple structure to only store x, y and z positions to allow
/// swapping of data without the need for allocation of a full Vec3 class
struct TempVec3 {
  float x;
  float y;
  float z;
};

#endif /* __TYPES_H__ */
