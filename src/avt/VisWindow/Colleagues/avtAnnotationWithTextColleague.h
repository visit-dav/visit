// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_ANNOTATION_WITH_TEXT_H
#define AVT_ANNOTATION_WITH_TEXT_H
#include <memory>
#include <viswindow_exports.h>
#include <avtAnnotationColleague.h>

class avtDataAttributes;

// ****************************************************************************
// Class: avtAnnotationWithTextColleague
//
// Purpose: A class that shares the bulk of functionality related to text
//    handling and in particular, text macros of the form $<keyword> that name
//    members of avtDataAttributes, for any annotation colleagues involving
//    text.
//
// Mark C. Miller, Mon Mar  7 10:26:40 PST 2022
//
// ****************************************************************************

class VISWINDOW_API avtAnnotationWithTextColleague : public avtAnnotationColleague
{
public:
    avtAnnotationWithTextColleague(VisWindowColleagueProxy &);
    virtual ~avtAnnotationWithTextColleague();

    virtual void UpdatePlotList(std::vector<avtActor_p> &lst);

protected:
    char *CreateAnnotationString(char const *fmtStr);

    static std::unique_ptr<avtDataAttributes> initialDataAttributes;

    char                     *textFormatString; // fmt string from user
    char                     *textString; // string produced from fmt

    avtDataAttributes        *currentDataAttributes;
};

#endif
