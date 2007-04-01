// ************************************************************************* //
//                           avtSamplePointsSource.h                         //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_SOURCE_H
#define AVT_SAMPLE_POINTS_SOURCE_H
#include <pipeline_exports.h>


#include <avtDataObjectSource.h>
#include <avtSamplePoints.h>


// ****************************************************************************
//  Class: avtSamplePointsSource
//
//  Purpose:
//      The source of a sample points object.  A source could be a transient
//      source (like a filter) or a terminating source.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:22:14 PDT 2001
//    Inherited from avtDataObjectSource.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsSource : virtual public avtDataObjectSource
{
  public:
                                avtSamplePointsSource();
    virtual                    ~avtSamplePointsSource();

    virtual avtDataObject_p     GetOutput(void);

    void                        SetTypedOutput(avtSamplePoints_p);
    avtSamplePoints_p           GetTypedOutput(void) { return samples; };

  protected:
    avtSamplePoints_p           samples;
};


#endif

    
