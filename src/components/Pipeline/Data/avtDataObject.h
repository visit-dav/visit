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
//                              avtDataObject.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_H
#define AVT_DATA_OBJECT_H

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataObjectInformation.h>
#include <avtContract.h>

class     avtDataObjectSource;
class     avtDataObjectWriter;
class     avtQueryableSource;
class     avtOriginatingSource;
class     avtWebpage;


// **************************************************************************** 
//  Class: avtDataObject
//
//  Purpose:
//      The is the fundamental type handed around by process objects.  A data
//      object contains data in some form.  The base class is defined to handle
//      issues with the Update/Execute model.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sat Feb 19 14:49:17 PST 2005
//    Added method to get the source of the data object.
//
//    Hank Childs, Thu May 25 16:40:20 PDT 2006
//    Add support for releasing data in the middle of a pipeline execution.
//
//    Hank Childs, Thu Dec 21 09:58:57 PST 2006
//    Added method for debug dumps.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtDataObject
{
  public:
                                     avtDataObject(avtDataObjectSource *);
    virtual                         ~avtDataObject();

    avtOriginatingSource            *GetOriginatingSource(void);
    avtQueryableSource              *GetQueryableSource(void);

    bool                             Update(avtContract_p);
    void                             SetSource(avtDataObjectSource *);
    avtDataObjectSource             *GetSource(void) { return source; };

    avtDataObjectInformation        &GetInfo(void) { return info; };
    const avtDataObjectInformation  &GetInfo(void) const { return info; };
    virtual int                      GetNumberOfCells(bool polysOnly = false)
                                                                     const = 0;

    virtual const char              *GetType(void) = 0;
    virtual void                     ReleaseData(void);

    avtDataObject                   *Clone(void);
    void                             Copy(avtDataObject *);
    void                             Merge(avtDataObject *, bool = false);
    virtual avtDataObject           *Instance(void);

    virtual avtDataObjectWriter     *InstantiateWriter(void);
    
    void                             SetTransientStatus(bool b)
                                                            { transient = b; };
    bool                             IsTransient(void) { return transient; };

    virtual void                     DebugDump(avtWebpage *, const char *);

  protected:
    avtDataObjectInformation         info;
    avtDataObjectSource             *source;

    // "Transient" implies that this object is only needed so that a filter
    // can process it and calculate its output.  Almost all data objects are
    // "transient".  An example of a non-transient data object is the
    // "intermediate data object" of a plot, which must not be deleted, 
    // because we need it for queries.
    bool                             transient;

    virtual void                     DerivedCopy(avtDataObject *);
    virtual void                     DerivedMerge(avtDataObject *);
    void                             CompatibleTypes(avtDataObject *);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataObject(const avtDataObject &) {;};
    avtDataObject       &operator=(const avtDataObject &) { return *this; };
};


typedef ref_ptr<avtDataObject> avtDataObject_p;


#endif


