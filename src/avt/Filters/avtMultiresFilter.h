/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtMultiresFilter.h                            //
// ************************************************************************* //

#ifndef AVT_MULTIRES_FILTER_H
#define AVT_MULTIRES_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

class vtkDataSet;

class avtMultiresSelection;

// ****************************************************************************
//  Class: avtMultiresFilter
//
//  Purpose:
//    Modify the input to only serve up a subset of the chunks based on the
//    view frustum and the smallest cell size.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:04:58 PDT 2011
//
//  Modifications:
//    Eric Brugger, Fri Dec 20 11:50:18 PST 2013
//    Add support for doing multi resolution data selections.
//
//    Eric Brugger, Thu Jan  2 15:15:54 PST 2014
//    Add support for 3d multi resolution data selections.
//
//    Eric Brugger, Wed Jan  8 16:58:34 PST 2014
//    I added a ViewArea to the multi resolution data selection since the
//    view frustum was insufficient in 3d.
//
// ****************************************************************************

class AVTFILTERS_API avtMultiresFilter : public avtDatasetToDatasetFilter
{
  public:
                           avtMultiresFilter(double *, double *, double *,
                               double *, int *, double, double,
                               double *, double *, double);
    virtual               ~avtMultiresFilter();

    virtual const char    *GetType(void)  {return "avtMultiresFilter";};
    virtual const char    *GetDescription(void) {return "Multires";};

  protected:
    int                    nDims;
    double                 transform2D[16];
    double                 transform3D[16];
    double                 transform[16];
    double                 viewport2D[6];
    double                 viewport3D[6];
    double                 viewport[6];
    int                    windowSize[2];
    double                 viewArea2D;
    double                 viewArea3D;
    double                 viewArea;
    double                 desiredExtents2D[6];
    double                 desiredExtents3D[6];
    double                 desiredExtents[6];
    double                 desiredCellArea;
    double                 actualCellArea;

    int                    selID;

    virtual void           Execute(void);

    virtual avtContract_p  ModifyContract(avtContract_p contract);
};


#endif
