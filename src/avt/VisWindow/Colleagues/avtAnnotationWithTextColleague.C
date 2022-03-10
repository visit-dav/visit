// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtAnnotationWithTextColleague.h>
#include <avtDataAttributes.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>

#include <vtkRenderer.h>
#include <vtkVisItTextActor.h>
#include <vtkTextProperty.h>

#include <memory>

#warning MAKE ME A UNIQUE_PTR
// std::unique_ptr<avtDataAttributes const *> avtAnnotationWithTextColleague::initialDataAttributes(new avtDataAttributes);
avtDataAttributes *avtAnnotationWithTextColleague::initialDataAttributes = new avtDataAttributes;

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::avtAnnotationWithText
//
// Purpose: 
//   Constructor for the avtAnnotationWithText class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:13:14 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//
//    Tom Fogal, Fri Jan 28 15:26:59 MST 2011
//    VTK API change.
//   
// ****************************************************************************

avtAnnotationWithTextColleague::avtAnnotationWithTextColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationColleague(m)
{
    textFormatString = new char[1];
    textFormatString[0] = '\0';
    textString = new char[1];
    textString[0] = '\0';
    currentDataAttributes = new avtDataAttributes;
    currentDataAttributes->Copy(*initialDataAttributes);
}

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::~avtAnnotationWithText
//
// Purpose: 
//   Destructor for the avtAnnotationWithText class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 14:18:31 PST 2003
//
// Modifications:
//    Burlen Loring, Mon Jul 14 14:04:31 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs operator delete)
//
// ****************************************************************************

avtAnnotationWithTextColleague::~avtAnnotationWithTextColleague()
{
    delete [] textString;
    delete [] textFormatString;
    delete currentDataAttributes;
}

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::UpdatePlotList
//
// Purpose: 
//   This method is called when the plot list changes. Its job is to make sure
//   that the time slider always shows the right time.
//
// Arguments:
//   lst : The plot list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 12:46:37 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Wed Mar 11 12:33:20 EDT 2009
//    Added $cycle support.
//
//    Brad Whitlock, Mon Dec 10 11:41:13 PST 2012
//    Set initial values from the current values.
//
// ****************************************************************************

void
avtAnnotationWithTextColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    if(lst.size() > 0 && textFormatString != 0)
    {
        // update current copy of data attributes
        currentDataAttributes->Copy(lst[0]->GetBehavior()->GetInfo().GetAttributes());

        // update initial data attributes to current
        initialDataAttributes->Copy(*currentDataAttributes);
    }
}

#define TIME_IDENTIFIER "$time"
#define CYCLE_IDENTIFIER "$cycle"
#define INDEX_IDENTIFIER "$index"

// Caller must delete what is returned
char *
avtAnnotationWithTextColleague::CreateAnnotationString(const char *formatString)
{
    char *retval;

    size_t len = strlen(formatString);
    std::string fmtStr(formatString);
    std::string::size_type pos;
    if((pos=fmtStr.find(TIME_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(TIME_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%g", currentDataAttributes->GetTime());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else if((pos=fmtStr.find(CYCLE_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(CYCLE_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%d", currentDataAttributes->GetCycle());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else if((pos=fmtStr.find(INDEX_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(INDEX_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%d", currentDataAttributes->GetTimeIndex());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else
    {
        retval = new char[len + 1];
        strcpy(retval, formatString);
    }

    return retval;


#if 0
    char *retval = new char[100]();
    if (!strcmp(formatString, "$time"))
        snprintf(retval, 100, "%g", currentDataAttributes->GetTime());
    else if (!strcmp(formatString, "$cycle"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetCycle());
    else if (!strcmp(formatString, "$index"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetTimeIndex());
    else
        snprintf(retval, 100, "%s", formatString);
    return retval;
#endif
}
