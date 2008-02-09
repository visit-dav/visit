/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
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
//                            avtParallelCoordinatesPlot.h                          //
// ************************************************************************* //

#ifndef AVT_PARALLEL_COORDINATES_PLOT_H
#define AVT_PARALLEL_COORDINATES_PLOT_H

#include <avtPlot.h>
#include <ParallelCoordinatesAttributes.h>

class avtParallelCoordinatesFilter;
class avtLevelsMapper;
class avtLookupTable;


// *****************************************************************************
//  Method: avtParallelCoordinatesPlot
//
//  Purpose: This class draws a parallel coordinate plot.  In this type of plot,
//           an n-dimensional space is represented as n parallel coordinate axes
//           and a point in that space is represented as a polyline of n vertices
//           and n-1 segments whose i-th vertex lies on the i-th axis at a point
//           corresponding to the point's i-th coordinate.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Notes: initial implementation taken from Mark Blair's ParallelAxis plot.
//
//  Modifications:
//
// ****************************************************************************

class avtParallelCoordinatesPlot : public avtSurfaceDataPlot
{
public:
                             avtParallelCoordinatesPlot();
    virtual                 ~avtParallelCoordinatesPlot();

    static avtPlot          *Create();

    virtual const char      *GetName(void) { return "ParallelCoordinatesPlot"; };

    virtual void             SetAtts(const AttributeGroup*);
    virtual void             ReleaseData(void);
    virtual bool             CanDo2DViewScaling(void) { return false;}

protected:
    avtParallelCoordinatesFilter   *parAxisFilter;
    avtLevelsMapper         *levelsMapper;

    avtLookupTable          *avtLUT;
    ParallelCoordinatesAttributes  atts;
    double                   bgColor[3];

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    avtContract_p EnhanceSpecification(avtContract_p in_spec);
    virtual avtLegend_p      GetLegend(void) { return NULL; };
    void                     SetColors();
    virtual bool             SetBackgroundColor(const double *);
};

#endif


