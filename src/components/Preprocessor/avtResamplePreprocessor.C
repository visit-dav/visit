/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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


