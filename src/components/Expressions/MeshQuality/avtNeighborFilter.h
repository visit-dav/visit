// ************************************************************************* //
//                            avtNeighborFilter.h                            //
// ************************************************************************* //

#ifndef AVT_NEIGHBOR_FILTER_H
#define AVT_NEIGHBOR_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtNeighborFilter
//
//  Purpose:
//    This is a filter that takes a mesh and decomposes it into a mesh of
//    vertexes. The points are also assigned a variable based on the distance
//    to the next nearest node.
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 20 15:51:54 PST 2004
//    Re-define GetVariableDimension.
//
// ****************************************************************************

class EXPRESSION_API avtNeighborFilter : public avtSingleInputExpressionFilter
{
  public:
                             avtNeighborFilter();
    virtual                 ~avtNeighborFilter();

  protected:
    static const char        *variableName;

    virtual const char       *GetType(void)   { return "avtNeighborFilter"; };
    virtual const char       *GetDescription(void)
                             { return "Create vertex mesh,"
                                      " find distance to nearest node"; };

    // Used to fullfill parent's requirement, but unused since
    // ExecuteData exists for this derived class.
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *) { return NULL; }

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             RefashionDataObjectInfo(void);

    virtual bool             IsPointVariable()     { return true; };
    virtual int              GetVariableDimension()   { return 1; };
};

#endif
