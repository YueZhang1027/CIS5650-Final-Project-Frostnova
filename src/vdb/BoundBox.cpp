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

#include "BoundBox.h"

BoundBox::BoundBox() {
  // default constructor
  init();
  // initialise all attributes to abitrary values
  m_centre = openvdb::Vec3f(0, 0, 0);
  m_width  = 2.0f;
  m_height = 2.0f;
  m_depth  = 2.0f;

  m_minX = m_centre.x() - (m_width / 2.0f);
  m_maxX = m_centre.x() + (m_width / 2.0f);
  m_minY = m_centre.y() - (m_height / 2.0f);
  m_maxY = m_centre.y() + (m_height / 2.0f);
  m_minZ = m_centre.z() - (m_depth / 2.0f);
  m_maxZ = m_centre.z() + (m_depth / 2.0f);

  // calculate the vertices from these default values but do not build
  calculateVerts(false);
}

BoundBox::BoundBox(float _minX, float _maxX, float _minY, float _maxY,
                   float _minZ, float _maxZ) {
  // set attrbiutes to passed in values
  init();
  m_minX = _minX;
  m_maxX = _maxX;
  m_minY = _minY;
  m_maxY = _maxY;
  m_minZ = _minZ;
  m_maxZ = _maxZ;

  // once set go off and calculate the bounding box attributes
  calculateAttributes();
}

BoundBox::BoundBox(float _width, float _height, float _depth,
                   openvdb::Vec3f _centre) {
  init();
  // another constructor just setting attributes to the passed in values
  m_centre = _centre;
  m_width  = _width;
  m_height = _height;
  m_depth  = _depth;

  // set mins and max as half of width, depth and height in both directions
  // from the centre
  m_minX = m_centre.x() - (m_width / 2.0f);
  m_maxX = m_centre.x() + (m_width / 2.0f);
  m_minY = m_centre.y() - (m_height / 2.0f);
  m_maxY = m_centre.y() + (m_height / 2.0f);
  m_minZ = m_centre.z() - (m_depth / 2.0f);
  m_maxZ = m_centre.z() + (m_depth / 2.0f);

  // calculate vertices and then build
  calculateVerts();
}

BoundBox::~BoundBox() {}

void BoundBox::set(float _minx, float _maxx, float _miny, float _maxy,
                   float _minz, float _maxz) {
  // set the min and max attributes and then recalculate all attributes for the
  // bounding box
  m_minX = _minx;
  m_maxX = _maxx;
  m_minY = _miny;
  m_maxY = _maxy;
  m_minZ = _minz;
  m_maxZ = _maxz;

  calculateAttributes();
}

void BoundBox::setCentre(float _x, float _y, float _z) {
#ifdef NDEBUG
#else
  spdlog::warn(
      "Warning: Centre of bounding box being set with no other update, could "
      "cause erroneous results");
#endif
  m_centre[0] = _x;
  m_centre[1] = _y;
  m_centre[2] = _z;
  // recalculate vertices now it has changed
  vertsFromCentre();
}

void BoundBox::setCentre(openvdb::Vec3f _centre) {
#ifdef NDEBUG
#else
  spdlog::warn(
      "Warning: Centre of bounding box being set with no other update, could "
      "cause erroneous results");
#endif
  m_centre = _centre;
  // recalculate vertices now it has changed
  vertsFromCentre();
}

void BoundBox::setWidth(float _w) {
  m_width = _w;
  // once the width has changed, need to recalculate the vertices
  vertsFromCentre();
}

void BoundBox::setHeight(float _h) {
  m_height = _h;
  // once the height has changed, need to recalculate the vertices
  vertsFromCentre();
}

void BoundBox::setDepth(float _d) {
  m_depth = _d;
  // once the depth has changed, need to recalculate the vertices
  vertsFromCentre();
}

void BoundBox::setwdh(float _a) {
  m_width  = _a;
  m_height = _a;
  m_depth  = _a;
  // setting width, depth and height. As they have all changed, need to
  // recalculate vertices from unchanged attributes
  vertsFromCentre();
}

void BoundBox::setwdh(float _w, float _h, float _d) {
  m_width  = _w;
  m_height = _h;
  m_depth  = _d;
  // setting width, depth and height. As they have all changed, need to
  // recalculate vertices from unchanged attributes
  vertsFromCentre();
}

void BoundBox::minX(const float &_x) {
  m_minX = _x;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::maxX(const float &_x) {
  m_maxX = _x;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::minY(const float &_y) {
  m_minY = _y;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::maxY(const float &_y) {
  m_maxY = _y;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::minZ(const float &_z) {
  m_minZ = _z;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::maxZ(const float &_z) {
  m_maxZ = _z;
  // need to recalculate all attributes as value changed
  calculateAttributes();
}

void BoundBox::drawMode(GLenum _mode) {
  m_drawMode = _mode;
  if (m_buildIndexed) {
    // if the bounding box has been set to be built as indexed, call index
    // function
    buildVAOIndexed();
  } else {
    // if not set to be indexed, just build normally
    buildVAO();
  }
}

void BoundBox::translate(const openvdb::Vec3f &_v) {
  m_centre += _v;
  // translate the centre only and then recalculate points from this
  vertsFromCentre();
}

void BoundBox::translate(const float _x, const float _y, const float _z) {
  openvdb::Vec3f addition = openvdb::Vec3f(_x, _y, _z);

  m_centre += addition;
  // translate the centre only and then recalculate points from this
  vertsFromCentre();
}

void BoundBox::vertsFromCentre() {
  // to calculate verticesfrom the centre, +/- the width, depth and height
  // from the centre to get the extremes
  m_minX = m_centre.x() - (m_width / 2.0f);
  m_maxX = m_centre.x() + (m_width / 2.0f);
  m_minY = m_centre.y() - (m_height / 2.0f);
  m_maxY = m_centre.y() + (m_height / 2.0f);
  m_minZ = m_centre.z() - (m_depth / 2.0f);
  m_maxZ = m_centre.z() + (m_depth / 2.0f);
  // now recalculate the vertices with these new attributes
  calculateVerts();
}

void BoundBox::scale(const float &_s) {
  m_width *= _s;
  m_height *= _s;
  m_depth *= _s;
  // now recalculate the vertices with these new attributes
  vertsFromCentre();
}

void BoundBox::scale(const openvdb::Vec3f &_s) {
  m_width *= _s.x();
  m_height *= _s.y();
  m_depth *= _s.z();
  // now recalculate the vertices with these new attributes
  vertsFromCentre();
}

void BoundBox::calculateAttributes() {
  // calculate all different attributes based on the min and max values
  calculateWDH();

  calculateCentre();

  calculateVerts();
}

void BoundBox::calculateWDH() {
  m_width  = m_maxX - m_minX;
  m_height = m_maxY - m_minY;
  m_depth  = m_maxZ - m_minZ;
}

void BoundBox::calculateCentre() {
  float x = m_maxX - (m_width / 2.0f);
  float y = m_maxY - (m_height / 2.0f);
  float z = m_maxZ - (m_depth / 2.0f);

  m_centre = openvdb::Vec3f(x, y, z);
}

void BoundBox::calculateVerts(bool _build) {
    // these will be ordered in a specific way. Will work around from bottom left
    // (so minX, minY, maxZ) box in a counter clockwise direction and then move to
    // the other edge
    /*
     *order stored within array
     *[0] (minX, minY, maxZ)
     *[1] (maxX, minY, maxZ)
     *[2] (maxX, maxY, maxZ)
     *[3] (minX, maxY, maxZ)
     *[4] (minX, minY, minZ)
     *[5] (maxX, minY, minZ)
     *[6] (maxX, maxY, minZ)
     *[7] (minX, maxY, minZ)
     */

    m_verts[0] = openvdb::Vec3f(m_minX, m_minY, m_maxZ);
    m_verts[1] = openvdb::Vec3f(m_maxX, m_minY, m_maxZ);
    m_verts[2] = openvdb::Vec3f(m_maxX, m_maxY, m_maxZ);
    m_verts[3] = openvdb::Vec3f(m_minX, m_maxY, m_maxZ);
    m_verts[4] = openvdb::Vec3f(m_minX, m_minY, m_minZ);
    m_verts[5] = openvdb::Vec3f(m_maxX, m_minY, m_minZ);
    m_verts[6] = openvdb::Vec3f(m_maxX, m_maxY, m_minZ);
    m_verts[7] = openvdb::Vec3f(m_minX, m_maxY, m_minZ);

    calculateNormals();
}

void BoundBox::calculateNormals() {
  // this will calculate the normals of each face
  // this can then be used quickly for collision detection instead of having to
  // calculate it at the point of collision just calculate whenever the bounding
  // box is updated
  /*
   *The faces will be organised as defined by these vertices
   *[0] = 0,1,2,3
   *[1] = 0,3,4,7
   *[2] = 4,5,6,7
   *[3] = 1,2,5,6
   *[4] = 2,3,6,7
   *[5] = 0,1,4,5
   */

  m_norms[0] = (m_verts[1] - m_verts[0]).cross(m_verts[3] - m_verts[0]);
  m_norms[1] = (m_verts[7] - m_verts[4]).cross(m_verts[0] - m_verts[4]);
  m_norms[2] = (m_verts[4] - m_verts[5]).cross(m_verts[6] - m_verts[5]);
  m_norms[3] = (m_verts[2] - m_verts[1]).cross(m_verts[5] - m_verts[1]);
  m_norms[4] = (m_verts[7] - m_verts[3]).cross(m_verts[2] - m_verts[3]);
  m_norms[5] = (m_verts[0] - m_verts[4]).cross(m_verts[5] - m_verts[4]);

  // now they are calculated normalize them all
  for (int i = 0; i < 6; i++) {
    // set all normals to length 1
    m_norms[i].normalize();
  }
}

void BoundBox::init() {
  // set as indexed by default
  m_buildIndexed = true;
  // default draw mode
  m_drawMode = GL_LINES;
  m_width = m_height = m_depth = 0;

  m_colour = openvdb::Vec3f(1.0f, 1.0f, 1.0f);

  // init the VAO
  /* m_vao = new VAO(m_drawMode);
   m_vao->create();*/
}

void BoundBox::buildVAO() {
  unsigned int buffSize = 24;
  vDat data[24];

  // now set each one to vDat format - done manually as quicker and easier if a
  // bit longer
  vdatPosFromVec(&data[0], m_verts[0]);
  vdatPosFromVec(&data[1], m_verts[1]);
  vdatPosFromVec(&data[2], m_verts[1]);
  vdatPosFromVec(&data[3], m_verts[2]);
  vdatPosFromVec(&data[4], m_verts[2]);
  vdatPosFromVec(&data[5], m_verts[3]);
  vdatPosFromVec(&data[6], m_verts[3]);
  vdatPosFromVec(&data[7], m_verts[0]);
  vdatPosFromVec(&data[8], m_verts[4]);
  vdatPosFromVec(&data[9], m_verts[5]);
  vdatPosFromVec(&data[10], m_verts[5]);
  vdatPosFromVec(&data[11], m_verts[6]);
  vdatPosFromVec(&data[12], m_verts[6]);
  vdatPosFromVec(&data[13], m_verts[7]);
  vdatPosFromVec(&data[14], m_verts[7]);
  vdatPosFromVec(&data[15], m_verts[4]);
  vdatPosFromVec(&data[16], m_verts[3]);
  vdatPosFromVec(&data[17], m_verts[7]);
  vdatPosFromVec(&data[18], m_verts[0]);
  vdatPosFromVec(&data[19], m_verts[4]);
  vdatPosFromVec(&data[20], m_verts[1]);
  vdatPosFromVec(&data[21], m_verts[5]);
  vdatPosFromVec(&data[22], m_verts[2]);
  vdatPosFromVec(&data[23], m_verts[6]);

  // end of sets
}

void BoundBox::buildVAOIndexed() {
  unsigned int buffSize = 8;
  vDat data[8];

  // only need 8 vertices as using indexes - one for each corner of the box

  vdatPosFromVec(&data[0], m_verts[0]);
  vdatPosFromVec(&data[1], m_verts[1]);
  vdatPosFromVec(&data[2], m_verts[2]);
  vdatPosFromVec(&data[3], m_verts[3]);
  vdatPosFromVec(&data[4], m_verts[4]);
  vdatPosFromVec(&data[5], m_verts[5]);
  vdatPosFromVec(&data[6], m_verts[6]);
  vdatPosFromVec(&data[7], m_verts[7]);

  // 24 elements needed to cover all edges of the box

  unsigned int elementSize = 24;
  // manually worked out and set the edges needed
  static const GLuint elements[24] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                      6, 7, 7, 4, 4, 0, 1, 5, 7, 3, 6, 2};
}

void BoundBox::draw() {
}

bool BoundBox::pointInside(openvdb::Vec3f _point) {
  // simple bounding box collision detection to see if the passed in point is
  // within the box
  if (_point.x() < m_maxX && _point.x() > m_minX) {
    if (_point.y() < m_maxY && _point.y() > m_minY) {
      if (_point.z() < m_maxZ && _point.z() > m_minZ) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

BBoxBare BoundBox::asBBoxBare() {
  // return only the min and max values as a BBoxBare used
  // for channel extreme data
  BBoxBare temp;
  temp.minx = m_minX;
  temp.maxx = m_maxX;

  temp.miny = m_minY;
  temp.maxy = m_maxY;

  temp.minz = m_minZ;
  temp.maxz = m_maxZ;

  return temp;
}

void BoundBox::vdatPosFromVec(vDat *_dat, openvdb::Vec3f _vec) {
  _dat->x = _vec.x();
  _dat->y = _vec.y();
  _dat->z = _vec.z();
}
