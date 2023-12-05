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

#include "Utilities.h"
#include <string>
#include <boost/lexical_cast.hpp>

#ifdef DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

float Utilities::u_radians(const float _deg) { return (_deg / 180.0f) * M_PI; }

float Utilities::u_degrees(const float _rad) { return (_rad / M_PI) * 180.0f; }

// TODO
std::string Utilities::glubyteToStdString(const GLubyte *_in) {
  // cast a GLubyte to a QString
  std::string temp((const char *)_in);
  // return the QString as a std::string
  return temp;
}

std::string Utilities::vdbCoordToStdString(const openvdb::Coord _in) {
  // use boost to cast the x, y and z paramters of a openvdb::coord to a string
  std::string coordStr = "";
  coordStr += "[" + boost::lexical_cast<std::string>(_in.x()) + ", ";
  coordStr += boost::lexical_cast<std::string>(_in.y()) + ", ";
  coordStr += boost::lexical_cast<std::string>(_in.z()) + "]";
  return coordStr;
}

openvdb::Vec3f Utilities::getColourFromLevel(int _level) {
  // colour values taken from viewer in OpenVDB library
  // RenderModules.cc
  switch (_level) {
    case (0): {
      return openvdb::Vec3f(0.00608299f, 0.279541f, 0.625f);
    } break;
    case (1): {
      return openvdb::Vec3f(0.871f, 0.394f, 0.01916f);
    } break;
    case (2): {
      return openvdb::Vec3f(0.0432f, 0.33f, 0.0411023f);
    } break;
    case (3): {
      return openvdb::Vec3f(0.045f, 0.045f, 0.045f);
    } break;
    default: {
      return openvdb::Vec3f(0.0f, 0.0f, 0.0f);
    } break;
  }
}

// TODO
// GLenum Utilities::checkGLError() {
//  GLenum errCode;
//  errCode = glGetError();
//  if (errCode != 0) {
//    std::cerr << "GL ERROR: " << errCode << std::endl;
//  }
//  return errCode;
//}

// TODO
std::vector<GLfloat> Utilities::u_Mat3ToFloatArray(openvdb::Mat3R _in) {
  // flatten a Mat3 and return as a single array of values to upload to the GPU
  std::vector<GLfloat> m;
  m.resize(9);

  m[0] = (GLfloat)_in(0, 0);
  m[1] = (GLfloat)_in(0, 1);
  m[2] = (GLfloat)_in(0, 2);
  m[3] = (GLfloat)_in(1, 0);
  m[4] = (GLfloat)_in(1, 1);
  m[5] = (GLfloat)_in(1, 2);
  m[6] = (GLfloat)_in(2, 0);
  m[7] = (GLfloat)_in(2, 1);
  m[8] = (GLfloat)_in(2, 2);

  return m;
}

std::vector<GLfloat> Utilities::u_Mat4ToFloatArray(openvdb::Mat4s _in) {
  // flatten a Mat4 and return as a single array of values to upload to the GPU
  std::vector<GLfloat> m;
  m.resize(16);

  m[0]  = (GLfloat)_in(0, 0);
  m[1]  = (GLfloat)_in(0, 1);
  m[2]  = (GLfloat)_in(0, 2);
  m[3]  = (GLfloat)_in(0, 3);
  m[4]  = (GLfloat)_in(1, 0);
  m[5]  = (GLfloat)_in(1, 1);
  m[6]  = (GLfloat)_in(1, 2);
  m[7]  = (GLfloat)_in(1, 3);
  m[8]  = (GLfloat)_in(2, 0);
  m[9]  = (GLfloat)_in(2, 1);
  m[10] = (GLfloat)_in(2, 2);
  m[11] = (GLfloat)_in(2, 3);
  m[12] = (GLfloat)_in(3, 0);
  m[13] = (GLfloat)_in(3, 1);
  m[14] = (GLfloat)_in(3, 2);
  m[15] = (GLfloat)_in(3, 3);

  return m;
}

std::string Utilities::PLATFORM_FILE_PATH(std::string _path) {
  std::string prefixString;
  prefixString.clear();

#ifdef DARWIN
  char prefix[FILENAME_MAX];
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL  = CFBundleCopyResourcesDirectoryURL(mainBundle);

  if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)prefix,
                                        FILENAME_MAX)) {
    std::cerr << "Could not get relative path of application bundle - exiting!"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  CFRelease(resourcesURL);
  prefixString = std::string(prefix);
#endif

#ifdef WIN32
  wchar_t buffer[FILENAME_MAX];
  _wgetcwd(buffer, FILENAME_MAX);
  std::wstring wstr(buffer);
  std::string winPath(wstr.begin(), wstr.end());
  std::replace(winPath.begin(), winPath.end(), '\\', '/');
  prefixString = std::string(winPath);
#endif

  std::string returnString = "";
  if (!prefixString.empty()) {
    returnString += prefixString;
    returnString += "/";
  }
  returnString += _path;

  return (returnString);
}
