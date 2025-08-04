/**
 *
 * Daniela Cabiddu
 * daniela.cabiddu@cnr.it
 *
**/

#include "dual_mesh.h"
#include "../utils/edge_center.h"

#include <cinolib/export_mesh_from_clusters.h>

namespace URBAN3D
{

inline
void dual_mesh(const cinolib::Trimesh<> &m, cinolib::Polygonmesh<> &dual_m)
{
    std::vector<cinolib::vec3d>    dual_verts;
    std::vector<std::vector<uint>> dual_polys;

    cinolib::Trimesh<> dual_tri;

    for (uint pid=0; pid < m.num_polys(); pid++)
        dual_m.vert_add(m.poly_centroid(pid));

    for (uint pid=0; pid < m.num_polys(); pid++)
        for (uint adj : m.adj_p2p(pid))
        {
            if (pid < adj)
            {
                uint shared_eid = m.edge_shared(pid, adj);

                int vid = dual_m.vert_add(URBAN3D::edge_center(m.edge_vert(shared_eid, 0), m.edge_vert(shared_eid, 1)));

                int eid = dual_m.edge_add(pid, vid);
                dual_m.edge_data(eid).flags[cinolib::MARKED] = true;

                eid = dual_m.edge_add(adj, vid);
                dual_m.edge_data(eid).flags[cinolib::MARKED] = true;
            }
        }

    std::cout << dual_m.num_verts() << " \\ " << dual_m.num_edges() << " \\ " << dual_m.num_polys() << std::endl;

    cinolib::triangle_wrap(dual_m.vector_verts(), dual_m.vector_edges(), std::vector<cinolib::vec3d>(), 0.0, "", dual_tri);

    for (uint eid=0; eid < dual_tri.num_edges(); eid++)
    {
        uint vid0=dual_tri.edge_vert_id(eid,0);
        uint vid1=dual_tri.edge_vert_id(eid,1);

        bool exists = false;

        for (uint e : dual_m.adj_v2e(vid0))
            if (dual_m.edge_contains_vert(e, vid1))
            {
                exists=true;
                dual_tri.edge_data(eid).flags[cinolib::MARKED] = true;
                break;
            }
    }

    std::vector<std::vector<uint>> polys;

    for (uint pid=0; pid < dual_tri.num_polys(); pid++)
    {
        if (dual_tri.poly_data(pid).label > -1) continue;

        std::queue<uint> pids;
        pids.push(pid);

        std::vector<uint> poly;
        poly.push_back(pid);
        dual_tri.poly_data(pid).label = polys.size();

        while (!pids.empty())
        {
            uint curr_pid = pids.front();
            pids.pop();

            for (uint adj_pid : dual_tri.adj_p2p(curr_pid))
            {
                if (dual_tri.edge_data(dual_tri.edge_shared(curr_pid, adj_pid)).flags[cinolib::MARKED] == true)
                    continue;

                if (dual_tri.poly_data(adj_pid).label > -1)
                    continue;

                poly.push_back(adj_pid);
                dual_tri.poly_data(adj_pid).label = polys.size();

                pids.push(adj_pid);
            }
        }

        polys.push_back(poly);
    }

    for (uint p=0; p < polys.size(); p++)
    {
        cinolib::export_mesh_from_clusters(dual_tri, dual_m);
    }
}

}
