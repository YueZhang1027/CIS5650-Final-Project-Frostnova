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

#include "Grid.h"

Grid::Grid(float _width, float _depth, int _subdivs) {
  // init parameters
  m_width   = _width;
  m_depth   = _depth;
  m_subdivs = _subdivs;
  // ensure vertices vector is empty
  m_verts.resize(0);

  m_created = false;

  // zero the transform matrix
  m_transform.setIdentity();
}

Grid::~Grid() {
  // empty destructor
}

void Grid::draw() {
  if (!m_created) {
    // if it hasnt been created then cant draw so just return
    return;
  }
  // bind, draw and then unbind
 /* m_vao->bind();
  m_vao->draw();
  m_vao->unbind();*/
}

void Grid::create() {
  // #########################################################################
  // function taken from Jon Maceys NGL function for creating a grid
  // http://nccastaff.bmth.ac.uk/jmacey/GraphicsLib
  //m_vao = new VAO(GL_LINES);
  //m_vao->create();
  vDat vert;

  float wstep = m_width / (float)m_subdivs;

  float ws2 = m_width / 2.0f;

  float v1 = -ws2;

  float dstep = m_depth / (float)m_subdivs;

  float ds2 = m_depth / 2.0f;

  float v2 = -ds2;

  for (int i = 0; i <= m_subdivs; ++i) {
    // vertex 1 x,y,z
    vert.x = -ws2;  // x
    vert.z = v1;    // y
    vert.y = 0.0;   // z
    m_verts.push_back(vert);
    // vertex 2 x,y,z
    vert.x = ws2;  // x
    vert.z = v1;   // y
    m_verts.push_back(vert);

    // vertex 1 x,y,z
    vert.x = v2;   // x
    vert.z = ds2;  // y
    m_verts.push_back(vert);
    // vertex 2 x,y,z
    vert.x = v2;    // x
    vert.z = -ds2;  // y
    m_verts.push_back(vert);

    // now change our step value
    v1 += wstep;
    v2 += dstep;
  }
  // #########################################################################

  // bind and set data for the VAO
 // m_vao->bind();
 /* m_vao->setData(m_verts.size() * sizeof(vDat), m_verts[0].u);
  m_vao->vertexAttribPointer(0, 3, GL_FLOAT, sizeof(vDat), 5);
  m_vao->setIndicesCount(m_verts.size());
  m_vao->unbind();*/

  // set as created
  m_created = true;
}
