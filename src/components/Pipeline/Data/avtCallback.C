// ************************************************************************* //
//                                avtCallback.C                              //
// ************************************************************************* //

#include <avtCallback.h>

// For NULL
#include <stdio.h>

#include <DebugStream.h>


using std::string;


WarningCallback               avtCallback::warningCallback = NULL;
void                         *avtCallback::warningCallbackArgs = NULL;

ImageCallback                 avtCallback::imageCallback = NULL;
void                         *avtCallback::imageCallbackArgs = NULL;

WindowAttributes              avtCallback::windowAtts;
LightList                     avtCallback::lightList;

bool                          avtCallback::nowinMode = false;

UpdatePlotAttributesCallback  avtCallback::updatePlotAttributesCallback = NULL;
void                         *avtCallback::updatePlotAttributesCallbackArgs 
                                                                        = NULL;


// ****************************************************************************
//  Method: avtCallback::RegisterWarningCallback
//
//  Purpose:
//      Registers a callback that all data object sources can issues warnings
//      through.
//
//  Arguments:
//      wc      The warning callback.
//      args    The arguments to the warning callback.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
// ****************************************************************************

void
avtCallback::RegisterWarningCallback(WarningCallback wc, void *args)
{
    warningCallback     = wc;
    warningCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtCallback::IssueWarning
//
//  Purpose:
//      Issues a warning through static functions.  Provided as a convenience
//      to derived types and to provide a single point of source.
//
//  Arguments:
//      msg     The message to issue for the warning.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
// ****************************************************************************

void
avtCallback::IssueWarning(const char *msg)
{
    if (warningCallback != NULL)
    {
        warningCallback(warningCallbackArgs, msg);
    }
    else
    {
        debug1 << "Would like to have issued warning \"" << msg 
               << "\", but no callback was registered." << endl;
    }
}


// ****************************************************************************
//  Method: avtCallback::RegisterImageCallback
//
//  Purpose:
//      Registers a callback that allows a plot to get a new image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCallback::RegisterImageCallback(ImageCallback ic, void *ica)
{
    imageCallback     = ic;
    imageCallbackArgs = ica;
}


// ****************************************************************************
//  Method: avtCallback::GetImage
//
//  Purpose:
//      Goes through the image callback to get the new image.
//
//  Arguments:
//      index   The plot index.
//      dob     A place to put the data object.
//
//  Returns:    A void * representation of the image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCallback::GetImage(int index, avtDataObject_p &dob)
{
    if (imageCallback != NULL)
    {
        imageCallback(imageCallbackArgs, index, dob);
    }
    else
    {
        debug1 << "Would like to have gotten a new image, but no callback "
               << "was registered." << endl;
        dob = NULL;
    }
}


// ****************************************************************************
//  Method: avtCallback::GetCurrentWindowAtts
//
//  Purpose:
//      Gets the current window attributes (assumes that those attributes
//      have been previously registered; that is typically done in avtPlot).
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

const WindowAttributes &
avtCallback::GetCurrentWindowAtts(void)
{
    return windowAtts;
}


// ****************************************************************************
//  Method: avtCallback::SetCurrentWindowAtts
//
//  Purpose:
//      Sets the current window attributes.  This is typically done by avtPlot.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
avtCallback::SetCurrentWindowAtts(const WindowAttributes &atts)
{
    windowAtts = atts;
}


// ****************************************************************************
//  Method: avtCallback::GetCurrentLightList
//
//  Purpose:
//      Gets the current light list.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  2, 2003
//
// ****************************************************************************

const LightList &
avtCallback::GetCurrentLightList(void)
{
    return lightList;
}


// ****************************************************************************
//  Method: avtCallback::SetCurrentLightList
//
//  Purpose:
//      Sets the current light list.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  2, 2003
//
// ****************************************************************************

void
avtCallback::SetCurrentLightList(const LightList &l)
{
    lightList = l;
}


// ****************************************************************************
//  Method: avtCallback::RegisterUpdatePlotAttributesCallback
//
//  Purpose:
//      Registers a callback that can be called to update plot attributes.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************

void
avtCallback::RegisterUpdatePlotAttributesCallback(
                             UpdatePlotAttributesCallback upac, void *upacArgs)
{
    updatePlotAttributesCallback     = upac;
    updatePlotAttributesCallbackArgs = upacArgs;
}


// ****************************************************************************
//  Method: avtCallback::UpdatePlotAttributes
//
//  Purpose:
//      Registers a callback that can be called to update plot attributes.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************

void
avtCallback::UpdatePlotAttributes(const string &str, int index,
                                  AttributeSubject *atts)
{
    if (updatePlotAttributesCallback == NULL)
    {
        debug1 << "Unable to update plot attributes since no callback has "
               << "been registered." << endl;
        return;
    }

    updatePlotAttributesCallback(updatePlotAttributesCallbackArgs, str, 
                                 index, atts);
}


