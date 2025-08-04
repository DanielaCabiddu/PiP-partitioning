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
bool point_in_polygon(const cinolib::Polygonmesh<> &m, const cinolib::vec3d p, const uint pid) {
    bool inside = false;
    std::vector<std::vector<uint>> tris = cinolib::polys_from_serialized_vids(m.poly_tessellation(pid), 3);
    for(std::vector<uint> t : tris) {
        cinolib::vec3d t0 = m.vert(t[0]);
        cinolib::vec3d t1 = m.vert(t[1]);
        cinolib::vec3d t2 = m.vert(t[2]);
        if (point_in_triangle_3d(p, t0, t1, t2)) {
            inside = true;
            break;
        }
    }
    return inside;
}

#endif // CITY_AUXILIARY
