// ************************************************************************* //
//                               avtCallback.h                               //
// ************************************************************************* //

#ifndef AVT_CALLBACK_H
#define AVT_CALLBACK_H
#include <pipeline_exports.h>


#include <string>

#include <avtDataObject.h>
class    AttributeSubject;


typedef   void  (*WarningCallback)(void *, const char *);
typedef   void  (*ImageCallback)(void *, int, avtDataObject_p &);
typedef   void  (*UpdatePlotAttributesCallback)(void *, const std::string &,
                                                int, AttributeSubject*);


#include <WindowAttributes.h>


// ****************************************************************************
//  Class: avtCallback
//
//  Purpose:
//      A static class where callbacks can be registered.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 21 10:47:55 PST 2001
//    Added a callback to get images and stored the current window attributes.
//
//    Hank Childs, Tue Apr 23 19:39:36 PDT 2002
//    Added support for nowin.
//
// ****************************************************************************

class PIPELINE_API avtCallback
{
  public:
    static void                  RegisterWarningCallback(WarningCallback,
                                                         void *);
    static void                  IssueWarning(const char *);

    static void                  RegisterImageCallback(ImageCallback, void *);
    static void                  GetImage(int, avtDataObject_p &);

    static void                  SetCurrentWindowAtts(const WindowAttributes&);
    static const WindowAttributes &
                                 GetCurrentWindowAtts(void);

    static void                  RegisterUpdatePlotAttributesCallback(
                                         UpdatePlotAttributesCallback, void *);
    static void                  UpdatePlotAttributes(const std::string &, int,
                                                      AttributeSubject *);

    static void                  SetNowinMode(bool b)
                                     { nowinMode = b; };
    static bool                  GetNowinMode(void)
                                     { return nowinMode; };

  protected:
    static WarningCallback       warningCallback;
    static void                 *warningCallbackArgs;

    static ImageCallback         imageCallback;
    static void                 *imageCallbackArgs;

    static WindowAttributes      windowAtts;

    static bool                  nowinMode;

    static UpdatePlotAttributesCallback
                                 updatePlotAttributesCallback;
    static void                 *updatePlotAttributesCallbackArgs;
};


#endif


