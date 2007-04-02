// ************************************************************************* //
//                               avtCallback.h                               //
// ************************************************************************* //

#ifndef AVT_CALLBACK_H
#define AVT_CALLBACK_H
#include <pipeline_exports.h>


#include <string>

#include <avtDataObject.h>
#include <avtDatabase.h>

class    AttributeSubject;


typedef   void  (*WarningCallback)(void *, const char *);
typedef   void  (*ImageCallback)(void *, int, avtDataObject_p &);
typedef   void  (*UpdatePlotAttributesCallback)(void *, const std::string &,
                                                int, AttributeSubject*);
typedef   ref_ptr<avtDatabase> (*GetDatabaseCallback)(void *,
                                       const std::string &, int, const char *);


#include <WindowAttributes.h>
#include <LightList.h>


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
//    Jeremy Meredith, Thu Oct  2 09:49:48 PDT 2003
//    Added support for lights.
//
//    Mark C. Miller, Wed Jul 21 09:51:18 PDT 2004
//    Changed return value of IssueWarning to bool
//
//    Hank Childs, Fri Aug 26 15:29:54 PDT 2005
//    Add GetDatabase.
//
//    Hank Childs, Sat Dec  3 20:27:16 PST 2005
//    Added swRendering.
//
//    Hank Childs, Tue Feb 28 14:12:16 PST 2006
//    Added rendering exceptions.
//
// ****************************************************************************

class PIPELINE_API avtCallback
{
  public:
    static void                  RegisterWarningCallback(WarningCallback,
                                                         void *);
    static bool                  IssueWarning(const char *);

    static void                  RegisterImageCallback(ImageCallback, void *);
    static void                  GetImage(int, avtDataObject_p &);

    static void                  SetCurrentWindowAtts(const WindowAttributes&);
    static const WindowAttributes &
                                 GetCurrentWindowAtts(void);

    static void                  SetCurrentLightList(const LightList&);
    static const LightList      &GetCurrentLightList(void);

    static void                  RegisterUpdatePlotAttributesCallback(
                                         UpdatePlotAttributesCallback, void *);
    static void                  UpdatePlotAttributes(const std::string &, int,
                                                      AttributeSubject *);

    static void                  SetNowinMode(bool b)
                                     { nowinMode = b; };
    static bool                  GetNowinMode(void)
                                     { return nowinMode; };

    static void                  SetSoftwareRendering(bool b)
                                     { swRendering = b; };
    static bool                  GetSoftwareRendering(void)
                                     { return swRendering; };

    static void                  RegisterGetDatabaseCallback(
                                                  GetDatabaseCallback, void *);
    static ref_ptr<avtDatabase>  GetDatabase(const std::string &, int,
                                             const char *);

    static void                  ClearRenderingExceptions(void)
                                       { haveRenderingException = false; };
    static void                  SetRenderingException(const std::string &s)
                                       { haveRenderingException = true;
                                         renderingExceptionMessage = s; };
    static std::string           GetRenderingException(void)
                                       { return (haveRenderingException
                                         ? renderingExceptionMessage : ""); };

  protected:
    static WarningCallback       warningCallback;
    static void                 *warningCallbackArgs;

    static ImageCallback         imageCallback;
    static void                 *imageCallbackArgs;

    static WindowAttributes      windowAtts;
    static LightList             lightList;

    static bool                  nowinMode;
    static bool                  swRendering;

    static UpdatePlotAttributesCallback
                                 updatePlotAttributesCallback;
    static void                 *updatePlotAttributesCallbackArgs;

    static GetDatabaseCallback   getDatabaseCallback;
    static void                 *getDatabaseCallbackArgs;

    static bool                  haveRenderingException;
    static std::string           renderingExceptionMessage;
};


#endif


