// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Python.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <Py2and3Support.h>

#include "visit_writer.h"
#include "py_visit_writer_doc.h"

#define version "1.0"

static PyObject *ErrorExc;

static int connectionTypes[][2] = {
    { VISIT_VERTEX, 1 }, { VISIT_LINE,       2 }, { VISIT_TRIANGLE, 3 }, { VISIT_QUAD,    4 }, 
    { VISIT_TETRA,  4 }, { VISIT_HEXAHEDRON, 8 }, { VISIT_WEDGE,    6 }, { VISIT_PYRAMID, 5 },
    { VISIT_PIXEL,  4 }, { VISIT_VOXEL,      8 } };

enum TokenTypes { endOfList = 0, logicalToken, integerToken };

typedef struct {
    enum TokenTypes type;
    int value;
    char *tok_string;
} visItWriterIntegerTokens;

static int convertDims( PyObject *dims_py, int pts[3] );
static int convertPts( long size, PyObject *pts_py, float **pts );
static int convertVarData( int npts, int ncells, PyObject *nameDimAndVarList, char ***varnames, int **vardim, int **centering, float ***vars );
static int convertVarDatum( int npts, int ncells, int n, PyObject *nameDimVar, char **varnames, int *vardim, int *centering, float **vars );
static int convertCells( PyObject *cellTypeConnection, int **cellTypes, int **connectivity );
static int convertCell( int nCellsDone, PyObject *cellItem, int *cellTypes, int *nConnDone, int *connLen, int **connectivity );
static int addCellConnectivity( int index, int len, PyObject *cellItem, int connectionType[2], int *nConnDone, int *connLen, int **connectivity );
static int checkMallocAndGetArrayOfNumbers( PyObject *py, float **a, char *s, int doIt );
static int getArrayOfNumbers( int npts, PyObject *list, float *pts, char *name );
static int getFloat( PyObject *item, float *f );
static int getIntegerToken( PyObject *o, visItWriterIntegerTokens *tokens );
static int getLength( PyObject *o, char *s );
static void *myGetMemory( size_t size, int doIt, char *var );
static void freeVarData( int nvars, int *vardim, char **varnames, int *centering, float **vars );
static int setErrorAndReturnInt( int Flag, char *Msg, ... );
static void setError( char *Msg, ... );


/* ****************************************************************************
 *  Function: writePointMesh
 *
 *  Purpose:
 *      Python wrapping to routine to write point mesh.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 *    Hank Childs, Thu Sep 20 22:50:55 PDT 2007
 *    Allow for data to be "cell centered".
 *
 * ************************************************************************* */

static char writePointMeshDoc[] = 
    "WritePointMesh( fileName, useBinary, points, nameDimensionAndVariables )\n   See modules documentation for more information.";

static PyObject *writePointMesh( PyObject *self, PyObject *args ) {

    int useBinary, npts, nvars, *vardim, *centering;
    char *fileName, **varnames;
    float *pts = NULL, **vars = NULL;
    PyObject *pts_py, *nameDimAndVarList;

    if( !PyArg_ParseTuple( args, "siOO", &fileName, &useBinary, &pts_py, &nameDimAndVarList ) ) return( NULL );

    if( ( npts = convertPts( -1, pts_py, &pts ) ) <= 0 ) return( NULL );
    npts = npts / 3;
    if( ( nvars = convertVarData( npts, npts, nameDimAndVarList, &varnames, &vardim, &centering, &vars ) ) < 0 ) {
        free( pts );
        return( NULL );
    }

    write_point_mesh( fileName, useBinary, npts, pts, nvars, vardim, (const char * const *) varnames, vars );

    free( pts );
    if (nvars > 0)
        freeVarData( nvars, vardim, varnames, centering, vars );

    Py_INCREF( Py_None );
    return( Py_None );
}


/* ****************************************************************************
 *  Function: writeUnstructuredMesh
 *
 *  Purpose:
 *      Python wrapping to routine to write an unstructured mesh.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 * ************************************************************************* */

static char writeUnstructuredMeshDoc[] = 
    "WriteUnstructuredMesh( fileName, useBinary, points, cellTypeConnections, nameDimensionAndVariables )\n   See modules documentation for more information.";

static PyObject *writeUnstructuredMesh( PyObject *self, PyObject *args ) {

    int useBinary, npts, nvars, *vardim, *centering, ncells, *cellTypes = NULL, *conn;
    char *fileName, **varnames;
    float *pts = NULL, **vars = NULL;
    PyObject *pts_py, *nameDimAndVarList, *cellTypeConnections;

    if( !PyArg_ParseTuple( args, "siOOO", &fileName, &useBinary, &pts_py, &cellTypeConnections, &nameDimAndVarList ) ) return( NULL );

    if( ( npts = convertPts( -1, pts_py, &pts ) ) <= 0 ) return( NULL );
    npts = npts / 3;
    if( ( ncells = convertCells( cellTypeConnections, &cellTypes, &conn ) ) <= 0 ) {
        free( pts );
        return( NULL );
    }
    if( ( nvars = convertVarData( npts, ncells, nameDimAndVarList, &varnames, &vardim, &centering, &vars ) ) < 0 ) {
        free( pts );
        free( cellTypes );
        free( conn );
        return( NULL );
    }

    write_unstructured_mesh( fileName, useBinary, npts, pts, ncells, cellTypes, conn, nvars, vardim, centering, (const char * const *) varnames, vars );

    free( pts );
    free( cellTypes );
    free( conn );
    if (nvars > 0)
        freeVarData( nvars, vardim, varnames, centering, vars );

    Py_INCREF( Py_None );
    return( Py_None );
}

/* ****************************************************************************
 *  Function: writeRegularMesh
 *
 *  Purpose:
 *      Python wrapping to routine to write a regular mesh.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 * ************************************************************************* */

static char writeRegularMeshDoc[] = 
    "WriteRegularMesh( fileName, useBinary, dimensions, nameDimensionAndVariables )\n   See modules documentation for more information.";

static PyObject *writeRegularMesh( PyObject *self, PyObject *args ) {

    int useBinary, npts, ncells, nvars, *vardim, *centering, dims[3];
    char *fileName, **varnames;
    float **vars = NULL;
    PyObject *dims_py, *nameDimAndVarList;

    if( !PyArg_ParseTuple( args, "siOO", &fileName, &useBinary, &dims_py, &nameDimAndVarList ) ) return( NULL );

    if( ( npts = convertDims( dims_py, dims ) ) < 0 ) return( NULL );
    ncells = ( dims[0] - 1 ) * ( dims[1] - 1 ) * ( dims[2] - 1 );
    if( ( nvars = convertVarData( npts, ncells, nameDimAndVarList, &varnames, &vardim, &centering, &vars ) ) < 0 ) return( NULL );

    write_regular_mesh( fileName, useBinary, dims, nvars, vardim, centering, (const char * const *) varnames, vars );

    if (nvars > 0)
        freeVarData( nvars, vardim, varnames, centering, vars );

    Py_INCREF( Py_None );
    return( Py_None );
}

/* ****************************************************************************
 *  Function: writeRectilinearMesh
 *
 *  Purpose:
 *      Python wrapping to routine to write a rectilinear mesh.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 *    Brad Whitlock, Mon Jul 16 09:10:37 PDT 2007
 *    Allow for 2D mesh.
 *
 * ************************************************************************* */

static char writeRectilinearMeshDoc[] = 
    "WriteRectilinearMesh( fileName, useBinary, x, y, z, nameDimensionAndVariables )\n   See modules documentation for more information.";

static PyObject *writeRectilinearMesh( PyObject *self, PyObject *args ) {

    int useBinary, npts, ncells, nvars = 0, *vardim, *centering, dims[3], doIt = 1;
    char *fileName, **varnames;
    float **vars = NULL, *x = NULL, *y = NULL, *z = NULL;
    PyObject *xmesh_py, *ymesh_py, *zmesh_py, *nameDimAndVarList;

    if( !PyArg_ParseTuple( args, "siOOOO", &fileName, &useBinary, &xmesh_py, &ymesh_py, &zmesh_py, &nameDimAndVarList ) ) return( NULL );

    if( ( dims[0] = checkMallocAndGetArrayOfNumbers( xmesh_py, &x, "x", doIt ) ) < 0 ) return( NULL );
    if( dims[0] == 0 ) doIt = setErrorAndReturnInt( 0, "length of x is zero" );
    if( ( dims[1] = checkMallocAndGetArrayOfNumbers( ymesh_py, &y, "y", doIt ) ) < 0 ) doIt = 0;
    if( doIt && ( dims[1] == 0 ) ) doIt = setErrorAndReturnInt( 0, "length of y is zero" );
    if( ( dims[2] = checkMallocAndGetArrayOfNumbers( zmesh_py, &z, "z", doIt ) ) < 0 ) doIt = 0;
    if( doIt && ( dims[2] == 0 ) ) doIt = setErrorAndReturnInt( 0, "length of z is zero" );
    if( doIt ) {
        int nz = dims[2] - 1;
        if(nz < 1) nz = 1;
        npts = dims[0] * dims[1] * dims[2];
        ncells = ( dims[0] - 1 ) * ( dims[1] - 1 ) * nz;
        if( ( nvars = convertVarData( npts, ncells, nameDimAndVarList, &varnames, &vardim, &centering, &vars ) ) < 0 ) doIt = 0;
    }
    if( !doIt ) {
        if( x ) free( x );
        if( y ) free( y );
        if( z ) free( z );
        return( NULL );
    }

    write_rectilinear_mesh( fileName, useBinary, dims, x, y, z, nvars, vardim, centering, (const char * const *) varnames, vars );

    free( x );
    free( y );
    free( z );
    if (nvars > 0)
        freeVarData( nvars, vardim, varnames, centering, vars );

    Py_INCREF( Py_None );
    return( Py_None );
}

/* ****************************************************************************
 *  Function: writeCurvilinearMesh
 *
 *  Purpose:
 *      Python wrapping to routine to write a curvilinear mesh.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 * ************************************************************************* */

static char writeCurvilinearMeshDoc[] = 
    "WriteCurvilinearMesh( fileName, useBinary, Mesh, nameDimensionAndVariables )\n   See modules documentation for more information.";

static PyObject *writeCurvilinearMesh( PyObject *self, PyObject *args ) {

    int useBinary, npts, ncells, nvars, *vardim, *centering, dims[3];
    char *fileName, **varnames;
    float **vars = NULL, *pts;
    PyObject *xyzdims_py, *pts_py, *nameDimAndVarList;

    if( !PyArg_ParseTuple( args, "siOOO", &fileName, &useBinary, &xyzdims_py, &pts_py, &nameDimAndVarList ) ) return( NULL );

    if( ( npts = convertDims( xyzdims_py, dims ) ) < 0 ) return( NULL );
    ncells = ( dims[0] - 1 ) * ( dims[1] - 1 ) * ( dims[2] - 1 );

    if( convertPts( npts * 3, pts_py, &pts ) <= 0 ) return( NULL );
    
    if( ( nvars = convertVarData( npts, ncells, nameDimAndVarList, &varnames, &vardim, &centering, &vars ) ) < 0 ) {
        free( pts );
        return( NULL );
    }
    write_curvilinear_mesh( fileName, useBinary, dims, pts, nvars, vardim, centering, (const char * const *) varnames, vars );

    free( pts );
    if (nvars > 0)
        freeVarData( nvars, vardim, varnames, centering, vars );

    Py_INCREF( Py_None );
    return( Py_None );
}
/*
****************************************************************
*/
static int convertDims( PyObject *dims_py, int dims[3] ) {

    int i, npts = 1, len;
    PyObject *item;
    char xyzChar[3] = { 'x', 'y', 'z' };

    if( ( len = getLength( dims_py, "dimensions" ) ) < 0 ) return( -1 );
    if( len != 3 ) return( setErrorAndReturnInt( -1, "improper length for dimensions agrument (not [nx, ny, nz])" ) );

    for( i = 0; i < 3; i++ ) {
        item = PySequence_GetItem(dims_py, i);
        if( !PyInt_Check( item ) ) {
            setError( "dimensions n%c value not an integer", xyzChar[i] );
            break;
        }
        dims[i] = PyInt_AS_LONG( item );
        if( dims[i] <= 0 ) {
            setError( "dimensions n%c value less equal to 0",  xyzChar[i] );
            break;
        }
        npts *= dims[i];
        Py_DECREF( item );
    }
    Py_DECREF( item );
    if( i != 3 ) {
        npts = -1;
        Py_DECREF( item );
    }
    return( npts );
}
/*
****************************************************************
*/
static int convertPts( long size, PyObject *pts_py, float **pts ) {

    int npts = -1;

    if( ( npts = getLength( pts_py, "points" ) ) < 0 ) return( -1 );
    if( npts == 0 ) return( setErrorAndReturnInt( -1, "points has zero elements" ) );
    if( size > 0 ) {
        if( npts != size ) return( setErrorAndReturnInt( -1, "points is not of size of nx * ny * nz" ) ); }
    else {
        if( ( npts / 3 ) * 3 != npts ) return( setErrorAndReturnInt( -1, "points is not a multiple of 3" ) );
    }
    npts = checkMallocAndGetArrayOfNumbers( pts_py, pts, "pionts", 1 );
    return( npts );
}

/* ****************************************************************************
 *  Function: convertVarData
 *
 *  Purpose:
 *      Routine to get variable data out of Python arrays.
 *
 *  Programmer: Bret Beck
 *  Creation:   April 1, 2005
 *
 *  Modifications:
 *
 *    Hank Childs, Fri Apr 22 09:39:52 PDT 2005
 *    Allow for empty variable lists.
 *
 * ************************************************************************* */

static int convertVarData( int npts, int ncells, PyObject *nameDimAndVarList, char ***varnames, int **vardim, int **centering, float ***vars ) {

    int n = 0, nvars, doIt = 1,  status, i;
    int numItems = 0;
    PyObject *item;

    if( ( nvars = getLength( nameDimAndVarList, "nameDimensionAndVariables" ) ) < 0 ) return( -1 );
    if( nvars == 0 ) return( 0 );
    if( ( *varnames = (char **)  myGetMemory( nvars * sizeof( char ** ), doIt, "varnames" ) ) == NULL ) doIt = 0;
    if( ( *vardim = (int *) myGetMemory( nvars * sizeof( int * ), doIt, "vardim" ) ) == NULL ) doIt = 0;
    if( ( *centering = (int *) myGetMemory( nvars * sizeof( int * ), doIt, "centering" ) ) == NULL ) doIt = 0;
    if( ( *vars = (float **) myGetMemory( nvars * sizeof( float ** ), doIt, "vars" ) ) == NULL ) doIt = 0;
    if( !doIt ) {
        if( *varnames ) free( *varnames );
        if( *vardim ) free( *vardim );
        if( *centering ) free( *centering );
        return( -1 );
    }

    numItems = PyObject_Size(nameDimAndVarList);
    for (i = 0 ; i < numItems ; i++)
    {
        item = PySequence_GetItem(nameDimAndVarList, i);
        status = convertVarDatum( npts, ncells, n, item, *varnames, *vardim, *centering, *vars );
        Py_DECREF( item );
        if( !status ) break;
        n++;
    }

    if( n != nvars ) {
        freeVarData( n, *vardim, *varnames, *centering, *vars );
        nvars = -1;
    }
    return( nvars );
}
/*
****************************************************************
*/
static int convertVarDatum( int npts, int ncells, int n, PyObject *nameDimVar, char **varnames, int *vardim, int *centering, float **vars ) {

    int np = -1, len, nvars = 0, status = 1, nptsOrncells = ncells;
    int count = 0;
    char Str[1024];
    PyObject *item;
    visItWriterIntegerTokens centeringToken[] = { { logicalToken, 0, "cellBased" }, { logicalToken, 1, "pointBased" }, { endOfList, 0, NULL } };

    vars[n] = NULL;
    sprintf( Str, "nameDimensionAndVariables element at index %d", n );
    if( ( len = getLength( nameDimVar, Str ) ) < 0 ) return( 0 );
    if( len != 4 ) return( setErrorAndReturnInt( 0, 
        "nameDimensionAndVariables element at index %d is not of length 4 (i.e. must be a list of [name, dim, centering, [vars]])", n ) );

    item = PySequence_GetItem(nameDimVar, count++);
    if( !PyString_Check( item ) ) {
        setError( "name for nameDimensionAndVariables element at index %d is not a string", n );
        status = 0;
    }
    if( status ){
        char *item_str =PyString_AsString( item );
        varnames[n] = strdup(item_str);
        PyString_AsString_Cleanup(item_str);
        Py_DECREF( item );

        item = PySequence_GetItem(nameDimVar, count++);
        if( !PyInt_Check( item ) ) {
            setError( "dimension element for nameDimensionAndVariables element at index %d is not an integer", n );
            status = 0;
        }
        if( status ) {
            vardim[n] = PyInt_AS_LONG( item );
            Py_DECREF( item );

            item = PySequence_GetItem(nameDimVar, count++);
            if( ( centering[n] = getIntegerToken( item, centeringToken ) ) < 0 ) {
                setError( "centering element for nameDimensionAndVariables element at index %d is invalid", n );
                status = 0;
            }
            if( status ) {
                Py_DECREF( item );

                if( centering[n] ) nptsOrncells = npts;
                item = PySequence_GetItem(nameDimVar, count++);
                sprintf( Str, "variable element of nameDimensionAndVariables element at index %d", n );
                if( ( nvars = getLength( item, Str ) ) < 0 ) status = 0;
                if( status ) {
                    if( nvars == vardim[n] * nptsOrncells ) {
                        if( ( vars[n] = (float *) myGetMemory( vardim[n] * nptsOrncells * sizeof( float ), 1, "vars[n]" ) ) == NULL ) {
                            status = 0; }
                        else {
                            np = getArrayOfNumbers( vardim[n] * nptsOrncells, item, vars[n], "nameDimensionAndVariables[Variables]" );
                        } }
                    else {
                        setError( "variable element for nameDimensionAndVariables element at index %d has wrong number of data values", n );
                    }
                }
            }
        }
    }
    Py_DECREF( item );
    if( np < 0 ) {
        nvars = 0;
        if( vars[n] ) free( vars[n] );
    }
    return( nvars );
}
/*
****************************************************************
*/
static int convertCells( PyObject *cellTypeConnections, int **cellTypes, int **connectivity ) {

    int nCellsDone = 0, ncells, nConnDone = 0, connLen = 0, status;
    int size = 0;
    int i = 0;
    PyObject *item;

    *connectivity = NULL;
    if( ( ncells = getLength( cellTypeConnections, "cellTypeConnection" ) ) < 0 ) return( ncells );
    if( ncells == 0 ) return( setErrorAndReturnInt( -1, "cellTypeConnection contains no data" ) );

    if( ( *cellTypes = (int *) myGetMemory( ncells * sizeof( int ), 1, "cellTypes" ) ) == NULL ) return( -1 );

    size = PyObject_Size(cellTypeConnections);
    for (i = 0 ; i < size ; i++)
    {
        item = PySequence_GetItem(cellTypeConnections, i);
        status = convertCell( nCellsDone, item, *cellTypes, &nConnDone, &connLen, connectivity );
        Py_DECREF( item );
        if( status <= 0 ) break;
        nCellsDone++;
    }

    if( nCellsDone != ncells ) {
        free( *cellTypes );
        if( *connectivity ) free( *connectivity );
        ncells = -1;
    }

    return( ncells );
}
/*
****************************************************************
*/
static int convertCell( int nCellsDone, PyObject *cellItem, int *cellTypes, int *nConnDone, int *connLen, int **connectivity ) {

    int i, len, status = 1, nconnTypes = sizeof( connectionTypes ) / sizeof( connectionTypes[0] );
    PyObject *item;
    visItWriterIntegerTokens cellTypeToken[] = { 
        { integerToken, VISIT_VERTEX, "vertex" }, { integerToken, VISIT_LINE, "line" }, { integerToken, VISIT_TRIANGLE, "triangle" }, 
        { integerToken, VISIT_QUAD, "quad" }, { integerToken, VISIT_TETRA, "tetrahedron" }, { integerToken, VISIT_HEXAHEDRON, "hexahedron" }, 
        { integerToken, VISIT_WEDGE, "wedge" }, { integerToken, VISIT_PYRAMID, "pyramid" }, {integerToken, VISIT_PIXEL, "pixel"}, {integerToken, VISIT_VOXEL, "voxel"}, { endOfList, 0, NULL } };

    if( ( len = getLength( cellItem, "cellItem" ) ) < 0 ) return( setErrorAndReturnInt( -1, "cell element at index %d of cellTypeConnection", nCellsDone ) );
    if( len == 0 ) return( setErrorAndReturnInt( 0, "cell element at index %d of cellTypeConnection is empty", nCellsDone ) );
        
    item = PySequence_GetItem(cellItem, 0);
    cellTypes[nCellsDone] = getIntegerToken( item, cellTypeToken );
    for( i = 0; i < nconnTypes; i++ ) if( connectionTypes[i][0] == cellTypes[nCellsDone] ) break;
    if( i == nconnTypes ) status = setErrorAndReturnInt( 0, "invalid cell type at index %d of cellTypeConnection", nCellsDone );
    Py_DECREF( item );
    if( status ) status = addCellConnectivity( nCellsDone, len, cellItem, connectionTypes[i], nConnDone, connLen, connectivity );
    return( status );
}
/*
****************************************************************
*/
static int addCellConnectivity( int index, int len, PyObject *cellItem, int connectionType[2], int *nConnDone, int *connLen, int **connectivity ) {

    int n = len - 1, size, status;
    int i, numItems = 0;
    PyObject *item;

    if( n != connectionType[1] ) return( setErrorAndReturnInt( 0, "cell at index %d of cellTypeConnection has wrong number of connections", index ) );
    if( ( *nConnDone + n ) > *connLen ) {
        *connLen += 100000;
        size = *connLen * sizeof( int );
        *connectivity = (int *)realloc( *connectivity, size );
        if( *connectivity == NULL ) {
            PyErr_NoMemory( );
            return( 0 );
        }
    }
    numItems = PyObject_Size(cellItem);
    for (i = 1 ; i < numItems ; i++)
    {
        item = PySequence_GetItem(cellItem, i);
        status = 1;
        if( PyInt_Check( item ) ) {
            (*connectivity)[*nConnDone] = PyInt_AS_LONG( item ); }
        else {
            status = setErrorAndReturnInt( 0, "cell at index %d of cellTypeConnection contains a non integer connectivity element", index );
        }
        Py_DECREF( item );
        if( !status ) return( 0 );
        (*nConnDone)++;
    }
    return( 1 );
}
/*
****************************************************************
*/
static int checkMallocAndGetArrayOfNumbers( PyObject *py, float **a, char *s, int doIt ) {

    int len;

    *a  = NULL;
    if( !doIt ) return( -1 );
    if( ( len = getLength( py, s ) ) < 0 ) return( -1 );
    if( len != 0 ) { 
        if( ( *a = (float *) myGetMemory( len * sizeof( float ), 1, s ) ) == NULL ) return( -1 );
        if( ( len = getArrayOfNumbers( len, py, *a, s ) ) < 0 ) {
            free( *a );
            *a = NULL;
        }
    }
    return( len );
}
/*
****************************************************************
*/
static int getArrayOfNumbers( int npts, PyObject *list, float *pts, char *name ) {
    int numItems, i;
    int n = 0, status;
    PyObject *item;

    numItems = PyObject_Size(list);
    for (i = 0 ; i < numItems ; i++)
    {
        item = PySequence_GetItem(list, i);
        status = getFloat( item, &(pts[n]) );
        Py_DECREF( item );
        if( !status ) break;
        n++;
    }
    if( n != npts ) n = setErrorAndReturnInt( -1, "object at index %d of %s is not a number", n, name );
    return( n );
}
/*
****************************************************************
*/
static int getFloat( PyObject *item, float *f ) {

    int status = ~0;

    if( PyInt_Check( item ) ) {
        *f = (float) PyInt_AS_LONG( item ); }
    else if( PyFloat_Check( item ) ) {
        *f = (float) PyFloat_AS_DOUBLE( item ); }
    else {
        status = 0;
    }
    return( status );
}
/*
****************************************************************
*/
static int getIntegerToken( PyObject *item, visItWriterIntegerTokens *tokens ) {

    int i;
    char *s;

    if( PyInt_Check( item ) )
    {
        return( PyInt_AS_LONG( item ) );
    }
    else if( PyString_Check( item ) )
    {
        s = PyString_AsString( item );
        for( i = 0; tokens[i].type; i++ )
        {
            if( strcmp( tokens[i].tok_string, s ) == 0 ) return( tokens[i].value );
        }
        return( -2 );
        PyString_AsString_Cleanup(s);
    }
    return( -1 );
}
/*
****************************************************************
*/
static int getLength( PyObject *o, char *s ) {

    int len = -1;

    if( PyList_Check( o ) ) {
        len = PyList_GET_SIZE( o ); }
    else if( PyTuple_Check( o ) ) {
        len = PyTuple_GET_SIZE( o ); }
    else {
        setError( "%s is not a list or tuple", s );
    }
    return( len );
}
/*
****************************************************************
*/
static void *myGetMemory( size_t size, int doIt, char *var ) {

    void *p = NULL;
    
    if( doIt ) {
        p = malloc( size );
        if( p == NULL ) PyErr_NoMemory( );
    }
    return( p );
}
/*  
****************************************************************
*/
static void freeVarData( int nvars, int *vardim, char **varnames, int *centering, float **vars ) {

    int i;

    free( vardim );
    free( centering );
    free( varnames );
    for( i = 0; i < nvars; i++ ) free( vars[i] );
    free( vars );
}
/*
****************************************************************
*/
static int setErrorAndReturnInt( int Flag, char *Msg, ... ) {

    char Str[1024];
    va_list args;

    va_start( args, Msg );
    vsnprintf( Str, sizeof( Str ), Msg, args );
    PyErr_SetString( ErrorExc, Str );
    va_end( args );
    return( Flag );
}
/*
****************************************************************
*/
static void setError( char *Msg, ... ) {

    char Str[1024];
    va_list args;

    va_start( args, Msg );
    vsnprintf( Str, sizeof( Str ), Msg, args );
    PyErr_SetString( ErrorExc, Str );
    va_end( args );
}
/*
****************************************************************
*/
static PyMethodDef visItWriterMethods[] = {

    { "WritePointMesh",        (PyCFunction) writePointMesh,        METH_VARARGS, writePointMeshDoc        },
    { "WriteUnstructuredMesh", (PyCFunction) writeUnstructuredMesh, METH_VARARGS, writeUnstructuredMeshDoc }, 
    { "WriteRegularMesh",      (PyCFunction) writeRegularMesh,      METH_VARARGS, writeRegularMeshDoc      },
    { "WriteRectilinearMesh",  (PyCFunction) writeRectilinearMesh,  METH_VARARGS, writeRectilinearMeshDoc  },
    { "WriteCurvilinearMesh",  (PyCFunction) writeCurvilinearMesh,  METH_VARARGS, writeCurvilinearMeshDoc  },
    { NULL, NULL, 0, NULL }                                             /* Sentinel (i.e., the end of the list) */
};
/*
****************************************************************
*/



//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// Module Init Code
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

struct module_state {
    PyObject *error;
};

//---------------------------------------------------------------------------//
#if defined(IS_PY3K)
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Extra Module Setup Logic for Python3
//---------------------------------------------------------------------------//
#if defined(IS_PY3K)
//---------------------------------------------------------------------------//
static int
visit_writer_module_traverse(PyObject *m, visitproc visit, void *arg)
{
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

//---------------------------------------------------------------------------//
static int 
visit_writer_clear(PyObject *m)
{
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}

//---------------------------------------------------------------------------//
static struct PyModuleDef visit_writer_module_def = 
{
        PyModuleDef_HEAD_INIT,
        "visit_writer",
        NULL,
        sizeof(struct module_state),
        visItWriterMethods,
        NULL,
        visit_writer_module_traverse,
        visit_writer_clear,
        NULL
};
#endif


//---------------------------------------------------------------------------//
// The module init function signature is different between py2 and py3
// This macro simplifies the process of returning when an init error occurs.
//---------------------------------------------------------------------------//
#if defined(IS_PY3K)
#define PY_MODULE_INIT_RETURN_ERROR return NULL
#else
#define PY_MODULE_INIT_RETURN_ERROR return
#endif

/******************************************************************/
#if defined(IS_PY3K) /* py 3 */
/******************************************************************/
/******************************************************************/
    #if __GNUC__ >= 4
    /* Ensure this function is visible even if -fvisibility=hidden was passed */
    __attribute__ ((visibility("default"))) PyObject * PyInit_visit_writer( void )
    #elif _WIN32
    __declspec(dllexport) PyObject * PyInit_visit_writer( void )
    #else
    PyObject * PyInit_visit_writer( void )
    #endif
/******************************************************************/
/******************************************************************/
#else /* py 2 */
/******************************************************************/
/******************************************************************/
    #if __GNUC__ >= 4
    /* Ensure this function is visible even if -fvisibility=hidden was passed */
    DL_EXPORT( void ) __attribute__ ((visibility("default"))) initvisit_writer( void )
    #else
    DL_EXPORT( void ) initvisit_writer( void )
    #endif
/******************************************************************/
#endif
/******************************************************************/
{
    PyObject *m, *member;
    ErrorExc = PyExc_ValueError;

#if defined(IS_PY3K)
    m = PyModule_Create(&visit_writer_module_def);
#else
    m = Py_InitModule3( "visit_writer", visItWriterMethods, visItWriterInitDoc );
#endif

    member = PyInt_FromLong( VISIT_VERTEX );
    PyModule_AddObject( m, "vertex", member );

    member = PyInt_FromLong( VISIT_LINE );
    PyModule_AddObject( m, "line", member );

    member = PyInt_FromLong( VISIT_TRIANGLE );
    PyModule_AddObject( m, "triangle", member );

    member = PyInt_FromLong( VISIT_QUAD );
    PyModule_AddObject( m, "quad", member );

    member = PyInt_FromLong( VISIT_TETRA );
    PyModule_AddObject( m, "tetrahedron", member );

    member = PyInt_FromLong( VISIT_HEXAHEDRON );
    PyModule_AddObject( m, "hexahedron", member );

    member = PyInt_FromLong( VISIT_WEDGE );
    PyModule_AddObject( m, "wedge", member );

    member = PyInt_FromLong( VISIT_PYRAMID );
    PyModule_AddObject( m, "pyramid", member );

    member = PyInt_FromLong( VISIT_PIXEL );
    PyModule_AddObject( m, "pixel", member );

    member = PyInt_FromLong( VISIT_VOXEL );
    PyModule_AddObject( m, "voxel", member );

    member = PyString_FromString( version );
    PyModule_AddObject( m, "version", member );

#if defined(IS_PY3K)
    return m;
#endif
}
