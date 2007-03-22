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
//                            avtVariableLegend.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_LEGEND_H
#define AVT_VARIABLE_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>

class  vtkVerticalScalarBarActor;
class  vtkLookupTable;


// ****************************************************************************
//  Class: avtVariableLegend
//
//  Purpose:
//      The legend for any plot that is variable based.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Dec  8 15:02:31 PST 2000 
//    Changed sBar to be of type vtkVerticalScalarBarActor, a modified
//    version of the original, better suited to VisIt.  Added method
//    'SetVarName' so that the variable name used for this plot could
//    be part of the title.
//
//    Kathleen Bonnell, Thu Mar 15 13:16:41 PST 2001 
//    Added SetTitle and SetMessage methods. 
//
//    Kathleen Bonnell, Fri Mar 30 12:30:33 PDT 2001 
//    Added SetScaling and SetVarRange methods. 
//
//    Brad Whitlock, Thu Apr 19 15:29:36 PST 2001
//    Added a second constructor that does not create the scalar bar actor.
//
//    Brad Whitlock, Fri Jun 15 10:07:21 PDT 2001
//    Added the SetLUTColors method.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Functionality of SetLUTColors is now in avtLookupTable.  Removed
//    this method.  Add SetLookupTable.
//
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Added member 'title'. 
//    
//    Eric Brugger, Mon Jul 14 15:53:54 PDT 2003
//    Moved SetVarName, SetTitle and SetMessage to avtLegend.
//
//    Eric Brugger, Wed Jul 16 08:15:15 PDT 2003
//    I added  GetLegendSize.  I renamed SetColorBar to SetColorBarVisibility,
//    SetRange to SetVarRangeVisibility and GetRange to GetVarRange.  I also
//    added data members to track the colorbar visibility and variable range
//    visibility.
//
//    Eric Brugger, Thu Jul 17 08:45:29 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Added virtual methods ChangeTitle, ChangeFontHeight.
// 
//    Brad Whitlock, Wed Mar 21 10:00:31 PDT 2007
//    Added virtual methods SetDrawLabels/GetDrawLabels,SetFormatString.
//
// ****************************************************************************

class PLOTTER_API avtVariableLegend : public avtLegend
{
  public:
                               avtVariableLegend();
                               avtVariableLegend(int arg);
    virtual                   ~avtVariableLegend();

    virtual void               GetLegendSize(double, double &, double &);

    virtual void               SetLabelVisibility(bool);
    virtual bool               GetLabelVisibility() const;
    virtual void               SetNumberFormat(const char *);

    virtual void               SetLegendScale(double xScale, double yScale);
    virtual void               SetBoundingBoxVisibility(bool);
    virtual void               SetBoundingBoxColor(const double *);
    virtual void               SetOrientation(LegendOrientation);
    virtual void               SetFont(int family, bool bold, bool italic, bool shadow);

    void                       SetColorBarVisibility(const int);
    void                       SetRange(double min, double max);
    void                       GetRange(double &, double &);
    void                       SetScaling(int mode = 0, double skew = 1.);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetVarRangeVisibility(const int);
    void                       SetVarRange(double min, double max);

  protected:
    double                     min, max;

    vtkVerticalScalarBarActor *sBar;
    vtkLookupTable            *lut;

    double                     scale[2];
    int                        barVisibility;
    int                        rangeVisibility;
    bool                       labelVisibility;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


