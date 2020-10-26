// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtConnCMFEExpression.C                          //
// ************************************************************************* //

#include <avtConnCMFEExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLongArray.h>
#include <vtkPointData.h>

#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <InvalidMergeException.h>


// ****************************************************************************
//  Method: avtConnCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

avtConnCMFEExpression::avtConnCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConnCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

avtConnCMFEExpression::~avtConnCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConnCMFEExpression::PerformCMFE
//
//  Purpose:
//      Performs a cross-mesh field evaluation based on connectivity
//      differencing.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
// 
// ****************************************************************************

avtDataTree_p
avtConnCMFEExpression::PerformCMFE(avtDataTree_p in1, avtDataTree_p in2,
                                   const std::string &invar,
                                   const std::string &outvar)
{
    avtDataTree_p outtree = ExecuteTree(in1, in2, invar, outvar);
    return outtree;
}


// ****************************************************************************
//  Method: avtConnCMFEExpression::ExecuteTree
// 
//  Purpose:
//      Executes a data tree for the ConnCMFE expression.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Oct  7 11:07:33 PDT 2005
//    Modify warning message to account for material selection.
//
//    Cyrus Harrison, Tue Mar 27 15:37:22 PDT 200
//    Fixed unreachable else statement and incorrect error message
//
//    Hank Childs, Sun Mar 22 14:13:16 CDT 2009
//    Beef up debug statements in exception case.
//
//    Eric Brugger, Tue Mar 19 10:31:48 PDT 2019
//    I added special handling for the case where we are mapping cell
//    data between a point mesh and a polyhedral mesh and the number of
//    points is the same as the number of polyhedra.
//
//    Eric Brugger, Thu Apr  4 14:58:20 PDT 2019
//    I added additional special handling for the case where we are mapping
//    cell data between a point mesh and a polygonal mesh and the number of
//    points is the same as the number of polygons.
//
//    Cyrus Harrison, Wed Apr 17 13:39:18 PDT 2019
//    Added parens to resolve "warning: '&&' within '||'"
//    [Wlogical-op-parentheses]
//
//    Eric Brugger, Wed Oct 14 08:31:02 PDT 2020
//    I modified the logic that determines if a mesh is a point mesh to
//    include poly data.
//
//    Eric Brugger, Thu Oct 22 13:12:05 PDT 2020
//    I added additional special handling for the case where we are mapping
//    cell data from a polyhedral mesh onto a point mesh when the mesh was
//    material selected.
//
// ****************************************************************************

avtDataTree_p
avtConnCMFEExpression::ExecuteTree(avtDataTree_p in1, avtDataTree_p in2,
                                   const std::string &invar,
                                   const std::string &outvar)
{
    int numNull = 0;
    if (*in1 == NULL)
        numNull++;
    if (*in2 == NULL)
        numNull++;
    if (numNull == 2)
        return NULL;

    debug5 << "avtConnCMFEExpression::ExecuteTree: numNull=" << numNull << std::endl;
    if (numNull == 1)
        EXCEPTION1(InvalidMergeException, "The databases cannot be compared "
                   "because they have a different number of domains.");

    int nc1 = in1->GetNChildren();
    int nc2 = in2->GetNChildren();

    debug5 << "avtConnCMFEExpression::ExecuteTree: nc1=" << nc1 << " nc2= " << nc2 << std::endl;
    if (nc1 != nc2)
        EXCEPTION1(InvalidMergeException, "The databases cannot be compared "
                   "because they have a different number of domains.");

    if (nc1 <= 0)
    {
        int numHaveData = 0;
        if (in1->HasData())
            numHaveData++;
        if (in2->HasData())
            numHaveData++;
        debug5 << "avtConnCMFEExpression::ExecuteTree: numHaveData " << numHaveData << std::endl;
        if (numHaveData == 1)
            EXCEPTION1(InvalidMergeException, "The databases cannot be "
                  "compared because they have a different number of domains.");
        if (numHaveData == 0)
            return NULL;

        // 
        // We finally get to get their VTK datasets and compare them.
        //
        vtkDataSet *in_ds1 = in1->GetDataRepresentation().GetDataVTK();
        vtkDataSet *in_ds2 = in2->GetDataRepresentation().GetDataVTK();

        vtkIdType nRealCells1 = in_ds1->GetNumberOfCells();
        vtkIdType nRealCells2 = in_ds2->GetNumberOfCells();
        vtkIdType nRealPoints1 = in_ds1->GetNumberOfPoints();
        vtkIdType nRealPoints2 = in_ds2->GetNumberOfPoints();

        //
        // Unstructured meshes can be either unstructured grids or
        // poly data.
        //
        bool in_ds1_unstructured =
            in_ds1->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ||
            in_ds1->GetDataObjectType() == VTK_POLY_DATA;
        bool in_ds2_unstructured =
            in_ds2->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ||
            in_ds2->GetDataObjectType() == VTK_POLY_DATA;

        //
        // Check for the case where we are mapping between a point mesh
        // and either a polyhedral or polygonal mesh and the number of
        // polyhedra or polygons is the same as the number of points.
        //
        bool handlePointPolyhedralMapping = false;
        if (nRealCells1 != nRealCells2 &&
            in_ds1_unstructured && in_ds2_unstructured)
        {
            debug5 << "avtConnCMFEExpression::ExecuteTree: The cell counts"
                   << " don't match and both grids are unstructured." << endl;
            //
            // Get the real number of cells in each dataset.
            //
            vtkDataArray *orig1 =
                in_ds1->GetCellData()->GetArray("avtOriginalCellNumbers");
            vtkDataArray *orig2 =
                in_ds2->GetCellData()->GetArray("avtOriginalCellNumbers");
            bool isCell =
                in_ds2->GetPointData()->GetArray(invar.c_str()) == NULL &&
                in_ds2->GetCellData()->GetArray(invar.c_str()) != NULL;

            if ( (orig1 != NULL || orig2 != NULL) && isCell)
            {
                if (orig1 != NULL)
                {
                    vtkIdType nTuples = orig1->GetNumberOfTuples();
                    double maxVal = orig1->GetTuple2(0)[1];
                    for (vtkIdType id = 1; id < nTuples; ++id)
                    {
                        double *vals = orig1->GetTuple2(id);
                        if (vals[1] > maxVal) maxVal = vals[1];
                    }
                    nRealCells1 = static_cast<vtkIdType>(maxVal) + 1;
                }

                if (orig2 != NULL)
                {
                    vtkIdType nTuples = orig2->GetNumberOfTuples();
                    double maxVal = orig2->GetTuple2(0)[1];
                    for (vtkIdType id = 1; id < nTuples; ++id)
                    {
                        double *vals = orig2->GetTuple2(id);
                        if (vals[1] > maxVal) maxVal = vals[1];
                    }
                    nRealCells2 = static_cast<vtkIdType>(maxVal) + 1;
                }

                //
                // If the cell counts now match, we can do the special
                // processing of mapping between a point mesh and a
                // polyhedral mesh.
                //
                if (nRealCells1 == nRealCells2)
                {
                    debug5 << "avtConnCMFEExpression::ExecuteTree:"
                           << " Doing special processing of mapping between"
                           << " a point mesh and a polyhedral mesh." << endl;
                    nRealPoints1 = nRealPoints2;
                    handlePointPolyhedralMapping = true;
                }
            }
        }
        else if (nRealCells1 == nRealCells2 && nRealPoints1 != nRealPoints2 &&
            in_ds1_unstructured && in_ds2_unstructured)
        {
            debug5 << "avtConnCMFEExpression::ExecuteTree: The cell counts"
                   << " match, the point counts don't match and both grids"
                   << " are unstructured." << endl;
            bool isCell =
                in_ds2->GetPointData()->GetArray(invar.c_str()) == NULL &&
                in_ds2->GetCellData()->GetArray(invar.c_str()) != NULL;
            if (isCell)
            {
                debug5 << "avtConnCMFEExpression::ExecuteTree:"
                       << " Doing special processing of mapping between"
                       << " a point mesh and a polyhedral mesh." << endl;
                nRealPoints1 = nRealPoints2;
            }
        }

        //
        // Make some basic checks to make sure the connectivities really do
        // agree.
        char msg[1024];
        if (nRealCells1 != nRealCells2)
        {
            avtSILRestrictionTraverser trav(firstDBSIL);
            if (trav.UsesAllMaterials())
            {
                snprintf(msg, 1024,
                             "The databases cannot be compared because they "
                             "have a different number of cells for domain %d."
                             " The cell counts are %d and %d.",
                             in1->GetDataRepresentation().GetDomain(),
                             static_cast<int>(nRealCells1),
                             static_cast<int>(nRealCells2));
            }
            else
            {
                snprintf(msg, 1024,
                             "The databases cannot be compared because they"
                             " have a different number of cells for domain %d."
                             " The cell counts are %d and %d.  It appears that"
                             " you have removed some materials.  Databases "
                             "cannot be compared in this manner when materials"
                             " have been removed (and the simulation is "
                             "Eulerian).",
                             in1->GetDataRepresentation().GetDomain(),
                             static_cast<int>(nRealCells1), 
                             static_cast<int>(nRealCells2));
            }
            EXCEPTION1(InvalidMergeException, msg);
        }
        if (nRealPoints1 != nRealPoints2)
        {
            avtSILRestrictionTraverser trav(firstDBSIL);
            if (trav.UsesAllMaterials())
            {
                snprintf(msg, 1024,
                             "The databases cannot be compared because they "
                             "have a different number of points for domain %d."
                             "  The point counts are %d and %d.", 
                         in1->GetDataRepresentation().GetDomain(),
                         static_cast<int>(nRealPoints1), 
                         static_cast<int>(nRealPoints2));
            }
            else
            {
                snprintf(msg, 1024,
                             "The databases cannot be compared because they "
                             "have a different number of points for domain %d."
                             "  The point counts are %d and %d.  It appears "
                             "that you have removed some materials.  Databases"
                             " cannot be compared in this manner when materials"
                             " have been removed (and the simulation is "
                             "Eulerian).",
                         in1->GetDataRepresentation().GetDomain(),
                         static_cast<int>(nRealPoints1), 
                         static_cast<int>(nRealPoints2));
            }
            EXCEPTION1(InvalidMergeException, msg);
        }

        //
        // Get the data array you are copying from and note whether it is
        // a point or cell variable.
        //
        bool isPoint       = true;
        vtkDataArray *var2 = NULL;
        var2               = in_ds2->GetPointData()->GetArray(invar.c_str());
        if (var2 == NULL)
        {
            var2  = in_ds2->GetCellData()->GetArray(invar.c_str());
            isPoint = false;
        }
        if (var2 == NULL)
        {
            sprintf(msg, "The databases cannot be compared because variable "
                         " \"%s\" cannot be located.", invar.c_str());
            EXCEPTION1(InvalidMergeException, msg);
        }

        // 
        // At this point copy field data from the second data set to
        // the first data set. This is a backdoor method for time
        // varying fields and moving higher element data that is
        // stored in the field data.
        //
        vtkFieldData *fd1 = in_ds1->GetFieldData();
        vtkFieldData *fd2 = in_ds2->GetFieldData();

        if( fd1->GetNumberOfArrays() && fd2->GetNumberOfArrays() )
        {
            for( int i=0; i<fd1->GetNumberOfArrays(); ++i )
            {
                // Copy only arrays found in both datasets.
                const char *name1 = fd1->GetArrayName(i);
                vtkAbstractArray *fp2 = fd2->GetAbstractArray(name1);
        
                if( fp2 )
                {
                    // Make a copy of the array found in dataset2
                    vtkAbstractArray *fp = fp2->NewInstance();
                    fp->DeepCopy( fp2 );

                    // Append '2' to the name so not to have any name
                    // conflicts.
                    char name[128];
                    sprintf( name, "%s2", name1);

                    fp->SetName(name);
                    in_ds1->GetFieldData()->AddArray(fp);
                    fp->Delete();
                }
            }
        }

        //
        // Copy the variable from input2 to input1.
        //
        vtkDataArray *addvar = NULL;
        bool deleteAddvar = false;
        
        if (handlePointPolyhedralMapping)
        {
            //
            // This is the special case of mapping between a point mesh and
            // a polyhedral mesh.
            //
            addvar = var2->NewInstance();
            addvar->SetName(outvar.c_str());
            vtkDataArray *orig1 =
                in_ds1->GetCellData()->GetArray("avtOriginalCellNumbers");
            vtkDataArray *orig2 =
                in_ds2->GetCellData()->GetArray("avtOriginalCellNumbers");
            if (orig1 != NULL && orig2 != NULL)
            {
                // Material selected mesh.
                int ntuples1 = orig1->GetNumberOfTuples();
                int ntuples2 = orig2->GetNumberOfTuples();
                if (ntuples1 < ntuples2)
                {
                    // Polyhedral mesh onto a point mesh.
                    addvar->SetNumberOfTuples(ntuples1);
                    vtkIdType *map = new vtkIdType[nRealCells1];
                    for (vtkIdType i = 0; i < nRealCells1; ++i)
                        map[i] = 0;
                    for (vtkIdType i = 0; i < ntuples1; ++i)
                        map[static_cast<vtkIdType>(orig1->GetTuple2(i)[1])] = i;
                    for (vtkIdType src = 0; src < ntuples2; ++src)
                    {
                        vtkIdType dst =
                            map[static_cast<vtkIdType>(orig2->GetTuple2(src)[1])];
                        addvar->SetTuple(dst, src, var2);
                    }
                    delete [] map;
                }
            }
            else if (orig2 != NULL)
            {
                // Polyhedral mesh onto a point mesh.
                addvar->SetNumberOfTuples(nRealCells1);
                int ntuples = orig2->GetNumberOfTuples();
                for (vtkIdType src = 0; src < ntuples; ++src)
                {
                    vtkIdType dst =
                        static_cast<vtkIdType>(orig2->GetTuple2(src)[1]);
                    addvar->SetTuple(dst, src, var2); 
                }
            }
            else
            {
                // Point mesh onto a polyhedral mesh.
                addvar->SetNumberOfTuples(nRealCells1);
                int ntuples = orig1->GetNumberOfTuples();
                for (vtkIdType dst = 0; dst < ntuples; ++dst)
                {
                    vtkIdType src =
                        static_cast<vtkIdType>(orig1->GetTuple2(dst)[1]);
                    addvar->SetTuple(dst, src, var2); 
                }
            }
            deleteAddvar = true;
        }
        else if (invar == outvar)
        {
            //
            // This is the case where the input and output are the same.
            //
            addvar       = var2;
            deleteAddvar = false;
        }
        else
        {
            //
            // This is the normal case.
            //
            addvar = var2->NewInstance();
            addvar->DeepCopy(var2);
            addvar->SetName(outvar.c_str());
            deleteAddvar = true;
        }

        //
        //  Create the output data tree.
        //
        vtkDataSet *new_obj = (vtkDataSet *) in_ds1->NewInstance();
        new_obj->ShallowCopy(in_ds1);
        if (isPoint)
            new_obj->GetPointData()->AddArray(addvar);
        else
            new_obj->GetCellData()->AddArray(addvar);
        if (deleteAddvar)
            addvar->Delete();
        avtDataTree_p rv = new avtDataTree(new_obj, 
                                      in1->GetDataRepresentation().GetDomain(),
                                      in1->GetDataRepresentation().GetLabel());
        new_obj->Delete();
        return rv;
    }
    else
    {
        //
        // There is more than one input dataset to process
        // and we need an output datatree for each.
        //
        avtDataTree_p *outDT = new avtDataTree_p[nc1];
        for (int j = 0; j < nc1; j++)
        {
            int numPresent = 0;
            if (in1->ChildIsPresent(j))
                numPresent++;
            if (in2->ChildIsPresent(j))
                numPresent++;
            if (numPresent == 0)
            {
                outDT[j] = NULL;
            }
            else if (numPresent == 2)
            {
                outDT[j] = ExecuteTree(in1->GetChild(j), in2->GetChild(j), 
                                       invar, outvar);
            }
            else
            {
                debug1 << "Number of children = " << nc1 << endl;
                debug1 << "In1(j) " << (int) in1->ChildIsPresent(j) << endl;
                debug1 << "In2(j) " << (int) in2->ChildIsPresent(j) << endl;
                EXCEPTION1(InvalidMergeException, "The databases cannot be "
                  "compared because they have a different number of domains.");
            }
        }
        avtDataTree_p rv = new avtDataTree(nc1, outDT);
        delete [] outDT;
        return (rv);
    }
}
