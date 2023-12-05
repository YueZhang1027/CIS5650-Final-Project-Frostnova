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

#ifndef __BOUNDBOX_H__
#define __BOUNDBOX_H__

#include "Types.h"
//#include "VAO.h"

/// @file BoundBox.h
/// @brief Class to create and control a bounding box. Handles its drawing and
/// update of its properties.
/// @author Callum James
/// @version 1.0
/// @date 12/02/2014
/// Revision History:
/// Initial Version 05/01/2014
/// @class BoundBox
/// @brief Class to create and control a bounding box. Handles its drawing and
/// update of its properties.
class BoundBox {
public:
  /// @brief Default constructor for BoundBox - defaults all parameters
  BoundBox();
  /// @brief Constructor for BoundBox
  /// @param [in] _height float - the height of the bounding box
  /// @param [in] _width float - the width of the bounding box
  /// @param [in] _depth float - depth of the bounding box
  /// @param [in] _centre opendvb::Vec3f - centre of the bounding box
  BoundBox(float _height, float _width, float _depth, openvdb::Vec3f _centre);
  /// @brief Constructor for BoundBox
  /// @param [in] _minx float - minimum x value for the bounding box
  /// @param [in] _maxx float - maximum x value for the bounding box
  /// @param [in] _miny float - minimum y value for the bounding box
  /// @param [in] _maxy float - maximum y value for the bounding box
  /// @param [in] _minz float - minimum z value for the bounding box
  /// @param [in] _maxz float - maximum z value for the bounding box
  BoundBox(float _minx, float _maxx, float _miny, float _maxy, float _minz,
           float _maxz);
  /// @brief Destructor for BoundBox
  ~BoundBox();

  /// @brief Method to return the centre of the bounding box - returns
  /// openvdb::Vec3f
  inline openvdb::Vec3f centre() { return m_centre; }
  /// @brief Method to return the minx value of the bounding box - returns float
  inline float minX() { return m_minX; }
  /// @brief Method to return the maxx value of the bounding box - returns float
  inline float maxX() { return m_maxX; }
  /// @brief Method to return the miny value of the bounding box - returns float
  inline float minY() { return m_minY; }
  /// @brief Method to return the maxy value of the bounding box - returns float
  inline float maxY() { return m_maxY; }
  /// @brief Method to return the minz value of the bounding box - returns float
  inline float minZ() { return m_minZ; }
  /// @brief Method to return the maxz value of the bounding box - returns float
  inline float maxZ() { return m_maxZ; }
  /// @brief Method to return the width of the bounding box - returns float
  inline float width() { return m_width; }
  /// @brief Method to return the height of the bounding box - returns float
  inline float height() { return m_height; }
  /// @brief Method to return the depth of the bounding box - returns float
  inline float depth() { return m_depth; }
  /// @brief Method to return the colour of the bounding box - returns
  /// openvdb::Vec3f
  inline openvdb::Vec3f colour() { return m_colour; }
  /// @brief Method to return if the bounding boxed has been built indexed or
  /// not - returns true or false
  inline bool getBuildIndexed() { return m_buildIndexed; }

  /// @brief Method to return the vertex at the specified index - returns
  /// openvdb::Vec3f
  /// @param [in] _index int - index to find the vertex
  inline openvdb::Vec3f vertAt(int _index) { return m_verts[_index]; }

  /// @brief Method to set the colour of the bounding box
  /// @param [in] _colour openvdb::Vec3f - colour to set
  inline void setColour(openvdb::Vec3f _colour) { m_colour = _colour; }

  /// @brief Method to set the maximums and minimums of the bounding box
  /// @param [in] _minx float - minimum x value for the bounding box
  /// @param [in] _maxx float - maximum x value for the bounding box
  /// @param [in] _miny float - minimum y value for the bounding box
  /// @param [in] _maxy float - maximum y value for the bounding box
  /// @param [in] _minz float - minimum z value for the bounding box
  /// @param [in] _maxz float - maximum z value for the bounding box
  void set(float _minx, float _maxx, float _miny, float _maxy, float _minz,
           float _maxz);
  /// @brief Method to set the centre of the bounding box
  /// @param [in] _x float - x value of the centre
  /// @param [in] _y float - y value of the centre
  /// @param [in] _z float - z value of the centre
  void setCentre(float _x, float _y, float _z);
  /// @brief Method to set the centre of the bounding box
  /// @param [in] _centre openvdb::Vec3f - centre to set to the bounding box
  void setCentre(openvdb::Vec3f _centre);

  /// @brief Method to set the width of the bounding box
  /// @param [in] _w float - width to set the bounding box to
  void setWidth(float _w);
  /// @brief Method to set the height of the bounding box
  /// @param [in] _h float - height to set the bounding box to
  void setHeight(float _h);
  /// @brief Method to set the depth of the bounding box
  /// @param [in] _d float - depth to set the bounding box to
  void setDepth(float _d);
  /// @brief Method to set the width, height and depth of the bounding box
  /// @param [in] _a float - value to set width, float and height of the
  /// bounding box to
  void setwdh(float _a);
  /// @brief Method to set the width, height and depth of the bounding box
  /// @param [in] _w float - width to set the bounding box to
  /// @param [in] _h float - height to set the bounding box to
  /// @param [in] _d float - depth to set the bounding box to
  void setwdh(float _w, float _h, float _d);

  /// @brief Method to set the minimum x value
  /// @param [in] _x const float - value to set the minimum x to
  void minX(const float &_x);
  /// @brief Method to set the maximum x value
  /// @param [in] _x const float - value to set the maximum x to
  void maxX(const float &_x);
  /// @brief Method to set the minimum y value
  /// @param [in] _y const float - value to set the minimum y to
  void minY(const float &_y);
  /// @brief Method to set the maximum y value
  /// @param [in] _y const float - value to set the maximum y to
  void maxY(const float &_y);
  /// @brief Method to set the minimum z value
  /// @param [in] _z const float - value to set the minimum z to
  void minZ(const float &_z);
  /// @brief Method to set the maximum z value
  /// @param [in] _z const float - value to set the maximum z to
  void maxZ(const float &_z);

  /// @brief Method to set whether or not the bounding box is to be built
  /// indexed or not
  /// @param [in] _indexed bool - value of indexed or not
  inline void setBuildIndexed(bool _indexed) { m_buildIndexed = _indexed; }

  /// @brief Method to set the draw mode of the bounding box
  /// @param [in] _mode GLenum - mode to set draw mode to
  void drawMode(GLenum _mode);

  /// @brief Method to translate the bounding box
  /// @param [in] _v const openvdb::Vec3f - vector to translate by
  void translate(const openvdb::Vec3f &_v);
  /// @brief Method to translate the bounding box
  /// @param [in] _x const float - x value to translate by
  /// @param [in] _y const float - y value to translate by
  /// @param [in] _z const float - z value to translate by
  void translate(const float _x, const float _y, const float _z);

  /// @brief Method to calculate the vertices from the centre of the bounding
  /// box
  void vertsFromCentre();

  /// @brief Method to scale the bounding box
  /// @param [in] _s const float - scale box in all axis by this value
  void scale(const float &_s);
  /// @brief Method to scale the bounding box
  /// @param [in] _s const openvdb::Vec3f - scale box by this vector in the
  /// corresponding axis
  void scale(const openvdb::Vec3f &_s);

  /// @brief Method to calculate the bounding boxes attributes
  void calculateAttributes();
  /// @brief Method to calculate the bounding boxes width, depth and height
  void calculateWDH();
  /// @brief Method to calculate the centre of the bounding box
  void calculateCentre();
  /// @brief Method to calculate the vertices of the bounding box
  /// @param [in] _build bool - boolean of whether to build the VAO of the box
  /// or not
  void calculateVerts(bool _build = true);
  /// @brief Method to calculate the normals of the bounding box faces
  void calculateNormals();

  /// @brief Method to build the VAO
  void buildVAO();
  /// @brief Method to build the VAO indexed
  void buildVAOIndexed();
  /// @brief Draw the bounding box
  void draw();

  // TODO
  /// @brief Returns the VAO of the bounding box - returns VAO
  // inline VAO getVAO() { return *m_vao; }

  /// @brief Method to calculate if a point lies within the bounding box -
  /// returns true or false
  /// @param [in] _point openvdb::Vec3f - the point to test against
  bool pointInside(openvdb::Vec3f _point);

  /// @brief Return this BoundingBox as core components in the form of a
  /// BBoxBare
  BBoxBare asBBoxBare();

private:
  /// @brief Method to init the bounding box
  void init();

  /// @brief Minimum X value
  float m_minX;
  /// @brief Maximum X value
  float m_maxX;
  /// @brief Minimum Y value
  float m_minY;
  /// @brief Maximum Y value
  float m_maxY;
  /// @brief Minimum Z value
  float m_minZ;
  /// @brief Maximum Z value
  float m_maxZ;

  /// @brief Width of the bounding box
  float m_width;
  /// @brief Height of the bounding box
  float m_height;
  /// @brief Depth of the bounding box
  float m_depth;

  /// @brief Centre of the bounding box
  openvdb::Vec3f m_centre;

  /// @brief Colour of the bounding box
  openvdb::Vec3f m_colour;

  /// @brief Vertices of the bounding box
  openvdb::Vec3f m_verts[8];
  /// @brief Fsce normals of the bounding box
  openvdb::Vec3f m_norms[6];

  /// @brief Draw mode of the bounding box
  GLenum m_drawMode;

  /// @brief Method to convert an openvdb::Vec3f into vDat format, writing the
  /// result to the passed in pointer
  /// @param [in,out] _dat vDat* - vDat structure to write result to
  /// @param [in] _vec openvdb::Vec3f - vector to convert
  void vdatPosFromVec(vDat *_dat, openvdb::Vec3f _vec);
  /// @brief Boolean to store if the bounding box is to be built indexed or not
  bool m_buildIndexed;
};

#endif /* __BOUNDBOX_H__ */
