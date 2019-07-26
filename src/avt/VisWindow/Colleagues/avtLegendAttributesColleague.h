// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtLegendAttributesColleague.h                   //
// ************************************************************************* //

#ifndef AVT_LEGEND_ATTRIBUTES_COLLEAGUE_H
#define AVT_LEGEND_ATTRIBUTES_COLLEAGUE_H
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>
#include <AnnotationObject.h>

// ****************************************************************************
// Class: avtLegendAttributesColleague
//
// Purpose:
//   This colleague contains attributes that set the properties for the legend.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 10:51:24 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class VISWINDOW_API avtLegendAttributesColleague : public avtAnnotationColleague
{
public:
    avtLegendAttributesColleague(VisWindowColleagueProxy &);
    virtual ~avtLegendAttributesColleague();

    virtual void AddToRenderer();
    virtual void RemoveFromRenderer();
    virtual void Hide();

    virtual std::string TypeName() const { return "Legend"; }

    // Methods to set and get the annotation's properties.
    virtual void SetOptions(const AnnotationObject &annot);
    virtual void GetOptions(AnnotationObject &annot);

    // Legend methods.
    virtual bool ManageLayout(avtLegend_p legend) const;
    virtual void CustomizeLegend(avtLegend_p legend);

    // Methods that are called in response to vis window events.
    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);
protected:
    AnnotationObject atts;
};


#endif


