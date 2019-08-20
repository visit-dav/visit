// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtContourPlot.h                               //
// ************************************************************************* //

#ifndef AVT_CONTOUR_PLOT_H
#define AVT_CONTOUR_PLOT_H

#include <avtPlot.h>
#include <ContourAttributes.h>

class     avtContourFilter;
class     avtFeatureEdgesFilter;
class     avtLevelsLegend;
class     avtLevelsMapper;
class     avtLineToPolylineFilter;
class     avtLookupTable;

// ****************************************************************************
//  Method: avtContourPlot
//
//  Purpose:
//      A concrete type of avtPlot for isocontours.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   Februray 15, 2001 
//
//  Modifications:
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made SetAtts virtual and take a generic AttributeGroup.
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Added legend and methods for setting legend parameters.
//    Removed data members varMin, varMax.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Mar 13 11:35:45 PST 2001 
//    Added SetLegend method.
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Tue Mar 27 14:38:23 PST 2001 
//    Removed SetVarExtents method, no longer necessary.  Added arguments
//    to SetIsoValues, CreatePercentValues, CreateNIsoValues methods. 
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made ContourAttributes a data member, so that all methods have 
//    access to the atts. 
//    
//    Hank Childs, Sun Jun 17 18:14:48 PDT 2001
//    Put code for determining isolevel values into avtContourFilter.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added avtLookupTable and numLevels.  
//    Changed the way single colors are handled (no longer with a lut). 
//    Removed SetLookupTableWithMultiColor and SetLookupTableWithSingleColor. 
//
//    Kathleen Bonnell, Sat Sep 22 12:13:57 PDT 2001 
//    Added CustomizeMapper. 
//    
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
// 
//    Brad Whitlock, Tue Dec 3 09:30:39 PDT 2002
//    I added the SetColorTable method.
//
//    Eric Brugger, Thu Mar 25 16:38:17 PST 2004
//    I added the GetDataExtents method.
//
//    Hank Childs, Fri Feb 15 15:41:15 PST 2008
//    Fix memory leak by adding new data member.
//
//    Hank Childs, Wed Aug 13 13:46:31 PDT 2008
//    Add NeedZBufferToCompositeEvenIn2D because thick lines can bleed into
//    other processor's portion of image space.
//
//    Brad Whitlock, Wed Sep  9 16:23:12 PDT 2009
//    Override SetCurrentExtents to add in the avtLineToPolylineFilter.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Rename Current->Actual.
//
//    Kathleen Biagas, Wed Feb 29 13:10:11 MST 2012
//    Add GetExtraInfoForPick.
//
// ****************************************************************************

class avtContourPlot : public avtSurfaceDataPlot
{
  public:
                    avtContourPlot();
    virtual        ~avtContourPlot();

    static avtPlot *Create();

    virtual const char *GetName(void) { return "ContourPlot"; };

    virtual void    SetAtts(const AttributeGroup*);
    virtual void    GetDataExtents(std::vector<double> &);
    virtual void    ReleaseData(void);
    virtual bool    SetColorTable(const char *ctName);

    void            SetLegend(bool);
    void            SetLineWidth(int);

    virtual bool    NeedZBufferToCompositeEvenIn2D(void);

    virtual const MapNode    &GetExtraInfoForPick(void);

  protected:
     virtual avtDataObject_p SetActualExtents(avtDataObject_p);

     avtContourFilter        *contourFilter;
     avtFeatureEdgesFilter   *edgeFilter;
     avtLineToPolylineFilter *lineFilter;
     avtLevelsMapper         *levelsMapper;
     avtLevelsLegend         *levelsLegend;
     avtLegend_p              levLegendRefPtr;
     avtLookupTable          *avtLUT;
     ContourAttributes        atts;
     int                      numLevels;

     virtual avtMapperBase   *GetMapper(void);
     virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
     virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
     virtual void             CustomizeBehavior(void);
     virtual void             CustomizeMapper(avtDataObjectInformation &);
     virtual avtLegend_p      GetLegend(void) { return levLegendRefPtr; };
     void                     SetColors();
};


#endif


