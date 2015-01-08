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
//                       avtMultiCurveLabelMapper.h                          //
// ************************************************************************* //

#ifndef AVT_MULTI_CURVE_LABEL_MAPPER_H
#define AVT_MULTI_CURVE_LABEL_MAPPER_H

#include <avtDecorationsMapper.h>
#include <ColorAttributeList.h>
#include <LineAttributes.h>

#include <vector>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtMultiCurveLabelMapper
//
//  Purpose:
//    A mapper for labels for the multi curve plot.  This extends the
//    functionality of a decorations mapper by mapping markers and ids
//    onto a dataset.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:02:11 PST 2009
//    I added the ability to display identifiers at each of the points.
//    I replaced setLabelVisibility with setMarkerVisibility and added
//    setIdVisibility.
//
//    Eric Brugger, Mon Mar  9 17:57:34 PDT 2009
//    I enhanced the plot so that the markers and identifiers displayed for
//    the points are in the same color as the curve, instead of always in
//    black.
//
//    Eric Brugger, Tue Feb 19 16:15:46 PST 2013
//    I added the ability to set a scale factor and the line width for the
//    markers.
//
// ****************************************************************************

class avtMultiCurveLabelMapper : public avtDecorationsMapper
{
  public:
                               avtMultiCurveLabelMapper();
    virtual                   ~avtMultiCurveLabelMapper();

    void                       SetScale(double);
    void                       SetMarkerLineWidth(_LineWidth);
    void                       SetMarkerVisibility(bool);
    void                       SetIdVisibility(bool);

    void                       SetColors(const ColorAttributeList &c);
    void                       GetLevelColor(const int, double[4]);

  protected:
    bool                       markerVisibility;
    bool                       idVisibility;
    double                     scale;
    _LineWidth                 markerLineWidth;

    ColorAttributeList         cal;
    std::vector<int>           colors;

    virtual void               CustomizeMappers(void);
    virtual void               SetDatasetInput(vtkDataSet *, int);
};


#endif
