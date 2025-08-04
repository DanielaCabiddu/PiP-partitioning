#include "edge_center.h"

namespace URBAN3D
{

    inline
    cinolib::vec3d edge_center(const cinolib::vec3d &v0, const cinolib::vec3d &v1)
    {
        return cinolib::vec3d( (v0.x() + v1.x()) / 2.0, (v0.y() + v1.y()) / 2.0, (v0.z() + v1.z()) / 2.0);
    }

} // END NAMESPACE
