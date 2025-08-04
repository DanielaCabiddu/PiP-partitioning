#ifndef EDGE_CENTER_H
#define EDGE_CENTER_H

#include <cinolib/geometry/vec_mat.h>

namespace URBAN3D
{

    cinolib::vec3d edge_center(const cinolib::vec3d &v0, const cinolib::vec3d &v1);

}

#ifndef static_lib
#include "edge_center.cpp"
#endif

#endif
