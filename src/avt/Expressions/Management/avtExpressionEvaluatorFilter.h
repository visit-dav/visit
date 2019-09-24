// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtExpressionEvaluatorFilter.h                    //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_EVALUATOR_FILTER_H
#define AVT_EXPRESSION_EVALUATOR_FILTER_H

#include <expression_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtQueryableSource.h>

#include <ExprPipelineState.h>

class avtDataBinning;
class avtSourceFromAVTDataset;


typedef avtDataBinning *   (*GetDataBinningCallback)(void *, const char *);


// ****************************************************************************
//  Class: avtExpressionEvaluatorFilter
//
//  Purpose:
//      This filter parses out expressions and turns them into executable
//      VTK networks.  It encapsulates the code that used to be in the
//      NetworkManager and the Viewer.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 14:06:00 PST 2003
//   Made it use the right API.
//
//   Hank Childs, Mon Nov 17 16:47:33 PST 2003
//   Add ReleaseData.
//
//   Kathleen Bonnell, Thu Nov 13 08:39:40 PST 2003 
//   Added 'FindElementForPoint'.
//
//   Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003
//   Added GetDomainName.
//
//   Hank Childs, Thu Feb  5 17:11:06 PST 2004
//   Moved inlined constructor and destructor definitions to .C files
//   because certain compilers have problems with them.
//
//   Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//   Added QueryZoneCenter.
//
//   Kathleen Bonnell, Thu Jun 10 18:29:08 PDT 2004
//   Rename QueryZoneCenter to QueryCoords, added bool arg.
//
//   Kathleen Bonnell, Mon Jun 28 08:01:45 PDT 2004 
//   Added currentTimeState, ExamineContract. 
//
//   Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004 
//   Added another bool arg to QueryCoords. 
//
//   Hank Childs, Wed Dec 29 08:02:40 PST 2004
//   Added friend access to avtMacroExpressionFilter.  Also cache the
//   terminating source for updates.
//
//   Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//   Added const char* arg to QueryCoords. 
//
//   Hank Childs, Tue Aug 16 16:50:56 PDT 2005
//   Add method VerifyVariableTypes.
//
//   Hank Childs, Sun Dec  4 17:31:14 PST 2005
//   Added description.
//
//   Hank Childs, Sun Feb 19 09:54:19 PST 2006
//   Add support for DDFs.
//
//   Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//   Added support for rectilinear grids with an inherent transform.
//
//   Hank Childs, Mon Dec 10 17:49:02 PST 2007
//   Add data member for keeping track of expressions generated.
//
//   Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//   Add data member for keeping track of whether we are doing on demand
//   processing.
//
//   Hank Childs, Sat Aug 21 14:02:28 PDT 2010
//   Renamed DDFs to DataBinnings.
//
//   Alister Maguire, Tue Sep 24 11:15:10 MST 2019
//   Added CanApplyToDirectDatabaseQOT. 
//
// ****************************************************************************

class EXPRESSION_API avtExpressionEvaluatorFilter 
    : virtual public avtDatasetToDatasetFilter,
      virtual public avtQueryableSource
{
    friend class             avtMacroExpressionFilter;

  public:
                             avtExpressionEvaluatorFilter();
    virtual                 ~avtExpressionEvaluatorFilter();
    virtual const char*      GetType(void)
                                     { return "avtExpressionEvaluatorFilter";};
    virtual const char      *GetDescription(void)
                                     { return "Creating expressions"; };


    virtual void             Query(PickAttributes *);
    virtual avtQueryableSource *
                             GetQueryableSource(void) { return this; };
    virtual void             ReleaseData(void);

    virtual bool             FindElementForPoint(const char *, const int, 
                                 const int, const char *, double[3], int &);
    virtual bool             QueryCoords(const std::string&, const int, 
                                 const int, const int, double[3], const bool,
                                 const bool, const char *mn = NULL);

    virtual void             GetDomainName(const std::string &, const int,
                                 const int , std::string &);

    static void              RegisterGetDataBinningCallback(GetDataBinningCallback, void *);

    virtual bool             CanApplyToDirectDatabaseQOT(void);

  protected:
    virtual void             PreExecute(void) {}
    virtual void             PostExecute(void) {}
    virtual void             Execute(void);
    virtual avtContract_p
                             ModifyContract(avtContract_p);
    virtual int              AdditionalPipelineFilters(void);
    virtual void             ExamineContract(avtContract_p);
    virtual bool             FilterUnderstandsTransformedRectMesh();

  protected:
    ExprPipelineState            pipelineState;
    avtContract_p                lastUsedSpec;
    avtSourceFromAVTDataset     *termsrc;
    std::vector<std::string>     expr_list_fromLastTime;

    static  GetDataBinningCallback  getDataBinningCallback;
    static  void                   *getDataBinningCallbackArgs;

  private:
    int                          currentTimeState;
    bool                         onDemandProcessing;
    bool                         replicateSingleDomainOnAllProcessors;

    void                         VerifyVariableTypes(void);
};


#endif


