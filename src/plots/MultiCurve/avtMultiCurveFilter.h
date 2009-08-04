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
//                           avtMultiCurveFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MultiCurve_FILTER_H
#define AVT_MultiCurve_FILTER_H


#include <avtDatasetToDatasetFilter.h>
#include <MultiCurveAttributes.h>


// ****************************************************************************
//  Class: avtMultiCurveFilter
//
//  Purpose:
//      This operator is the implied operator associated with a MultiCurve
//      plot.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Tue Mar  3 15:06:26 PST 2009
//    I added yAxisTickSpacing, so that I could pass it along in the
//    plot information, so that the plot could include it in the legend.
//
//    Eric Brugger, Fri Mar  6 08:19:58 PST 2009
//    I modified the filter could would also accept as input a collection
//    of poly data data sets representing the individual curves to display.
//
// ****************************************************************************

class avtMultiCurveFilter : public avtDatasetToDatasetFilter
{
  public:
                              avtMultiCurveFilter(MultiCurveAttributes &);
    virtual                  ~avtMultiCurveFilter();

    virtual const char       *GetType(void)   { return "avtMultiCurveFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Performing MultiCurve"; };

    void                      SetAttributes(const MultiCurveAttributes &);

  protected:
    MultiCurveAttributes      atts;
    bool                      setYAxisTickSpacing;
    double                    yAxisTickSpacing;

    virtual void              Execute(void);

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
};


#endif
