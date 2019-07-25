// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtLinearTransformFilter.C
// ************************************************************************* //

#include <avtLinearTransformFilter.h>

#include <avtExtents.h>

#include <BadVectorException.h>


// ****************************************************************************
//  Method: avtLinearTransformFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtLinearTransformFilter::avtLinearTransformFilter()
{
    M = NULL;
    invM = NULL;
}


// ****************************************************************************
//  Method: avtLinearTransformFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtLinearTransformFilter::~avtLinearTransformFilter()
{
    if (M)
        M->Delete();
    M = NULL;
    if (invM)
        invM->Delete();
    invM = NULL;
}


// ****************************************************************************
//  Method:      avtLinearTransformFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
//    Dave Pugmire, Fri May 14 08:04:43 EDT 2010
//    Flag for vector transformations.
//
// ****************************************************************************

void
avtLinearTransformFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const LinearTransformAttributes*)a;
    if (M)
        M->Delete();
    M = NULL;
    if (invM)
        invM->Delete();
    invM = NULL;
    SetVectorTransform(atts.GetTransformVectors());
}


// ****************************************************************************
//  Method:  avtLinearTransformFilter::SetupMatrix
//
//  Purpose:
//    Setup the vtk matrix from the transform attributs
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 15, 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Jul 29 10:27:34 MDT 2010
//    Account for 4x4 transforms.
//
//    Kathleen Biagas, Mon Aug 15 14:15:54 PDT 2016
//    VTK-8: vtkMatrix4x4 no longer has operator[], use SetElement instead.
//
// ****************************************************************************

void
avtLinearTransformFilter::SetupMatrix()
{
    if (M)
        return;

    M = vtkMatrix4x4::New();
    M->Identity();
    invM = vtkMatrix4x4::New();
    invM->Identity();

    M->Identity();
    M->SetElement(0, 0, atts.GetM00());
    M->SetElement(0, 1, atts.GetM01());
    M->SetElement(0, 2, atts.GetM02());
    M->SetElement(0, 3, atts.GetM03());

    M->SetElement(1, 0, atts.GetM10());
    M->SetElement(1, 1, atts.GetM11());
    M->SetElement(1, 2, atts.GetM12());
    M->SetElement(1, 3, atts.GetM13());

    M->SetElement(2, 0, atts.GetM20());
    M->SetElement(2, 1, atts.GetM21());
    M->SetElement(2, 2, atts.GetM22());
    M->SetElement(2, 3, atts.GetM23());

    M->SetElement(3, 0, atts.GetM30());
    M->SetElement(3, 1, atts.GetM31());
    M->SetElement(3, 2, atts.GetM32());
    M->SetElement(3, 3, atts.GetM33());

    if (atts.GetInvertLinearTransform())
    {
        invM->DeepCopy(M);
        vtkMatrix4x4::Invert(invM, M);
    }
    else
    {
        vtkMatrix4x4::Invert(M, invM);
    }
}


// ****************************************************************************
//  Method: avtLinearTransformFilter::PerformRestriciton
//
//  Purpose:
//    Turn on Zone numbers flag if needed, so that original cell array
//    will be propagated throught the pipeline.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtLinearTransformFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);
    if (rv->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (rv->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtLinearTransformFilter::PostExecute
//
//  Purpose:
//      This is called to set the inverse transformation matrix in the output.  
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//    Kathleen Biagas, Mon Aug 15 14:15:54 PDT 2016
//    VTK-8: vtkMatrix4x4 no longer has operator[], use DeepCopy instead.
//
// ****************************************************************************

void
avtLinearTransformFilter::PostExecute()
{
    double m[16];
    vtkMatrix4x4::DeepCopy(m, invM);
    GetOutput()->GetInfo().GetAttributes().SetInvTransform(m);
    vtkMatrix4x4::DeepCopy(m, M);
    GetOutput()->GetInfo().GetAttributes().SetTransform(m);
}
