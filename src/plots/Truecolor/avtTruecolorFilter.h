// ************************************************************************* //
//                              avtTruecolorFilter.h                              //
// ************************************************************************* //

#ifndef AVT_Truecolor_FILTER_H
#define AVT_Truecolor_FILTER_H


#include <avtStreamer.h>


// ****************************************************************************
//  Class: avtTruecolorFilter
//
//  Purpose:
//      This operator is the implied operator associated with an Truecolor plot.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday June 15, 2004
//
//  Modifications:
//
//     Chris Wojtan Mon Jun 21 15:45 PDT 2004
//     Added "variable_name" member variable
//     Added SetVarName member function
//
// ****************************************************************************

class avtTruecolorFilter : public avtStreamer
{
  public:
                              avtTruecolorFilter();
    virtual                  ~avtTruecolorFilter();

    virtual const char       *GetType(void)   { return "avtTruecolorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Performing Truecolor"; };
    void                      SetVarName(const char*name)
                                  {variable_name = name;}

  protected:
    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
    const char               *variable_name;
};


#endif
