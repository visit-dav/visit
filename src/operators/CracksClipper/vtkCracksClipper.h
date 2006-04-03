#ifndef VTK_CRACKS_CLIPPER_H
#define VTK_CRACKS_CLIPPER_H

#include <vtkVisItClipper.h>
#include <vtkImplicitFunction.h>

class vtkImplicitBoolean;


class AlwaysNegative : public vtkImplicitFunction
{
public:
  vtkTypeMacro(AlwaysNegative, vtkImplicitFunction);
  static AlwaysNegative *New();

  virtual double EvaluateFunction(double x[3])
      { return -1.; }
 virtual void EvaluateGradient(double x[3], double g[3]) 
      {g[0] = g[1] = g[2] = 1.; }; 

  void SetReturnNeg(bool val) { returnNeg = val; }

protected:
  AlwaysNegative();
  ~AlwaysNegative();

private:
  bool returnNeg;
  AlwaysNegative(const AlwaysNegative&);  // Not implemented.
  void operator=(const AlwaysNegative&);  // Not implemented.
};


// ****************************************************************************
//  Class:  vtkCracksClipper
//
//  Purpose:
//    Clips a dataset using an implicit function, defined on a per-cell
//    basis based upon crack-direction vectors and strain variables.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 22, 2005 
//
//  Modifications:
//
// ****************************************************************************

class vtkCracksClipper : public vtkVisItClipper
{
  public:
    vtkTypeRevisionMacro(vtkCracksClipper,vtkVisItClipper);

    static vtkCracksClipper *New();

    virtual void SetUpClipFunction(int);
    virtual void SetUseOppositePlane(bool val) { useOppositePlane = val;};

    vtkSetStringMacro(CrackDir);
    vtkSetStringMacro(CrackWidth);
    vtkSetStringMacro(CellCenters);

  protected:
    vtkCracksClipper();
    ~vtkCracksClipper();

  private:
    vtkCracksClipper(const vtkCracksClipper&);  // Not implemented.
    void operator=(const vtkCracksClipper&);  // Not implemented.

    vtkImplicitBoolean *cf;
    AlwaysNegative *emptyFunc;
    bool useOppositePlane;

    char *CrackDir;
    char *CrackWidth;
    char *CellCenters;
};



#endif


