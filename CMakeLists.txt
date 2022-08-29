cmake_minimum_required(VERSION 3.8)

project(vklib)

set(CMAKE_C_STANDARD 99)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_BUILD_TYPE Release)

set(DEFINITIONS
    VKLIB_FILESYSTEM
    VKLIB_MONGO
)

FILE(GLOB MONGO_SOURCES src/vklib_mongo/*)

if (WIN32)
    set(SOURCES
        src/vklib.c src/vklib.h
        src/vklib_fs.c src/vklib_fs.h
        src/vklib_compat.c src/vklib_compat.h
        src/dirent.h
    )
else()
    set(SOURCES
        src/vklib.c src/vklib.h
        src/vklib_fs.c src/vklib_fs.h
        src/vklib_compat.c src/vklib_compat.h
    )
endif()

# Definitions
add_compile_definitions(${DEFINITIONS})

link_directories(${CMAKE_SOURCE_DIR}/dep/lua/lib/ ${CMAKE_SOURCE_DIR}/dep/mongodb/lib)
add_library(vklib SHARED ${SOURCES} ${MONGO_SOURCES})

# Set Properties
SET_TARGET_PROPERTIES(vklib PROPERTIES PREFIX "")
set_target_properties(vklib PROPERTIES LINKER_LANGUAGE C)

# Set Include Directories
target_include_directories(vklib PUBLIC src/ dep/lua/include dep/mongodb/include)

# Link Libraries
if (WIN32)
    target_link_libraries(vklib lua bson-1.0.lib mongoc-1.0.lib)
else()
    target_link_libraries(vklib lua libbson-1.0.so libmongoc-1.0.so)
endif()