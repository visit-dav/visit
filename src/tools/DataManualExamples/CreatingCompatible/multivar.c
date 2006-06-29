#include <silo.h>
#include <stdio.h>
#include <string.h>

void
write_domains(void)
{
    float x[] = {0., 1., 2.5, 5.};
    float y[] = {0., 2., 2.25, 2.55,  5.};
    float var[20];
    int dims[] = {4, 5};
    int dom, i, ndims = 2;

    float tx[] = {0., -5., -5., 0.};
    float ty[] = {0., 0., -5., -5.};
    int index = 0;
    for(dom = 0; dom < 4; ++dom)
    {
        DBfile *dbfile = NULL;
        float xc[4], yc[5];
        float *coords[] = {xc, yc};
        char filename[100];

        for(i = 0; i < 4; ++i)
            xc[i] = x[i] + tx[dom];
        for(i = 0; i < 5; ++i)
            yc[i] = y[i] + ty[dom];
        for(i = 0; i < 4*5; ++i)
            var[i] = (float)index++;

        /* Open the Silo file */
        sprintf(filename, "multivar.%d", dom);
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
        sprintf(tmp, "multivar.%d:quadmesh", dom);
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
write_multivar(DBfile *dbfile)
{
    char **varnames = NULL;
    int dom, nvar = 4, *vartypes = NULL;

    /* Create the list of var names. */
    varnames = (char **)malloc(nvar * sizeof(char *));
    for(dom = 0; dom < nvar; ++dom)
    {
        char tmp[100];
        sprintf(tmp, "multivar.%d:var", dom);
        varnames[dom] = strdup(tmp);
    }
    /* Create the list of var types. */
    vartypes = (int *)malloc(nvar * sizeof(int));
    for(dom = 0; dom < nvar; ++dom)
        vartypes[dom] = DB_QUADVAR;

    /* Write the multivar. */
    DBPutMultivar(dbfile, "var", nvar, varnames, vartypes, NULL);

    /* Free the memory*/
    for(dom = 0; dom < nvar; ++dom)
        free(varnames[dom]);
    free(varnames);
    free(vartypes);
}

void
write_master(void)
{
    DBfile *dbfile = NULL;
    char **meshnames = NULL, **varnames = NULL;
    int dom, nmesh = 4, nvar = 4;
    int *meshtypes = NULL, *vartypes = NULL;

    /* Open the Silo file */
    dbfile = DBCreate("multivar.root", DB_CLOBBER, DB_LOCAL,
    "Master file", DB_HDF5);    

    write_multimesh(dbfile);
    write_multivar(dbfile);

    /* Close the Silo file. */
    DBClose(dbfile);
}

int
main(int argc, char *argv[])
{
    /* Add other Silo calls here. */
    write_domains();
    write_master();

    return 0;
}
