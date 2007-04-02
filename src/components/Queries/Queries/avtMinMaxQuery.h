// ************************************************************************* //
//                             avtMinMaxQuery.h                              //
// ************************************************************************* //

#ifndef AVT_MINMAX_QUERY_H
#define AVT_MINMAX_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <MinMaxInfo.h>

class avtMatrix;
class vtkDataArray;
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
//    Kathleen Bonnell, Tue Jul  6 16:59:26 PDT 2004 
//    Encapsulated elNum, vals, domain, coords in class MinMaxInfo.
//    Removed CreateMinMessage, CreateMaxMessage.  
//    Added InfoToString, CreateMessage, FindElement, FinalizeZoneCoord,
//    FinalizeNodeCoord.
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
    bool                    nodeCentered;
    std::string             minMsg;
    std::string             maxMsg;
    std::string             elementName;

    MinMaxInfo              minInfo1;
    MinMaxInfo              minInfo2;
    MinMaxInfo              maxInfo1;
    MinMaxInfo              maxInfo2;

    std::string             nodeMsg1;
    std::string             nodeMsg2;
    std::string             zoneMsg1;
    std::string             zoneMsg2;

    bool                    doMin;
    bool                    doMax;

    avtQueryableSource     *src;

    const avtMatrix        *invTransform;

    void                    GetNodeCoord(vtkDataSet *ds, const int id, 
                                        double coord[3]);
    void                    GetCellCoord(vtkDataSet *ds, const int id, 
                                        double coord[3]);

    void                    CreateResultMessage(const int);

    std::string             InfoToString(const MinMaxInfo &);
    void                    CreateMessage(const int, const MinMaxInfo &, 
                                          const MinMaxInfo &, std::string &,
                                          doubleVector &);

    void                    FindElement(MinMaxInfo &);
    void                    FinalizeZoneCoord(vtkDataSet *, 
                                              vtkDataArray *, 
                                              MinMaxInfo &, bool);
    void                    FinalizeNodeCoord(vtkDataSet *, MinMaxInfo &);
};


#endif
