/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
 
    static void                  RegisterResetTimeoutCallback(
                                           ResetTimeoutCallback, void *);
    static void                  ResetTimeout(int);

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

    static ResetTimeoutCallback  resetTimeoutCallback;
    static void                 *resetTimeoutCallbackArgs;
};


#endif


