// ************************************************************************* //
//                           avtResamplePreprocessor.C                       //
// ************************************************************************* //

#include <avtResamplePreprocessor.h>

#include <vtkDataSetWriter.h>

#include <avtResampleFilter.h>
#include <avtTerminatingSource.h>

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
    
    avtTerminatingSource *src = dob->GetTerminatingSource();
    avtPipelineSpecification_p spec = src->GetGeneralPipelineSpecification();

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


