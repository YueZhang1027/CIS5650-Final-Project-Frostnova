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
#include <glm/glm.hpp>


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

    m_loaded = false;

    delete m_gridDims;
    delete m_gridNames;

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
        if (!m_grid->empty()) 
        {
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
        else 
        {
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
        if (m_metadata != NULL) 
        {
#ifdef DEBUG
            std::cout << "Metadata found in file" << std::endl;
#endif
        }
        else 
        {
            std::cerr << "Failed to find metadata!!" << std::endl;
        }

        m_fileVersion = vdbFile.version();  // get the file version
        if (m_fileVersion != "") 
        {
#ifdef DEBUG
            std::cout << "File version found in file" << std::endl;
#endif
        }
        else 
        {
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

bool VDB::loadBasic()
{
    if (!m_initialised) {
        std::cerr << "Unable to load data as OpenVDB has not been initialised"
            << std::endl;
        return false;
    }

    if (!m_fileRead) {
        std::cerr << "Unable to load data as no VDB has been opened or read"
            << std::endl;
        return false;
    }

    if (!loadBBox())  // load the bounding box in first - independant
    {
        std::cerr << "Failed to load Bounding Box" << std::endl;
        return false;
    }


    if (!loadVDBTree())  // next load in the VDB tree
    {
        std::cerr << "Failed to load VDB Tree" << std::endl;
        return false;
    }

    std::cout << "All basic data loaded from file..." << std::endl;

    // if everything has gone well and all loaded return positively
    m_loaded = true;

    return true;
}

bool VDB::loadExt()
{
    if (!loadMesh())  // load in the high resolution data volume
    {
        std::cerr << "Failed to load VDB Mesh" << std::endl;
        return false;
    }

    if (m_channelValueData) 
    {
        m_channelValueData->clear();
        delete m_channelValueData;
    }

    std::cout << "High resolution data loaded..." << std::endl;

    return true;
}

void VDB::initParams()
{
    m_numChannels = 0;
    m_numGrids = 0;
    m_gridNames = new std::vector<std::string>;
    m_gridNames->resize(0);
    m_gridDims = new std::vector<openvdb::Coord>;
    m_gridDims->resize(0);

    m_vdbGridsInitialized = false;
    m_extremesInit = false;

    m_numPoints.resize(1);
    m_numPoints.at(0) = 0;
    m_s.resize(1);
    m_s.at(0) = 0;

    m_currentActiveChannelPoints = m_currentActiveChannelVectors = 0;

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

    for (int i = 0; i < 4; i++) {
        m_drawTreeLevels[i] = 1;
    }
}

bool VDB::loadBBox()
{
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

bool VDB::loadVDBTree()
{
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

void VDB::pushBackVDBVert(std::vector<vDat>* _v, openvdb::Vec3f _point, vDat _vert)
{
    _vert.x = _point.x();
    _vert.y = _point.y();
    _vert.z = _point.z();
    _v->push_back(_vert);
}

void VDB::processTypedTree(openvdb::GridBase::Ptr grid)
{
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

void VDB::buildBBox(float _minx, float _maxx, float _miny, float _maxy, float _minz, float _maxz)
{
    m_bbox = new BoundBox(_minx, _maxx, _miny, _maxy, _minz, _maxz);
    m_bbox->buildVAOIndexed();
}

template<typename GridType>
inline void VDB::getTreeValues(typename GridType::Ptr _grid)
{
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