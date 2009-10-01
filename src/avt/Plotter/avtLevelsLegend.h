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
//                            avtLevelsLegend.h                              // 
// ************************************************************************* //

#ifndef AVT_LEVELS_LEGEND_H
#define AVT_LEVELS_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>
#include <vector>
#include <string>
#include <maptypes.h>

class  vtkVisItScalarBarActor;
class  vtkLookupTable;


// ****************************************************************************
//  Class: avtLevelsLegend
//
//  Purpose:
//      Provides access to the vtk class that builds a legend. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr  2 18:30:33 PDT 2001
//    Added method 'SetVarRange' so that limits text in legend will always
//    be the extents of the var, even if user sets artificial limits.
//
//    Kathleen Bonnell, Mon Sep 24 15:58:35 PDT 2001 
//    Added member vtkLookupTable. 
//
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Added member varName. 
//
//    Kathleen Bonnell, Tue Sep 10 14:14:18 PDT 2002 
//    Added method 'SetLabelColorMap', which maps a label to its corresponding
//    index into the lookup table. 
//
//    Brad Whitlock, Fri Nov 15 10:13:14 PDT 2002
//    I changed the map type.
//
//    Kathleen Bonnell, Mon May 19 13:47:48 PDT 2003  
//    Added method 'SetReverseOrder'.
//
//    Eric Brugger, Mon Jul 14 15:52:46 PDT 2003
//    Moved SetVarName, SetTitle and SetMessage to avtLegend.
//
//    Eric Brugger, Wed Jul 16 08:03:26 PDT 2003
//    I added GetLegendSize.  I renamed SetColorBar to SetColorBarVisibility
//    and SetRange to SetVarRangeVisibility.  I also added data members to
//    track the number of levels, the colorbar visibility and variable range
//    visibility.
//
//    Eric Brugger, Thu Jul 17 08:17:40 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Added virtual methods ChangeTitle, ChangeFontHeight.
// 
//    Brad Whitlock, Wed Mar 21 21:39:27 PST 2007
//    Added some new methods for setting legend properties.
//
//    Dave Bremer, Mon Oct 13 12:36:09 PDT 2008
//    Added SetNumberFormat()
//
//    Hank Childs, Fri Jan 23 15:43:01 PST 2009
//    Added min/max visibility.
//
//    Kathleen Bonnell, Thu Oct  1 14:18:11 PDT 2009
//    Added methods and ivars that allow more user control of tick marks
//    and tick labels.
//
// ****************************************************************************

class PLOTTER_API avtLevelsLegend : public avtLegend
{
  public:
                    avtLevelsLegend();
    virtual        ~avtLevelsLegend();

    virtual void    GetLegendSize(double, double &, double &);

    virtual void    SetTitleVisibility(bool);
    virtual bool    GetTitleVisibility() const;
    virtual void    SetLabelVisibility(int);
    virtual int     GetLabelVisibility() const;
    virtual void    SetMinMaxVisibility(bool);
    virtual bool    GetMinMaxVisibility() const;

    virtual void    SetLegendScale(double xScale, double yScale);
    virtual void    SetBoundingBoxVisibility(bool);
    virtual void    SetBoundingBoxColor(const double *);
    virtual void    SetOrientation(LegendOrientation);
    virtual void    SetFont(int family, bool bold, bool italic, bool shadow);
    virtual void    SetNumberFormat(const char *);

    void            SetColorBarVisibility(const int);
    void            SetRange(double min, double max);
    void            SetLevels(const std::vector<double> &);
    void            SetLevels(const std::vector<std::string> &);
    void            SetLookupTable(vtkLookupTable *);
    void            SetLabelColorMap(const LevelColorMap &);
    void            SetReverseOrder(const bool);
    void            SetVarRangeVisibility(const int);
    void            SetVarRange(double min, double max);

    virtual void    SetUseSuppliedLabels(bool);
    virtual bool    GetUseSuppliedLabels(void); 
    virtual void    SetSuppliedLabels(const stringVector &);
    virtual void    GetCalculatedLabels(stringVector &);
    virtual int     GetType(void) { return 1; }


  protected:
    double                     min, max;
    int                        nLevels;
    double                     scale[2];
    double                     maxScale;
    bool                       setMaxScale;

    vtkLookupTable            *lut;
    vtkVisItScalarBarActor    *sBar;

    int                        barVisibility;
    int                        rangeVisibility;
    bool                       titleVisibility;
    int                        labelVisibility;
    bool                       minmaxVisibility;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


