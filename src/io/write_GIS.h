#ifndef WRITE_GIS_H
#define WRITE_GIS_H

#include "gis_data.h"
#include <string>

namespace URBAN3D
{

void write_GIS (const std::string filename, const GISData &gis_data );

} // END namespace

#ifndef static_lib
#include "write_GIS.cpp"
#endif

#endif
