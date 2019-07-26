// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCallback.h                               //
// ************************************************************************* //

#ifndef AVT_CALLBACK_H
#define AVT_CALLBACK_H
#include <pipeline_exports.h>


#include <string>

#include <avtDataObject.h>
#include <avtDatabase.h>
#include <GlobalAttributes.h>

class    AttributeSubject;


typedef   void  (*WarningCallback)(void *, const char *);
typedef   ref_ptr<avtDatabase> (*GetDatabaseCallback)(void *,
                                       const std::string &, int, const char *);
typedef   void  (*ResetTimeoutCallback)(void *, int);


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
//    Hank Childs, Tue Sep  5 10:51:08 PDT 2006
//    Added ResetTimeout callback.
//
//    Hank Childs, Fri Dec 31 11:45:48 PST 2010
//    Add a GetSafeMode callback.
//
//    Hank Childs, Tue Jan 18 07:38:18 PST 2011
//    Add auxsessionkey support for UCHC folks.
//
//    Cameron Christensen, Thursday, July 3, 2014
//    Add backend type callback.
//
//    Alok Hota, Tue Feb 23 19:10:32 PST 2016
//    Add support for OSPRay.
//
// ****************************************************************************

class PIPELINE_API avtCallback
{
  public:
    static void                  RegisterWarningCallback(WarningCallback,
                                                         void *);
    static bool                  IssueWarning(const char *);

    static void                  SetCurrentWindowAtts(const WindowAttributes&);
    static const WindowAttributes &
                                 GetCurrentWindowAtts(void);

    static void                  SetCurrentLightList(const LightList&);
    static const LightList      &GetCurrentLightList(void);

    static void                  SetNowinMode(bool b)
                                     { nowinMode = b; }
    static bool                  GetNowinMode(void)
                                     { return nowinMode; }

    static void                    SetNowinInteractionMode(bool b)
                                     { nowinInteractionMode = b; }

    static bool                  GetNowinInteractionMode()
                                     { return nowinInteractionMode; }

    static void                  SetSoftwareRendering(bool b)
                                     { swRendering = b; };
    static bool                  GetSoftwareRendering(void)
                                     { return swRendering; };
#ifdef VISIT_OSPRAY
    static void                  SetOSPRayMode(bool b)
                                     { useOSPRay = b; }
    static bool                  UseOSPRay(void)
                                     { return useOSPRay; }
#endif
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
 
    static void                  RegisterResetTimeoutCallback(
                                           ResetTimeoutCallback, void *);
    static void                  ResetTimeout(int);

    static void                  EnableSafeMode(void) { safeMode = true; };
    static bool                  GetSafeMode(void) { return safeMode; };

    // This method is used to pass a string to database readers.
    static std::string           GetAuxSessionKey(void) 
                                                  { return auxSessionKey; };
    static void                  SetAuxSessionKey(const std::string &k)
                                                  { auxSessionKey = k; };

    static void                  SetBackendType(GlobalAttributes::BackendType type);
    static GlobalAttributes::BackendType GetBackendType();

  protected:
    static WarningCallback       warningCallback;
    static void                 *warningCallbackArgs;

    static WindowAttributes      windowAtts;
    static LightList             lightList;

    static bool                  nowinMode;
    static bool                  nowinInteractionMode;
    static bool                  swRendering;
#ifdef VISIT_OSPRAY
    static bool                  useOSPRay;
#endif
    static bool                  safeMode;

    static GlobalAttributes::BackendType backendType;

    static std::string           auxSessionKey;

    static GetDatabaseCallback   getDatabaseCallback;
    static void                 *getDatabaseCallbackArgs;

    static bool                  haveRenderingException;
    static std::string           renderingExceptionMessage;

    static ResetTimeoutCallback  resetTimeoutCallback;
    static void                 *resetTimeoutCallbackArgs;
};


#endif


