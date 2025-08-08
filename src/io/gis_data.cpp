#include "gis_data.h"
#include "gdal.h"
#include "ogrsf_frmts.h"

#include <cinolib/merge_meshes_at_coincident_vertices.h>
#include <filesystem>


void SafeCopyShapefile(const std::string& src, const std::string& dst)
{
    // Remove all existing files with the same base name
    std::string base = dst.substr(0, dst.find_last_of('.'));
    std::vector<std::string> extensions = {".shp", ".shx", ".dbf", ".prj", ".cpg"};
    for (const auto& ext : extensions) {
        std::filesystem::remove(base + ext);
    }

    GDALAllRegister();
    GDALDataset* poSrc = static_cast<GDALDataset*>(GDALOpenEx(src.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (!poSrc) {
        std::cerr << "Failed to open source shapefile." << std::endl;
        return;
    }

    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (!poDriver) {
        std::cerr << "Shapefile driver not available." << std::endl;
        GDALClose(poSrc);
        return;
    }

    GDALDataset* poDst = poDriver->Create(dst.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    if (!poDst) {
        std::cerr << "Failed to create destination shapefile." << std::endl;
        GDALClose(poSrc);
        return;
    }

    for (int i = 0; i < poSrc->GetLayerCount(); ++i) {
        OGRLayer* srcLayer = poSrc->GetLayer(i);
        if (!srcLayer) continue;
        if (!poDst->CopyLayer(srcLayer, srcLayer->GetName(), nullptr)) {
            std::cerr << "Failed to copy layer: " << srcLayer->GetName() << std::endl;
        }
    }

    GDALClose(poDst);
    GDALClose(poSrc);
}


GISData::GISData(const std::string i_filename, const std::string o_filename)
{
    GDALDataset *ds = read(i_filename, GDAL_OF_VECTOR);

    const char *pszDriverName = nullptr;
    std::string ext = i_filename.substr(i_filename.find_last_of("."));

    if (ext == ".shp")
        pszDriverName = "ESRI Shapefile";
    else if (ext == ".gpkg")
        pszDriverName = "GPKG";
    else if (ext == ".geojson")
        pszDriverName = "GeoJSON";
    else {
        std::cerr << "Unsupported extension: " << ext << std::endl;
        return;
    }

    poDriver_out = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (!poDriver_out) {
        std::cerr << pszDriverName << " driver not available.\n";
        return;
    }

    SafeCopyShapefile (i_filename, o_filename);
    copy_filename = o_filename;

//     // Now reopen output in update mode
//     poDS_out = read(o_filename, GDAL_OF_VECTOR | GDAL_OF_UPDATE );
// //static_cast<GDALDataset*>(GDALOpenEx(o_filename.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr));
//     if (!poDS_out) {
//         std::cerr << "Failed to reopen output file for update: " << o_filename << std::endl;
//         exit(1);
//     }
}

inline
GDALDataset * GISData::read(const std::string filename, unsigned int nOpenFlags)
{
    lines.clear();
    points.clear();
    polygons.clear();

    GDALAllRegister();

    // 2. Open file
    GDALDataset *ds = static_cast<GDALDataset*> (GDALOpenEx(filename.c_str(), nOpenFlags, NULL, NULL, NULL ));
    if( ds == NULL )
    {
        std::cerr << "Error while loading Shapefile file " << filename << std::endl;
        return ds;
    }

    int numLayers = GDALDatasetGetLayerCount(ds);
    std::cout << "Number of layers: " << numLayers << std::endl;

    for (unsigned int l=0; l < numLayers; l++)
    {
        OGRLayer *poLayer = ds->GetLayer(l);
        if(poLayer == NULL)
        {
            std::cerr << "ERROR: Failed to get layer " << std::endl;
            return ds;
        }

        std::cout << "Layer " << l << " Name: " << poLayer->GetName() << std::endl;

        // Get spatial reference
        OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
        if (poSRS == nullptr) {
            std::cerr << "No spatial reference found." << std::endl;
            // GDALClose(ds);
            // return;
        }
        else
        {
            // Try to get EPSG code
            if (poSRS->AutoIdentifyEPSG() == OGRERR_NONE) {
                const char* pszAuthorityName = poSRS->GetAuthorityName(nullptr);
                const char* pszAuthorityCode = poSRS->GetAuthorityCode(nullptr);

                if (pszAuthorityName != nullptr && pszAuthorityCode != nullptr)
                {
                    std::cout << "EPSG Code: " << pszAuthorityCode << std::endl;
                    set_epsg(std::atoi(pszAuthorityCode));
                }
                else
                {
                    std::cerr << "EPSG code not found in spatial reference." << std::endl;
                }
            } else {
                std::cerr << "Failed to identify EPSG code." << std::endl;
            }

            set_epsg(poLayer->GetSpatialRef()->GetEPSGGeogCS());
        }

        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

        // 3. Reading features from the layer

        OGRFeature *poFeature;
        poLayer->ResetReading(); //to ensure we are starting at the beginning of the layer

        int numFeature = poLayer->GetFeatureCount();
        std::cout << "GDAL - Number of features: " << numFeature << std::endl;

        while( (poFeature = poLayer->GetNextFeature()) != NULL )
        {
            for( int iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
            {
                OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );

                switch( poFieldDefn->GetType() )
                {
                case OFTInteger:
                    break;
                case OFTInteger64:
                    break;
                case OFTReal:
                    break;
                case OFTString:
                    break;
                default:
                    break;
                }

                std::string field_name = poFieldDefn->GetNameRef();
            }

            // Extract geometry from the feature
            OGRGeometry *poGeometry = poFeature->GetGeometryRef();

            if( poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
            {
                OGRPoint *poPoint = (OGRPoint *) poGeometry;

                cinolib::vec3d point;
                point.x() = poPoint->getX();
                point.y() = poPoint->getY();
                point.z() = poPoint->getZ();

                add_point(point);

                // std::cout << point << std::endl;
                //points.at(points.size()-1).push_back(point);
            }
            else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
            {
                // std::cout << "Geometry Type: LINE" << std::endl;

                OGRLineString *ls = (OGRLineString*) poGeometry;
                // OGRLineString* ls = (OGRLineString*)poGeometry->Boundary();

                std::vector<cinolib::vec3d> line;

                for(int i = 0; i < ls->getNumPoints(); i++ )
                {
                    OGRPoint p;
                    ls->getPoint(i, &p);

                    cinolib::vec3d point;
                    point.x() = p.getX();
                    point.y() = p.getY();
                    point.z() = p.getZ();

                    line.push_back(point);
                }

                add_line(line);

                // Read and store data fields
                std::vector<GISDataField> fields;

                for (uint f=0; f < poFeature->GetFieldCount(); f++)
                {
                    GISDataField field;
                    field.name = poFeature->GetFieldDefnRef(f)->GetNameRef();

                    if (poFeature->GetFieldDefnRef(f)->GetType() == OFTString)
                    {
                        field.value = poFeature->GetFieldAsString(f);
                        field.type = "string";
                    }
                    else
                        if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger)
                        {
                            field.value = std::to_string(poFeature->GetFieldAsInteger(f));
                            field.type = "int";
                        }
                        else
                            if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger64)
                            {
                                field.value = std::to_string(poFeature->GetFieldAsInteger64(f));
                                field.type = "int64";
                            }
                            else
                                if (poFeature->GetFieldDefnRef(f)->GetType() == OFTReal)
                                {
                                    field.value = std::to_string(poFeature->GetFieldAsDouble(f));
                                    field.type = "double";
                                }

                    fields.push_back(field);
                }

                add_line_field(fields);
            }
            else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
            {
                // std::cout << "Geometry Type: POLYGON" << std::endl;

                OGRPolygon *ls = (OGRPolygon*) poGeometry;
                // OGRLineString* ls = (OGRLineString*)poGeometry->Boundary();

                std::vector<cinolib::vec3d> polygon;

                // std::cout << "Number of points: " << ls->getExteriorRing()->getNumPoints() << std::endl;

                for(int i = 0; i < ls->getExteriorRing()->getNumPoints(); i++ )
                {
                    OGRPoint p;
                    ls->getExteriorRing()->getPoint(i,&p);

                    cinolib::vec3d point;
                    point.x() = p.getX();
                    point.y() = p.getY();
                    point.z() = p.getZ();

                    polygon.push_back(point);
                }

                add_polygon(polygon);

                // Read and store data fields
                std::vector<GISDataField> fields;

                for (uint f=0; f < poFeature->GetFieldCount(); f++)
                {
                    GISDataField field;
                    field.name = poFeature->GetFieldDefnRef(f)->GetNameRef();

                    if (poFeature->GetFieldDefnRef(f)->GetType() == OFTString)
                    {
                        field.value = poFeature->GetFieldAsString(f);
                        field.type = "string";
                    }
                    else
                        if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger)
                        {
                            field.value = std::to_string(poFeature->GetFieldAsInteger(f));
                            field.type = "int";
                        }
                        else
                            if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger64)
                            {
                                field.value = std::to_string(poFeature->GetFieldAsInteger64(f));
                                field.type = "int64";
                            }
                            else
                                if (poFeature->GetFieldDefnRef(f)->GetType() == OFTReal)
                                {
                                    field.value = std::to_string(poFeature->GetFieldAsDouble(f));
                                    field.type = "double";
                                }

                    fields.push_back(field);
                }

                add_polygon_field(fields);
            }
            else
            {
                // std::cout << poGeometry->getGeometryType() << "; name = " << poGeometry->getGeometryName() << std::endl;
                // std::cerr << "Unsupported." << std::endl;
                // return false;

                // std::cout << "Geometry Type: POLYGON" << std::endl;

                OGRMultiPolygon *ls = (OGRMultiPolygon*) poGeometry;
                // OGRLineString* ls = (OGRLineString*)poGeometry->Boundary();

                std::vector<cinolib::vec3d> polygon;

                // std::cout << "Number of points: " << ls->getExteriorRing()->getNumPoints() << std::endl;

                // for(int i = 0; i < ls->get->getExteriorRing()->getNumPoints(); i++ )
                // {
                //     OGRPoint p;
                //     ls->getExteriorRing()->getPoint(i,&p);

                //     cinolib::vec3d point;
                //     point.x() = p.getX();
                //     point.y() = p.getY();
                //     point.z() = p.getZ();

                //     polygon.push_back(point);
                // }

                //add_polygon(polygon);

                // Read and store data fields
                std::vector<GISDataField> fields;

                for (uint f=0; f < poFeature->GetFieldCount(); f++)
                {
                    GISDataField field;
                    field.name = poFeature->GetFieldDefnRef(f)->GetNameRef();

                    if (poFeature->GetFieldDefnRef(f)->GetType() == OFTString)
                    {
                        field.value = poFeature->GetFieldAsString(f);
                        field.type = "string";
                    }
                    else
                        if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger)
                        {
                            field.value = std::to_string(poFeature->GetFieldAsInteger(f));
                            field.type = "int";
                        }
                        else
                            if (poFeature->GetFieldDefnRef(f)->GetType() == OFTInteger64)
                            {
                                field.value = std::to_string(poFeature->GetFieldAsInteger64(f));
                                field.type = "int64";
                            }
                            else
                                if (poFeature->GetFieldDefnRef(f)->GetType() == OFTReal)
                                {
                                    field.value = std::to_string(poFeature->GetFieldAsDouble(f));
                                    field.type = "double";
                                }

                    fields.push_back(field);
                }

                add_polygon_field(fields);
            }
        }

        OGRFeature::DestroyFeature( poFeature );
    }

    // GDALClose(ds);

    std::cout << "Load - completed." << std::endl;

    return ds;
}

inline
GISData::GISData (const cinolib::Polygonmesh<> &m, const uint m_epsg)
{
    for (uint pid=0; pid < m.num_polys(); pid++)
    {
        std::vector<cinolib::vec3d> polygon;

        for (uint vid : m.adj_p2v(pid))
            polygon.push_back(m.vert(vid));

        polygon.push_back(polygon.at(0));

        polygons.push_back(polygon);
        polygons_fields.push_back(std::vector<GISDataField>());
    }

    epsg = m_epsg;
}

inline
bool GISData::convert_to_epsg (const uint epsg_target)
{
    std::cout << __FUNCTION__ << std::endl;
    OGRSpatialReference oSourceSRS, oTargetSRS;

    oTargetSRS.importFromEPSG ( epsg_target );
    oSourceSRS.importFromEPSG( epsg );

    std::cout << "original epsg: " << epsg << std::endl;
    std::cout << "target epsg: " << epsg_target << std::endl;

    OGRCoordinateTransformation *poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );

    double x, y;

    for (uint p=0; p < points.size(); p++)
    {
        x = points.at(p).x();
        y = points.at(p).y();

        // std::cout << x << ", " << y << std::endl;

        if (!poCT->Transform( 1, &y, &x ))
        {
            std::cerr << "EPSG conversione error." << std::endl;
            return false;
        }

        points.at(p).x() = x;
        points.at(p).y() = y;
    }


    for (uint l=0; l < lines.size(); l++)
    {
        for (uint p=0; p < lines.at(l).size(); p++)
        {
            x = lines.at(l).at(p).x();
            y = lines.at(l).at(p).y();

            // std::cout << x << ", " << y << std::endl;

            if (!poCT->Transform( 1, &y, &x ))
            {
                std::cerr << "EPSG conversione error." << std::endl;
                return false;
            }

            lines.at(l).at(p).x() = x;
            lines.at(l).at(p).y() = y;
        }
    }

    for (uint b=0; b < polygons.size(); b++)
    {
        for (uint p=0; p < polygons.at(b).size(); p++)
        {
            x = polygons.at(b).at(p).x();
            y = polygons.at(b).at(p).y();

            if (!poCT->Transform( 1, &y, &x ))
            {
                std::cerr << "EPSG conversione error." << std::endl;
                return false;
            }

            polygons.at(b).at(p).x() = x;
            polygons.at(b).at(p).y() = y;
        }
    }

    epsg = epsg_target;

    return true;
}

inline
cinolib::Polygonmesh<> GISData::convert_to_polygon_mesh () const
{
    cinolib::Polygonmesh<> mesh;

    if (!polygons.empty())
    for (uint p=0; p < polygons.size(); p++)
    {
        std::vector<unsigned int> vert_ids;

        // cinolib::Polygonmesh<> tmp;

        if(!polygons.empty())
        for (uint pp=0; pp < polygons.at(p).size()-1; pp++)
        {
            unsigned int id = mesh.vert_add(polygons.at(p).at(pp));
            vert_ids.push_back(id);
        }

        mesh.poly_add(vert_ids);

        //cinolib::merge_meshes_at_coincident_vertices(mesh, tmp, mesh);
    }

    // if (!lines.empty())
    // for (uint l=0; l < lines.size(); l++)
    // {
    //     std::vector<unsigned int> vert_ids;

    //     cinolib::Polygonmesh<> tmp;

    //     for (uint p=0; p < lines.at(l).size()-1; p++)
    //     {
    //         unsigned int id = tmp.vert_add(lines.at(l).at(p));
    //         vert_ids.push_back(id);
    //     }

    //     tmp.poly_add(vert_ids);

    //     cinolib::merge_meshes_at_coincident_vertices(mesh, tmp, mesh);
    // }

    return mesh;
}

inline
void GISData::set_z_from_mesh (const cinolib::Polygonmesh<> &mesh)
{
    cinolib::Octree octree;
    octree.build_from_mesh_polys(mesh);

    set_z_from_octree(octree);
}

inline
void GISData::set_z_from_octree (const cinolib::Octree &octree)
{
    std::cout << __FUNCTION__ << std::endl;

    double x, y;

    double dist;
    uint id;

    for (uint a=0; a < points.size(); a++)
    {
        x = points.at(a).x();
        y = points.at(a).y();

        if (octree.intersects_ray(cinolib::vec3d(x, y, 0), cinolib::vec3d(0,0,1), dist, id))
            points.at(a).z() = dist;
        else
            points.at(a).z() = DBL_MAX;
    }

    for (uint l=0; l < lines.size(); l++)
    {
        for (uint p=0; p < lines.at(l).size(); p++)
        {
            x = lines.at(l).at(p).x();
            y = lines.at(l).at(p).y();

            if (octree.intersects_ray(cinolib::vec3d(x, y, 0), cinolib::vec3d(0,0,1), dist, id))
                lines.at(l).at(p).z() = dist;
            else
                lines.at(l).at(p).z() = DBL_MAX;
        }
    }

    for (uint b=0; b < polygons.size(); b++)
    {
        for (uint p=0; p < polygons.at(b).size(); p++)
        {
            x = polygons.at(b).at(p).x();
            y = polygons.at(b).at(p).y();

            if (octree.intersects_ray(cinolib::vec3d(x, y, 0), cinolib::vec3d(0,0,1), dist, id))
                polygons.at(b).at(p).z() = dist;
            else
                polygons.at(b).at(p).z() = DBL_MAX;
        }
    }
}

inline
void GISData::add_field_to_layer (const std::vector<double> &f, const std::string &layer_name, const std::string &field_name)
{
    std::cout << __FUNCTION__ << std::endl;

    // Now reopen output in update mode
    poDS_out = read(copy_filename, GDAL_OF_VECTOR | GDAL_OF_UPDATE );
    //static_cast<GDALDataset*>(GDALOpenEx(o_filename.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr));
    if (!poDS_out)
    {
        std::cerr << "Failed to reopen output file for update: " << copy_filename << std::endl;
        exit(1);
    }

    std::cout << "Reopened file for update: " << copy_filename << std::endl;

    int numLayers = poDS_out->GetLayerCount();
    std::cout << "---Number of layers: " << numLayers << std::endl;

    for (uint l=0; l < numLayers; l++)
    {
        OGRLayer *poLayer = poDS_out->GetLayer(l);
        if(poLayer == NULL)
        {
            std::cerr << "ERROR: Failed to get layer " << std::endl;
            return;
        }

        std::cout << poLayer->GetName() << std::endl;

        if (std::string(poLayer->GetName()).compare(layer_name) != 0)
        {
            std::cerr << "Layer name does not match: " << poLayer->GetName() << " != " << layer_name << std::endl;
            continue;
        }

        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

        // 3. Reading features from the layer

        OGRFeature *poFeature;
        poLayer->ResetReading(); //to ensure we are starting at the beginning of the layer

        int numFeature = poLayer->GetFeatureCount();
        std::cout << "GDAL - Number of features OUT: " << numFeature << std::endl;
        std::cout << "Field Size: " << f.size() << std::endl;

        if (numFeature != f.size())
        {
            std::cerr << "Error updating layer " << layer_name << std::endl;
            return;
        }

        std::cout << "creating new field : " << field_name << " ... " << std::endl;

        // Create a new field definition for a double
        OGRFieldDefnH hFieldDefn = OGR_Fld_Create(field_name.c_str(), OFTReal); // OFTReal for Double

        // Add the new field to the layer
        if (OGR_L_CreateField(poLayer, hFieldDefn, TRUE) != OGRERR_NONE)
        {
            printf("Failed to create new field.\n");
            OGR_Fld_Destroy(hFieldDefn);
            GDALClose(poDS_out);
            return ;
        }

        std::cout << "creating new field : " << field_name << " ... done" << std::endl;

        std::cout << "updating new field records : " << field_name << " ... " << std::endl;


        OGR_Fld_Destroy(hFieldDefn);

        // Update records in the new field
        OGRFeatureH hFeature;
        OGR_L_ResetReading(poLayer); // Reset the layer reading

        uint val_id=0;

        while ((hFeature = OGR_L_GetNextFeature(poLayer)) != NULL)
        {
            // Set the new field value (example value)
            OGR_F_SetFieldDouble(hFeature, OGR_F_GetFieldIndex(hFeature, field_name.c_str()), f.at(val_id++));

            // Update the feature in the layer
            OGR_L_SetFeature(poLayer, hFeature);
            OGR_F_Destroy(hFeature);
        }

        std::cout << "updating new field records : " << field_name << " ... done" << std::endl;

    }

    write();
}

inline
bool GISData::write ()
{
    GDALClose( poDS_out );
    return true;
}
