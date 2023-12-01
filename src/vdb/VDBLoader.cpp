#include "VDBLoader.h"

void VDBLoader::Load(const std::string filename) {

    // load the VDB file
    vdb_ = std::make_unique<VDB>(filename);

    // load the basic information from the file
    if (vdb_->loadBasic()) 
    {
        is_basic_loaded_ = true;
    }

    // if it has been chosen to load high resolution, load
    if (vdb_->loadExt()) 
    {
        is_detail_loaded_ = true;
    }

    is_vdb_loaded_ = true;
}