// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtShiftCenteringFilter.h                         //
// ************************************************************************* //

#ifndef AVT_SHIFT_CENTERING_FILTER_H
#define AVT_SHIFT_CENTERING_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtShiftCenteringFilter
//
//  Purpose:
//    A filter which creates node-centered data from point-centered data or
//    vice-versa depending upon the desired centering. 
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
//    Added UpdateDataObjectInfo.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Wed Aug 11 09:47:46 PDT 2004
//    Added ModifyContract.
//
//    Sean Ahern, Wed Sep 10 13:13:18 EDT 2008
//    For ease of reading code, I forced the argument to be avtCentering, not
//    integer.
//
//    Eric Brugger, Tue Jul 22 08:00:59 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtShiftCenteringFilter : public avtDataTreeIterator
{
  public:
                            avtShiftCenteringFilter(avtCentering);
    virtual                ~avtShiftCenteringFilter();

    virtual const char     *GetType(void) { return "avtShiftCenteringFilter"; };
    virtual const char     *GetDescription(void) 
                                  { return "Re-centering data"; };

  protected:
    avtCentering            centeringTarget;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void            UpdateDataObjectInfo(void);
    virtual avtContract_p
                            ModifyContract(avtContract_p);
};


#endif


