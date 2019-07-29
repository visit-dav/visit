// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtDataObjectSource.h                        //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_SOURCE_H
#define AVT_DATA_OBJECT_SOURCE_H

#include <pipeline_exports.h>

#include <avtDataObject.h>
#include <avtContract.h>


typedef   bool (*AbortCallback)(void *);
typedef   void (*ProgressCallback)(void *, const char *, const char *,int,int);


class     avtNamedSelection;
class     avtOriginatingSource;
class     avtQueryableSource;


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
//    Hank Childs, Mon Feb  9 15:09:29 PST 2009
//    Added method CreateNamedSelection. 
//
//    Kathleen Biagas, Mon Jun  5 16:29:43 PDT 2017
//    Added ResetAllExtents.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectSource
{
  public:
                                    avtDataObjectSource();
    virtual                        ~avtDataObjectSource();

    virtual bool                    Update(avtContract_p) = 0;
    virtual void                    ResetAllExtents(void) = 0;

    virtual avtOriginatingSource   *GetOriginatingSource(void) = 0;
    virtual avtQueryableSource     *GetQueryableSource(void) = 0;

    virtual avtDataObject_p         GetOutput(void) = 0;
    virtual void                    ReleaseData(void);

    static void                     RegisterAbortCallback(AbortCallback,void*);
    static void                     RegisterProgressCallback(ProgressCallback,
                                                             void *);

    virtual avtNamedSelection      *CreateNamedSelection(avtContract_p, 
                                                         const std::string &)
                                          { return NULL; };

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


