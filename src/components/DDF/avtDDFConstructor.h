// ************************************************************************* //
//                             avtDDFConstructor.h                           //
// ************************************************************************* //

#ifndef AVT_DDF_CONSTRUCTOR_H
#define AVT_DDF_CONSTRUCTOR_H

#include <ddf_exports.h>

#include <avtOriginatingDatasetSink.h>

#include <vector>
#include <string>


class     ConstructDDFAttributes;

class     avtDDF;


// ****************************************************************************
//  Class: avtDDFConstructor
//
//  Purpose:
//      This is a data set sink that will generate a DDF.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

class AVTDDF_API avtDDFConstructor : public virtual avtOriginatingDatasetSink
{
  public:
                           avtDDFConstructor();
    virtual               ~avtDDFConstructor();
  
    avtDDF                *ConstructDDF(ConstructDDFAttributes *,
                                        avtPipelineSpecification_p);
};


#endif


