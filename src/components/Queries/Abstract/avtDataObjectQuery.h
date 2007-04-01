// ************************************************************************* //
//                           avtDataObjectQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_QUERY_H
#define AVT_DATA_OBJECT_QUERY_H

#include <query_exports.h>

#include <avtDataObjectSink.h>
#include <vectortypes.h>


class QueryAttributes;
class avtSILRestriction;
class SILRestrictionAttributes;

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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Tue Feb 10 15:00:32 PST 2004 
//    Added OriginalData method. 
//
//    Kathleen Bonnell, Thu Apr  1 16:04:44 PST 2004
//    Added PerformQueryInTime, SetTimeVarying, SetSILUseSet.
//
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004 
//    Changed args to PerformQueryInTime. 
//
//    Kathleen Bonnell, Tue May  4 14:18:26 PDT 2004 
//    Replaced SetSILUseSet with SetILRestriction. 
//
//    Kathleen Bonnell, Thu Jun 24 07:45:47 PDT 2004 
//    Added intVector and string arguments to PerformQueryInTime. 
//
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004 
//    Added Set/GetUnits. 
//
// ****************************************************************************

class QUERY_API avtDataObjectQuery : public virtual avtDataObjectSink
{
  public:
                                  avtDataObjectQuery();
    virtual                      ~avtDataObjectQuery();

    virtual const char           *GetType(void) = 0;
    virtual const char           *GetDescription(void) { return NULL; };

    virtual bool                  OriginalData(void) { return false; };
    virtual void                  PerformQuery(QueryAttributes *) = 0;
    virtual void                  PerformQueryInTime(QueryAttributes *,
                                      const int, const int, const int,
                                      const int, doubleVector &, intVector &,
                                      std::string &) {;};
    virtual std::string           GetResultMessage(void) = 0;

    static void                   RegisterInitializeProgressCallback(
                                          InitializeProgressCallback, void *);

    static void                   RegisterProgressCallback(ProgressCallback,
                                                           void *);

    virtual void                  SetTimeVarying(bool val) { timeVarying = val;};

    void                          SetSILRestriction(const SILRestrictionAttributes *);
    std::string                  &GetUnits(void) { return units; };
    void                          SetUnits(const std::string &_units) { units  = _units;};

  protected:
    static InitializeProgressCallback
                                  initializeProgressCallback;
    static void                  *initializeProgressCallbackArgs;

    static ProgressCallback       progressCallback;
    static void                  *progressCallbackArgs;

    void                          Init(const int ntimesteps = 1);
    virtual int                   GetNFilters();

    void                          UpdateProgress(int, int);
    virtual void                  ChangedInput(void);
    virtual void                  VerifyInput(void);

    std::string                   units;
    bool                          timeVarying;
    avtSILRestriction_p           querySILR;
};


#endif


