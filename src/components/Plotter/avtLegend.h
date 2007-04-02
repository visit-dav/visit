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
//                                avtLegend.h                                //
// ************************************************************************* //

#ifndef AVT_LEGEND_H
#define AVT_LEGEND_H
#include <plotter_exports.h>


#include <ref_ptr.h>


class  vtkActor2D;
class  vtkRenderer;


// ****************************************************************************
//  Class: avtLegend
//
//  Purpose:
//      Make a legend for the plots.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 15:57:46 PST 2000
//    Removed inheritance from avtDecorator.
//
//    Eric Brugger, Mon Jul 14 15:50:29 PDT 2003
//    Added GetLegendOn, SetFontHeight, SetTitle, SetDatabase, SetVarName,
//    SetMessage and Update.
//
//    Eric Brugger, Wed Jul 16 07:59:54 PDT 2003
//    Made private data members protected.
//
//    Eric Brugger, Thu Jul 17 08:13:45 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Brad Whitlock, Tue Jul 20 16:40:37 PST 2004
//    Added SetVarUnits.
//
//    Kathleen Bonnell, Thu Aug 12 13:07:29 PDT 2004 
//    Added SetGlobalVisibility.
//
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Changed 'legend' from vtkVerticalScalarBarActor to Actor2D to allow
//    different derived types.  Added pure virtual methods ChangeTitle,
//    ChangeFontHeight.
//
//    Brad Whitlock, Tue Mar 20 16:49:18 PST 2007
//    Added method to set/get some legend properties.
//
// ****************************************************************************

class PLOTTER_API  avtLegend
{
  public:
    typedef enum {
        VerticalTextOnRight,
        VerticalTextOnLeft,
        HorizontalTextOnTop,
        HorizontalTextOnBottom,
    } LegendOrientation;

                                  avtLegend();
    virtual                      ~avtLegend();

    virtual void                  Add(vtkRenderer *);
    virtual void                  Remove(void);

    virtual void                  GetLegendPosition(double &, double &);
    virtual void                  SetLegendPosition(double, double);
    virtual void                  GetLegendSize(double, double &, double &);
    virtual void                  LegendOn(void);
    virtual void                  LegendOff(void);
    bool                          GetLegendOn(void) const;
    bool                          GetCurrentlyDrawn() const;

    virtual void                  SetTitleVisibility(bool);
    virtual bool                  GetTitleVisibility() const;
    virtual void                  SetLabelVisibility(bool);
    virtual bool                  GetLabelVisibility() const;
    virtual void                  SetNumberFormat(const char *);

    virtual void                  SetLegendScale(double xScale, double yScale);
    virtual void                  SetBoundingBoxVisibility(bool);
    virtual void                  SetBoundingBoxColor(const double *);
    virtual void                  SetOrientation(LegendOrientation);
    virtual void                  SetFont(int family, bool bold, bool italic, bool shadow);

    virtual void                  SetForegroundColor(const double [3]);
    virtual void                  SetFontHeight(double);

    void                          SetTitle(const char *);
    void                          SetDatabaseInfo(const char *);
    void                          SetVarName(const char *);
    void                          SetVarUnits(const char *);
    void                          SetMessage(const char *);
    void                          SetGlobalVisibility(bool);

    void                          Update();

  protected:
    double                        position[2];
    double                        size[2];
    bool                          legendOn;
    bool                          globalVisibility;
    bool                          currentlyDrawn;
    vtkActor2D                   *legend;
    vtkRenderer                  *renderer;

    double                        fontHeight;
    char                         *title;
    char                         *databaseInfo;
    char                         *varName;
    char                         *varUnits;
    char                         *message;

    virtual void                  ChangePosition(double, double) = 0;
    virtual void                  ChangeTitle(const char *) = 0;
    virtual void                  ChangeFontHeight(double) = 0;
};


typedef ref_ptr<avtLegend> avtLegend_p;


#endif


