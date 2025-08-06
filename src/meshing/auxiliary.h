/**
 *
 * Tommaso Sorgente
 * tommaso.sorgente@cnr.it
 *
**/

#ifndef CITY_AUXILIARY
#define CITY_AUXILIARY

#include <cinolib/meshes/meshes.h>
#include <cinolib/triangle_wrap.h>

double TOLL = 1e-6;

inline
cinolib::Trimesh<> triangulate_with_holes(const cinolib::Polygonmesh<> &m, const std::vector<uint> &holes)
{
    std::vector<cinolib::vec2d> verts_in;
    for(uint vid=0; vid<m.num_verts(); ++vid) {
        verts_in.push_back(cinolib::vec2d(m.vert(vid).x(), m.vert(vid).y()));
    }

    std::vector<uint>  edges_in;
    for(uint eid=0; eid<m.num_edges(); ++eid) {
        edges_in.push_back(m.edge_vert_id(eid,0));
        edges_in.push_back(m.edge_vert_id(eid,1));
    }

    std::vector<cinolib::vec2d> holes_centers;
    for(uint pid : holes) {
        std::vector<uint> tess = m.poly_tessellation(pid);
        cinolib::vec3d v0 = m.vert(tess.at(0));
        cinolib::vec3d v1 = m.vert(tess.at(1));
        cinolib::vec3d v2 = m.vert(tess.at(2));
        cinolib::vec3d c = (v0 + v1 + v2) / 3.;
        holes_centers.push_back(cinolib::vec2d(c.x(), c.y()));
    }

    cinolib::Trimesh<> m_tri;
    std::string t_flags = "QY";
    // double area = 10. * pow(m.edge_avg_length(), 2);
    // t_flags += "qa" + std::to_string(area);
    triangle_wrap(verts_in, edges_in, holes_centers, 0., t_flags.c_str(), m_tri);
    return m_tri;
}


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
