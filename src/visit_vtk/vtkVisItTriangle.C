// ************************************************************************* //
//                          vtkVisItTriangle.C                               //
// ************************************************************************* //

#include <vtkVisItTriangle.h>

#include <vtkObjectFactory.h>
#include <vtkPlane.h>

vtkCxxRevisionMacro(vtkVisItTriangle, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkVisItTriangle);

// ****************************************************************************
//  Method: vtkVisItTriangle constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003
//
//
// **************************************************************************
vtkVisItTriangle::vtkVisItTriangle() : vtkTriangle()
{
}


// ****************************************************************************
//  Method: vtkVisItTriangle::IntersectWithLine
//
//  Purpose:   Determines if line intersects this cell.
//
//  Notes:
//     Plane intersection plus in/out test on triangle. The in/out test is 
//     performed using tol as the tolerance.
//
//  Modified from vtkTriangle to return dist2 in t if EvaluatePosition
//  passes the tolerance test.  Also only uses values from EvaluatePosition
//  if return value == 1.
//
//  Returns:    0 if line does not intersect cell, 1 otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr 14 16:16:42 PDT 2003 
//    Made this work the same as Kitware's version,  as the changes 
//    previously implemented cause other problems with our test data.
//    Will leave this class in the system until the real problem can
//    be found. 
//
//    Kathleen Bonnell, Thu May  8 08:35:25 PDT 2003 
//    Changes to assignments of pcoords, by Will Shroder at Kitware.
//    Works in conjuction with new GetParamatricDistance cell method.
//    (which we will have access to when our VTK gets updated.) 
//    
// **************************************************************************

int 
vtkVisItTriangle::IntersectWithLine(float p1[3], float p2[3], float tol, 
                                  float& t, float x[3], float pcoords[3], 
                                  int& subId)
{
  float *pt1, *pt2, *pt3, n[3];
  float tol2 = tol*tol;
  float closestPoint[3];
  float dist2, weights[3];
  
  subId = 0;
  //
  // Get normal for triangle
  //
  pt1 = this->Points->GetPoint(1);
  pt2 = this->Points->GetPoint(2);
  pt3 = this->Points->GetPoint(0);

  vtkVisItTriangle::ComputeNormal (pt1, pt2, pt3, n);
  //
  // Intersect plane of triangle with line
  //
  if ( ! vtkPlane::IntersectWithLine(p1,p2,n,pt1,t,x) )
    {
    pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
    return 0;
    }

  //
  // Evaluate position
  //
  if (this->EvaluatePosition(x, closestPoint, subId, pcoords, dist2, weights)
      >= 0)  
    {
    if ( dist2 <= tol2 )
      {
      pcoords[2] = 0.0;
      return 1;
      }
    }
  
  // so the easy test failed. The line is not intersecting the triangle.
  // Let's now do the 3d case check to see how close the line comes.
  // basically we just need to test against the three lines of the triangle
  this->Line->PointIds->InsertId(0,0);
  this->Line->PointIds->InsertId(1,1);

  if (pcoords[2] < 0.0)
    {
    this->Line->Points->InsertPoint(0,pt1);
    this->Line->Points->InsertPoint(1,pt2);
    if (this->Line->IntersectWithLine(p1,p2,tol,t,x,pcoords,subId)) 
      {
      pcoords[2] = 0.0;
      return 1;
      }
    }

  if (pcoords[0] < 0.0)
    {
    this->Line->Points->InsertPoint(0,pt2);
    this->Line->Points->InsertPoint(1,pt3);
    if (this->Line->IntersectWithLine(p1,p2,tol,t,x,pcoords,subId)) 
      {
      pcoords[2] = 0.0;
      return 1;
      }
    }

  if (pcoords[1] < 0.0)
    {
    this->Line->Points->InsertPoint(0,pt3);
    this->Line->Points->InsertPoint(1,pt1);
    if (this->Line->IntersectWithLine(p1,p2,tol,t,x,pcoords,subId)) 
      {
      pcoords[2] = 0.0;
      return 1;
      }
    }
  
  pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
  return 0;
}

