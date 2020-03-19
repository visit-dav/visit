// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtResamplePreprocessor.C                       //
// ************************************************************************* //

#include <avtResamplePreprocessor.h>

#include <vtkDataSetWriter.h>

#include <avtResampleFilter.h>
#include <avtOriginatingSource.h>

#include <ImproperUseException.h>

#include <ResampleAttributes.h>


// ****************************************************************************
//  Method: avtResamplePreprocessor constructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtResamplePreprocessor::avtResamplePreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtResamplePreprocessor destructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtResamplePreprocessor::~avtResamplePreprocessor()
{
    ;
}


// ****************************************************************************
//  Method: avtResamplePreprocessor::Preprocess
//
//  Purpose:
//      Performs the resampling.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtResamplePreprocessor::Preprocess(void)
{
    avtDataObject_p dob = GetInput();

    ResampleAttributes atts;
    avtResampleFilter resampler(&atts);
    resampler.SetInput(dob);
    
    avtOriginatingSource *src = dob->GetOriginatingSource();
    avtContract_p spec = src->GetGeneralContract();

    avtDataObject_p output = resampler.GetOutput();
    output->Update(spec);

    //
    // Play a trick here, there should be something better set up.
    //
    SetInput(output);
    avtDataTree_p tree = GetInputDataTree();

    int nLeaves;
    vtkDataSet **vtkds = tree->GetAllLeaves(nLeaves);

    if (nLeaves != 1)
    {
        EXCEPTION0(ImproperUseException);
    }

    char filename[1024];
    sprintf(filename, "%s.resample.vtk", stem);

    vtkDataSetWriter *writer = vtkDataSetWriter::New();
    writer->SetInput(vtkds[0]);
    writer->SetFileName(filename);
    writer->Write();

    delete [] vtkds;
}


