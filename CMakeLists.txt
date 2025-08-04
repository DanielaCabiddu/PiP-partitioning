cmake_minimum_required(VERSION 3.16)

project(PBF-FR-partitioning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (USE_VTK OFF)
set (USE_OPENMP ON)

set (USE_CINOLIB_GUI ON)
set (CINOLIB_USES_TRIANGLE    ON)

#########################################################

# make a bin folder to host all the executables
make_directory(${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# include_directories(${urban_INCLUDE_DIR})

#########################################################

##Cinolib

if (USE_CINOLIB_GUI)
    add_definitions(-DUSE_CINOLIB_GUI)
    set(CINOLIB_USES_OPENGL_GLFW_IMGUI    ON)
endif()

set (cinolib_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/cinolib)
find_package(cinolib REQUIRED)
link_libraries(cinolib)

include_directories(${cinolib_DIR}/include)
include_directories(${cinolib_DIR}/external/eigen)

#########################################################

## Deps
include(${CMAKE_CURRENT_SOURCE_DIR}/external/CMakeLists.txt)

#########################################################

if (USE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

#########################################################

## TCLAP
set (tclap_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tclap)
include_directories(${tclap_DIR}/include)

#########################################################


add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${GDAL_LIB})

if (MSVC)
    file (GLOB MY_PROJ_DLLs ${PROJ_INSTALL}/bin/*.dll    )
    file (GLOB MY_GDAL_DLLs ${GDAL_INSTALL}/bin/*.dll    )
    file (GLOB MY_SQLITE_DLLs ${SQLite3_INCLUDE_DIR}/../bin/*.dll     )
    # file (COPY ${MY_PROJ_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    # file (COPY ${MY_GDAL_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    # file (COPY ${MY_SQLITE_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
