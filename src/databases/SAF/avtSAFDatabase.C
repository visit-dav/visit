/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtSAFDatabase.C                            //
// ************************************************************************* //

#include <visitstream.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <vector>

#include <visit-config.h>

#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <InvalidFilesException.h>
#include <avtSILEnumeratedNamespace.h>
#include <avtDataTree.h>

#include "avtDatabaseMetaData.h"
#include "avtSAFDatabase.h"
#include "DebugStream.h"

using     std::vector;

// ****************************************************************************
//  Constructor: avtSAFDatabase constructor
//
//  Arguments:
//    filename   The name of the SAF file.
//
//  Notes:
//    This constructor throws the "InvalidFilesException" exception
//    if the file could not be opened for reading.
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  5 09:43:08 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

avtSAFDatabase::avtSAFDatabase(const char*filename)
{
    SAF_DbProps  dbprops;
    SAF_LibProps libprops;

    //
    // At this point we have no cache (yet). Clearly it isn't tagged either.
    //
    initialized   = false;
    opened        = false;

    //
    // Initialize the library.  Note that this may have already been done,
    // we don't know!
    //
    libprops = saf_createProps_lib();
    saf_setProps_ErrorMode(libprops, SAF_ERRMODE_RETURN);
    saf_setProps_ErrorLogging(libprops, "none");
    saf_init(libprops);
    initialized = true;

    //
    // Open (for read only) the database.  The resulting handle is saved
    // as part of this object.  The database will remain open until this
    // object is closed at which time the database will be closed.
    //
    dbprops = saf_createProps_database();
    saf_setProps_ReadOnly(dbprops);
    if ((db = saf_open_database(filename,dbprops)) == SAF_ERROR_HANDLE)
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    opened = true;

    //
    // Determine the node, edge, face and element categories.
    //
    int nCats = 0;
    SAF_Cat *cats = NULL;
    saf_find_categories(SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY,
                        0, &nCats, &cats);
    nodeCat = SAF_NOT_SET_CAT;
    if (nCats > 0)
    {
        nodeCat = cats[0];
        free(cats);  // SAF allocated.
    }

    nCats = 0;
    cats = NULL;
    saf_find_categories(SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY,
                        1, &nCats, &cats);
    edgeCat = SAF_NOT_SET_CAT;
    if (nCats > 0)
    {
        edgeCat = cats[0];
        free(cats);  // SAF allocated.
    }

    nCats = 0;
    cats = NULL;
    saf_find_categories(SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY,
                        2, &nCats, &cats);
    faceCat = SAF_NOT_SET_CAT;
    if (nCats > 0)
    {
        faceCat = cats[0];
        free(cats);  // SAF allocated.
    }

    nCats = 0;
    cats = NULL;
    saf_find_categories(SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY,
                        3, &nCats, &cats);
    elemCat = SAF_NOT_SET_CAT;
    if (nCats > 0)
    {
        elemCat = cats[0];
        free(cats);  // SAF allocated.
    }

    //
    // Read the states and suites.
    //
    nSuites = 0;
    suites = NULL;
    saf_find_suites(SAF_ALL, db, SAF_ANY_NAME, &nSuites, &suites);

    if (nSuites != 1)
    {
        cerr << "Expected 1 suite, found " << nSuites << "." << endl;
    }

    suiteNames = new char*[nSuites];
    suiteCoords = new SAF_Field[nSuites];
    suiteCoordsCount = new size_t[nSuites];
    suiteCoordsValues = new int*[nSuites];
    int i;
    for (i = 0; i < nSuites; i++)
    {
        DSL_Type suiteType; 

        suiteNames[i] = NULL;
        saf_describe_suite(SAF_ALL, suites[i], &suiteNames[i], NULL,
                           &suiteCoords[i]);

        saf_get_count_and_type_for_field(SAF_ALL, suiteCoords[i],
                                         &suiteCoordsCount[i], &suiteType);

        if (!DSL_same_type(suiteType, DSL_INT))
        {
            cerr << "Expecting coordinate types to be integer" << endl;
        }

        suiteCoordsValues[i] = NULL;
        saf_read_field(SAF_ALL, suiteCoords[i], SAF_WHOLE_FIELD,
                       (void **)&suiteCoordsValues[i]);
    }

    int nStateFields = 0;
    stateFields = NULL;
    saf_find_state_fields(SAF_ALL, suites[0], SAF_ANY_NAME, &nStateFields,
                          &stateFields);
    free(suites);  // SAF allocated

    if (nStateFields != 1)
    {
        cerr << "Expecting 1 state field in the suite, found "
             << nStateFields << "." << endl;
    }

    nStates = 0;
    saf_describe_state_field(SAF_ALL, stateFields[0], &stateTemplate, NULL,
                             &nStates);

    nFields = 0;
    saf_describe_state_tmpl(SAF_ALL, stateTemplate, NULL, NULL, &nFields,
                            NULL);

    fields = new SAF_Field*[nStates];
    for (i = 0; i < nStates; i++)
    {
        fields[i] = NULL;
    }
    fieldNames = new char*[nFields];
    fieldMeshNames = new char*[nFields];
    fieldNComponents = new int[nFields];
    fieldSelfStored = new DSL_Boolean_t[nFields];
    fieldCentering = new avtCentering[nFields];
    coords = -1;

    int suiteIndex = 0;
    int *stateCoords = NULL;
    saf_read_state(SAF_ALL, stateFields[0], &suiteIndex, (void**)&stateCoords,
                   &fields[0]);
    free(stateCoords);  // SAF allocated.

    //
    // Read the field information.
    //
    for (i = 0; i < nFields; i++)
    {
        DSL_Boolean_t isCoord;
        SAF_Cat coeffAssoc;
        int assocRatio;
        SAF_FieldTmpl fieldTemplate;
        SAF_Set baseSpace;

        fieldNames[i] = NULL;
        saf_describe_field(SAF_ALL, fields[0][i], &fieldTemplate,
                           &fieldNames[i], NULL, &isCoord, NULL,
                           &coeffAssoc, &assocRatio, NULL, NULL, NULL,
                           &fieldNComponents[i], NULL, NULL, NULL, NULL);

        //
        // If this is the first coordinate field, then assume it is the
        // coordinate field for the state.
        // HACK!!!  This is a saf library issue, since the default
        // coordinate for a set doesn't work with multiple time steps.
        //
        if (isCoord && coords == -1)
        {
            coords = i;
        }

        //
        // Determine the field centering.
        // HACK!!!  If we can't determine the centering we assume
        // zone centered.
        //
        if (assocRatio == 1)
        {
            if (SAF_EQUIV(coeffAssoc, nodeCat))
            {
                fieldCentering[i] = AVT_NODECENT;
            }
            else if (SAF_EQUIV(coeffAssoc, edgeCat) ||
                     SAF_EQUIV(coeffAssoc, faceCat) ||
                     SAF_EQUIV(coeffAssoc, elemCat))
            {
                fieldCentering[i] = AVT_ZONECENT;
            }
            else
            {
                fieldCentering[i] = AVT_ZONECENT;
            }
        }
        else
        {
            fieldCentering[i] = AVT_ZONECENT;
        }

        saf_describe_field_tmpl(SAF_ALL, fieldTemplate, NULL, &baseSpace,
                                NULL, NULL, NULL, NULL,
                                NULL);

        saf_is_self_stored_field(SAF_ALL, fields[0][i],
                                 &fieldSelfStored[i]);

        if (!fieldSelfStored[i])
        {
            fieldNComponents[i] = GetFieldNComponents(fields[0][i]);
        }

        fieldMeshNames[i] = NULL;
        saf_describe_set(SAF_ALL, baseSpace, &fieldMeshNames[i],
                         NULL, NULL, NULL, NULL, NULL, NULL);
    }

    //
    // Read the set information.
    //
    ReadSetInformation();
}

// ****************************************************************************
//  Destructor: avtSAFDatabase destructor
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  5 09:43:08 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

avtSAFDatabase::~avtSAFDatabase()
{
    if (initialized)
    {
        if (opened)
        {
            //
            //    Close the open database.
            //
            saf_close_database(db);
            opened = false;

            // HACK!!! We are not freeing up memory.
        }
    }
}

// ****************************************************************************
//  Method: avtSAFDatabase::GetOutput
//
//  Purpose:
//    Construct an avtDataTree_p (a tree of avtDatasets). Each avtDataset
//    contains a VTK dataset. This method is used by a VisIt engine to
//    retrieve actual field data (often problem size). Note that it is
//    returned in a VTK friendly form.
//
//  Arguments:
//    spec       A data specification (contains variable, timestep, and SIL
//                                     restriction info)
//    dv         The data validity object for the output.
//
//  Returns:     A data tree.
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  5 09:43:08 PDT 2002
//    Completely rewritten.
//
//    Eric Brugger, Tue Sep 17 10:11:34 PDT 2002
//    Modified to account for changes in avtDataSpecification.
//
// ****************************************************************************

avtDataTree_p
avtSAFDatabase::GetOutput(avtDataSpecification_p spec, avtSourceFromDatabase *)
{
    avtDataTree_p tree = NULL;

    avtSILSpecification sil_spec = spec->GetSIL();
    avtSILRestriction_p restric  = spec->GetRestriction();

    int timestep = spec->GetTimestep();

    //
    // If this is the first time that this state has been accessed
    // then read the state information.
    //
    if (fields[timestep] == NULL)
    {
        int *coords = NULL;
        saf_read_state(SAF_ALL, stateFields[0], &timestep, (void**)&coords,
                       &fields[timestep]);
        free(coords);  // SAF allocated.
    }

    //
    // Initialize the data sets and the set ids.
    //
    vtkDataSet **ds = new vtkDataSet*[nTopologySets];
    vector<int> setIds;
    int i;
    for (i = 0 ; i < nTopologySets; i++)
    {
        ds[i] = NULL;
        setIds.push_back(i);
    }

    //
    // Determine the field if one is present.
    //
    int fieldIndex = -1;

    const char *variable = spec->GetVariable();
    if (variable != NULL)
    {
        int i;
        for (i = 0; i < nFields; i++)
        {
            if (strcmp(variable, fieldNames[i]) == 0)
               break;
        } 
        if (i < nFields)
        {
            fieldIndex = i;
        } 
    }

    //
    // Recursively read the field.  iblock is used to loop over unique
    // blocks in the mesh (sets with self stored topology).  This
    // coding assumes that the tree for the field matches the tree
    // for the sets and topology.
    //
    iblock = 0;

    GetDataSets(ds, restric, fieldIndex, fields[timestep][coords],
                fields[timestep][fieldIndex]);

    //
    // If we need domain labels, then dummy up some names using a
    // zero origin count of domains.
    // HACK!!! The domain label logic needs to be modified when
    // subselections other than domain subselections are allowed.
    // Also, we assume MetaData at time 0 is sufficient for this information
    //
    bool needDomainLabels =
        (GetMetaData(0)->DetermineVarType(variable) != AVT_MATERIAL) &&
        (GetMetaData(0)->DetermineSubsetType(variable) == AVT_DOMAIN_SUBSET);
    if (needDomainLabels)
    {
        int bigHackDomainCounter = 0;

        avtDataTree_p *treelist = new avtDataTree_p[nTopologySets];
        for (int kk = 0 ; kk < nTopologySets; kk++)
        {
            if (ds[kk] == NULL)
            {
                treelist[kk] = NULL;
            }
            else
            {
                char name[1024];
                sprintf(name, "%d", bigHackDomainCounter);
                treelist[kk] = new avtDataTree(ds[kk], setIds[kk],
                                               string(name));
                bigHackDomainCounter++;
            }
        }
        tree = new avtDataTree(nTopologySets, treelist);
        delete [] treelist; // Automatically removes references
    }
    else
    {
        tree = new avtDataTree(nTopologySets, ds, setIds);
    }

    return tree;
}

// ****************************************************************************
//  Method: avtSAFDatabase::SetDatabaseMetaData
//
//  Purpose:
//    Fill a newly created database metadata object (a TOC) with various
//    info such as: mesh name, mesh type (rectilinear, curvilinear,
//    unstructured), mesh dimensions (spatial and topological) number
//    of blocks (this may fall under the SIL), and extents (only if it
//    doesn't have to be computed).
//
//  Arguments:
//    Pmd        A brand new object (is a TOC) which is to be filled.
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  5 09:43:08 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

void
avtSAFDatabase::SetDatabaseMetaData(avtDatabaseMetaData* Pmd)
{
    //
    // Set the mesh information.
    // HACK!!! ALE3D. Assuming spatialDim, topoDim and meshType.
    //
    int spatialDim = 3;
    int topoDim = 3;
    avtMeshType meshType = AVT_UNSTRUCTURED_MESH;
    avtMeshMetaData *mmd;
    mmd = new avtMeshMetaData(SIL->setName, nTopologySets, 0, 0,
                              spatialDim, topoDim, meshType);

    Pmd->Add(mmd);

    //
    // Set the variable information.
    //
    int i;
    for (i = 0; i < nFields; i++)
    {
        if (fieldNComponents[i] == 1)
        {
            avtScalarMetaData *smd =
                new avtScalarMetaData(fieldNames[i], fieldMeshNames[i],
                                      fieldCentering[i]);

            Pmd->Add(smd);
        }
        else if (fieldNComponents[i] == spatialDim)
        {
            avtVectorMetaData *vmd =
                new avtVectorMetaData(fieldNames[i], fieldMeshNames[i],
                                      fieldCentering[i], fieldNComponents[i]);

            Pmd->Add(vmd);
        }
        // HACK!!! We are not handling tensors.  We should probably expose
        // the components as scalars.
    }

    //
    // Set the state count information.
    //
    Pmd->SetNumStates(nStates);

    std::vector<int> cycles;
    for (i = 0; i < nStates; i++)
    {
        cycles.push_back(suiteCoordsValues[0][i]);
    }
    Pmd->SetCycles(cycles);
}

// ****************************************************************************
//  Method: avtSAFDatabase::PopulateSIL
//
//  Purpose:
//    Fill a newly created (avt/wimpy) SIL object with various info such
//    as avtSILSet(s), avtSILCollection(s) and in particular "top" sets.
//
//  Arguments:
//    Psil       The SIL to populate.
//
//  Notes:
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  5 09:43:08 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

void
avtSAFDatabase::PopulateSIL(avtSIL* Psil)
{
    //
    // Reset the SIL ids to zero.  This gives each set a unique id
    // within a SIL.
    //
    SILId = 0;

    //
    // Populate the avtSIL using the SAF SIL.
    //
    AddSet(Psil, SIL, true);
}

// ****************************************************************************
//  Method: avtSAFDatabase::GetAuxiliaryData
//
//  Purpose:
//      ...
//
//  Arguments:
//    spec       A data specification (variable name, timestep, and
//               SIL restriction)
//    rv         A void reference list that makes up the return value.
//    type       The type of auxiliary data we are interested in.
//    args       Additional argument (currently unused).
//
//  Notes:
//    This routine is not yet implemented and just returns an empty list.
//
//  Programmer:  Jim Reus
//  Creation:    March 16, 2001
//
// ****************************************************************************

void
avtSAFDatabase::GetAuxiliaryData(avtDataSpecification_p, VoidRefList &rv,
                                 const char*, void*)
{
    rv.nList = 0;
}

// ****************************************************************************
//  Method: avtSAFDatabase::Query
//
//  Purpose:
//    Queries the db regarding var info for a specific pick point.
//
//  Arguments:
//    dspec      A database specification.
//    pa         The pick attributes in which to story the var information.
//
//  Notes:
//    This routine is not yet implemented and just returns.  Whoever
//    implements this routine can put new info into the Programmer and
//    Creation fields.
//
//  Programmer:  Eric Brugger
//  Creation:    March 7, 2002
//
// ****************************************************************************

void
avtSAFDatabase::Query(avtDataSpecification_p, PickAttributes *)
{
    return;
}

// ****************************************************************************
//  Method: avtSAFDatabase::AddSet
//
//  Purpose:
//    Add a SAF SIL to the avt sil.
//
//  Arguments:
//    Psil       The SIL to populate.
//    SIL        The SAF SIL to use to populate the avt sil.
//    top        A boolean indicating if the top set in the SIL is the top.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

int
avtSAFDatabase::AddSet(avtSIL *Psil, struct SILNode *SIL, bool top)
{
    //
    // Add the set.
    //
    avtSILSet_p set = new avtSILSet(SIL->setName, SILId);
    SILId++;
    
    if (top)
    {
        SIL->setId = Psil->AddWhole(set);
    }
    else
    {
        SIL->setId = Psil->AddSubset(set);
    }

    //
    // Recursively add the sets for each collection category.  For
    // each collection category we add a collection and a set for each
    // set in the collection.
    //
    int i;
    for (i = 0; i < SIL->nCats; i++)
    {
        vector<int> list;

        int j;
        for (j = 0; j < SIL->catNSubsets[i]; j++)
        {
            list.push_back(AddSet(Psil, SIL->catSubsets[i][j], false));
        }
        avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
        avtSILCollection_p coll = new avtSILCollection(SIL->catNames[i],
                                                       SIL->catRoles[i],
                                                       SIL->setId,
                                                       ns);

        Psil->AddCollection(coll);
    }

    return SIL->setId;
}

// ****************************************************************************
//  Method: avtSAFDatabase::GetDataSets
//
//  Purpose:
//    Get the data sets using the specified coordinate and variable fields.
//    This routine is recursive to handle indirect field handles.
//
//  Arguments:
//    ds             The data set array to contian the datasets.
//    restric        The field restriction.
//    fieldIndex     The index of the field to read.
//    coordField     The field handle for the coordinate field.
//    variableField  The field handle for the variable field.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

void
avtSAFDatabase::GetDataSets(vtkDataSet **ds, avtSILRestriction_p restric,
                            int fieldIndex, SAF_Field coordField,
                            SAF_Field variableField)
{
    DSL_Boolean_t isSelfStored;
    saf_is_self_stored_field(SAF_ALL, coordField, &isSelfStored);

    //
    // If the topology is self stored read it, otherwise get the field
    // indirections for the coordinates and variable and recursively
    // read those.
    //
    if (isSelfStored)
    {
        // HACK!!! Assuming a relation type of SAF_UNSTRUCTURED.
        if (restric->UsesSetData(topologySets[iblock]->setId) != NoneUsed)
        {
            ds[iblock] = GetUnstructuredGrid(fieldIndex, coordField,
                                             variableField);
        }

        iblock++;
    }
    else
    {
        //
        // Read the coordinate and variable field indirections.
        //
        size_t coordCount;
        SAF_Field *coordSubFields = NULL;

        saf_get_count_and_type_for_field(SAF_ALL, coordField,
                                         &coordCount, NULL);

        saf_read_field(SAF_ALL, coordField, SAF_WHOLE_FIELD,
                       (void **)&coordSubFields);

        size_t variableCount;
        SAF_Field *variableSubFields = NULL;

        saf_get_count_and_type_for_field(SAF_ALL, variableField,
                                         &variableCount, NULL);

        saf_read_field(SAF_ALL, variableField, SAF_WHOLE_FIELD,
                       (void **)&variableSubFields);

        //
        // Loop over the sub fields.
        //
        int i;
        for (i = 0; i < coordCount; ++i)
        {
            GetDataSets(ds, restric, fieldIndex, coordSubFields[i],
                        variableSubFields[i]);
        }

        free(coordSubFields);     // SAF allocated.
        free(variableSubFields);  // SAF allocated.
    }
}

// ****************************************************************************
//  Method: avtSAFDatabase::GetFieldNComponents
//
//  Purpose:
//    Determine the number of components in the field.
//
//  Arguments:
//    field      The field to get the number of components for.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

int
avtSAFDatabase::GetFieldNComponents(SAF_Field field)
{
    int nComponents;
    DSL_Boolean_t isSelfStored;

    SAF_Field *subFields=NULL;
    saf_read_field(SAF_ALL, field, SAF_WHOLE_FIELD, (void **)&subFields);

    saf_is_self_stored_field(SAF_ALL, subFields[0], &isSelfStored);

    if (isSelfStored)
    {
       saf_describe_field(SAF_ALL, subFields[0], NULL, NULL, NULL, NULL,
                          NULL, NULL, NULL, NULL, NULL, NULL, &nComponents,
                          NULL, NULL, NULL, NULL);
    }
    else
    {
        nComponents = GetFieldNComponents(subFields[0]);
    }

    free(subFields);  // SAF allocated

    return nComponents;
}

// ****************************************************************************
//  Method: avtSAFDatabase::GetUnstructuredGrid
//
//  Purpose:
//    Get an unstructured grid using the specified coordinate and variable
//    fields.
//
//  Arguments:
//    fieldIndex     The index of the field to read.
//    coordField     The field handle for the coordinate field.
//    variableField  The field handle for the variable field.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

vtkUnstructuredGrid *
avtSAFDatabase::GetUnstructuredGrid(int fieldIndex, SAF_Field coordField,
                                    SAF_Field variableField)
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
       
    //
    // Set the coordinate values.
    //
    size_t fieldCount = 0;
    saf_get_count_and_type_for_field(SAF_ALL, coordField, &fieldCount, NULL);
    int nCoords = fieldCount / 3;

    // HACK!!! Assuming double.
    double *coords = NULL;
    saf_read_field(SAF_ALL, coordField, SAF_WHOLE_FIELD, (void **)&coords);

    vtkPoints *pts = vtkPoints::New();
    ugrid->SetPoints(pts);
    pts->SetNumberOfPoints(nCoords);
    int i;
    for (i = 0; i < nCoords; ++i)
    {
        float X = coords[i];
        float Y = coords[nCoords+i];
        float Z = coords[2*nCoords+i];
        pts->SetPoint(i, X, Y, Z);
    }
 
    free(coords);  // SAF allocated.

    //
    // Set the connectivity.
    //
    SAF_Rel topo = topologySets[iblock]->setTopoRelation;

    SAF_RelRep PrepType;
    size_t topoCount = 0;
    saf_get_count_and_type_for_topo_relation(SAF_ALL, topo, &PrepType,
                                             NULL, NULL, &topoCount, NULL);

    int *abuf = NULL, *bbuf = NULL;
    saf_read_topo_relation(SAF_ALL, topo, (void **)&abuf, (void **)&bbuf);

    int cellType = topologySets[iblock]->setCellType;
    int cellSize = abuf[0];
    int nCells = topoCount / cellSize;
    int *topology = bbuf;
    int offset = 0;
    for (i = 0 ; i < nCells; ++i)
    {
        ugrid->InsertNextCell(cellType, cellSize, topology + offset);
        offset += cellSize;
    }

    free(abuf);  // SAF allocated.
    free(bbuf);  // SAF allocated.

    if (fieldIndex != -1)
    {
        //
        // Set the variable.
        //
        size_t fieldCount = 0;
        saf_get_count_and_type_for_field(SAF_ALL, variableField, &fieldCount,
                                         NULL);

        // HACK!!! Assuming double.
        double *var = NULL;
        saf_read_field(SAF_ALL, variableField, SAF_WHOLE_FIELD, (void **)&var);

        if (fieldNComponents[fieldIndex] == 1)
        {
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfComponents(1);
            scalars->SetNumberOfTuples(fieldCount);
            int i;
            for (i = 0 ; i < fieldCount; ++i)
            {
                float V = (float) var[i];
                scalars->SetTuple(i, &V);
            }

            if (fieldCentering[fieldIndex] == AVT_NODECENT)
            {
                ugrid->GetPointData()->SetScalars(scalars);
            }
            else
            {
                ugrid->GetCellData()->SetScalars(scalars);
            }
        }
        // HACK!!! Assuming a 3d coordinate field.
        else if (fieldNComponents[fieldIndex] == 3)
        {
            fieldCount /= 3;
            vtkFloatArray *vectors = vtkFloatArray::New();
            vectors->SetNumberOfComponents(3);
            vectors->SetNumberOfTuples(fieldCount);
            int i;
            for (i = 0 ; i < fieldCount; ++i)
            {
                float V0 = var[i];
                float V1 = var[fieldCount+i];
                float V2 = var[2*fieldCount+i];
                vectors->SetTuple3(i, V0, V1, V2);
            }

            if (fieldCentering[fieldIndex] == AVT_NODECENT)
            {
                ugrid->GetPointData()->SetVectors(vectors);
            }
            else
            {
                ugrid->GetCellData()->SetVectors(vectors);
            }
        } 

        free(var);  // SAF allocated.
    }

    return ugrid;
}

// ****************************************************************************
//  Method: avtSAFDatabase::PopulateTopologySets
//
//  Purpose:
//    Populate the topology sets list of sets with self stored topology
//    by recursively traversing the SIL.
//
//  Arguments:
//    SIL        The SAF SIL to recursively traverse.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

void
avtSAFDatabase::PopulateTopologySets(SILNode *SIL)
{
    if (SIL->setSelfStored)
    {
        topologySets[nTopologySets] = SIL;
        nTopologySets++;
    }

    int i;
    for (i = 0; i < SIL->nCats; i++)
    {
        int j;
        for (j = 0; j < SIL->catNSubsets[i]; j++)
        {
            PopulateTopologySets(SIL->catSubsets[i][j]);
        }
    }
}

// ****************************************************************************
//  Method: avtSAFDatabase::ReadSet
//
//  Purpose:
//    Read the SIL for a set based on the specified collection categories.
//
//  Arguments:
//    set        The set to read.
//    nCats      The number of collection categories.
//    cats       The collection categories.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

struct SILNode *
avtSAFDatabase::ReadSet(SAF_Set set, int nCats, SAF_Cat *cats)
{
    struct SILNode *SIL = new struct SILNode;
    int i, j;

    //
    // Determine the name of the set.
    //
    SIL->setSet = set;
    SIL->setName = NULL;
    saf_describe_set(SAF_ALL, set, &(SIL->setName),
                     NULL, NULL, NULL, NULL, NULL, NULL);

    //
    // Determine the topology relation defined on the set.
    // HACK!!! The self stored nature of the topology should be determinable
    // checking SAF_EQUIV(ancestor, set), but that doesn't seem to work.
    //
    SAF_Set ancestor;
    int nRelations = 0;
    SAF_Rel *relations = NULL;
    saf_find_topo_relations(SAF_ALL, set, &ancestor, &nRelations, &relations);

    if (nRelations == 1)
    {
        DSL_Boolean_t isSelfStored;

        saf_is_self_stored_topo_relation(SAF_ALL, relations[0], &isSelfStored);
        if (isSelfStored)
        {
            SIL->setSelfStored = true;
            SIL->setTopoRelation = relations[0];

            SAF_Cat the_pieces;
            saf_describe_topo_relation(SAF_ALL, relations[0], NULL,
                                       &the_pieces, NULL, NULL, NULL, NULL,
                                       NULL, NULL);
 
            SAF_CellType cellType;
            saf_describe_collection(SAF_ALL, set, the_pieces, &cellType,
                                    NULL, NULL, NULL, NULL);

            switch (cellType)
            {
              case SAF_CELLTYPE_POINT:
                SIL->setCellType = VTK_VERTEX;     break;
              case SAF_CELLTYPE_LINE:
                SIL->setCellType = VTK_LINE;       break;
              case SAF_CELLTYPE_TRI:
                SIL->setCellType = VTK_TRIANGLE;   break;
              case SAF_CELLTYPE_QUAD:
                SIL->setCellType = VTK_QUAD;       break;
              case SAF_CELLTYPE_TET:
                SIL->setCellType = VTK_TETRA;      break;
              case SAF_CELLTYPE_PYRAMID:
                SIL->setCellType = VTK_PYRAMID;    break;
              case SAF_CELLTYPE_PRISM:
                SIL->setCellType = VTK_WEDGE;      break;
              case SAF_CELLTYPE_HEX:
                SIL->setCellType = VTK_HEXAHEDRON; break;
              default:
                SIL->setCellType = VTK_EMPTY_CELL; break;
            }

            nTopologySets++;
        }
        else
        {
            SIL->setSelfStored = false;
        }
    }
    else
    {
        SIL->setSelfStored  = false;
    }
    if (relations != NULL) free(relations);

    //
    // Determine the collections in the sil.
    //
    int *nSubsets = new int[nCats];
    SAF_Set **subsets = new SAF_Set*[nCats];
    for (i = 0; i < nCats; i++)
    {
        nSubsets[i] = 0;
        subsets[i] = NULL;
        saf_find_sets(SAF_ALL, SAF_FSETS_SUBS, set, cats[i],
                      &nSubsets[i], &subsets[i]);
    }

    //
    // Determine the non-empty collections.
    //
    int nRealCats = 0;
    for (i = 0; i < nCats; i++)
    {
        if (nSubsets[i] > 0) nRealCats++;
    }

    if (nRealCats == 0)
    {
        SIL->nCats = 0;
        SIL->catNames = NULL;
        SIL->catRoles = NULL;
        SIL->catNSubsets = NULL;
        SIL->catSubsets = NULL;
    }
    else
    {
        SIL->nCats = nRealCats;
        SIL->catNames = new char*[nRealCats];
        SIL->catRoles = new SILCategoryRole[nRealCats];
        SIL->catNSubsets = new int[nRealCats];
        SIL->catSubsets = new struct SILNode**[nRealCats];

        nRealCats = 0;
        for (i = 0; i < nCats; i++)
        {
            if (nSubsets[i] > 0)
            {
                SAF_Role role;

                SIL->catNames[nRealCats] = NULL;
                saf_describe_category(cats[i], &SIL->catNames[nRealCats],
                                      &role, NULL);
                if (saf_equiv((SAF_Role) role, SAF_ASSEMBLY))
                {
                    SIL->catRoles[i] = SIL_ASSEMBLY;
                }
                else if (saf_equiv((SAF_Role) role, SAF_BLOCK))
                {
                    SIL->catRoles[i] = SIL_BLOCK;
                }
                else if (saf_equiv((SAF_Role) role, SAF_DOMAIN))
                {
                    SIL->catRoles[i] = SIL_DOMAIN;
                }
                else if (saf_equiv((SAF_Role) role, SAF_MATERIAL))
                {
                    SIL->catRoles[i] = SIL_MATERIAL;
                }
                else if (saf_equiv((SAF_Role) role, SAF_PROCESSOR))
                {
                    SIL->catRoles[i] = SIL_PROCESSOR;
                }
                else if (saf_equiv((SAF_Role) role, SAF_TOPOLOGY))
                {
                    SIL->catRoles[i] = SIL_TOPOLOGY;
                }
                else
                {
                    SIL->catRoles[i] = SIL_USERD;
                }

                SIL->catNSubsets[nRealCats] = nSubsets[i];

                SIL->catSubsets[nRealCats] = new struct SILNode*[nSubsets[i]];
                for (j = 0; j < nSubsets[i]; j++)
                {
                    SIL->catSubsets[nRealCats][j] = ReadSet(subsets[i][j],
                                                            nCats, cats); 
                }
                nRealCats++;
            }
        }
    }

    //
    // Free temporary storage.
    //
    delete [] nSubsets;
    for (i = 0; i < nCats; i++)
    {
        free(subsets[i]); 
    }
    delete [] subsets;

    return SIL;
}

// ****************************************************************************
//  Method: avtSAFDatabase::ReadSetInformation
//
//  Purpose:
//    Read the set information from the SAF file.
//
//  Programmer:  Eric Brugger
//  Creation:    September 5, 2002
//
// ****************************************************************************

void
avtSAFDatabase::ReadSetInformation()
{
    nTopSets = 0;
    topSets = NULL;
    saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SPACE,
                           SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                           SAF_TOP_TRUE, &nTopSets, &topSets);

    int nCats = 0;
    SAF_Cat *cats = NULL;
    saf_find_categories(topSets[0], SAF_ANY_NAME, SAF_ANY_ROLE,
                        SAF_ANY_TOPODIM, &nCats, &cats);

    //
    // Create a new list of categories, eliminating the node, edge, face,
    // and elem categories.
    //
    int nCats2 = 0;
    SAF_Cat *cats2 = new SAF_Cat[nCats];
    int i;
    for (i = 0; i < nCats; i++)
    {
        if (!(SAF_EQUIV(nodeCat, cats[i]) || SAF_EQUIV(edgeCat, cats[i]) ||
              SAF_EQUIV(faceCat, cats[i]) || SAF_EQUIV(elemCat, cats[i])))
        {
            cats2[nCats2] = cats[i]; 
            nCats2++;
        }
    }

    nTopologySets = 0;
    SIL = ReadSet(topSets[0], nCats2, cats2);

    topologySets = new SILNode*[nTopologySets];
    nTopologySets = 0;
    PopulateTopologySets(SIL);

    //
    // Free temporary storage from the SAF library.  cats is from SAF
    // so we use free, cats2 is from new so we use delete.
    //
    free(cats);  // SAF allocated.
    delete [] cats2;  // VisIt allocated.
}
