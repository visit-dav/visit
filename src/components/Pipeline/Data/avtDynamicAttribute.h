// ************************************************************************* //
//                           avtDynamicAttribute.h                           //
// ************************************************************************* //

#ifndef AVT_DYNAMIC_ATTRIBUTE_H
#define AVT_DYNAMIC_ATTRIBUTE_H
#include <pipeline_exports.h>


#include <avtDataObject.h>
#include <avtPipelineSpecification.h>


// ****************************************************************************
//  Class: avtDynamicAttribute
//
//  Purpose:
//      Contains attributes that may be dynamically resolved.  This is a base
//      type that provides an interface.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

class PIPELINE_API avtDynamicAttribute
{
  public:
                               avtDynamicAttribute() {;};
    virtual                   ~avtDynamicAttribute() {;};

    virtual avtPipelineSpecification_p
                               PerformRestriction(avtPipelineSpecification_p);
    virtual void               ResolveAttributes(avtDataObject_p) = 0;
};


#endif


