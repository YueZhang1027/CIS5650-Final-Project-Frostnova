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

#ifndef __GRID_H__
#define __GRID_H__

//#include "VAO.h"
#include "Types.h"

#include <openvdb/openvdb.h>

/// @file Grid.h
/// @brief Grid class to create and draw a simple grid in the scene for
/// reference
/// @author Callum James
/// @version 1.0
/// @date 12/02/2014
/// Revision History:
/// Initial Version 05/01/2014
/// @class Grid
/// @brief This grid class creates and draw a simple reference grid into the
/// scene. The creation algorithm was taken from Jon Maceys NGL
/// http://nccastaff.bmth.ac.uk/jmacey/GraphicsLib/
class Grid {
public:
  /// @brief Constructor of Grid class
  /// @param _width float - width of the grid to make
  /// @param _depth float - depth of the grid to make
  /// @param _subdivs int - number of subdivisions to use in both axes when
  /// making the grid
  Grid(float _width, float _depth, int _subdivs);
  /// @brief Destructor of Grid
  ~Grid();

  /// @brief Return the transform matrix of the grid - returns openvdb::Mat4s
  inline openvdb::Mat4s transform() { return m_transform; }

  /// @brief Method to create the grid
  void create();
  /// @brief Draw the grid
  void draw();

private:
  /// @brief VAO used for drawing the grid
  /*VAO *m_vao;*/

  /// @brief Vector of vDat structure to hold the grid vertices and data to
  /// upload to VAO
  std::vector<vDat> m_verts;

  /// @brief Grid transform matrix
  openvdb::Mat4s m_transform;

  /// @brief Width of the grid
  float m_width;
  /// @brief Depth of the grid
  float m_depth;
  /// @brief Subdivisions of the grid
  int m_subdivs;

  /// @brief Boolena of whether the grid has been created or not
  bool m_created;
};

#endif /* __GRID_H__ */
