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
    textFormatString = 0;
    textString = 0;
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

// Caller must delete what is returned
char *
avtAnnotationWithTextColleague::CreateAnnotationString(const char *formatString)
{
    char *retval = new char[100]();
    if (!strcmp(formatString, "$time"))
        snprintf(retval, 100, "%g", currentDataAttributes->GetTime());
    else if (!strcmp(formatString, "$cycle"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetCycle());
    else if (!strcmp(formatString, "$index"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetTimeIndex());
    else
        snprintf(retval, 100, "%s", formatString);
#warning FIXME: USE C++ EQUIV OF STRCPY
    return retval;
}
