/*****************************************************************************
*
* Copyright (c) 2013, Intelligent Light
*
*****************************************************************************/

// ************************************************************************* //
//                             avtGridInformationQuery.h                     //
// ************************************************************************* //

#ifndef AVT_GRID_INFORMATION_QUERY_H
#define AVT_GRID_INFORMATION_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <map>

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
//  Class: avtGridInformationQuery
//
//  Purpose:
//    A query that retrieves grid information such as the logical extents.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep  6 10:50:34 PDT 2013
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtGridInformationQuery : virtual public avtDatasetQuery
{
public:
                            avtGridInformationQuery();
    virtual                ~avtGridInformationQuery();

    virtual const char     *GetType(void)   
                                { return "avtGridInformationQuery"; };
    virtual const char     *GetDescription(void)
                                { return "Calculating Grid Information."; };

    virtual void            SetInputParams(const MapNode &);
    static  void            GetDefaultInputParams(MapNode &);

protected:
    virtual void            PreExecute(void);
    virtual void            Execute(vtkDataSet *, const int);
    virtual void            PostExecute(void);
//    virtual void            VerifyInput(void);   

private:
    static const int RIGHT_HANDED;
    static const int LEFT_HANDED;

    struct GridInfo
    {
        avtMeshType meshType;
        int         dims[3];
        double      extents[6];
        int         handedness;
        int         ghosttypes;
    };

    bool                    getExtents;
    bool                    getHandedness;
    bool                    getGhostTypes;
    std::map<int, GridInfo> gridInfo;
};


#endif
