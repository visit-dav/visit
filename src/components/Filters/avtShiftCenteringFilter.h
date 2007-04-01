// ************************************************************************* //
//                         avtShiftCenteringFilter.h                         //
// ************************************************************************* //

#ifndef AVT_SHIFT_CENTERING_FILTER_H
#define AVT_SHIFT_CENTERING_FILTER_H

#include <filters_exports.h>

#include <avtStreamer.h>


class vtkDataSet;
class vtkCellDataToPointData;
class vtkPointDataToCellData;


// ****************************************************************************
//  Class: avtShiftCenteringFilter
//
//  Purpose:
//    A filter which creates node-centered data from point-centered data or
//    vice-verse depending upon the desired centering. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 19, 2001 
//
//  Modifications:
//
//    Jeremy Meredith, Fri Jun 29 15:11:44 PDT 2001
//    Added description.
//
//    Hank Childs, Fri Feb  8 18:33:39 PST 2002
//    Removed all references to PC atts.
//
//    Hank Childs, Wed Feb 27 13:03:32 PST 2002
//    Added RefashionDataObjectInfo.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Wed Aug 11 09:47:46 PDT 2004
//    Added PerformRestriction.
//
// ****************************************************************************

class AVTFILTERS_API avtShiftCenteringFilter : public avtStreamer
{
  public:
                            avtShiftCenteringFilter(int);
    virtual                ~avtShiftCenteringFilter();

    virtual const char     *GetType(void) { return "avtShiftCenteringFilter"; };
    virtual const char     *GetDescription(void) 
                                  { return "Re-centering data"; };

  protected:
    int                     centeringInstruction;

    virtual vtkDataSet     *ExecuteData(vtkDataSet *, int, std::string);
    virtual void            RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                            PerformRestriction(avtPipelineSpecification_p);
};


#endif


