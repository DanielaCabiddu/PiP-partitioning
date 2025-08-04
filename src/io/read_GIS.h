#ifndef READ_GIS_H
#define READ_GIS_H

#include "gis_data.h"
#include <string>

namespace Urban3D
{

namespace IO
{

void read_GIS (const std::string filename, GISData &gis_data );

}   //end namespace IO
}   //end namespace Urban3D

#ifndef static_lib
#include "read_GIS.cpp"
#endif

#endif
