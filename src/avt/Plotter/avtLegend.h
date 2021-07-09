// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtLegend.h                                //
// ************************************************************************* //

#ifndef AVT_LEGEND_H
#define AVT_LEGEND_H
#include <plotter_exports.h>


#include <ref_ptr.h>
#include <vectortypes.h>


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
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Added orientation member
//
//    Hank Childs, Fri Jan 23 15:36:35 PST 2009
//    Added min/max member.
//
//    Kathleen Bonnell, Thu Oct  1 14:18:11 PDT 2009
//    Added methods and ivars that allow more user control of tick marks
//    and tick labels.
//
//    Kathleen Biagas, Thu May 20, 2021
//    Add support for custom title.
//
// ****************************************************************************

class PLOTTER_API  avtLegend
{
  public:
    typedef enum {
        VerticalTextOnRight,
        VerticalTextOnLeft,
        HorizontalTextOnTop,
        HorizontalTextOnBottom
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
    virtual void                  SetLabelVisibility(int);
    virtual int                   GetLabelVisibility() const;
    virtual void                  SetMinMaxVisibility(bool);
    virtual bool                  GetMinMaxVisibility() const;
    virtual void                  SetNumberFormat(const char *);

    virtual void                  SetLegendScale(double xScale, double yScale);
    virtual void                  SetBoundingBoxVisibility(bool);
    virtual void                  SetBoundingBoxColor(const double *);
    virtual void                  SetOrientation(LegendOrientation);
    virtual void                  SetFont(int family, bool bold, bool italic,
                                          bool shadow);

    virtual void                  SetForegroundColor(const double [3]);
    virtual void                  SetFontHeight(double);

    void                          SetTitle(const char *);
    void                          SetDatabaseInfo(const char *);
    void                          SetVarName(const char *);
    void                          SetVarUnits(const char *);
    void                          SetMessage(const char *);
    void                          SetGlobalVisibility(bool);

    virtual void                  SetNumTicks(int);
    virtual void                  SetUseSuppliedLabels(bool);
    virtual bool                  GetUseSuppliedLabels(void) { return false;}
    virtual void                  SetMinMaxInclusive(bool);
    virtual void                  SetSuppliedValues(const doubleVector &);
    virtual void                  SetSuppliedLabels(const stringVector &);
    virtual void                  GetCalculatedLabels(doubleVector &);
    virtual void                  GetCalculatedLabels(stringVector &);
    virtual int                   GetType(void) { return 2; }

    void                          Update();

    void                          UseCustomTitle(bool);
    void                          SetCustomTitle(const std::string &);

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
    LegendOrientation             orientation;

    bool                          useCustomTitle;
    std::string                   customTitle;

    virtual void                  ChangePosition(double, double) = 0;
    virtual void                  ChangeTitle(const char *) = 0;
    virtual void                  ChangeFontHeight(double) = 0;
};


typedef ref_ptr<avtLegend> avtLegend_p;


#endif


