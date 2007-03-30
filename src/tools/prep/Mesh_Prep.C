// ************************************************************************* //
//                               Mesh_Prep.C                                 //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include <IntervalTree.h>
#include <Mesh_Prep.h>
#include <SiloObjLib.h>


//
// Declaration of static constants.  They are not allowed as initializers in
// class definitions.
//

char * const   Mesh_Prep::SILO_LOCATION      = "/";
char * const   Mesh_Prep::COORDS_NAME        = "Coords";


// ****************************************************************************
//  Method: Mesh_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Mesh_Prep::Mesh_Prep()
{
    readCoordinateDimension  = false;
    readDataType             = false;
    readLabels               = false;
    readMeshType             = false;
    readUnits                = false;
}


// ****************************************************************************
//  Method: Mesh_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Mesh_Prep::~Mesh_Prep()
{
    // Do nothing
}


// ****************************************************************************
//  Method: Mesh_Prep::SetName
//
//  Purpose:
//      Names the mesh.
//  
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
Mesh_Prep::SetName(char *n)
{
    char   s[LONG_STRING];

    name = CXX_strdup(n);

    sprintf(s, "%s%s", name, SILO_TOPOLOGY_NAME);
    topology = CXX_strdup(s);

    sprintf(s, "%s%s", name, COORDS_NAME);
    coordinates = CXX_strdup(s);

    sprintf(s, "%s%s%s", name, COORDS_NAME, IntervalTree::NAME);
    intervalTree = CXX_strdup(s);
}


// ****************************************************************************
//  Method: Mesh_Prep::SetDomains
//
//  Purpose:
//      Sets the number of domains.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
Mesh_Prep::SetDomains(int n)
{
    nDomains = n;
}


// ****************************************************************************
//  Method: Mesh_Prep::ReadMesh (DBpointmesh)
//
//  Purpose: 
//      Reads in the information from a DBpointmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Add new argument for meshType to ReadMesh.
//
// ****************************************************************************

void
Mesh_Prep::ReadMesh(DBpointmesh *pm)
{
    ReadMesh(pm->datatype, pm->labels, pm->units, pm->ndims, POINT);
}


// ****************************************************************************
//  Method: Mesh_Prep::ReadMesh (DBucdmesh)
//
//  Purpose: 
//      Reads in the information from a DBucdmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Add new argument for meshType to ReadMesh.
//
// ****************************************************************************

void
Mesh_Prep::ReadMesh(DBucdmesh *um)
{
    ReadMesh(um->datatype, um->labels, um->units, um->ndims, UNSTRUCTURED);
}


// ****************************************************************************
//  Method: Mesh_Prep::ReadMesh (DBquadmesh)
//
//  Purpose: 
//      Reads in the information from a DBquadmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Add new argument for meshType to ReadMesh.
//
// ****************************************************************************

void
Mesh_Prep::ReadMesh(DBquadmesh *qm)
{
    MESH_TYPE_e mType;
    if (qm->coordtype == DB_COLLINEAR)
    {
        mType = RECTILINEAR;
    }
    else
    {
        mType = CURVILINEAR;
    }

    ReadMesh(qm->datatype, qm->labels, qm->units, qm->ndims, mType);
}


// ****************************************************************************
//  Method: Mesh_Prep::ReadMesh
//
//  Purpose:
//      Assigns the data members the values read out of the SILO meshes.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Add new argument for meshType to ReadMesh.
//
// ****************************************************************************

void
Mesh_Prep::ReadMesh(int d, char **l, char **u, int c, MESH_TYPE_e m)
{
    if (readDataType == false && d != -1)
    {
        dataType = d;
        readDataType = true;
    }
 
    if (readMeshType == false)
    {
        meshType = m;
        readMeshType = true;
    }

    if (readLabels == false && l != NULL && l[0] != NULL)
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            if (l[i] == NULL)
            {
                continue;
            }
            labels[i] = CXX_strdup(l[i]);
        }
        readLabels = true;
    }

    if (readUnits == false && u != NULL && u[0] != NULL)
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            if (u[i] == NULL)
            {
                continue;
            }
            units[i] = CXX_strdup(u[i]);
        }
        readUnits = true;
    }

    if (readCoordinateDimension == false && c != -1)
    {
        coordinateDimension = c;
        readCoordinateDimension = true;
    }
}


// ****************************************************************************
//  Method:  Mesh_Prep::Consolidate
//
//  Purpose:
//      Gives the Mesh object a chance to set up fields after the initial pass
//      is done.  
//
//  Note:       This is currently nothing, but the function was put in in case
//              the coords Field was pushed down to this object.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Mesh_Prep::Consolidate(void)
{
    // Do nothing
}


// ****************************************************************************
//  Method: Mesh_Prep::WrapUp
//
//  Purpose:
//      Gives the Mesh object a chance to set up fields after the final pass
//      is done.
//
//  Note:       This is currently nothing, but the function was put in in case
//              the coords Field was pushed down to this object.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Mesh_Prep::WrapUp(void)
{
    // Do nothing
}


// ****************************************************************************
//  Method: Mesh_Prep::Write
//
//  Purpose:
//      Does the meshtvprep work needed before the Mesh is written.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Mesh_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    //
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Now that the overhead for meshtvprep is done, call the 
    // real Write function.
    //
    Mesh::Write(dbfile);
}


