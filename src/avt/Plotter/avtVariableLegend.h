/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                            avtVariableLegend.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_LEGEND_H
#define AVT_VARIABLE_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>

class  vtkVisItScalarBarActor;
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
//    Changed sBar to be of type vtkVisItScalarBarActor, a modified
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
//    Hank Childs, Fri Jan 23 15:39:06 PST 2009
//    Add support for Set/GetMinMax.
//
//    Kathleen Bonnell, Thu Oct  1 14:18:11 PDT 2009
//    Added methods and ivars that allow more user control of tick marks
//    and tick labels.
//
//    Kathleen Biagas, Wed Dec 26 13:15:24 PST 2018
//    Add functions for below and above range colors.
//
//    Alister Maguire, Wed Jan 23 11:04:08 PST 2019
//    Added nanColor and setter. 
//
// ****************************************************************************

class PLOTTER_API avtVariableLegend : public avtLegend
{
  public:
                               avtVariableLegend();
                               avtVariableLegend(int arg);
    virtual                   ~avtVariableLegend();

    virtual void               GetLegendSize(double, double &, double &);

    virtual void               SetTitleVisibility(bool);
    virtual bool               GetTitleVisibility() const;
    virtual void               SetLabelVisibility(int);
    virtual int                GetLabelVisibility() const;
    virtual void               SetMinMaxVisibility(bool);
    virtual bool               GetMinMaxVisibility() const;
    virtual void               SetNumberFormat(const char *);

    virtual void               SetLegendScale(double xScale, double yScale);
    virtual void               SetBoundingBoxVisibility(bool);
    virtual void               SetBoundingBoxColor(const double *);
    virtual void               SetOrientation(LegendOrientation);
    virtual void               SetFont(int family, bool bold, bool italic, bool shadow);

    void                       SetColorBarVisibility(const bool);
    void                       SetRange(double min, double max);
    void                       GetRange(double &, double &);
    void                       SetScaling(int mode = 0, double skew = 1.);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetVarRangeVisibility(const bool);
    void                       SetVarRange(double min, double max);

    virtual void               SetNumTicks(int);
    virtual void               SetUseSuppliedLabels(bool);
    virtual bool               GetUseSuppliedLabels(void); 
    virtual void               SetMinMaxInclusive(bool);
    virtual void               SetSuppliedValues(const doubleVector &);
    virtual void               SetSuppliedLabels(const stringVector &);
    virtual void               GetCalculatedLabels(doubleVector &);
    virtual int                GetType(void) { return 0;}

    void                       SetBelowRangeColor(double, double, double, double);
    void                       SetAboveRangeColor(double, double, double, double);
    void                       UseBelowRangeColor(bool);
    void                       UseAboveRangeColor(bool);

    void                       SetNanColor(double, double, double, double);

  protected:
    double                     min, max;

    vtkVisItScalarBarActor    *sBar;
    vtkLookupTable            *lut;

    double                     scale[2];
    double                     nanColor[4];
    bool                       barVisibility;
    bool                       rangeVisibility;
    bool                       titleVisibility;
    int                        labelVisibility;
    bool                       minmaxVisibility;
    int                        numTicks;
    bool                       useSuppliedLabels;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


