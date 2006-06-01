/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               Init.h                                      //
// ************************************************************************* //

#ifndef INIT_H
#define INIT_H

#include <misc_exports.h>


typedef void  (*ErrorFunction)(void *, const char *);


// ****************************************************************************
//  Namespace: Init
//
//  Purpose:
//      A module that does initialization procedures.  It has been extended to
//      contain callbacks for components that allow it to get the compnent
//      name, issue warnings, etc.
//
//  Programmer: "Hank Childs"    (Added documentation)
//  Creation:   August 8, 2003   (Added documentation)
//
//  Modifications:
//
//    Hank Childs, Tue Jun  1 11:47:36 PDT 2004
//    Added Finalize (should I have renamed this namespace?).
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added ComponentNameToID and ComponentIDToName
//
//    Jeremy Meredith, Wed May 25 13:24:24 PDT 2005
//    Added ability to disable custom signal handlers.
//
// ****************************************************************************

namespace Init
{
    MISC_API void Initialize(int &argc, char *argv[], int r=0, int n=1,
                             bool strip=true, bool sigs=true);
    MISC_API void Finalize();
    MISC_API void SetComponentName(const char *); 
    MISC_API const char * GetExecutableName(); 
    MISC_API const char * GetComponentName(); 
    MISC_API const int ComponentNameToID(const char *);
    MISC_API const char * ComponentIDToName(const int);
    MISC_API bool  IsComponent(const char *);
    MISC_API void ComponentIssueError(const char *);
    MISC_API void ComponentRegisterErrorFunction(ErrorFunction, void *);
    const char *const CatchAllMeshName = "for active plot";
}

#endif


