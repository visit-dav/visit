// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_VISIT_INTERPOLATED_VELOCITY_FIELD
#define VTK_VISIT_INTERPOLATED_VELOCITY_FIELD

#include <vtkObject.h>
#include <visit_vtk_exports.h>

class vtkDataSet;
class vtkVisItCellLocator;

class VISIT_VTK_API vtkVisItInterpolatedVelocityField  : public vtkObject
{
  public:
  vtkTypeMacro(vtkVisItInterpolatedVelocityField,vtkObject);
                  vtkVisItInterpolatedVelocityField();
    virtual      ~vtkVisItInterpolatedVelocityField();

    static vtkVisItInterpolatedVelocityField *New();
    void          SetDataSet(vtkDataSet *ds);
    void          SetLocator(vtkVisItCellLocator *loc);
    bool          Evaluate(double *pt, double *vel, double t = 0.);
 
    void          SetDoPathlines(bool b) { doPathlines = b; };
    void          SetCurrentTime(double t) { curTime = t; };
    void          SetNextTime(double t) { nextTime = t; };
    void          SetNextTimeName(const std::string &s) { nextTimeName = s; };

    vtkDataSet   *GetDataSet(void) { return ds; };
    double       *GetLastWeights(void) { return weights; };
    int           GetLastCell(void) { return lastCell; };
    double       *GetLastPCoords(void) { return pcoords; };

  private:
    vtkDataSet  *ds;
    vtkVisItCellLocator *locator;
    double       weights[1024];
    int          lastCell;
    double       pcoords[3];
    bool         doPathlines;
    std::string  nextTimeName;
    double       curTime;
    double       nextTime;
};

#endif

