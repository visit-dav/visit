// ************************************************************************* //
//                           avtLabelsMapper.h                               //
// ************************************************************************* //

#ifndef AVT_LABELS_MAPPER_H
#define AVT_LABELS_MAPPER_H

#include <avtUserDefinedMapper.h>
#include <vector>
#include <string>

// ****************************************************************************
//  Class: avtLabelsMapper
//
//  Purpose:
//      A mapper that allows us to pass 
//      
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 3 17:01:06 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtLabelsMapper : public avtUserDefinedMapper
{
  public:
                               avtLabelsMapper(avtCustomRenderer_p);
    virtual                   ~avtLabelsMapper();

    virtual void               SetLabels(std::vector<std::string> &L, bool);

  protected:
    vtkDataSetMapper          *CreateMapper(void);

    virtual void               CustomizeMappers(void);
    std::vector<std::string>   labelNames;
};

#endif


