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
    std::string polys_path   ;
    std::string las_path  = ""; // Not used in this example, but kept for consistency

    std::string output_las_folder;

    uint boundary_epsg;

    try
    {
        // Define command line parser and arguments
        TCLAP::CmdLine cmd("PiP-Partitioning", ' ', "version 0.5");

        // Define main functionalities options
        TCLAP::ValueArg<std::string> polys_arg("p", "polys", "Polygons", true, "name_ground", "string", cmd);

        TCLAP::ValueArg<std::string> pc_arg("l", "las", "Point Cloud (LAS)", true, "name_pav", "string", cmd);
        TCLAP::ValueArg<std::string> o_pc_arg("L", "output-las-folder", "OutputLAS folder", true, "name_pav", "string", cmd);

        // Parse the argv array
        cmd.parse(argc, argv);

        polys_path = polys_arg.getValue();

        las_path = pc_arg.getValue();
        output_las_folder = o_pc_arg.getValue();

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

    cinolib::Polygonmesh<> polys_mesh;

    GISData gis_data;
    Urban3D::IO::read_GIS(polys_path, gis_data);
    boundary_epsg = gis_data.get_epsg();

    polys_mesh = gis_data.convert_to_polygon_mesh();


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

        std::vector<std::vector<uint>> region2point (polys_mesh.num_polys(), std::vector<uint>());
        Points.reserve(nPoints);

        while (reader.ReadNextPoint())
        {
            liblas::Point p = reader.GetPoint();  // safer than using a const ref
            Points.push_back(p);
        }

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



            for (uint pid=0; pid < polys_mesh.num_polys(); pid++)
            {
                if (point_in_polygon(polys_mesh, cinolib::vec3d(Points.at(j).GetX(), Points.at(j).GetY(),Points.at(j).GetZ()), pid))
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
        for (uint pid=0; pid < polys_mesh.num_polys(); pid++)
        {
            if (region2point.at(pid).size() == 0)
            {
                std::cout << "Region " << pid << " has no points." << std::endl;
                continue;
            }

            std::ofstream outFile;

            std::string outFolder = output_las_folder + "/" + std::to_string(pid);
            std::string outName = outFolder + "/" + std::to_string(pid) + ".las";

            // create output directory if it does not exist
            if (!fs::exists(outFolder))
            {
                fs::create_directories(outFolder);
            }

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


    return 0;
}
