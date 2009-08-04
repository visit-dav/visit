/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                      avtDatasetToVTKDatasetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_VTK_DATASET_FILTER_H
#define AVT_DATASET_TO_VTK_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>


class   vtkAVTPipelineBridge;
class   vtkAppendFilter;
class   vtkDataObject;


// ****************************************************************************
//  Class: avtDatasetToVTKDatasetFilter
//
//  Purpose:
//      This is a filter that converts avtDatasets to a VTK dataset.  It runs
//      all of its domains through an append data set filter to make sure there
//      are no issues with multiple domains.  To make the VTK pipeline magic
//      work, it needs a VTK class that it has hooks into. 
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToVTKDatasetFilter : public avtDatasetToDatasetFilter
{
  public:
                                 avtDatasetToVTKDatasetFilter();
    virtual                     ~avtDatasetToVTKDatasetFilter();

    virtual const char          *GetType()
                                    { return "avtDatasetToVTKDatasetFilter"; };

    virtual vtkDataSet          *GetVTKOutput(void);

  protected:
    vtkAVTPipelineBridge        *bridge;
    vtkAppendFilter             *appendFilter;

    virtual void                 CalcDomainList(void);
    virtual void                 Execute(class avtDomainList *);
};


#include <vtkUnstructuredGridSource.h>

// ****************************************************************************
//  Class: vtkAVTPipelineBridge
//
//  Purpose:
//      An object of this type acts as a bridge between a VTK pipeline and an
//      AVT pipeline.  When a VTK pipeline does an update on the dataset of
//      this source's output, it goes to this bridge, which then starts the
//      AVT pipeline.  This allows for there to be no central executive in
//      the transition between AVT and VTK.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 08:40:06 PDT 2000
//    Change UpdateInformation to UpdateData so we can hook into the VTK
//    pipeline a little better.
//
// ****************************************************************************

class PIPELINE_API vtkAVTPipelineBridge : public vtkUnstructuredGridSource
{
  public:
    static vtkAVTPipelineBridge   *New();
    void                           SetFilter(avtDatasetToVTKDatasetFilter *);

    virtual void                   UpdateData(vtkDataObject *);

  protected:
                                   vtkAVTPipelineBridge();

    avtDatasetToVTKDatasetFilter  *filter;
};


#endif


