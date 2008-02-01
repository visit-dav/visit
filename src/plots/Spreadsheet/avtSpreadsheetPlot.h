/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                                 avtSpreadsheetPlot.h                      //
// ************************************************************************* //

#ifndef AVT_Spreadsheet_PLOT_H
#define AVT_Spreadsheet_PLOT_H


#include <avtLegend.h>
#include <avtPlot.h>

#include <SpreadsheetAttributes.h>
#include <avtSpreadsheetRenderer.h>

class     avtUserDefinedMapper;
class     avtSpreadsheetFilter;
class     avtSpreadsheetMapper;

// ****************************************************************************
//  Class:  avtSpreadsheetPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Spreadsheet plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007
//    Set CanDo2DViewScaling to false.
//
//    Hank Childs, Fri Feb  1 13:16:26 PST 2008
//    Define UtilizeRenderingFilters.
//
// ****************************************************************************

class avtSpreadsheetPlot : public avtVolumeDataPlot
{
  public:
                                avtSpreadsheetPlot();
    virtual                    ~avtSpreadsheetPlot();

    virtual const char         *GetName(void) { return "Spreadsheet"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);
    virtual bool                SetForegroundColor(const double *fg);
    virtual bool                CanDo2DViewScaling(void) { return false;}

  protected:
    SpreadsheetAttributes       atts;

    avtSpreadsheetFilter       *SpreadsheetFilter;
    avtUserDefinedMapper       *mapper;
    avtSpreadsheetRenderer_p    renderer;
    bool                        colorsInitialized;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return NULL; };

    virtual bool                UtilizeRenderingFilters(void) { return false; };
};


#endif
