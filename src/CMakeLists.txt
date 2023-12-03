set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderprogram/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderprogram/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vdb/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vdb/*.h

)

file(GLOB IMAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/*.bmp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/hiResCloudShape/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/images/lowResCloudShape/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/images/vdb/example1/tga/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/images/vdb/example2/tga/*.tga
    ${CMAKE_CURRENT_SOURCE_DIR}/images/vdb/example1/*.vdb
    ${CMAKE_CURRENT_SOURCE_DIR}/images/vdb/example2/*.vdb
)

foreach(IMAGE ${IMAGES})
    get_filename_component(fname ${IMAGE} NAME)
    configure_file(${IMAGE} ${CMAKE_CURRENT_BINARY_DIR}/images/${fname} COPYONLY)
endforeach()

file(GLOB_RECURSE SHADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/*.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/*.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.tese
    ${CMAKE_CURRENT_SOURCE_DIR}/*.tesc
)

source_group("Shaders" FILES ${SHADER_SOURCES})

if(WIN32)
    add_executable(vulkan_volumetric_cloud WIN32 ${SOURCES} ${SHADER_SOURCES})
    target_link_libraries(vulkan_volumetric_cloud ${WINLIBS})
else(WIN32)
    add_executable(vulkan_volumetric_cloud ${SOURCES})
    target_link_libraries(vulkan_volumetric_cloud ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
    set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    if(WIN32)
        get_filename_component(fname ${SHADER_SOURCE} NAME)
        add_custom_target(${fname}.spv
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR} && 
            $ENV{VK_SDK_PATH}/Bin/glslangValidator.exe -V ${SHADER_SOURCE} -o ${SHADER_DIR}/${fname}.spv -g
            SOURCES ${SHADER_SOURCE}
        )
        ExternalTarget("Shaders" ${fname}.spv)
        add_dependencies(vulkan_volumetric_cloud ${fname}.spv)
    endif(WIN32)

    # TODO: Build shaders on not windows
endforeach()

target_link_libraries(vulkan_volumetric_cloud ${ASSIMP_LIBRARIES} Vulkan::Vulkan glfw)
target_include_directories(vulkan_volumetric_cloud PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLM_INCLUDE_DIR}
  ${STB_INCLUDE_DIR}
)

InternalTarget("" vulkan_volumetric_cloud)
