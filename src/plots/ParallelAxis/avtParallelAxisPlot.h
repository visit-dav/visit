/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtParallelAxisPlot.h                          //
// ************************************************************************* //

#ifndef AVT_PARALLEL_AXIS_PLOT_H
#define AVT_PARALLEL_AXIS_PLOT_H

#include <ParallelAxisAttributes.h>

#include <avtPlot.h>
#include <avtParallelAxisRenderer.h>

class avtParallelAxisFilter;
class avtUserDefinedMapper;


// *****************************************************************************
//  Method: avtParallelAxisPlot
//
//  Purpose: This class draws a parallel coordinate plot.  In this type of plot,
//           an n-dimensional space is represented as n parallel coordinate axes
//           and a point in that space is represented as a polyline of n vertices
//           and n-1 segments whose i-th vertex lies on the i-th axis at a point
//           corresponding to the point's i-th coordinate.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//      Jeremy Meredith, Mon Mar 19 11:28:57 EDT 2007
//      Added background color so context will fade nicely into background.
//   
//      Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007
//      Set CanDo2DViewScaling to false.
//
//      Mark Blair, Thu Jul  5 19:06:33 PDT 2007
//      Set up to use custom renderer.  Also removed references to unused
//      levels mapper and color lookup table.
//   
// ****************************************************************************

class avtParallelAxisPlot : public avtSurfaceDataPlot
{
public:
                                avtParallelAxisPlot();
    virtual                    ~avtParallelAxisPlot();

    static avtPlot             *Create();

    virtual const char         *GetName(void) { return "ParallelAxisPlot"; };

    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData(void);
    virtual bool                CanDo2DViewScaling(void) { return false;}

protected:
    avtParallelAxisFilter      *parAxisFilter;
    avtUserDefinedMapper       *parAxisMapper;
    avtParallelAxisRenderer_p   renderer;

    ParallelAxisAttributes      atts;
    double                      bgColor[3];

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);
    avtPipelineSpecification_p  EnhanceSpecification(
                                    avtPipelineSpecification_p in_spec);
    virtual avtLegend_p         GetLegend(void) { return NULL; };
    void                        SetColors();
    virtual bool                SetBackgroundColor(const double *);
};

#endif


