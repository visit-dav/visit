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
//                              VisWinAxesArray.h                            //
// ************************************************************************* //

#ifndef VIS_WIN_AXES_ARRAY_H
#define VIS_WIN_AXES_ARRAY_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkVisItAxisActor2D;
class VisWindowColleagueProxy;

// ****************************************************************************
//  Class: VisWinAxesArray
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It puts
//      updating axes around plots with arrays of parallel axes, like the
//      ParallelCoordinates plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:59:55 EST 2008
//    Added support for array variables and bin-defined x positions.
//
//    Jeremy Meredith, Tue Nov 18 15:50:49 EST 2008
//    Added support for a few missing features and new font attributes.
//
// ****************************************************************************

class VISWINDOW_API VisWinAxesArray : public VisWinColleague
{
  public:
                              VisWinAxesArray(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxesArray();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              UpdateView(void);
    virtual void              SetViewport(double, double, double, double);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    virtual void              StartAxisArrayMode(void);
    virtual void              StopAxisArrayMode(void);

    void                      SetTitles();

    void                      SetLabelVisibility(int);
    void                      SetTitleVisibility(int);
    void                      SetVisibility(int);
    void                      SetTickVisibility(bool,bool);
    void                      SetTickLocation(int);
    void                      SetGridVisibility(int);
    void                      SetAutoSetTicks(int);
    void                      SetMajorTickMinimum(double);
    void                      SetMajorTickMaximum(double);
    void                      SetMajorTickSpacing(double);
    void                      SetMinorTickSpacing(double);
    void                      SetLabelFontHeight(double);
    void                      SetTitleFontHeight(double);
    void                      SetLineWidth(int);
    void                      SetLabelScaling(bool, int);
    void                      SetTitleTextAttributes(
                                  const VisWinTextAttributes &atts);
    void                      SetLabelTextAttributes(
                                  const VisWinTextAttributes &atts);
  protected:
    struct AxisInfo {
        vtkVisItAxisActor2D *axis;
        int lastPow;
        int lastAxisDigits;
        int pow;
        char units[256];
        char title[256];
        double range[2];
        double xpos;
        AxisInfo()
             : axis(NULL), lastPow(0), lastAxisDigits(3), pow(0)
        {
            xpos = 0;
            units[0] = '\0';
            title[0] = '\0';
        }
        AxisInfo(vtkVisItAxisActor2D *a, int lp, int lad, int p, int up)
            : axis(a), lastPow(lp), lastAxisDigits(lad), pow(p)
        {
            xpos = 0;
            units[0] = '\0';
            title[0] = '\0';
        }
    };
    std::vector<AxisInfo> axes;

    bool                      addedAxes;
    bool                      autolabelScaling;
    double                    vl, vr;
    double                    vb, vt;

    double                    fr, fg, fb;
    int                       axisVisibility;
    bool                      labelVisibility;
    bool                      titleVisibility;
    int                       tickLocation;
    bool                      tickVisibility;
    bool                      tickLabelVisibility;
    bool                      gridVisibility;
    bool                      autoSetTicks;
    double                    majorTickMinimum;
    double                    majorTickMaximum;
    double                    majorTickSpacing;
    double                    minorTickSpacing;
    double                    labelFontHeight;
    double                    titleFontHeight; 
    int                       lineWidth;
    VisWinTextAttributes      titleTextAttributes;
    VisWinTextAttributes      labelTextAttributes;
    int                       userPow;

    void                      SetNumberOfAxes(int);
    bool                      AdjustValues(int, double, double);
    void                      AdjustRange(int, double, double);
    void                      GetRange(double &, double &, double &, double &);
    void                      AddAxesToWindow(void);
    void                      RemoveAxesFromWindow(void);
    bool                      ShouldAddAxes(void);
    void UpdateTitleTextAttributes(double fr, double fg, double fb);
    void UpdateLabelTextAttributes(double fr, double fg, double fb);
};


#endif


