#include "read_GIS.h"

#include <ogrsf_frmts.h>

#include <fstream>
#include <iostream>

namespace Urban3D
{

namespace IO
{

inline
void read_GIS (const std::string filename, GISData &gis_data )
{
    int flags = GDAL_OF_VECTOR;
    gis_data.read(filename, flags);
}

}   //end namespace IO
}   //end namespace Urban3D
