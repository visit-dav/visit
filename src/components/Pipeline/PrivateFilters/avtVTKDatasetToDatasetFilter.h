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
//                      avtVTKDatasetToDatasetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_VTK_DATASET_TO_DATASET_FILTER_H
#define AVT_VTK_DATASET_TO_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>
#include <avtSourceFromDataset.h>


// ****************************************************************************
//  Class: avtVTKDatasetToDatasetFilter
//
//  Purpose:
//      A filter that converts a group of VTK datasets into an avtDataset.  
//      Since avtDatasets are really collections of vtkDataSets, this is
//      really only magic with the pipeline updates.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.
//
// ****************************************************************************

class PIPELINE_API avtVTKDatasetToDatasetFilter : public avtDatasetToDatasetFilter
{
  public:
                               avtVTKDatasetToDatasetFilter();
    virtual                   ~avtVTKDatasetToDatasetFilter();

    virtual const char        *GetType(void)
                                    { return "avtVTKDatasetToDatasetFilter"; };

    virtual void               SetInput(vtkDataSet **, int);

  protected:
    avtSourceFromDataset      *source;

    virtual void               CalcDomainList(void);
    virtual void               Execute(avtDomainList *);
    virtual bool               UpdateInput(avtDomainList *);
};


#endif


