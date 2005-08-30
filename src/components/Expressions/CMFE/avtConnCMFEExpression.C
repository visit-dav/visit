// ************************************************************************* //
//                          avtConnCMFEExpression.C                          //
// ************************************************************************* //

#include <avtConnCMFEExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

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

    if (numNull == 1)
        EXCEPTION1(InvalidMergeException, "The databases cannot be compared "
                   "because they have a different number of domains.");

    int nc1 = in1->GetNChildren();
    int nc2 = in1->GetNChildren();

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

        //
        // Make some basic checks to make sure the connectivities really do
        // agree.
        //
        char msg[1024];
        if (in_ds1->GetNumberOfCells() != in_ds2->GetNumberOfCells())
        {
            sprintf(msg, "The databases cannot be compared because they have "
                         " a different number of cells for domain %d.  The "
                         " cell counts are %d and %d.", 
                         in1->GetDataRepresentation().GetDomain(),
                         in_ds1->GetNumberOfCells(), 
                         in_ds2->GetNumberOfCells());
            EXCEPTION1(InvalidMergeException, msg);
        }
        if (in_ds1->GetNumberOfPoints() != in_ds2->GetNumberOfPoints())
        {
            sprintf(msg, "The databases cannot be compared because they have "
                         " a different number of points for domain %d.  The "
                         " point counts are %d and %d.", 
                         in1->GetDataRepresentation().GetDomain(),
                         in_ds1->GetNumberOfPoints(), 
                         in_ds2->GetNumberOfPoints());
            EXCEPTION1(InvalidMergeException, msg);
        }

        bool isPoint      = true;
        vtkDataArray *arr = NULL;
        arr               = in_ds2->GetPointData()->GetArray(invar.c_str());
        if (arr == NULL)
        {
            arr     = in_ds2->GetCellData()->GetArray(invar.c_str());
            isPoint = false;
        }
        if (arr == NULL)
        {
            sprintf(msg, "The databases cannot be compared because variable "
                         " \"%s\" cannot be located.", invar.c_str());
            EXCEPTION1(InvalidMergeException, msg);
        }

        vtkDataArray *addvar = NULL;
        bool deleteAddvar = false;
        if (invar == outvar)
        {
            addvar       = arr;
            deleteAddvar = false;
        }
        else
        {
            addvar = arr->NewInstance();
            addvar->DeepCopy(arr);
            addvar->SetName(outvar.c_str());
            deleteAddvar = true;
        }
        
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
                EXCEPTION1(InvalidMergeException, "The databases cannot be "
                  "compared because they have a different number of domains.");
            }
        }
        avtDataTree_p rv = new avtDataTree(nc1, outDT);
        delete [] outDT;
        return (rv);
    }
}


