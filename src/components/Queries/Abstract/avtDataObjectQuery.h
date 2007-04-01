// ************************************************************************* //
//                           avtDataObjectQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_QUERY_H
#define AVT_DATA_OBJECT_QUERY_H
#include <query_exports.h>


#include <avtDataObjectSink.h>


class QueryAttributes;
class avtDataSpecification;

typedef void (*InitializeProgressCallback)(void *, int); 
typedef void (*ProgressCallback)(void *, const char *, const char *,int,int);                      


// ****************************************************************************
//  Class: avtDataObjectQuery
//
//  Purpose:
//      The base class for a query object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002
//    Add ChangedInput, VerifyInput.
//  
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Removed unnecessary SetAtts.
//
//    Kathleen Bonnell, Fri Jul 11 16:33:16 PDT 2003 
//    Added units, rename GetMessage as GetResultMessage.
//
//    Kathleen Bonnell, Tue Feb 10 15:00:32 PST 2004 
//    Added OriginalData method. 
//
// ****************************************************************************

class QUERY_API avtDataObjectQuery : public virtual avtDataObjectSink
{

  public:
                                  avtDataObjectQuery() {;};
    virtual                      ~avtDataObjectQuery() {;};

    virtual const char           *GetType(void) = 0;
    virtual const char           *GetDescription(void) { return NULL; };

    virtual bool                  OriginalData(void) { return false; };
    virtual void                  PerformQuery(QueryAttributes *) = 0;
    virtual std::string           GetResultMessage(void) = 0;

    static void                   RegisterInitializeProgressCallback(
                                          InitializeProgressCallback, void *);

    static void                   RegisterProgressCallback(ProgressCallback,
                                                           void *);


  protected:
    static InitializeProgressCallback
                                  initializeProgressCallback;
    static void                  *initializeProgressCallbackArgs;

    static ProgressCallback       progressCallback;
    static void                  *progressCallbackArgs;

    void                          Init(void);
    virtual int                   GetNFilters();

    void                          UpdateProgress(int, int);
    virtual void                  ChangedInput(void);
    virtual void                  VerifyInput(void);

    std::string                   units;
};


#endif


