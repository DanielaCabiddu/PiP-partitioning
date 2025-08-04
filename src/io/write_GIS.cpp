#include "write_GIS.h"

#include <ogrsf_frmts.h>

#include <fstream>
#include <iostream>

namespace URBAN3D
{

inline
void write_GIS (const std::string filename, const GISData &gis_data )
{
    GDALAllRegister();

    const char *pszDriverName = "ESRI Shapefile";
    GDALDriver *poDriver;

    GDALAllRegister();

    poDriver = (GDALDriver*) GDALGetDriverByName(pszDriverName );
    if( poDriver == NULL )
    {
        printf( "%s driver not available.\n", pszDriverName );
        exit( 1 );
    }

    GDALDataset *poDS;

    poDS = poDriver->Create( filename.c_str() , 0, 0, 0, GDT_Unknown, NULL );
    if( poDS == NULL )
    {
        printf( "Creation of output file failed.\n" );
        exit( 1 );
    }

    // Define spatial reference
    OGRSpatialReference oSRS;
    oSRS.importFromEPSG(gis_data.get_epsg());

    OGRLayer *poLayer;

    poLayer = poDS->CreateLayer( "polygons_out", &oSRS, wkbPolygon, NULL );
    if( poLayer == NULL )
    {
        printf( "Layer creation failed.\n" );
        exit( 1 );
    }

    OGRFieldDefn oField( "Name", OFTString );

    oField.SetWidth(32);

    if( poLayer->CreateField( &oField ) != OGRERR_NONE )
    {
        printf( "Creating Name field failed.\n" );
        exit( 1 );
    }

    for (uint pid=0; pid < gis_data.get_polygons().size(); pid++)
    {
        OGRFeature *poFeature;

        poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
        // poFeature->SetField( "Name", szName );

        OGRPolygon poly;

        OGRLinearRing ring;

        for (uint p=0; p < gis_data.get_polygons().at(pid).size(); p++)
            ring.addPoint(gis_data.get_polygons().at(pid).at(p).x(),
                          gis_data.get_polygons().at(pid).at(p).y(),
                          gis_data.get_polygons().at(pid).at(p).z());

        poly.addRing(&ring);

        poFeature->SetGeometry( &poly );

        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in shapefile.\n" );
            exit( 1 );
        }

        OGRFeature::DestroyFeature( poFeature );
    }


    GDALClose(poDS);

    std::cout << "Write - completed." << std::endl;
}

} // END namespace
