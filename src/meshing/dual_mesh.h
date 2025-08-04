/**
 *
 * Daniela Cabiddu
 * daniela.cabiddu@cnr.it
 *
**/

#ifndef DUAL_MESH_H
#define DUAL_MESH_H

#include <cinolib/meshes/meshes.h>
#include <cinolib/triangle_wrap.h>

namespace URBAN3D
{

void dual_mesh(const cinolib::Trimesh<> &m, cinolib::Polygonmesh<> &dual_m);

}

#ifndef static_lib
#include "dual_mesh.cpp"
#endif


#endif // DUAL_MESH_H
