// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtSamplePoints.h                             //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_H
#define AVT_SAMPLE_POINTS_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtCellList.h>


class  avtSamplePointsSource;
class  avtVolume;
class  avtWebpage;


// ****************************************************************************
//  Class: avtSamplePoints
//
//  Purpose:
//      Contains samples, whether they be sample points or cells that will
//      later be converted into sample points.  This is an example of a data
//      object in the avt pipeline.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//     Hank Childs, Sat Jan 27 16:28:44 PST 2001
//     Modified the concept of sample points to be either cells or a volume.
//
//     Hank Childs, Mon Jun  4 08:31:22 PDT 2001
//     Inherited from avtDataObject.
//
//     Hank Childs, Sun Dec  4 19:16:23 PST 2005
//     Add [Get|Set]UseWeightingScheme.
//
//     Hank Childs, Thu Jun 21 11:19:02 PDT 2007
//     Add DebugDump.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
// ****************************************************************************

class PIPELINE_API  avtSamplePoints : public avtDataObject
{
  public:
                                avtSamplePoints(avtDataObjectSource *);
    virtual                    ~avtSamplePoints();

    void                        SetNumberOfVariables(std::vector<int> &, 
                                                   std::vector<std::string> &);
    int                         GetNumberOfVariables(void);
    int                         GetNumberOfRealVariables(void) 
                                                   { return static_cast<int>(varnames.size()); };
    const std::string          &GetVariableName(int);
    int                         GetVariableSize(int);
    virtual long long           GetNumberOfCells(bool = false) const
                                   { return celllist->GetNumberOfCells(); };
    void                        SetVolume(int, int, int);
    avtVolume                  *GetVolume(void)  { return volume; };

    avtCellList                *GetCellList(void);
    void                        ResetCellList(void);

    virtual const char         *GetType(void)  { return "avtSamplePoints"; };

    void                        SetUseWeightingScheme(bool b)
                                    { useWeightingScheme = b; };
    bool                        GetUseWeightingScheme(void)
                                    { return useWeightingScheme; };

    virtual void                DebugDump(avtWebpage *, const char *);

  protected:
    avtVolume                  *volume;
    avtCellList                *celllist;
    std::vector<std::string>    varnames;
    std::vector<int>            varsize;
    bool                        useWeightingScheme;
};


typedef ref_ptr<avtSamplePoints>  avtSamplePoints_p;


#endif


