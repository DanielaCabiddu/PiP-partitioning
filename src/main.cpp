/********************************************************************************
 *
 *  This file is part of Urban3D
 *  Copyright(C) 2025: Daniela Cabiddu
 *
 *  Author(s):
 *
 *  Tommaso Sorgente [tommaso.sorgente@cnr.it]
 *  Daniela Cabiddu [daniela.cabiddu@cnr.it]
 *
 ********************************************************************************/

#include "meshing/auxiliary.h"
#include "io/read_GIS.h"
#include "io/write_GIS.h"

#include "meshing/dual_mesh.h"

// #include "urban3D/utils/point_in_polygon.h"

#include <cinolib/meshes/meshes.h>
#include <cinolib/merge_meshes_at_coincident_vertices.h>

#ifdef USE_CINOLIB_GUI
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/surface_mesh_controls.h>
#endif

#include <tclap/CmdLine.h>
#include <liblas/liblas.hpp>

#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    std::string external_boundary_path   ;
    std::string internal_boundaries_path ;
    std::string las_path  = ""; // Not used in this example, but kept for consistency

    std::string output_path;

    uint boundary_epsg;

    try
    {
        // Define command line parser and arguments
        TCLAP::CmdLine cmd("Urban-PBF", ' ', "version 0.5");

        // Define main functionalities options
        TCLAP::ValueArg<std::string> external_arg("e", "external", "External Boundary", true, "name_ground", "string", cmd);
        TCLAP::ValueArg<std::string> internal_arg("i", "internal", "Internal Boundaries file", true, "name_pav", "string", cmd);

        TCLAP::ValueArg<std::string> pc_arg("l", "las", "Point Cloud (LAS)", false, "name_pav", "string", cmd);

        TCLAP::ValueArg<std::string> o_arg("o", "output", "Output File ", true, "name_out", "string", cmd);

        // Parse the argv array
        cmd.parse(argc, argv);

        external_boundary_path = external_arg.getValue();
        internal_boundaries_path = internal_arg.getValue();

        if (pc_arg.isSet())
            las_path = pc_arg.getValue();

        output_path = o_arg.getValue();

    }
    catch (TCLAP::ArgException &e) // catch exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-3);
    }

    // if (!fs::exists(pavements_path)) {
    //     std::cerr << "the buildings folder does not exist" << std::endl;
    //     assert(false);
    // }

    cinolib::Polygonmesh<> external_boundary;

    if (fs::path(external_boundary_path).extension().compare(".off") == 0)
        external_boundary.load(external_boundary_path.c_str());
    else
        if (fs::path(external_boundary_path).extension().compare(".shp") == 0)
        {
            GISData gis_data;
            Urban3D::IO::read_GIS(external_boundary_path, gis_data);
            boundary_epsg = gis_data.get_epsg();

            external_boundary = gis_data.convert_to_polygon_mesh();
        }
        else
        {
            std::cerr << "Unsupported input format : " << external_boundary_path << std::endl;
            exit(1);
        }

    cinolib::vec3d c = external_boundary.centroid();
    std::map<cinolib::vec3d, double> z_coordinates;
    std::vector<uint> buildings_holes;
    std::vector<std::string> outside_buildings;

    // translate the ground to the origin and project to the plane z=0
    external_boundary.translate(-c);
    for (cinolib::vec3d &v : external_boundary.vector_verts()) {
        double tmp = v.z();
        v.z() = 0.;
        z_coordinates[v] = tmp;
    }
    external_boundary.update_bbox();

    cinolib::Polygonmesh<> buildings;

    if (fs::path(internal_boundaries_path).extension().compare(".off") == 0)
        buildings.load(internal_boundaries_path.c_str());
    else
    if (fs::path(internal_boundaries_path).extension().compare(".shp") == 0)
    {
        GISData gis_data;
        Urban3D::IO::read_GIS(internal_boundaries_path, gis_data);

        buildings = gis_data.convert_to_polygon_mesh();
    }
    else
    {
        std::cerr << "Unsupported input format : " << internal_boundaries_path << std::endl;
        exit(1);
    }

    for (uint p=0; p < buildings.num_polys(); p++)
    {
        cinolib::Polygonmesh<> building;

        std::vector<uint> poly;

        for (const int v : buildings.adj_p2v(p))
        {
            int id = building.vert_add(buildings.vert(v));
            poly.push_back(id);
        }

        building.poly_add(poly);

        // translate the building to the origin and project to the plane z=0
        building.translate(-c);
        for (cinolib::vec3d &v : building.vector_verts()) {
            double tmp = v.z();
            v.z() = 0.;
            z_coordinates[v] = tmp;
        }

        // // check that the building is contained within the ground polygon
        // bool check = true;
        // for (cinolib::vec3d &v : building.poly_verts(0)) {
        //     if (!point_in_polygon(external_boundary, v, 0)) {
        //         check = false;
        //         outside_buildings.push_back(fs::path(building_path).filename().string());
        //         break;
        //     }
        // }
        // if (!check) continue;

        // merge the building with the ground
        cinolib::Polygonmesh<> tmp = external_boundary;
        merge_meshes_at_coincident_vertices(tmp, building, external_boundary);

        // add the building id to the buildings_holes list
        std::vector<uint> vlist;
        for (cinolib::vec3d &v : building.poly_verts(0)) {
            uint vid = external_boundary.pick_vert(v);
            vlist.push_back(vid);
        }
        int pid = external_boundary.poly_id(vlist);
        assert(pid >= 0);
        buildings_holes.push_back(pid);
    }

    // triangulate the ground with the buildings holes
    cinolib::Trimesh<> ground_tri = triangulate_with_holes(external_boundary, buildings_holes);

    // translate and project back to the original position
    for (cinolib::vec3d &v : ground_tri.vector_verts()) {
        auto it = z_coordinates.find(v);
        v.z() = it->second;
    }
    ground_tri.translate(c);
    ground_tri.update_bbox();
    // ground_tri.save(output_path.c_str());

    // print the buildings outside the ground polygon
    if (outside_buildings.size() > 0) {
        std::cout << "WARNING: buildings outside the ground polygon:" << std::endl;
        for (const std::string &s : outside_buildings) {
            std::cout << s << std::endl;
        }
    }

    cinolib::DrawablePolygonmesh<> dual_m;
    cinolib::DrawableTrimesh<> dual_tri;
    URBAN3D::dual_mesh(ground_tri, dual_m/*, dual_tri*/);

    std::cout << "Dual_m size (#polys) :: " << dual_m.num_polys() << std::endl;

    dual_m.show_wireframe(true);
    dual_m.show_marked_edge_color(cinolib::Color::BLUE());
    dual_m.show_vert_color();
    dual_m.updateGL_marked();
    dual_m.updateGL();

    dual_tri.show_poly_color();
    dual_tri.updateGL();

    GISData shapefile (dual_m, boundary_epsg);
    URBAN3D::write_GIS(output_path, shapefile);

#ifdef USE_CINOLIB_GUI

    cinolib::DrawableTrimesh<> ground_tri_DM(ground_tri.vector_verts(), ground_tri.vector_polys());
    // ground_tri_DM.translate(-c);
    ground_tri_DM.edge_mark_boundaries();
    ground_tri_DM.updateGL();
    ground_tri_DM.show_marked_edge_width(7.5);

    cinolib::GLcanvas gui(1000, 1000);
    gui.push(&ground_tri_DM);
    gui.push(&dual_m);
    gui.push(&dual_tri);

    for (uint vid=0; vid < dual_m.num_verts(); vid++)
        gui.push_marker(cinolib::vec3d(dual_m.vert(vid)));

    cinolib::DrawableSegmentSoup ss;

    for (uint eid=0; eid < dual_m.num_edges(); eid++)
        ss.push_seg(dual_m.edge_vert(eid,0), dual_m.edge_vert(eid,1));

    ss.default_color = cinolib::Color::BLUE();
    ss.use_gl_lines = true;
    ss.thickness = 5.;

    gui.push(&ss);

    cinolib::SurfaceMeshControls<cinolib::DrawableTrimesh<>> menu(&ground_tri_DM, &gui, "Ground");
    gui.push(&menu);
    gui.launch();

#endif

/////////////////////////////////////////
///
///
    if (las_path.length() > 0)
    {
        // Check if the LAS file exists
        std::ifstream ifs;
        ifs.open(las_path.c_str(), std::ios::in | std::ios::binary);

        if (ifs.is_open())
        {
            std::cout << "Processing LAS file: " << las_path << std::endl;
            liblas::Reader reader(ifs);
            liblas::Header const& header = reader.GetHeader();

            int nPoints = header.GetPointRecordsCount();

            std::cout << "Number of points in the LAS file: " << nPoints << std::endl;

            // Store the points and the point-to-region mapping
            std::vector<liblas::Point> Points;
            std::vector<uint> point2region (nPoints, UINT_MAX);

            std::vector<std::vector<uint>> region2point (shapefile.get_polygons().size(), std::vector<uint>());
            Points.reserve(nPoints);

            while (reader.ReadNextPoint())
            {
                liblas::Point p = reader.GetPoint();  // safer than using a const ref
                Points.push_back(p);
            }

            cinolib::Polygonmesh<> shp_mesh = shapefile.convert_to_polygon_mesh();

            std::atomic<int> lastPercentagePrinted{0};


            #pragma omp parallel for ordered schedule(static,1)
            for (int j = 0; j < nPoints; j++)
            {
                // if (!point_in_polygon(external_boundary, cinolib::vec3d(Points.at(j).GetX(), Points.at(j).GetY(),Points.at(j).GetZ()), 0))
                //     continue;
                // reader.ReadNextPoint();
                // const liblas::Point p = reader.GetPoint();
                // Points.push_back(p);

                // std::cout << "Processing point " << j << " with coordinates: "
                //           << Points.at(j).GetX() << ", "
                //           << Points.at(j).GetY() << ", "
                //           << Points.at(j).GetZ() << std::endl;



                for (uint pid=0; pid < shp_mesh.num_polys(); pid++)
                {
                    if (point_in_polygon(shp_mesh, cinolib::vec3d(Points.at(j).GetX(), Points.at(j).GetY(),Points.at(j).GetZ()), pid))
                    {
                        // region2point.at(pid).push_back(j);
                        point2region.at(j) = pid;
                        // std::cout << "Point " << j << " belongs to region " << pid << std::endl;
                        break;
                    }
                }

                int currentPercentage = static_cast<int>((100.0 * j) / nPoints);

                // Only one thread at a time checks & updates this
                if (j==0 || currentPercentage > lastPercentagePrinted+4) // Update every 5%)
                {
                    // atomic compare-and-swap: only one thread will succeed
                    int expected = lastPercentagePrinted;
                    if (lastPercentagePrinted.compare_exchange_strong(expected, currentPercentage))
                    {
                        #pragma omp ordered
                        {
                            std::stringstream ss;
                            ss << "Processed " << j << " points / " << nPoints
                               << " total points (" << currentPercentage << "%)";

                            if (currentPercentage == 100)
                                ss << " - done!";
                            else
                                ss << "...";

                            std::cout << ss.str() << std::endl;
                        }
                    }
                }
            }

            for (int j = 0; j < nPoints; j++)
            {
                if (point2region.at(j) < UINT_MAX)
                    region2point.at(point2region.at(j)).push_back(j);
            }

            ///
            for (uint pid=0; pid < shapefile.get_polygons().size(); pid++)
            {
                if (region2point.at(pid).size() == 0)
                {
                    std::cout << "Region " << pid << " has no points." << std::endl;
                    continue;
                }

                std::ofstream outFile;

                std::string outName = output_path + "_" + std::to_string(pid) + ".las";

                std::cout << "Writing LAS file: " << outName << std::endl;

                outFile.open(outName, std::ios::out | std::ios::binary);
                if (!outFile.is_open())
                {
                    std::cerr << "Error opening output LAS file: " << outName << std::endl;
                    continue;
                    // exit(1);
                }

                liblas::Header h = header;
                h.SetPointRecordsCount(region2point.at(pid).size());
                liblas::Writer *writer = new liblas::Writer(outFile, h);

                for (uint i=0; i < region2point.at(pid).size(); i++)
                {
                    writer->WritePoint(Points.at(region2point.at(pid).at(i)));
                }

                outFile.close();
            }

            ifs.close();
        }
        else
        {
            std::cerr << "Error opening LAS file: " << las_path << std::endl;
            exit(1);
        }
    }


    return 0;
}
