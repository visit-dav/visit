// ************************************************************************* //
//                            avtPlotMinMaxQuery.h                           //
// ************************************************************************* //

#ifndef AVT_PLOT_MINMAX_QUERY_H
#define AVT_PLOT_MINMAX_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

class avtCondenseDatasetFilter;
class avtMatrix;
class vtkDataSet;


// ****************************************************************************
//  Class: avtPlotMinMaxQuery
//
//  Purpose:
//    A query that retrieves var information about a mesh given a 
//    particular domain and zone number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtPlotMinMaxQuery : public avtDatasetQuery
{
  public:
                              avtPlotMinMaxQuery();
    virtual                  ~avtPlotMinMaxQuery();


    virtual const char       *GetType(void)   { return "avtPlotMinMaxQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating min/max for plot."; };

  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);   
    virtual int                     GetNFilters() { return 1; };   
    virtual void                    VerifyInput(void);   

  private:
    int                             dimension;
    int                             topoDim;
    int                             blockOrigin;
    int                             cellOrigin;
    bool                            singleDomain;
    bool                            scalarCurve;
    float                           minVal;
    float                           maxVal;
    int                             minElementNum;
    int                             maxElementNum;
    int                             minDomain;
    int                             maxDomain;
    float                           minCoord[3];
    float                           maxCoord[3];
    string                          minMsg;
    string                          maxMsg;
    string                          elementName;

    avtCondenseDatasetFilter       *condense;
    avtQueryableSource             *src;

    const avtMatrix                *invTransform;

    void                            GetNodeCoord(vtkDataSet *ds, const int id, 
                                        float coord[3]);
    void                            GetCellCoord(vtkDataSet *ds, const int id, 
                                        float coord[3]);
    void                            CreateMinMessage(void);
    void                            CreateMaxMessage(void);
    void                            CreateResultMessage(void);
};


#endif
