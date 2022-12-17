// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtExpressionDataTreeIterator.C                       //
// ************************************************************************* //

#include <avtExpressionDataTreeIterator.h>

#include <math.h>
#include <float.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtExtents.h>
#include <avtCallback.h>
#include <avtCommonDataFunctions.h>

#include <avtExprNode.h>

#ifdef HAVE_LIBVTKM
#include <avtVtkmDataSet.h>
#endif

#include <DebugStream.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtExpressionDataTreeIterator constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
//  Modifications:
//
//    Eddie Rusu, Tue Jan 14 13:41:26 PST 2020
//    Initialize the volumeDependent array. Default dependency to false.
//
// ****************************************************************************

avtExpressionDataTreeIterator::avtExpressionDataTreeIterator()
{
    volumeDependent = vtkBitArray::New();
    volumeDependent->SetName("VolumeDependent");
    volumeDependent->SetNumberOfComponents(1);
    volumeDependent->SetNumberOfTuples(1);
    volumeDependent->SetComponent(0, 0, false); // Default volume dependency to false
}


// ****************************************************************************
//  Method: avtExpressionDataTreeIterator destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
//  Modifications:
//
//    Eddie Rusu, Tue Jan 14 13:41:26 PST 2020
//    Delete the volumeDependent array to keep memory clean.
//
// ****************************************************************************

avtExpressionDataTreeIterator::~avtExpressionDataTreeIterator()
{
    volumeDependent->Delete();
}


// ****************************************************************************
//  Method: avtExpressionDataTreeIterator::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      in_dr     The input data representation.
//
//  Returns:      The output data representation.
//
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
//  Modifications:
//
//    Sean Ahern, Tue Jun 11 16:35:48 PDT 2002
//    Modified to work with both point and cell data.  Note, this doesn't
//    yet handle filters that convert from point to cell, or vice versa.
//
//    Akira Haddox, Thu Aug 15 16:41:44 PDT 2002
//    Modified to work with vector expressions.
//
//    Akira Haddox, Mon Aug 19 16:39:07 PDT 2002 
//    Modified to use IsPointVariable to determine variable type.
//
//    Hank Childs, Mon Sep  2 18:53:40 PDT 2002
//    Do not use enhanced connectivity points when calculating ranges.
//
//    Hank Childs, Tue Nov 19 08:50:42 PST 2002
//    Use the dimension of the returned type instead of the virtual function
//    call to decide if the variable is a vector.  Did not remove virtual
//    function to determine this, since it may be used before execution (by the
//    GUI, for example to decide if something is a vector,scalar, etc).
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Use NewInstance instead of MakeObject, new vtk api. 
//
//    Hank Childs, Thu Jan 23 11:19:14 PST 2003
//    Set only the cumulative extents.
//
//    Hank Childs, Mon Sep 22 08:34:14 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Thu Jan 29 16:22:06 PST 2004
//    Fix some odd centering cases that can come up when a variable can get
//    misidentified.
//
//    Hank Childs, Wed Feb 25 14:48:31 PST 2004
//    Make sure that the extents get associated with the correct variable.
//
//    Kathleen Bonnell, Thu Mar 11 11:24:40 PST 2004 
//    DataExtents now always only 2 components.  Allow for Tensor var's range
//    to be computed. 
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data arrays.
//
//    Hank Childs, Mon Dec 27 10:28:51 PST 2004
//    Moved from avtExpressionFilter to avtExpressionDataTreeIterator.
//
//    Hank Childs, Mon Jan  3 09:15:40 PST 2005
//    Check to see if the variable already exists.  If so, no need to 
//    re-execute.
//
//    Hank Childs, Thu Jul 28 09:06:00 PDT 2005
//    Fix UMR that can occur with array variables.
//
//    Hank Childs, Tue Aug 30 13:46:19 PDT 2005
//    Move code for calculating cumulative extents to avtExpressionFilter.
//
//    Hank Childs, Tue Nov 15 11:41:28 PST 2005
//    Make a comment to debug logs when we can "bypass".
//
//    Hank Childs, Sun Feb 19 10:26:49 PST 2006
//    Fix a crash.
//
//    Hank Childs, Fri Jun  9 14:18:11 PDT 2006
//    Remove unused variable.
//
//    Hank Childs, Sun Jan 13 20:26:34 PST 2008
//    Add support for constant singletons.
//
//    Kathleen Bonnell, Tue Apr  7 07:55:25 PDT 2009
//    Delete dat before early return.
//
//    Jeremy Meredith, Tue Apr 28 13:53:48 EDT 2009
//    Had it detect singletons or ambiguous cases before comparing
//    with ncells and npts.  (The old way would do the wrong thing
//    for single-cell data sets.)
//
//    David Camp, Tue May 21 13:56:12 PDT 2013
//    Removed the currentDomainsLabel and currentDomainsIndex variables.
//    This was done to thread the code.
//
//    Eric Brugger, Wed Aug 20 16:25:34 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eddie Rusu, Tue Jan 14 13:41:26 PST 2020
//    Add the volumeDependent tracking array to the vtkDataSet's field data.
//
// ****************************************************************************

avtDataRepresentation *
avtExpressionDataTreeIterator::ExecuteData(avtDataRepresentation *in_dr)
{
    avtDataRepresentation *out_dr = NULL;

#ifdef HAVE_LIBVTKM
    // TODO: Each expression should have a SupportsVTKm() method that we can
    //       call here to see if we should even enter the ExecuteData_VTKm
    //       method.
    if (in_dr->GetDataRepType() == DATA_REP_TYPE_VTKM ||
        avtCallback::GetBackendType() == GlobalAttributes::VTKM)
    {
        out_dr = this->ExecuteData_VTKm(in_dr);
    }
    else
#endif
    {
        out_dr = this->ExecuteData_VTK(in_dr); 
    }

    return out_dr;
}

avtDataRepresentation *
avtExpressionDataTreeIterator::ExecuteData_VTK(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set and domain number.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain = in_dr->GetDomain();

    //
    // Sometimes we are asked to calculate a variable twice.  The easiest way
    // to catch this is to see if we already have the requested variable and
    // not re-derive it if we do.
    //
    vtkDataArray *dat = NULL;
    dat = in_ds->GetPointData()->GetArray(outputVariableName);
    if (dat == NULL)
        dat = in_ds->GetCellData()->GetArray(outputVariableName);
    if (dat != NULL)
    {
        debug1 << "NOTE: variable " << outputVariableName 
               << " already exists and it is not being recalculated." << endl;
        dat->Register(NULL);  // At the end of the routine, we will free this.
    }

    //
    // Start off by having the derived type calculate the derived variable.
    //
    if (dat == NULL)
    {
        dat = DeriveVariable(in_ds, domain);
        if (dat == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, "an unknown error occurred while " 
                  "trying to calculate your expression.  Please contact a "
                  "VisIt developer.");
        }
        dat->SetName(outputVariableName);
    }

    int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    int npts   = rv->GetNumberOfPoints();
    int ncells = rv->GetNumberOfCells();
    int ntups  = dat->GetNumberOfTuples();
    bool isPoint = false;
    if ((ntups == npts) && (ntups == ncells))
    {
        isPoint = IsPointVariable();
    }
    else if (ntups == 1) // Constant singleton.
    {
        isPoint = IsPointVariable();
    }
    else if ((ntups == ncells) && (ntups != npts))
    {
        isPoint = false;
    }
    else if ((ntups == npts) && (ntups != ncells))
    {
        isPoint = true;
    }
    else
    {
        debug1 << "Number of tuples cannot be point or cell variable."
               << endl;
        debug1 << "Var = " << dat->GetName() << endl;
        debug1 << "Ntuples = " << ntups << endl;
        debug1 << "Ncells = " << ncells << endl;
        debug1 << "Npts = " << npts << endl;
        dat->Delete();

        avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
            in_dr->GetDomain(), in_dr->GetLabel());

        rv->Delete();

        return out_dr;
    }

    if (isPoint)
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetPointData()->SetActiveTensors(outputVariableName);
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetCellData()->SetActiveTensors(outputVariableName);
    }

    //
    // Make sure that we don't have any memory leaks.
    //
    dat->Delete();

    rv->GetFieldData()->AddArray(this->volumeDependent);

    avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
        in_dr->GetDomain(), in_dr->GetLabel());

    rv->Delete();

    return out_dr;
}

// ****************************************************************************
// Method: avtExpressionDataTreeIterator::ExecuteData_VTKm
//
// Purpose:
//   Execute the expression using VTKm.
//
// Arguments:
//   in_dr : The input data.
//
// Returns:    
//
// Note:       This is a little different from DeriveVariable in that we pass
//             the avtVtkmDataSet and permit the expression DeriveVariableVTKm
//             method to add the new field to the dataset directly.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar  9 17:19:29 PST 2017
//
// Modifications:
//
// ****************************************************************************

avtDataRepresentation *
avtExpressionDataTreeIterator::ExecuteData_VTKm(avtDataRepresentation *in_dr)
{
#ifdef HAVE_LIBVTKM
    //
    // Get the VTK data set and domain number.
    //
    avtVtkmDataSet *in_ds = in_dr->GetDataVTKm();
    int domain = in_dr->GetDomain();

    // Make a new avtVtkmDataSet and copy the VTKm part from the input to
    // the output. Hopefully that is a shallow copy with some ref counting.
    avtVtkmDataSet *out = new avtVtkmDataSet;
    out->ds = in_ds->ds;
    out->ds.PrintSummary(std::cout);

    // We need to compute the field so let the derived class compute 
    // the field and add it to the output dataset.
    if(!in_ds->ds.HasField(outputVariableName))
    {
        DeriveVariableVTKm(out, domain, activeVariable, outputVariableName);
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(out,
            in_dr->GetDomain(), in_dr->GetLabel());

    return out_dr;
#else
    return NULL;
#endif
}

void
avtExpressionDataTreeIterator::DeriveVariableVTKm(avtVtkmDataSet *, 
    int currentDomainsIndex, const std::string &activeVar, const std::string &outputVar)
{
    /* Dummy Implementation */
}
