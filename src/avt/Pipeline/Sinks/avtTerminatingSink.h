// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTerminatingSink.h                          //
// ************************************************************************* //

#ifndef AVT_TERMINATING_SINK_H
#define AVT_TERMINATING_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtContract.h>

class     avtWebpage;


typedef  bool (*GuideFunction)(void *, int);


// ****************************************************************************
//  Class: avtTerminatingSink
//
//  Purpose:
//      This sink object serves as the terminator of a pipeline.  It 
//      understands that there are many pipelines and what its pipeline index
//      is.  It also understands that streaming may occur and
//      that it may have to execute a pipeline multiple times.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added DynamicLoadBalanceCleanUp.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Wed Mar  2 11:16:01 PST 2005
//    Take a full-blown pipeline specification rather than a data spec and a
//    pipeline index.
//
//    Hank Childs, Thu Dec 21 09:43:22 PST 2006
//    Add support for debug dumps
//
//    Hank Childs, Fri Jun 15 16:11:00 PDT 2007
//    Add support for indentation of debug dumps.
//
//    Cyrus Harrison, Wed Feb 13 11:31:58 PST 2008
//    Global debug dump flag was migrated to avtDebugDumpOptions.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingSink : virtual public avtDataObjectSink
{
  public:
                              avtTerminatingSink();
    virtual                  ~avtTerminatingSink();

    void                      Execute(avtContract_p);

    static void               SetGuideFunction(GuideFunction, void *);
    static void               GetGuideFunction(GuideFunction &, void *&);

    static void               AddDumpReference(const char *, const char *, int);

  protected:
    virtual void              InputIsReady(void);
    virtual void              StreamingCleanUp(void);

    static avtWebpage        *webpage;

    void                      FinalizeWebpage(void);
    void                      InitializeWebpage(void);

  private:
    static GuideFunction      guideFunction;
    static void              *guideFunctionArgs;
};


#endif


