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

#include "vdb.h"

#include <openvdb/tools/VolumeToMesh.h>

#include <typeinfo>

#include "Utilities.h"
#include "math.h"
#include "glm/glm.hpp"


#ifdef NVIDIA
#include "NVidiaDefines.h"
using namespace NVidiaDef;
#endif
#ifdef AMD
#include "AMDDefines.h"
using namespace AMDDef;
#endif

VDB::VDB() {
    // init paramaters for the class
    initParams();
}

VDB::VDB(std::string _file) {
    // init paramaters for the class
    initParams();
    // init openvdb
    init();
    // open file
    openFile(_file);
}

VDB::~VDB() {
    // TODO
    // m_vdbTreeVAO->remove();
    m_gridNames->resize(0);
    m_gridDims->resize(0);
    m_numPoints.clear();
    m_s.clear();

    m_grid.reset();

    // TODO
    //// delete texture buffer
    // glDeleteBuffers(1, &m_gridsTBO);

    m_loaded = false;

    delete m_gridDims;
    delete m_gridNames;
    // TODO
    // delete m_vdbTreeVAO;

    // TODO
    /* if (m_extremesInit) {
       m_channelExtremes->clear();
       delete m_channelExtremes;
     }*/

     // TODO
     /*if (m_vdbGridsInitialized) {
       for (size_t i = 0; i < m_vdbGrids->size(); i++) {
         m_vdbGrids->at(i).remove();
       }
       m_vdbGrids->clear();
       delete m_vdbGrids;
     }*/
     // uninit openvdb system
    if (m_initialised) {
        openvdb::uninitialize();
        m_initialised = false;
    }
}

void VDB::init() {
    // init the openvdb system
    if (!m_initialised) {
        openvdb::initialize();
        m_initialised = true;
    }
}

void VDB::openFile(std::string _file) {
    openvdb::io::File vdbFile(_file);  // openvdb::file type
    m_fileName = _file;
    vdbFile.open();
    if (vdbFile.isOpen()) {
        std::cout << "VDB file " << _file << " opened successfully..." << std::endl;
        m_fileOpened = true;

        // now load in data to pointers from file
        m_grid = vdbFile.getGrids();
        if (!m_grid->empty()) {
#ifdef DEBUG
            std::cout << "Grids found in file" << std::endl;
#endif
            m_allG.resize(0);
            m_allG.clear();
            // insert all grids into single structure
            m_allG.insert(m_allG.end(), m_grid->begin(), m_grid->end());
#ifdef DEBUG
            std::cout << "Grids inserted" << std::endl;
#endif
        }
        else {
            std::cerr << "Grids not found in file!!" << std::endl;
            return;
        }
        // get the total number of grids in the file
        m_numGrids = m_grid->size();

        for (int i = 0; i < m_numGrids; ++i) {
            // store the grid nmes and dimensions
            const std::string name = m_grid->at(i)->getName();
            openvdb::Coord voxDim = m_grid->at(i)->evalActiveVoxelDim();

            m_gridNames->push_back(name.empty() ? "__grid__unamed__" : name);
            m_gridDims->push_back(voxDim);
        }

        m_metadata = vdbFile.getMetadata();  // store the file metadata
        if (m_metadata != NULL) {
#ifdef DEBUG
            std::cout << "Metadata found in file" << std::endl;
#endif
        }
        else {
            std::cerr << "Failed to find metadata!!" << std::endl;
        }

        m_fileVersion = vdbFile.version();  // get the file version
        if (m_fileVersion != "") {
#ifdef DEBUG
            std::cout << "File version found in file" << std::endl;
#endif
        }
        else {
            std::cerr << "File version could not be found in file!!" << std::endl;
        }

        m_fileRead = true;  // notify that the file has been read now

        // as finished with the file now close it
        vdbFile.close();

        m_metaNames.resize(0);
        m_metaValues.resize(0);

        for (int i = 0; i < m_numGrids; ++i) {
            for (openvdb::MetaMap::MetaIterator iter = m_grid->at(i)->beginMeta();
                iter != m_grid->at(i)->endMeta(); ++iter) {
                // for all grids, loop through and store all meta data found in the file
                const std::string& name = iter->first;
                openvdb::Metadata::Ptr value = iter->second;
                m_metaNames.push_back(name);
                m_metaValues.push_back(value->str());
            }
        }

        m_numMeta = m_metaNames.size();  // number of meta types

#ifndef DARWIN
#ifdef DEBUG
        printFileInformation();
#endif
#endif

        // get channel and variable information to allow the trees to be created
        int numChannels = 0;

        openvdb::GridPtrVec::const_iterator pBegin = m_grid->begin();
        openvdb::GridPtrVec::const_iterator pEnd = m_grid->end();

        while (pBegin != pEnd) {
            if ((*pBegin)) {
                // store channel names and types for each grid
                numChannels++;
                m_variableNames.push_back((*pBegin)->getName());
                m_variableTypes.push_back((*pBegin)->valueType());
            }
            ++pBegin;
        }

        m_numChannels = numChannels;

        // for testing setting the current channel to the length
        m_channel = m_numChannels;
    }
    else {
        std::cerr << "Failed to open VDB file " << _file << "!!!" << std::endl;
    }
}

// taken from reading online documentation and forums on how to load in VDB
// files

bool VDB::loadBasic() {
    if (!m_initialised) {
        std::cerr << "Unable to load data as OpenVDB has not been initialised"
            << std::endl;
        return false;
    }
#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

    if (!m_fileRead) {
        std::cerr << "Unable to load data as no VDB has been opened or read"
            << std::endl;
        return false;
    }

#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

    if (!loadBBox())  // load the bounding box in first - independant
    {
        std::cerr << "Failed to load Bounding Box" << std::endl;
        return false;
    }

#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

    if (!loadVDBTree())  // next load in the VDB tree
    {
        std::cerr << "Failed to load VDB Tree" << std::endl;
        return false;
    }

#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

    std::cout << "All basic data loaded from file..." << std::endl;

    // if everything has gone well and all loaded return positively
    m_loaded = true;

    return true;
}

bool VDB::loadExt() {
    if (!loadMesh())  // load in the high resolution data volume
    {
        std::cerr << "Failed to load VDB Mesh" << std::endl;
        return false;
    }

#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

  // TODO : texture being Created Here
  // TODO
  // create a texture buffer
  // GLuint channelID;
  // glGenBuffers(1, &channelID);

  // glBindBuffer(GL_TEXTURE_BUFFER, channelID);
  //// set sixe of texture buffer and data from reading the grids
  // glBufferData(GL_TEXTURE_BUFFER, m_tboSize * sizeof(openvdb::Vec4f), NULL,
  //             GL_DYNAMIC_DRAW);
  //// create a subbuffer data set
  // glBufferSubData(GL_TEXTURE_BUFFER, 0, m_tboSize * sizeof(openvdb::Vec4f),
  //                &m_channelValueData->at(0)[0]);  // Fill

  // glGenTextures(1, &m_gridsTBO);
  // glActiveTexture(GL_TEXTURE0);
  //// bind the texture
  // glBindTexture(GL_TEXTURE_BUFFER, m_gridsTBO);

  // glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, channelID);

  // // the data stred in the TBO is no longer needed so remove
    m_channelValueData->clear();
    delete m_channelValueData;

#ifdef EXT_GPU
#ifdef NVIDIA
    m_current_available_GPU_mem_kb = currentAvailableGPUMemKB();
#ifdef DEBUG
    std::cout << "Current available GPU memory: "
        << m_current_available_GPU_mem_kb / 1024 << "MB" << std::endl;
#endif
#else
#ifdef DEBUG
    printAMDNotSupported();
#endif
#endif
#else
#ifdef DEBUG
    // printNoExtGPU();
#endif
#endif

    std::cout << "High resolution data loaded..." << std::endl;

    return true;
}

// TODO : Removing Old Mesh Data Happening Here
void VDB::removeMeshVAO() {
    // TODO

    // if (m_vdbGridsInitialized)  // if grids have been initilaised go ahead and
    //                            // remove them
    //{
    //  for (size_t i = 0; i < m_vdbGrids->size(); i++) {
    //    m_vdbGrids->at(i).remove();
    //  }
    //  m_vdbGridsInitialized = false;  // set init to false
    //  m_vdbGrids->clear();
    //  delete m_vdbGrids;  // delete the grids
    //}

    // if (m_extremesInit) {  // if extremes have been initialised and used, clear
    //                       // them
    //  m_channelExtremes->clear();
    //  delete m_channelExtremes;
    //  m_extremesInit = false;
    //}
}

bool VDB::changeLOD() {
    // when a new LOD is set, need to reclalculate the values
    float loadFactor = m_loadPercentFactor * 0.01f;
    int numLoadedPoints =
        (int)((float)m_numPoints.at(m_currentActiveChannelPoints) * loadFactor);
    // prevent any attempt to divide by 0
    if (numLoadedPoints == 0) {
        m_s.at(m_currentActiveChannelPoints) = 0;
    }
    else {
        // calculate and set s for this channel - used for LOD display
        m_s.at(m_currentActiveChannelPoints) = float(
            (m_numPoints.at(m_currentActiveChannelPoints)) / (numLoadedPoints));
    }

    return true;
}

void VDB::setPointChannel(int _channel) {
    m_currentActiveChannelPoints = _channel;
}

void VDB::setVectorChannel(int _channel) {
    m_currentActiveChannelVectors = _channel;
}

int VDB::getNumPointsAtGrid(int _grid) {
    if (_grid >= 0 && _grid < int(m_numPoints.size())) {
        return m_numPoints.at(_grid);
    }
    return -1;
}

// TODO : Very Imp! Drawing of VDB Happening Here
void VDB::drawVDB() {
    // TODO
    // bind VAO and texture buffer and then draw
    /*m_vdbGrids->at(m_currentActiveChannelPoints).bind();

    glBindTexture(GL_TEXTURE_BUFFER, m_gridsTBO);

    m_vdbGrids->at(m_currentActiveChannelPoints).draw();
    m_vdbGrids->at(m_currentActiveChannelPoints).unbind();*/
}

// TODO
// void VDB::drawTree(ShaderLibrary *_shadLib) {
//  // set whether the tree levels are being drawn or not, bind and then draw
//  _shadLib->setShaderParam4f(
//      "loadLevels", float(m_drawTreeLevels[0]), float(m_drawTreeLevels[1]),
//      float(m_drawTreeLevels[2]), float(m_drawTreeLevels[3]));
//  m_vdbTreeVAO->bind();
//  m_vdbTreeVAO->draw();
//  m_vdbTreeVAO->unbind();
//}

// TODO
// void VDB::drawVectors(ShaderLibrary *_shadLib) {
//  // set the colour,, bind the VAO and the texture buffer and then draw
//  _shadLib->setShaderParam4f("u_colour", m_vectorColour.x(),
//  m_vectorColour.y(),
//                             m_vectorColour.z(), 1.0f);
//  m_vdbGrids->at(m_currentActiveChannelVectors).bind();
//  glBindTexture(GL_TEXTURE_BUFFER, m_gridsTBO);
//  m_vdbGrids->at(m_currentActiveChannelVectors).draw();
//  m_vdbGrids->at(m_currentActiveChannelVectors).unbind();
//}

// TODO
// void VDB::drawCrop(ShaderLibrary *_shadLib) {
//  for (int i = 0; i < m_numCropsToDraw; i++) {
//    // draw all currently active crop boxes with their correct colour
//    _shadLib->setShaderParam4f("u_colour", m_crop[i].colour().x(),
//                               m_crop[i].colour().y(), m_crop[i].colour().z(),
//                               1.0f);
//    m_crop[i].draw();
//  }
//}

void VDB::changeLoadPercentFactor(float _delta) {
    m_loadPercentFactor = _delta;
    if (m_loadPercentFactor > 100.0f || m_loadPercentFactor < 0.0f) {
        if (m_loadPercentFactor > 100.0f) {
            m_loadPercentFactor = 100.0f;
        }
        else {
            m_loadPercentFactor = 0.0f;
        }
    }
    changeLOD();  // recalculate LOD
}

void VDB::changeLoadPercentFactor(int _delta) {
    changeLoadPercentFactor((float)_delta);
}

void VDB::setNumCropToDraw(int _n) {
    if (_n > 3 || _n < 0) {
#ifdef DEBUG
        std::cout << "Invalid number of crop boxes set to draw - defaulting to 1"
            << std::endl;
#endif
    }
    m_numCropsToDraw = _n;
}

float VDB::getS() { return m_s.at(m_currentActiveChannelPoints); }

// TODO
void VDB::buildBBox(float _minx, float _maxx, float _miny, float _maxy,
    float _minz, float _maxz) {
    // build the bounding box of the VDB file
    m_bbox = new BoundBox(_minx, _maxx, _miny, _maxy, _minz, _maxz);
    m_bbox->buildVAOIndexed();
}

// TODO
// set the crop box at a specific index multiple ways
// void VDB::setCrop(openvdb::Vec3f _min, openvdb::Vec3f _max, int _index) {
//  m_crop[_index].set(_min.x(), _max.x(), _min.y(), _max.y(), _min.z(),
//                     _max.z());
//  m_crop[_index].buildVAO();
//}

// TODO
// void VDB::setCrop(float _minx, float _maxx, float _miny, float _maxy,
//                  float _minz, float _maxz, int _index) {
//  m_crop[_index].set(_minx, _maxx, _miny, _maxy, _minz, _maxz);
//  m_crop[_index].buildVAO();
//}

// TODO
// void VDB::setCrop(openvdb::Vec3f _centre, float _w, float _h, float _d,
//                  int _index) {
//  m_crop[_index].setCentre(_centre);
//  m_crop[_index].setwdh(_w, _h, _d);
//  m_crop[_index].buildVAO();
//}
// -------------------------------------------------------------

// TODO
// void VDB::setCropW(float _w, int _index) {
//  // set crop width at index
//  m_crop[_index].setWidth(_w);
//  m_crop[_index].buildVAO();
//}

// TODO
// void VDB::setCropH(float _h, int _index) {
//  // set crop height at index
//  m_crop[_index].setHeight(_h);
//  m_crop[_index].buildVAO();
//}
//
// void VDB::setCropD(float _d, int _index) {
//  // set depth width at index
//  m_crop[_index].setDepth(_d);
//  m_crop[_index].buildVAO();
//}
//
// void VDB::setCrop(BoundBox _box, int _index) {
//  // pass in a previous BoundBox and set this as the crop
//  m_crop[_index] = _box;
//  m_crop[_index].buildVAO();
//}

// TODO
// void VDB::setAllCropStorage() {
//  // set all crop boxes to storage
//  for (int i = 0; i < m_numCropsToDraw; i++) {
//    setCropStorage(i);
//  }
//}

// TODO
// void VDB::setCropStorage(int _index) {
//  // set the specified crop box to storage
//  m_cropStorage[_index] = m_crop[_index];
//}

// TODO
// void VDB::returnAllFromStorage() {
//  // return all crop boxes from storage - normally after a scan
//  for (int i = 0; i < m_numCropsToDraw; i++) {
//    returnFromStorage(i);
//  }
//}

// TODO
// void VDB::returnFromStorage(int _index) {
//  // return the specified crop from strorage
//  m_crop[_index].setCentre(m_cropStorage[_index].centre());
//  m_crop[_index].setwdh(m_cropStorage[_index].width(),
//                        m_cropStorage[_index].height(),
//                        m_cropStorage[_index].depth());
//  m_crop[_index].setColour(m_cropStorage[_index].colour());
//}

void VDB::printFileInformation() {
    // print out the version of VDB found in the file
    std::cout << "VDB Version: " << m_fileVersion << std::endl;

    // if metadata exists print it out
    if (m_metadata) {
        std::string out;
        out.clear();
        const openvdb::MetaMap::ConstMetaIterator begin;
        const openvdb::MetaMap::ConstMetaIterator end;

        for (openvdb::MetaMap::ConstMetaIterator it = begin; it != end; ++it) {
            if (it->second) {
                std::string temp = it->second->str();
                if (!temp.empty()) {
                    out += temp;
                    out += " ";
                }
            }
        }

        if (!out.empty()) {
            std::cout << "Print out of retrieved metadata: " << out << std::endl;
        }
    }

    // now print our for each grid found its name, its dimensions and information
    // stored within it
    int index = 0;
    for (openvdb::GridPtrVec::const_iterator it = m_grid->begin();
        it != m_grid->end(); ++it) {
        if (openvdb::GridBase::ConstPtr g = *it) {
            std::cout << "Grid Name: " << m_gridNames->at(index) << std::endl;
            std::cout << "Voxel Dimensions: " << m_gridDims->at(index)[0] << " by "
                << m_gridDims->at(index)[1] << " by "
                << m_gridDims->at(index)[2] << " voxels" << std::endl;
            std::cout << "\nOther metadata: \n" << std::endl;
            g->print(std::cout, 3);
        }
        index++;
    }
}

openvdb::Coord VDB::gridDimAt(int _grid) {
    if (_grid < m_numGrids && _grid >= 0) {
        return m_gridDims->at(_grid);
    }
    else {
        // if not valid, return a null coord
        openvdb::Coord zero;
        zero.setX(0.0f);
        zero.setY(0.0f);
        zero.setZ(0.0f);
        return zero;
    }
}

int VDB::voxelCountAtTreeDepth(int _depth) {
    if (_depth < m_treeDepth && _depth >= 0) {
        return m_levelCounts[_depth];
    }
    else {
        return -1;
    }
}

void VDB::initParams() {
    // initialise data for VDB
    m_numChannels = 0;
    m_numGrids = 0;
    m_gridNames = new std::vector<std::string>;
    m_gridNames->resize(0);
    m_gridDims = new std::vector<openvdb::Coord>;
    m_gridDims->resize(0);

    m_vdbGridsInitialized = false;
    m_extremesInit = false;

    // TODO
    /*m_tboSize = 0*/;

    m_numPoints.resize(1);
    m_numPoints.at(0) = 0;
    m_s.resize(1);
    m_s.at(0) = 0;

    m_currentActiveChannelPoints = m_currentActiveChannelVectors = 0;

    // TODO
    // m_crop[0].setBuildIndexed(false);
    // m_crop[1].setBuildIndexed(false);
    // m_crop[2].setBuildIndexed(false);

    m_initialised = false;
    m_fileOpened = false;
    m_fileRead = false;
    m_loaded = false;
    m_variableNames.resize(0);
    m_variableTypes.resize(0);
    m_channel = 1;
    m_loadPercentFactor = 50;
    m_treeDepth = 0;

    m_vectorSize = 0.5f;
    m_vectorColour = openvdb::Vec3f(0.4, 0.6, 0.8);

    m_levelCounts.resize(0);

    m_numCropsToDraw = 1;

    // TODO
    // m_total_GPU_mem_kb             = 0;
    // m_current_available_GPU_mem_kb = 0;

    for (int i = 0; i < 4; i++) {
        m_drawTreeLevels[i] = 1;
    }
    // ----------------------------
}

void VDB::resetParams() {
    m_variableNames.resize(0);
    m_variableTypes.resize(0);
    m_channel = 1;
    m_treeDepth = 0;

    m_loaded = false;
}

// TODO
// logic of this function taken from studying The GL viewer provided with the
// OpenVDB library
template <typename GridType>
void VDB::getMeshValuesScalar(typename GridType::ConstPtr _grid) {
    typedef typename GridType::ValueType ValueType;
    typedef typename GridType::TreeType TreeType;

    const TreeType& tree = _grid->tree();

    openvdb::tree::ValueAccessor<const TreeType> acc(
        tree);  // accesoor into the file of the correct type

    int j = 0;

    // create Vec4 ready for data for texture buffer
    openvdb::Vec4f channelTemp;
    channelTemp[3] = 1.0f;

    // TODO
    std::vector<vDat> pointStore;  // store point data and normal data
    pointStore.resize(0);

    /*if (pointChannel() >0) {
      pointStore = AllPoints;
    }*/
    bool Creategrid = false;
    if (AllPoints.size() == 0) {
        Creategrid = true;
    }

    vDat point;

    openvdb::Coord coord;  // coord to get from file

    BBoxBare channelExtremes;
    // init extremes ready
    channelExtremes.minx = channelExtremes.miny = channelExtremes.minz = 0.0f;
    channelExtremes.maxx = channelExtremes.maxy = channelExtremes.maxz = 0.0f;

    for (typename GridType::ValueOnCIter it = _grid->cbeginValueOn(); it; ++it) {
        // will always be a rounding issue here as must be an integer step
        // however this could then cause it to miss out a few hundred thousand
        // points instead it will over compensate and draw a couple hundred thousand
        // more prevents a model with a hole in it all caused by rounding issues

        coord = it.getCoord();        // retirve coordinate
        ValueType vec = acc.getValue(coord);  // get vector value (colour)
        openvdb::Vec3d worldSpace =
            _grid->indexToWorld(coord);  // convert coordinate into world space
                                         /* if (pointChannel() == 0) {
                                            point.x = worldSpace[0];
                                            point.y = worldSpace[1];
                                            point.z = worldSpace[2];
                                            point.u = j;
                                          }*/
        point.x = worldSpace[0];
        point.y = worldSpace[1];
        point.z = worldSpace[2];
        point.u = j;
        j++;  // incremenet point count

        if (channelName(pointChannel()) == "density") {
            point.d = (float)vec;
        }

        if (channelName(pointChannel()) == "temperature") {
            float tempVal = log((float)vec) + 273.15;
            point.temp = tempVal;
            glm::vec3 flameColor =
                glm::normalize(glm::vec3(200, 88, 34)) * (float)vec * 1000.f;
            point.cx =
                flameColor[0];  // set colour to normal for rendering on the shader
            point.cy = flameColor[1];
            point.cz = flameColor[2];
        }
        // Give Every Particle color of Smoke
        else {
            glm::vec3 smokeColor =
                glm::normalize(glm::vec3(100, 100, 100)) * (float)vec * 1000.f;
            point.cx =
                smokeColor[0];  // set colour to normal for rendering on the shader
            point.cy = smokeColor[1];
            point.cz = smokeColor[2];
        }

        channelTemp[0] = vec;  // store value for texture buffer
        channelTemp[1] = vec;
        channelTemp[2] = vec;

        point.v = 0;  // type scalar

        m_channelValueData->push_back(channelTemp);  // add to texture store
        m_tboSize++;
        pointStore.push_back(point);  // add to point store

        // check for minimum
        if (point.nx < channelExtremes.minx || point.ny < channelExtremes.miny ||
            point.nz < channelExtremes.minz) {
            if (point.nx < channelExtremes.minx) {
                channelExtremes.minx = point.nx;
            }
            if (point.ny < channelExtremes.miny) {
                channelExtremes.miny = point.ny;
            }
            if (point.nz < channelExtremes.minz) {
                channelExtremes.minz = point.nz;
            }
        }
        // check for maximum
        if (point.nx > channelExtremes.maxx || point.ny > channelExtremes.maxy ||
            point.nz > channelExtremes.maxz) {
            if (point.nx > channelExtremes.maxx) {
                channelExtremes.maxx = point.nx;
            }
            if (point.ny > channelExtremes.maxy) {
                channelExtremes.maxy = point.ny;
            }
            if (point.nz > channelExtremes.maxz) {
                channelExtremes.maxz = point.nz;
            }
        }
    }

    // TODO : Very Imp!!! pointStore are directly Being Pushed in VAO so make sure
    // we do it in vulkan
    // VAO temp(GL_POINTS);
    // temp.create();
    // AllPoints.push_back(pointStore);
    // if (pointChannel() == 0) {
    //  AllPoints.insert(AllPoints.end(), pointStore.begin(), pointStore.end());
    //}
    // else {
    //  AllPoints = pointStore;
    //}
    if (Creategrid) {
        AllPoints.insert(AllPoints.end(), pointStore.begin(), pointStore.end());
    }
    else {
        int size = AllPoints.size() > pointStore.size() ? pointStore.size()
            : AllPoints.size();
        for (int i = 0; i < size; i++) {
            if (channelName(pointChannel()) == "temperature") {
                AllPoints[i].cx = pointStore[i].cx;
                AllPoints[i].cy = pointStore[i].cy;
                AllPoints[i].cz = pointStore[i].cz;
                AllPoints[i].temp = pointStore[i].temp;
            }
        }
    }
    //// create VAO for this grid
    // temp.bind();
    // temp.setIndicesCount(j);
    // temp.setData(pointStore.size() * sizeof(vDat), pointStore.at(0).u);
    // temp.vertexAttribPointer(0, 3, GL_FLOAT, sizeof(vDat), 5);
    // temp.vertexAttribPointer(1, 2, GL_FLOAT, sizeof(vDat), 0);
    // temp.vertexAttribPointer(2, 3, GL_FLOAT, sizeof(vDat), 2, GL_TRUE);
    // temp.unbind();
    // once created, push back into the vectors of VAOs

    // TODO
    /// VAO Object
    // m_vdbGrids->push_back(temp);

    // store the extremes for this channel
    m_channelExtremes->push_back(channelExtremes);

    pointStore.clear();
}

// TODO
// logic of this function taken from studying The GL viewer provided with the
// OpenVDB library this function is idnetical to the one above except it handles
// vector types so has a few differences which are hihglighted
template <typename GridType>
void VDB::getMeshValuesVector(typename GridType::ConstPtr _grid) {
    typedef typename GridType::ValueType ValueType;
    typedef typename GridType::TreeType TreeType;

    const TreeType& tree = _grid->tree();

    openvdb::tree::ValueAccessor<const TreeType> acc(
        tree);  // accessor into grid of the right type

    int j = 0;

    openvdb::Vec4f channelTemp;
    channelTemp[3] = 1.0f;

    std::vector<vDat> pointStore;
    pointStore.resize(0);

    vDat point;

    openvdb::Coord coord;
    bool Creategrid = false;
    if (AllPoints.size() == 0) {
        Creategrid = true;
    }
    BBoxBare channelExtremes;
    channelExtremes.minx = channelExtremes.miny = channelExtremes.minz = 0.0f;
    channelExtremes.maxx = channelExtremes.maxy = channelExtremes.maxz = 0.0f;

    std::string channelname = channelName(pointChannel());

    for (typename GridType::ValueOnCIter it = _grid->cbeginValueOn(); it; ++it) {
        // will always be a rounding issue here as must be an integer step
        // however this could then cause it to miss out a few hundred thousand
        // points instead it will over compensate and draw a couple hundred thousand
        // more prevents a model with a hole in it all caused by rounding issues

        coord = it.getCoord();
        ValueType vec = acc.getValue(coord);
        openvdb::Vec3d worldSpace = _grid->indexToWorld(coord);

        point.x = worldSpace[0];
        point.y = worldSpace[1];
        point.z = worldSpace[2];
        point.u = j;

        j++;

        vec.normalize();  // normalize vector before setting

        if (channelName(pointChannel()) == "v") {
            point.vx = vec[0];
            point.vy = vec[1];
            point.vz = vec[2];
        }

        point.nx = vec[0];
        point.ny = vec[1];
        point.nz = vec[2];

        channelTemp[0] = vec[0];
        channelTemp[1] = vec[1];
        channelTemp[2] = vec[2];

        channelTemp.normalize();  // normalize data again to ensure between 0 and 1

        point.v = 1;  // type is vector - used on the shader

        m_channelValueData->push_back(channelTemp);
        m_tboSize++;
        pointStore.push_back(point);

        // check for minimum
        if (point.nx < channelExtremes.minx || point.ny < channelExtremes.miny ||
            point.nz < channelExtremes.minz) {
            if (point.nx < channelExtremes.minx) {
                channelExtremes.minx = point.nx;
            }
            if (point.ny < channelExtremes.miny) {
                channelExtremes.miny = point.ny;
            }
            if (point.nz < channelExtremes.minz) {
                channelExtremes.minz = point.nz;
            }
        }
        // check for maximum
        if (point.nx > channelExtremes.maxx || point.ny > channelExtremes.maxy ||
            point.nz > channelExtremes.maxz) {
            if (point.nx > channelExtremes.maxx) {
                channelExtremes.maxx = point.nx;
            }
            if (point.ny > channelExtremes.maxy) {
                channelExtremes.maxy = point.ny;
            }
            if (point.nz > channelExtremes.maxz) {
                channelExtremes.maxz = point.nz;
            }
        }
    }
    if (Creategrid) {
        AllPoints.insert(AllPoints.end(), pointStore.begin(), pointStore.end());
    }
    else {
        // Since Grid has already been created for density channelwe will just fetch
        // all points
        int size = AllPoints.size() > pointStore.size() ? pointStore.size()
            : AllPoints.size();
        for (int i = 0; i < size; i++) {
            if (channelName(pointChannel()) == "v") {
                AllPoints[i].vx = pointStore[i].vx;
                AllPoints[i].vy = pointStore[i].vy;
                AllPoints[i].vz = pointStore[i].vz;
            }
        }
    }

    // TODO : Very Imp!!! pointStore are directly Being Pushed in VAO so make sure
    // we do it in vulkan
    // VAO temp(GL_POINTS);
    // temp.create();
    //// bind and create the VAO for this gird
    // temp.bind();
    // temp.setIndicesCount(j);
    // temp.setData(pointStore.size() * sizeof(vDat), pointStore.at(0).u);
    // temp.vertexAttribPointer(0, 3, GL_FLOAT, sizeof(vDat), 5);
    // temp.vertexAttribPointer(1, 2, GL_FLOAT, sizeof(vDat), 0);
    // temp.vertexAttribPointer(2, 3, GL_FLOAT, sizeof(vDat), 2, GL_TRUE);
    // temp.unbind();
    //// store to vector of VAOs
    // m_vdbGrids->push_back(temp);
    //// store extremes for this channel
    // m_channelExtremes->push_back(channelExtremes);

    pointStore.clear();
}

// TODO : Some Stuff
// get the data values for the VDB tree
template <typename GridType>
void VDB::getTreeValues(typename GridType::Ptr _grid) {
    m_treeDepth = _grid->tree().treeDepth();  //  get tree depth

    m_levelCounts.resize(m_treeDepth);

    // first count how many is in each level
    for (typename GridType::TreeType::NodeCIter it = _grid->tree(); it; ++it) {
        m_levelCounts[it.getLevel()]++;  // store the voxel count at each tree depth
    }

    m_totalVoxels = 0;
    for (int i = 0; i < m_treeDepth; i++) {
        m_totalVoxels +=
            m_levelCounts[i];  // calculatye the total voxels in the tree
    }

    // TODO
    /* m_vdbTreeVAO = new VAO(GL_LINES);
     m_vdbTreeVAO->create();*/
     // create VAO for the tree - going to be indexed

    int level = -1;

    // create blueprint elemtns array for each voxel
    static const GLuint elementsBare[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                            6, 7, 7, 4, 4, 0, 1, 5, 7, 3, 6, 2 };

    std::vector<vDat> vertices;
    vertices.resize(0);
    std::vector<GLint> indexes;
    indexes.resize(0);
    int totalVertices = m_totalVoxels * 8;   // 8vertices per voxel
    int totalElements = m_totalVoxels * 24;  // 24 elements per voxel

    openvdb::CoordBBox area;
    openvdb::Vec3f point(0.0f, 0.0f, 0.0f);
    openvdb::Vec3f min(0.0f, 0.0f, 0.0f);
    openvdb::Vec3f max(0.0f, 0.0f, 0.0f);
    openvdb::Vec3f colour(0.0f, 0.0f, 0.0f);
    // TODO
    vDat pointVDat;

    int count = 0;

#ifdef DEBUG
    std::cout << "Getting tree data" << std::endl;
#endif
    for (typename GridType::TreeType::NodeCIter it = _grid->tree(); it; ++it) {
        it.getBoundingBox(area);

        min = _grid->indexToWorld(
            area.min().asVec3s());  // minus 0.5 off to prevent gaps in the voxels
        min[0] -= 0.5;
        min[1] -= 0.5;
        min[2] -= 0.5;
        max = _grid->indexToWorld(
            area.max().asVec3s());  // add 0.5 on to prevent gaps in the voxels
        max[0] += 0.5;
        max[1] += 0.5;
        max[2] += 0.5;

        level = it.getLevel();

        // get colour
        colour = Utilities::getColourFromLevel(level);
        // TODO
        pointVDat.nx = colour.x();  // store colour for this voxel level from pre
                                    // defined function
        pointVDat.ny = colour.y();
        pointVDat.nz = colour.z();
        // get level
        pointVDat.u = level;
        pointVDat.v = level;

        // Rubbish
        /**[0] (minX, minY, maxZ)
         *[1] (maxX, minY, maxZ)
         *[2] (maxX, maxY, maxZ)
         *[3] (minX, maxY, maxZ)
         *[4] (minX, minY, minZ)
         *[5] (maxX, minY, minZ)
         *[6] (maxX, maxY, minZ)
         *[7] (minX, maxY, minZ)*/

         // TODO
         // get and store vertices
        point = openvdb::Vec3f(min.x(), min.y(), max.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(max.x(), min.y(), max.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(max.x(), max.y(), max.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(min.x(), max.y(), max.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(min.x(), min.y(), min.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(max.x(), min.y(), min.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(max.x(), max.y(), min.z());
        pushBackVDBVert(&vertices, point, pointVDat);
        point = openvdb::Vec3f(min.x(), max.y(), min.z());
        pushBackVDBVert(&vertices, point, pointVDat);

        // TODO
        // push back the corresponding element array
        for (int j = 0; j < 24; j++) {
            indexes.push_back((count * 8) + elementsBare[j]);
        }

        ++count;
    }

    // TODO : Very Imp!!! Vertices are directly Being Pushed in VAO so make sure
    // we do it in vulkan
    // m_vdbTreeVAO->bind();
    // m_vdbTreeVAO->setIndexedData(totalVertices * sizeof(vDat), vertices[0].u,
    //                             totalElements, &indexes[0], GL_UNSIGNED_INT);
    // m_vdbTreeVAO->vertexAttribPointer(0, 3, GL_FLOAT, sizeof(vDat), 5);
    // m_vdbTreeVAO->vertexAttribPointer(1, 2, GL_FLOAT, sizeof(vDat), 0);
    // m_vdbTreeVAO->vertexAttribPointer(2, 3, GL_FLOAT, sizeof(vDat), 2);
    // m_vdbTreeVAO->setIndicesCount(totalElements);
    // m_vdbTreeVAO->unbind();

    // TODO
    vertices.clear();

#ifdef DEBUG
    std::cout << "VDB Tree successfully built" << std::endl;
#endif
}

bool VDB::loadBBox() {
    // lets start by calculating the bounding box
    // to build the bounding box will need to find the maximums and minimums
    // can do this using in built openvdb functions - thank you openvdb!!

    openvdb::Vec3d min(std::numeric_limits<double>::max());
    openvdb::Vec3d max(-min);

    // as OpenVDB uses multiple layers of grids, need to scan them all
    for (unsigned int i = 0; i < m_allG.size(); i++) {
        openvdb::CoordBBox b = m_allG[i]->evalActiveVoxelBoundingBox();
        // found this little beauty in the online documentation
        // http://www.openvdb.org/documentation/doxygen/functions_func_0x6d.html#index_m
        min = openvdb::math::minComponent(min, m_allG[i]->indexToWorld(b.min()));
        max = openvdb::math::maxComponent(max, m_allG[i]->indexToWorld(b.max()));
    }

    // TODO
    buildBBox(min.x(), max.x(), min.y(), max.y(), min.z(), max.z());

    return true;
}

bool VDB::loadMesh() {
    // now we need to find out how many points we have in the file
    openvdb::GridPtrVec::const_iterator pBegin = m_grid->begin();
    openvdb::GridPtrVec::const_iterator pEnd = m_grid->end();

    m_numPoints.resize(0);

    while (pBegin != pEnd) {
        if ((*pBegin)) {
            m_numPoints.push_back((*pBegin)->activeVoxelCount());
        }
        ++pBegin;
    }

    // points found

    m_vdbGridsInitialized = true;
    // TODO
    // m_vdbGrids            = new std::vector<VAO>;
    // m_vdbGrids->resize(0);

    // TODO
    m_channelExtremes = new std::vector<BBoxBare>;
    m_channelExtremes->resize(0);
    m_extremesInit = true;

    // now to actually load in the values - for now to test I will load in the
    // colours to the normals vector I have a feeling that in the future I may
    // need to have a couple of VAO's depending on what elements I am drawing or
    // at least some more vectors somewhere to store some data for now I will only
    // load in the positions and colours

    pBegin = m_grid->begin();
    pEnd = m_grid->end();

    m_channel = 0;
    setPointChannel(m_channel);
    // TODO
    m_channelValueData = new std::vector<openvdb::Vec4f>;
    m_channelValueData->resize(0);

    while (pBegin != pEnd) {
        if ((*pBegin)) {
            float loadFactor = m_loadPercentFactor * 0.01f;
            int numLoadedPoints = (int)((float)m_numPoints[m_channel] * loadFactor);

            // prevent any seg faulting from rounding issues from float/int maths
            if (numLoadedPoints > m_numPoints.at(m_channel)) {
                numLoadedPoints = m_numPoints.at(m_channel);
            }

            m_s.push_back(float((m_numPoints.at(m_channel)) / (numLoadedPoints)));
            // TODO
            // work out the type of grid and then get values
            processTypedGrid((*pBegin));
        }
        ++pBegin;
        ++m_channel;
        setPointChannel(m_channel);
    }

    return true;
}

bool VDB::loadVDBTree() {
    openvdb::GridPtrVec::const_iterator pBegin = m_grid->begin();
    openvdb::GridPtrVec::const_iterator pEnd = m_grid->end();

    while (pBegin != pEnd)  // for each grid
    {
        if ((*pBegin)) {
            if ((*pBegin)->getName() == m_variableNames[m_channel - 1]) {
                // work out the type of grid and then get values
                // TODO
                processTypedTree((*pBegin));
            }
        }
        ++pBegin;
    }
    return true;
}

// TODO
void VDB::pushBackVDBVert(std::vector<vDat>* _v, openvdb::Vec3f _point,
    vDat _vert) {
    _vert.x = _point.x();
    _vert.y = _point.y();
    _vert.z = _point.z();
    _v->push_back(_vert);
}

void VDB::reportStringGridTypeError() {
    std::cerr << "File contain grid of type std::string - std::string is "
        "currently not supported, closing application"
        << std::endl;
    exit(EXIT_FAILURE);
}

template <typename GridType>
void VDB::callGetValuesGridScalar(typename GridType::Ptr grid) {
    // call the function to get high res data values with a scalar type
    getMeshValuesScalar<GridType>(grid);
}
//
template <typename GridType>
void VDB::callGetValuesGridVector(typename GridType::Ptr grid) {
    // call the function to get high res data values with a vector type
    getMeshValuesVector<GridType>(grid);
}
//
template <typename GridType>
void VDB::callGetValuesTree(typename GridType::Ptr grid) {
    // call the function to get tree data values
    getTreeValues<GridType>(grid);
}

// inspiration taken from openvdb code examples in doxygen
// http://www.openvdb.org/documentation/doxygen/codeExamples.html

// TODO
//// process the type of grid being passed to it using templates and then call
/// the / correct function for scalar or vector to get high res data values
void VDB::processTypedGrid(openvdb::GridBase::Ptr grid) {
    // scalar types
    if (grid->isType<openvdb::BoolGrid>())
        callGetValuesGridScalar<openvdb::BoolGrid>(
            openvdb::gridPtrCast<openvdb::BoolGrid>(grid));
    else if (grid->isType<openvdb::FloatGrid>())
        callGetValuesGridScalar<openvdb::FloatGrid>(
            openvdb::gridPtrCast<openvdb::FloatGrid>(grid));
    else if (grid->isType<openvdb::DoubleGrid>())
        callGetValuesGridScalar<openvdb::DoubleGrid>(
            openvdb::gridPtrCast<openvdb::DoubleGrid>(grid));
    else if (grid->isType<openvdb::Int32Grid>())
        callGetValuesGridScalar<openvdb::Int32Grid>(
            openvdb::gridPtrCast<openvdb::Int32Grid>(grid));
    else if (grid->isType<openvdb::Int64Grid>())
        callGetValuesGridScalar<openvdb::Int64Grid>(
            openvdb::gridPtrCast<openvdb::Int64Grid>(grid));

    else if (grid->isType<openvdb::MaskGrid>())
        callGetValuesGridScalar<openvdb::MaskGrid>(
            openvdb::gridPtrCast<openvdb::MaskGrid>(grid));

    else if (grid->isType<openvdb::ScalarGrid>())
        callGetValuesGridScalar<openvdb::ScalarGrid>(
            openvdb::gridPtrCast<openvdb::ScalarGrid>(grid));

    else if (grid->isType<openvdb::TopologyGrid>())
        callGetValuesGridScalar<openvdb::TopologyGrid>(
            openvdb::gridPtrCast<openvdb::TopologyGrid>(grid));

    else if (grid->isType<openvdb::VectorGrid>())
        callGetValuesGridVector<openvdb::VectorGrid>(
            openvdb::gridPtrCast<openvdb::VectorGrid>(grid));

    // vector types
    else if (grid->isType<openvdb::Vec3IGrid>())
        callGetValuesGridVector<openvdb::Vec3IGrid>(
            openvdb::gridPtrCast<openvdb::Vec3IGrid>(grid));
    else if (grid->isType<openvdb::Vec3SGrid>())
        callGetValuesGridVector<openvdb::Vec3SGrid>(
            openvdb::gridPtrCast<openvdb::Vec3SGrid>(grid));
    else if (grid->isType<openvdb::Vec3DGrid>())
        callGetValuesGridVector<openvdb::Vec3DGrid>(
            openvdb::gridPtrCast<openvdb::Vec3DGrid>(grid));
    else {
        std::cerr << "File contain grid of type std::string - std::string is "
            "currently not supported, closing application"
            << std::endl;
        /*exit(EXIT_FAILURE);*/
    }
}

// TODO
// process the type of grid being passed to it using templates and then call the
// correct function for scalar or vector to get tree values
void VDB::processTypedTree(openvdb::GridBase::Ptr grid) {
    // scalar types
    if (grid->isType<openvdb::BoolGrid>())
        getTreeValues<openvdb::BoolGrid>(
            openvdb::gridPtrCast<openvdb::BoolGrid>(grid));
    else if (grid->isType<openvdb::FloatGrid>())
        getTreeValues<openvdb::FloatGrid>(
            openvdb::gridPtrCast<openvdb::FloatGrid>(grid));
    else if (grid->isType<openvdb::DoubleGrid>())
        getTreeValues<openvdb::DoubleGrid>(
            openvdb::gridPtrCast<openvdb::DoubleGrid>(grid));
    else if (grid->isType<openvdb::Int32Grid>())
        getTreeValues<openvdb::Int32Grid>(
            openvdb::gridPtrCast<openvdb::Int32Grid>(grid));
    else if (grid->isType<openvdb::Int64Grid>())
        getTreeValues<openvdb::Int64Grid>(
            openvdb::gridPtrCast<openvdb::Int64Grid>(grid));
    // vector types
    else if (grid->isType<openvdb::Vec3IGrid>())
        getTreeValues<openvdb::Vec3IGrid>(
            openvdb::gridPtrCast<openvdb::Vec3IGrid>(grid));
    else if (grid->isType<openvdb::Vec3SGrid>())
        getTreeValues<openvdb::Vec3SGrid>(
            openvdb::gridPtrCast<openvdb::Vec3SGrid>(grid));
    else if (grid->isType<openvdb::Vec3DGrid>())
        getTreeValues<openvdb::Vec3DGrid>(
            openvdb::gridPtrCast<openvdb::Vec3DGrid>(grid));
}
