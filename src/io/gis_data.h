#ifndef GIS_DATA_H
#define GIS_DATA_H

#include "gdal_priv.h"
#include <cinolib/geometry/vec_mat.h>

#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/octree.h>

class GISDataField
{
public:
    std::string name;
    std::string type;
    std::string value;
};

class GISData
{
private:

    // GDALDataset *ds = nullptr;

    GDALDriver *poDriver_out = nullptr;
    GDALDataset *poDS_out = nullptr;

    std::string copy_filename;

    unsigned int epsg;

    std::vector<cinolib::vec3d> points;
    std::vector<std::vector<cinolib::vec3d>> lines;
    std::vector<std::vector<GISDataField>> lines_fields;

    std::vector<std::vector<cinolib::vec3d>> polygons;
    std::vector<std::vector<GISDataField>> polygons_fields;

public:

    GISData () {}
    GISData (const cinolib::Polygonmesh<> &m, const uint m_epsg);
    GISData (const std::string i_filename, const std::string o_filename);

    GDALDataset *read(const std::string filename, unsigned int nOpenFlags);
    bool write ();

    void add_field_to_layer (const std::vector<double> &f, const std::string &layer_name, const std::string &field_name);

    void set_epsg(const unsigned int e) { epsg = e; }

    uint get_epsg () const { return epsg; }

    const std::vector<cinolib::vec3d>& get_points () const {return points; }
    const cinolib::vec3d& get_point (const uint i) const {return points.at(i); }

    const std::vector<std::vector<cinolib::vec3d>> & get_lines () const {return lines;}
    const std::vector<cinolib::vec3d> & get_line (const uint i) const {return lines.at(i);}
    const std::vector<std::vector<GISDataField>> & get_lines_fields () const {return lines_fields;}
    const std::vector<GISDataField> & get_line_fields (const uint i) const {return lines_fields.at(i);}

    const std::vector<std::vector<cinolib::vec3d>> & get_polygons () const {return polygons;}
    const std::vector<cinolib::vec3d> & get_polygon (const uint i) const {return polygons.at(i);}

    const std::vector<std::vector<GISDataField>> & get_polygons_fields () const {return polygons_fields;}
    const std::vector<GISDataField> & get_polygon_fields (const uint i) const {return polygons_fields.at(i);}

    ////

    void add_point (const cinolib::vec3d &p) { points.push_back(p); }
    void add_line (const std::vector<cinolib::vec3d> &l) { lines.push_back(l); }
    void add_polygon (const std::vector<cinolib::vec3d> &p) { polygons.push_back(p); }
    void add_polygon_field (const std::vector<GISDataField> &fields ) { polygons_fields.push_back(fields); }
    void add_line_field (const std::vector<GISDataField> &fields ) { lines_fields.push_back(fields); }


    bool convert_to_epsg(const uint epsg_target);

    void set_z_from_mesh (const cinolib::Polygonmesh<> &mesh);
    void set_z_from_octree (const cinolib::Octree &octree);

    cinolib::Polygonmesh<> convert_to_polygon_mesh () const;

};

#ifndef STATIC_VIEWER
#include "gis_data.cpp"
#endif

#endif
