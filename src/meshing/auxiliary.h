#ifndef CITY_AUXILIARY
#define CITY_AUXILIARY

#include <cinolib/meshes/meshes.h>
#include <shapefil.h>

inline
int pnpoly(SHPObject * region, double testx, double testy)
{
    int nvert = region->nVertices;
    double *vertx = region->padfX;
    double *verty = region->padfY;

    if (region->nParts > 1)
    {
        nvert = region->panPartStart[1];
    }

    int i, j, c = 0;

    for (i = 0, j = nvert - 1; i < nvert; j = i++) {

        bool bbout = false;

        if (testx < region->dfXMin || testy < region->dfYMin ||
            testx > region->dfXMax || testy > region->dfYMax )
            bbout = true;

        if (!bbout && ((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
            c = !c;
    }

#ifdef _DEEPTIMING_
    pnp_time += hmt_get (&pnp);
#endif
    return c;
}


#endif // CITY_AUXILIARY
