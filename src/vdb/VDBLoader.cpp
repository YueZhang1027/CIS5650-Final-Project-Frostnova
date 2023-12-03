#include "VDBLoader.h"
#include <iostream>
#include <filesystem>

void VDBLoader::Load(const std::string filename) 
{
    namespace fs = std::filesystem;

    const fs::path src_dir = fs::path(PROJECT_DIRECTORY);
    const std::string file = (src_dir / filename).string();

    // load the VDB file
    vdb_ = std::make_unique<VDB>(file);

    // load the basic information from the file
    if (vdb_->loadBasic()) 
    {
        is_basic_loaded_ = true;
    }
    else 
    {
        std::cout << "Basic not loaded" << std::endl;
    }

    // if it has been chosen to load high resolution, load
    if (vdb_->loadExt()) 
    {
        is_detail_loaded_ = true;
    }
    else 
    {
        std::cout << "Details not loaded" << std::endl;
    }

    is_vdb_loaded_ = true;
}