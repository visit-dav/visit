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
// ****************************************************************************

class PIPELINE_API avtDataObjectSource
{
  public:
                                    avtDataObjectSource() {;};
    virtual                        ~avtDataObjectSource() {;};

    virtual bool                    Update(avtPipelineSpecification_p) = 0;

    virtual avtTerminatingSource   *GetTerminatingSource(void) = 0;
    virtual avtQueryableSource     *GetQueryableSource(void) = 0;

    virtual avtDataObject_p         GetOutput(void) = 0;

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
};


#endif


