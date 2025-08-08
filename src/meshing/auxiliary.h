#ifndef CITY_AUXILIARY
#define CITY_AUXILIARY

#include <cinolib/meshes/meshes.h>


inline
bool point_in_polygon(const cinolib::Polygonmesh<> &m, const cinolib::vec3d p, const uint pid)
{
    int i, j, c = 0;

    int nvert = m.poly_verts(pid).size();

    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double maxx = -DBL_MAX;
    double maxy = -DBL_MAX;

    for (unsigned int vid : m.poly_verts_id(pid))
    {
        cinolib::vec3d v = m.vert(vid);
        if (v.x() < minx) minx = v.x();
        if (v.y() < miny) miny = v.y();
        if (v.x() > maxx) maxx = v.x();
        if (v.y() > maxy) maxy = v.y();
    }

    for (i = 0, j = nvert - 1; i < nvert; j = i++) {

        bool bbout = false;

        // Check if the point is outside the bounding box of the polygon
        if (p.x() < minx || p.y() < miny ||
            p.x() > maxx || p.y() > maxy )
            bbout = true;

        // Check if the point is inside the polygon using the ray-casting algorithm
        if (!bbout && ((m.poly_vert(pid,i).y() > p.y()) != (m.poly_vert(pid,j).y() > p.y())) &&
            (p.x() < (m.poly_vert(pid,j).x() - m.poly_vert(pid,i).x()) * (p.y() - m.poly_vert(pid,i).y()) / (m.poly_vert(pid,j).y() - m.poly_vert(pid,i).y()) + m.poly_vert(pid,i).x()))
            c = !c;
    }

    return c;//!(c == 0);

}

#endif // CITY_AUXILIARY
