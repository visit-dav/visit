/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                         avtMultiresSelection.h                            //
// ************************************************************************* //

#ifndef AVT_MULTIRES_SELECTION_H
#define AVT_MULTIRES_SELECTION_H
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtMultiresSelection
//
//  Purpose:
//    Specify a data selection with a multi resolution representation
//    that may be specified by one of two methods. The first is by specifying
//    the composite projection transform matrix, the viewport, the window
//    size and the desired cell size. The second is by specifying the desired
//    extents, the view area and the desired cell size. The data selection
//    also contains the actual data extents and cell area provided.
//
//  Programmer: Eric Brugger
//  Creation:   December 20, 2013
//
//  Modifications:
//
// ****************************************************************************

class PIPELINE_API avtMultiresSelection : public avtDataSelection
{
  public:
                            avtMultiresSelection() {} ;
    virtual                ~avtMultiresSelection() {} ;

    virtual const char *    GetType() const
                                { return "Multi Resolution Data Selection"; }
    virtual std::string     DescriptionString(void);

    void                    SetCompositeProjectionTransformMatrix(
                                const double matrix[16]);
    void                    GetCompositeProjectionTransformMatrix(
                                double matrix[16]) const;
    void                    SetViewport(const double viewport[6]);
    void                    GetViewport(double viewport[6]) const;
    void                    SetSize(const int size[2])
                                { windowSize[0] = size[0];
                                  windowSize[1] = size[1]; }
    void                    GetSize(int size[2]) const
                                { size[0] = windowSize[0];
                                  size[1] = windowSize[1]; }
    void                    SetViewArea(double area)
                                { viewArea = area; }
    double                  GetViewArea() const
                                { return viewArea; }
    void                    SetDesiredExtents(const double extents[6]);
    void                    GetDesiredExtents(double extents[6]) const;
    void                    SetActualExtents(const double extents[6]);
    void                    GetActualExtents(double extents[6]) const;
    void                    SetDesiredCellArea(double area)
                                { desiredCellArea = area; }
    double                  GetDesiredCellArea() const
                                { return desiredCellArea; }
    void                    SetActualCellArea(double area)
                                { actualCellArea = area; }
    double                  GetActualCellArea() const
                                { return actualCellArea; }

  private:
    double transformMatrix[16];
    double viewport[6];
    int    windowSize[2];
    double viewArea;
    double desiredExtents[6];
    double actualExtents[6];
    double desiredCellArea;
    double actualCellArea;
};

typedef ref_ptr<avtMultiresSelection> avtMultiresSelection_p;

#endif
