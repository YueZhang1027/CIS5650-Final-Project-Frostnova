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
