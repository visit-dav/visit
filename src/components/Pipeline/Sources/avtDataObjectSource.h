/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                              avtDataObjectSource.h                        //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_SOURCE_H
#define AVT_DATA_OBJECT_SOURCE_H

#include <pipeline_exports.h>

#include <avtDataObject.h>
#include <avtPipelineSpecification.h>


typedef   bool (*AbortCallback)(void *);
typedef   void (*ProgressCallback)(void *, const char *, const char *,int,int);


class     avtQueryableSource;
class     avtTerminatingSource;


// ****************************************************************************
//  Class: avtDataObjectSource
//
//  Purpose:
//      A source to an avtDataObject.  This class exists only to be a
//      convenient base class for filters and data objects that already exist
//      (or are coming out of a database).
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 16:58:20 PDT 2001
//    Pushed progress/abort callbacks into this type from derived type
//    avtFilter.
//
//    Hank Childs, Mon Jul 28 16:27:58 PDT 2003
//    Added notion of a queryable source.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sat Feb 19 14:39:06 PST 2005
//    Moved ReleaseData from avtFilter to this class.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectSource
{
  public:
                                    avtDataObjectSource();
    virtual                        ~avtDataObjectSource();

    virtual bool                    Update(avtPipelineSpecification_p) = 0;

    virtual avtTerminatingSource   *GetTerminatingSource(void) = 0;
    virtual avtQueryableSource     *GetQueryableSource(void) = 0;

    virtual avtDataObject_p         GetOutput(void) = 0;
    virtual void                    ReleaseData(void);

    static void                     RegisterAbortCallback(AbortCallback,void*);
    static void                     RegisterProgressCallback(ProgressCallback,
                                                             void *);

  protected:
    static AbortCallback            abortCallback;
    static void                    *abortCallbackArgs;
    static ProgressCallback         progressCallback;
    static void                    *progressCallbackArgs;

    void                            CheckAbort(void);
    void                            UpdateProgress(int, int, const char *,
                                                   const char *);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataObjectSource(const avtDataObjectSource &) {;};
    avtDataObjectSource &operator=(const avtDataObjectSource &) 
                                                            { return *this; };
};


#endif


