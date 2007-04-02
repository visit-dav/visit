#include <silo.h>
#include <stdio.h>
#include <string.h>

void
write_domains(double extents[4][2])
{
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    float var[20];
    int dims[] = {4, 5};
    int dom, i, j, ndims = 2;

    float tx[] = {0., -5., -5., 0.};
    float ty[] = {0., 0., -5., -5.};

    for(dom = 0; dom < 4; ++dom)
    {
        DBfile *dbfile = NULL;
        float xc[4], yc[5];
        float *coords[] = {xc, yc};
        char filename[100];
        int index = 0;

        for(i = 0; i < 4; ++i)
            xc[i] = x[i] + tx[dom];
        for(i = 0; i < 5; ++i)
            yc[i] = y[i] + ty[dom];

        for(j = 0; j < 5; ++j)
        {
            for(i = 0; i < 4; ++i)
            {
                float dx, dy;
                dx = xc[i] - 5.;
                dy = yc[j] - 5.;
                var[index++] = (float)sqrt(dx*dx + dy*dy);
            }
        }

        /* Figure out the extents for the domain. */
        extents[dom][0] = extents[dom][1] = var[0];
        for(i = 1; i < 4*5; ++i)
        {
            extents[dom][0] = (var[i] < extents[dom][0]) ? var[i] : extents[dom][0];
            extents[dom][1] = (var[i] > extents[dom][1]) ? var[i] : extents[dom][1];
        }

        /* Open the Silo file */
        sprintf(filename, "dataextents.%d", dom);
        dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
        "domain data", DB_HDF5);    

        /* Write a rectilinear mesh. */
        DBPutQuadmesh(dbfile, "quadmesh", NULL, coords, dims, ndims,
                      DB_FLOAT, DB_COLLINEAR, NULL);

        /* Write a node-centered var. */
        DBPutQuadvar1(dbfile, "var", "quadmesh", var, dims,
                      ndims, NULL, 0, DB_FLOAT, DB_NODECENT, NULL);

        /* Close the Silo file. */
        DBClose(dbfile);
    }
}

void
write_multimesh(DBfile *dbfile)
{
    char **meshnames = NULL;
    int dom, nmesh = 4, *meshtypes = NULL;

    /* Create the list of mesh names. */
    meshnames = (char **)malloc(nmesh * sizeof(char *));
    for(dom = 0; dom < nmesh; ++dom)
    {
        char tmp[100];
        sprintf(tmp, "dataextents.%d:quadmesh", dom);
        meshnames[dom] = strdup(tmp);
    }
    /* Create the list of mesh types. */
    meshtypes = (int *)malloc(nmesh * sizeof(int));
    for(dom = 0; dom < nmesh; ++dom)
        meshtypes[dom] = DB_QUAD_RECT;

    /* Write the multimesh. */
    DBPutMultimesh(dbfile, "quadmesh", nmesh, meshnames, meshtypes, NULL);

    /* Free the memory*/
    for(dom = 0; dom < nmesh; ++dom)
        free(meshnames[dom]);
    free(meshnames);
    free(meshtypes);
}

void
write_multivar(DBfile *dbfile, double extents[4][2])
{
    char **varnames = NULL;
    int dom, nvar = 4, two = 2, *vartypes = NULL;
    DBoptlist *optlist = NULL;

    /* Create the list of var names. */
    varnames = (char **)malloc(nvar * sizeof(char *));
    for(dom = 0; dom < nvar; ++dom)
    {
        char tmp[100];
        sprintf(tmp, "dataextents.%d:var", dom);
        varnames[dom] = strdup(tmp);
    }
    /* Create the list of var types. */
    vartypes = (int *)malloc(nvar * sizeof(int));
    for(dom = 0; dom < nvar; ++dom)
        vartypes[dom] = DB_QUADVAR;

    /* Write the multivar. */
    optlist = DBMakeOptlist(2);
    DBAddOption(optlist, DBOPT_EXTENTS_SIZE, (void *)&two);
    DBAddOption(optlist, DBOPT_EXTENTS, (void *)extents);
    DBPutMultivar(dbfile, "var", nvar, varnames, vartypes, optlist);
    DBFreeOptlist(optlist);

    /* Free the memory*/
    for(dom = 0; dom < nvar; ++dom)
        free(varnames[dom]);
    free(varnames);
    free(vartypes);
}

void
write_master(double extents[4][2])
{
    DBfile *dbfile = NULL;
    char **meshnames = NULL, **varnames = NULL;
    int dom, nmesh = 4, nvar = 4;
    int *meshtypes = NULL, *vartypes = NULL;

    /* Open the Silo file */
    dbfile = DBCreate("dataextents.root", DB_CLOBBER, DB_LOCAL,
    "Master file with data extents", DB_HDF5);    

    write_multimesh(dbfile);
    write_multivar(dbfile, extents);

    /* Close the Silo file. */
    DBClose(dbfile);
}

int
main(int argc, char *argv[])
{
    double extents[4][2];

    /* Add other Silo calls here. */
    write_domains(extents);
    write_master(extents);

    return 0;
}
