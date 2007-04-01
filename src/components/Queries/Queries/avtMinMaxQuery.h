// ************************************************************************* //
//                             avtMinMaxQuery.h                              //
// ************************************************************************* //

#ifndef AVT_MINMAX_QUERY_H
#define AVT_MINMAX_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

class avtCondenseDatasetFilter;
class avtMatrix;
class vtkDataSet;


// ****************************************************************************
//  Class: avtMinMaxQuery
//
//  Purpose:
//    A query that retrieves min and max information about a variable. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb  3 17:54:19 PST 2004
//    Renamed from avtPlotMinMaxQuery. Made into parent class. 
//
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004 
//    Added args to constructor. 
//
// ****************************************************************************

class QUERY_API avtMinMaxQuery : virtual public avtDatasetQuery
{
  public:
                            avtMinMaxQuery(bool mn = true, bool mx = true);
    virtual                ~avtMinMaxQuery();


    virtual const char     *GetType(void)   
                                { return "avtMinMaxQuery"; };
    virtual const char     *GetDescription(void)
                                { return "Calculating min/max."; };

  protected:
    virtual void            Execute(vtkDataSet *, const int);
    virtual void            PreExecute(void);
    virtual void            PostExecute(void);
    virtual void            VerifyInput(void);   
            void            Preparation(avtDataObject_p);   

  private:
    int                     dimension;
    int                     topoDim;
    int                     blockOrigin;
    int                     cellOrigin;
    bool                    singleDomain;
    bool                    scalarCurve;
    float                   minVal;
    float                   maxVal;
    int                     minElementNum;
    int                     maxElementNum;
    int                     minDomain;
    int                     maxDomain;
    float                   minCoord[3];
    float                   maxCoord[3];
    string                  minMsg;
    string                  maxMsg;
    string                  elementName;

    bool                    doMin;
    bool                    doMax;

    avtQueryableSource     *src;

    const avtMatrix        *invTransform;

    void                    GetNodeCoord(vtkDataSet *ds, const int id, 
                                        float coord[3]);
    void                    GetCellCoord(vtkDataSet *ds, const int id, 
                                        float coord[3]);
    void                    CreateMinMessage(void);
    void                    CreateMaxMessage(void);
    void                    CreateResultMessage(void);
};


#endif
