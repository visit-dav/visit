// ************************************************************************* //
//                            avtPointAttribute.h                            //
// ************************************************************************* //

#ifndef AVT_POINT_ATTRIBUTE_H
#define AVT_POINT_ATTRIBUTE_H

#include <pipeline_exports.h>

#include <string>

#include <avtDynamicAttribute.h>

#include <Point.h>


// ****************************************************************************
//  Class: avtPointAttribute
//
//  Purpose:
//      Contains the knowledge of how to convert a standard point attribute
//      into an actual point.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtPointAttribute : public avtDynamicAttribute
{
  public:
                             avtPointAttribute(Point &);
    virtual                 ~avtPointAttribute();

    void                     GetPoint(double [3]);

    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);
    virtual void             ResolveAttributes(avtDataObject_p);

  protected:
    Point                   &attributes;
    double                   point[3];
    std::string              origVar;
    bool                     resolvedAttributes;

    void                     ResolvePoint(void);
    void                     ResolveMin(avtDataObject_p &);
    void                     ResolveMax(avtDataObject_p &);
    void                     ResolveZone(avtDataObject_p &);
    void                     ResolveNode(avtDataObject_p &);
};


#endif


