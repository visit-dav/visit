// ************************************************************************* //
//                             IntervalTree.C                                //
// ************************************************************************* //

#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <IntervalTree.h>
#include <SiloObjLib.h>


// 
// Declaration of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const   IntervalTree::NAME                    = "IntervalTree";
int    const   IntervalTree::N_DIMS_LIMIT            = 100;
int    const   IntervalTree::SILO_NUM_COMPONENTS     = 4;
char * const   IntervalTree::SILO_TYPE               = IntervalTree::NAME;

char * const   IntervalTree::SILO_N_NODES_NAME       = "NNodes";
char * const   IntervalTree::SILO_NODE_EXTENTS_NAME  = "NodeExtents";
char * const   IntervalTree::SILO_NODE_IDS_NAME      = "NodeIDs";
char * const   IntervalTree::SILO_N_DIMS_NAME        = "NDims";
char * const   IntervalTree::SILO_OBJ_NAME           = IntervalTree::NAME;


// ****************************************************************************
//  Method: IntervalTree constructor
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

IntervalTree::IntervalTree()
{
    nNodes      = 0;
    nDomains    = 0;
    nDims       = 0;
    nodeExtents = NULL;
    nodeIDs     = NULL;

    name        = NULL;
}


// ****************************************************************************
//  Method: IntervalTree destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

IntervalTree::~IntervalTree()
{
    if (nodeExtents != NULL)
    {
        delete [] nodeExtents;
    }
    if (nodeIDs != NULL)
    {
        delete [] nodeIDs;
    }
    if (name != NULL)
    {
        delete [] name;
    }
}


// ****************************************************************************
//  Method: IntervalTree::Write
//
//  Purpose:
//      Writes out the interval tree to a SILO file.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree::Write(DBfile *dbfile)
{
    char absName[LONG_STRING];
   
    //
    // Create an object ot be written into the SILO file.
    //
    sprintf(absName, "%s%s", name, SILO_OBJ_NAME);
    DBobject *siloObj = DBMakeObject(absName, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Cannot create a SILO object for the interval tree." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);

    DBAddIntComponent(siloObj, SILO_N_NODES_NAME, nNodes);
    DBAddIntComponent(siloObj, SILO_N_DIMS_NAME, nDims);
    
    int  totalValues = nNodes * 2 * nDims;
    sprintf(absName, "%s%s", name, SILO_NODE_EXTENTS_NAME);
    DBWrite(dbfile, absName, nodeExtents, &totalValues, 1, DB_FLOAT);
    DBAddVarComponent(siloObj, SILO_NODE_EXTENTS_NAME, absName);

    sprintf(absName, "%s%s", name, SILO_NODE_IDS_NAME);
    DBWrite(dbfile, absName, nodeIDs, &nNodes, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_NODE_IDS_NAME, absName);

    //
    // Write and free the SILO object.
    //
    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: IntervalTree::PrintSelf
//
//  Purpose:
//      Prints out the interval tree.  Intended for only debugging purposes.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree::PrintSelf(ostream &out)
{
    out << "IntervalTree (" << name << ")" << endl;
   
    out << "\tnNodes = " << nNodes << endl;
    out << "\tnDomain = " << nDomains << endl;
    out << "\tnDims = " << nDims << endl;

    out << "\tnodeIDs = { " ;
    if (nodeIDs == NULL)
    {
        out << "(nil)";
    }
    else
    {
        for (int i = 0 ; i < nNodes ; i++)
        {
            out << nodeIDs[i] << ", ";
        }
    }
    out << " }" << endl;

    out << "\tnodeExtents = { " ;
    if (nodeExtents == NULL)
    {
        out << "(nil)" ;
    }
    else
    {
        out << endl;
        for (int i = 0 ; i < nNodes ; i++)
        {
            out << "\t\t" ;
            for (int j = 0 ; j < nDims ; j++)
            {
                out << "(" << nodeExtents[i*nDims+2*j] << ", " 
                    << nodeExtents[i*nDims+2*j+1] << "), " ;
            }
            out << endl;
        }
        out << "\t" ;
    }
    out << " }" << endl;
}


// ****************************************************************************
//  Method: IntervalTree::Read
//
//  Purpose:
//      Reads in an IntervalTree object from a DBobject.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
IntervalTree::Read(DBobject *siloObj, DBfile *dbfile)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create an IntervalTree object from a " << siloObj->type
             << " object." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);

    //
    // Populate the object by going through the components.
    //
    char  *nodeExtentsName = NULL;
    char  *nodeIDsName     = NULL;
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routine to get the value from the PDB
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_N_NODES_NAME) == 0)
        {
            nNodes = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_DIMS_NAME) == 0)
        {
            nDims  = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_NODE_EXTENTS_NAME) == 0)
        {
            nodeExtentsName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_NODE_IDS_NAME) == 0)
        {
            nodeIDsName = siloObj->pdb_names[i];
        }
        else
        {
            cerr << "Invalid component " << siloObj->comp_names[i] 
                 << " for IntervalTree." << endl;
            exit(EXIT_FAILURE);
        }
    }   // End 'for' over all components in the DBobject
 
    //
    // Read in the node extents array.
    // We will allocate the space so that purify will not give us a free
    // memory mismatch from C-alloc/C++-dealloc.
    //
    if (nodeExtentsName == NULL)
    {
        cerr << "Never read in the node extents name, cannot get array." 
             << endl;
        exit(EXIT_FAILURE);
    }
    if (nDims <= 0 || nNodes <= 0)
    {
        cerr << "Did not read in the information needed to construct an array"
             << " for the extents." << endl;
        exit(EXIT_FAILURE);
    }
    nodeExtents = new float[nDims*nNodes*2];
    DBReadVar(dbfile, nodeExtentsName, nodeExtents);

    //
    // Read in the node IDs array.
    //
    if (nodeIDsName == NULL)
    {
        cerr << "Never read in the node IDs name, cannot get array." 
             << endl;
        exit(EXIT_FAILURE);
    }
    nodeIDs = new int[nNodes];
    DBReadVar(dbfile, nodeIDsName, nodeIDs);

    //
    // The number of leaf nodes is 1/2 the number of total nodes. Start with
    // one node, it has 1 leaf.  If you add two children to the root, then the 
    // number of leaves increases by 1  (gained two leaves and lost one) and 
    // the number of nodes increases by two.  Repeat x times.  nNodes = 2x, 
    // nLeaves = x.  The number of domains is the number of leaves for an 
    // interval tree.
    //
    nDomains = nNodes / 2;
    if (nNodes % 2 == 1)
    {   
        // Off by one case
        nDomains++;
    }
}


// ****************************************************************************
//  Method: IntervalTree::GetExtents
//
//  Purpose:
//      Sees what the range of values the variable contained by the interval
//      tree has.  This is all contained in the first node.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2000
//
// ****************************************************************************

void
IntervalTree::GetExtents(float *extents)
{
    if (nodeExtents == NULL)
    {
        cerr << "The node extents have not been set, so the extents cannot be "
             << "determined." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Copy the root cell into the extents.
    //
    for (int i = 0 ; i < nDims*2 ; i++)
    {
        extents[i] = nodeExtents[i];
    }
}


