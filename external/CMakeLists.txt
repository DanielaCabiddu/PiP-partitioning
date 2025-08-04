##cmake_minimum_required(VERSION 3.16)

# project( LANGUAGES CXX)

#####################################################################################################

if(MSVC)

    if(NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})

        set(VCPKG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_cpptruths)

        if(NOT EXISTS ${VCPKG_ROOT})
            message("Cloning vcpkg in ${VCPKG_ROOT}")
            execute_process(COMMAND git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT})
            # If a reproducible build is desired (and potentially old libraries are # ok), uncomment the
            # following line and pin the vcpkg repository to a specific githash.
            # execute_process(COMMAND git checkout 745a0aea597771a580d0b0f4886ea1e3a94dbca6 WORKING_DIRECTORY ${VCPKG_ROOT})
        else()
            # The following command has no effect if the vcpkg repository is in a detached head state.
            message("Auto-updating vcpkg in ${VCPKG_ROOT}")
            execute_process(COMMAND git pull WORKING_DIRECTORY ${VCPKG_ROOT})
        endif()

        message (${VCPKG_ROOT}/README.md)

        if(NOT EXISTS ${VCPKG_ROOT}/README.md)
            message(FATAL_ERROR "***** FATAL ERROR: Could not clone vcpkg *****")
        endif()

        if(WIN32)
            set(BOOST_INCLUDEDIR ${VCPKG_ROOT}/installed/x86-windows/include)
            set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg.exe)
            set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.bat)
        else()
            set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg)
            set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.sh)
        endif()

        if(NOT EXISTS ${VCPKG_EXEC})
            message("Bootstrapping vcpkg in ${VCPKG_ROOT}")
            execute_process(COMMAND ${VCPKG_BOOTSTRAP} WORKING_DIRECTORY ${VCPKG_ROOT})
        endif()

        if(NOT EXISTS ${VCPKG_EXEC})
            message(FATAL_ERROR "***** FATAL ERROR: Could not bootstrap vcpkg *****")
        endif()

        set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

        #message(STATUS "***** Checking project third party dependencies in ${VCPKG_ROOT} *****")
        execute_process(
                    COMMAND ${VCPKG_EXEC} install sqlite3[core,tool] --triplet x64-windows --recurse
                    WORKING_DIRECTORY ${VCPKG_ROOT})

        execute_process(
                    COMMAND ${VCPKG_EXEC} install liblas --triplet x64-windows --recurse
                    WORKING_DIRECTORY ${VCPKG_ROOT})

        execute_process(
                    COMMAND ${VCPKG_EXEC} install curl --triplet x64-windows --recurse
                    WORKING_DIRECTORY ${VCPKG_ROOT})

        set (SQLite3_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include" )
        set (SQLite3_LIBRARY "${SQLite3_INCLUDE_DIR}/../lib/sqlite3.lib" )
        set (SQLite3_BINARY "${SQLite3_INCLUDE_DIR}/../tools/sqlite3.exe" )

        set (libLAS_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include" )
        set (libLAS_LIBRARY "${SQLite3_INCLUDE_DIR}/../lib/liblas.lib" )

        set (CURL_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include" )
        set (CURL_LIBRARY "${CURL_INCLUDE_DIR}/../lib/libcurl.lib" )

    endif()
endif()

#####################################################################################################
#proj

set (PROJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/proj)
set (PROJ_BUILD ${PROJ_DIR}/build-${CMAKE_SYSTEM_NAME})
set (PROJ_INSTALL ${PROJ_DIR}/install-${CMAKE_SYSTEM_NAME})

file(MAKE_DIRECTORY ${PROJ_BUILD})
file(MAKE_DIRECTORY ${PROJ_INSTALL})

if(APPLE)
    set (PROJ_LIB ${PROJ_INSTALL}/lib/libproj.25.dylib)
elseif(UNIX)
    set (PROJ_LIB ${PROJ_INSTALL}/lib/libproj.so)
else()
    set (PROJ_LIB ${PROJ_INSTALL}/lib/proj.lib)
endif()

set(PROJ_H ${PROJ_INSTALL}/include)

#####################################################################################################
##gdal

set (GDAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/gdal)
set (GDAL_BUILD ${GDAL_DIR}/build-${CMAKE_SYSTEM_NAME})
set (GDAL_INSTALL ${GDAL_DIR}/install-${CMAKE_SYSTEM_NAME})

set (GDAL_LIB ${GDAL_INSTALL}/lib/libgdal.so)

file(MAKE_DIRECTORY ${GDAL_BUILD})
file(MAKE_DIRECTORY ${GDAL_INSTALL})

if (APPLE)
    set (GDAL_LIB ${GDAL_INSTALL}/lib/libgdal.dylib)
endif()

if (MSVC)

    set (GDAL_LIB ${GDAL_INSTALL}/lib/gdal.lib)

    add_custom_command(
            OUTPUT ${GDAL_LIB}
            COMMAND cmake -DEXE_SQLITE3=${SQLite3_BINARY} -DSQLITE3_INCLUDE_DIR=${SQLite3_INCLUDE_DIR} -DSQLITE3_LIBRARY=${SQLite3_LIBRARY} -DBUILD_APPS=OFF -DENABLE_CURL=OFF -DENABLE_TIFF=OFF -DCMAKE_INSTALL_PREFIX=${PROJ_INSTALL} .. && cmake --build . --config Release && cmake --install . && cd ${GDAL_BUILD} && cmake .. -DCMAKE_INSTALL_PREFIX=${GDAL_INSTALL} -DGDAL_BUILD_OPTIONAL_DRIVERS=OFF -DOGR_BUILD_OPTIONAL_DRIVERS=OFF -DPROJ_INCLUDE_DIR=${PROJ_INSTALL}/include -DPROJ_LIBRARY_RELEASE=${PROJ_LIB} -DBUILD_PYTHON_BINDINGS=OFF && cmake --build . --config Release --parallel 16 && cmake --install .
            DEPENDS ${GDAL_INSTALL}/bin/gdal.exe
            WORKING_DIRECTORY ${PROJ_BUILD}
    )
else()
    add_custom_command(
            OUTPUT ${GDAL_LIB}
            COMMAND cmake -DBUILD_APPS=OFF -DENABLE_CURL=OFF -DENABLE_TIFF=OFF -DCMAKE_INSTALL_PREFIX=${PROJ_INSTALL} .. && cmake --build . --parallel 16 && cmake --install . && cd ${GDAL_BUILD} && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${GDAL_INSTALL} -DGDAL_BUILD_OPTIONAL_DRIVERS=OFF -DOGR_BUILD_OPTIONAL_DRIVERS=OFF -DPROJ_INCLUDE_DIR=${PROJ_INSTALL}/include -DPROJ_LIBRARY_RELEASE=${PROJ_LIB} -DBUILD_PYTHON_BINDINGS=OFF && cmake --build . --parallel 16 && cmake --install .
            #DEPENDS ${GDAL_INSTALL}/bin/gdal
            WORKING_DIRECTORY ${PROJ_BUILD}
    )
endif()

include_directories(${GDAL_INSTALL}/include)

#####################################################################################################
##liblas

if (NOT MSVC)

    set (LIBLAS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/libLAS)
    set (LIBLAS_BUILD ${LIBLAS_DIR}/build-${CMAKE_SYSTEM_NAME})
    set (LIBLAS_INSTALL ${LIBLAS_DIR}/install-${CMAKE_SYSTEM_NAME})

    set (LIBLAS_LIB ${LIBLAS_INSTALL}/lib/liblas.so)

    file(MAKE_DIRECTORY ${LIBLAS_BUILD})
    file(MAKE_DIRECTORY ${LIBLAS_INSTALL})

    if (APPLE)
        set (LIBLAS_LIB ${LIBLAS_INSTALL}/lib/liblas.dylib)
    endif()

    add_custom_command(
        OUTPUT ${LIBLAS_LIB}
        COMMAND cmake -DCMAKE_INSTALL_PREFIX=${LIBLAS_INSTALL} -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --parallel 16 && cmake --install . && cmake --build . --parallel 16 && cmake --install .
        #DEPENDS ${GDAL_INSTALL}/bin/gdal
        WORKING_DIRECTORY ${LIBLAS_BUILD}
    )

    set (libLAS_INCLUDE_DIR ${LIBLAS_INSTALL}/include )
    set (libLAS_LIBRARY ${LIBLAS_LIB} )

endif()

#####################################################################################################
##curl

if (NOT MSVC)

    find_package(CURL REQUIRED)

    set (CURL_INCLUDE_DIR ${CURL_INCLUDE_DIRS} )
    set (CURL_LIBRARY ${CURL_LIBRARIES})

endif()

#####################################################################################################


if (MSVC)
    file (GLOB MY_PROJ_DLLs ${PROJ_INSTALL}/bin/*.dll    )
    file (GLOB MY_GDAL_DLLs ${GDAL_INSTALL}/bin/*.dll    )
    file (GLOB MY_ZLIB_DLLs ${ZLIB_INSTALL}/bin/*.dll    )
    file (GLOB MY_LIBZIP_DLLs ${LIBZIP_INSTALL}/bin/*.dll    )
    file (GLOB MY_SQLITE_DLLs ${SQLite3_INCLUDE_DIR}/../bin/*.dll     )
    file (COPY ${MY_PROJ_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file (COPY ${MY_GDAL_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file (COPY ${MY_SQLITE_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file (COPY ${MY_ZLIB_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file (COPY ${MY_LIBZIP_DLLs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_custom_target(UrbanServices-Dependences ALL DEPENDS ${GDAL_LIB} ${LIBLAS_LIB} ${LIBZIP_LIB} )
