// ************************************************************************* //
//                             avtSamplePoints.h                             //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_H
#define AVT_SAMPLE_POINTS_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObject.h>


class  avtCellList;
class  avtSamplePointsSource;
class  avtVolume;


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
// ****************************************************************************

class PIPELINE_API  avtSamplePoints : public avtDataObject
{
  public:
                                avtSamplePoints(avtDataObjectSource *);
    virtual                    ~avtSamplePoints();

    void                        SetNumberOfVariables(int);
    int                         GetNumberOfVariables(void) { return numVars; };
    void                        SetVolume(int, int, int);
    avtVolume                  *GetVolume(void)  { return volume; };

    avtCellList                *GetCellList(void);

    virtual const char         *GetType(void)  { return "avtSamplePoints"; };

  protected:
    avtVolume                  *volume;
    avtCellList                *celllist;
    int                         numVars;
};


typedef ref_ptr<avtSamplePoints>  avtSamplePoints_p;


#endif


