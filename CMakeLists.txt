cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

if(${ESP_PLATFORM})
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    # Include core as a component
    set(EXTRA_COMPONENT_DIRS
            externals/core/smooth_component)
    project(street_lights_esp32s3)
else()
    if(${APPLE})
    include_directories(SYSTEM /usr/local/include)
    link_directories(/usr/local/lib)
    endif()
    add_subdirectory(main)
    add_subdirectory(externals/core/lib)
    add_subdirectory(externals/core/mock-idf)
endif()