#include <vtkVisItQuad.h>

#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkTriangle.h>

vtkCxxRevisionMacro(vtkVisItQuad, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkVisItQuad);


// Construct the quad with four points.
vtkVisItQuad::vtkVisItQuad()
{
  this->Triangle = vtkTriangle::New();
}

vtkVisItQuad::~vtkVisItQuad()
{
  this->Triangle->Delete(); 
}


// Intersect plane; see whether point is in quadrilateral. This code
// splits the quad into two triangles and intersects them (because the
// quad may be non-planar).
//
int vtkVisItQuad::IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                              float x[3], float pcoords[3], int& subId)
{
  int diagonalCase;
  float d1 = vtkMath::Distance2BetweenPoints(this->Points->GetPoint(0), 
                                             this->Points->GetPoint(2));
  float d2 = vtkMath::Distance2BetweenPoints(this->Points->GetPoint(1), 
                                             this->Points->GetPoint(3));
  subId = 0;
  
  // Figure out how to uniquely tessellate the quad. Watch out for 
  // equivalent triangulations (i.e., the triangulation is equivalent
  // no matter where the diagonal). In this case use the point ids as 
  // a tie breaker to insure unique triangulation across the quad.
  //
  if ( d1 == d2 ) //rare case; discriminate based on point id
    {
    int i, id, maxId=0, maxIdx=0;
    for (i=0; i<4; i++) //find the maximum id
      {
      if ( (id=this->PointIds->GetId(i)) > maxId )
        {
        maxId = id;
        maxIdx = i;
        }
      }
    if ( maxIdx == 0 || maxIdx == 2) diagonalCase = 0;
    else diagonalCase = 1;
    }
  else if ( d1 < d2 )
    {
    diagonalCase = 0;
    }
  else //d2 < d1
    {
    diagonalCase = 1;
    }

  // Note: in the following code the parametric coords must be adjusted to
  // reflect the use of the triangle parametric coordinate system.
  switch (diagonalCase)
    {
    case 0:
      this->Triangle->Points->SetPoint(0,this->Points->GetPoint(0));
      this->Triangle->Points->SetPoint(1,this->Points->GetPoint(1));
      this->Triangle->Points->SetPoint(2,this->Points->GetPoint(2));
      if (this->Triangle->IntersectWithLine(p1, p2, tol, t, x, pcoords, subId) )
        {
        pcoords[0] = pcoords[0] + pcoords[1];
        return 1;
        }
      this->Triangle->Points->SetPoint(0,this->Points->GetPoint(2));
      this->Triangle->Points->SetPoint(1,this->Points->GetPoint(3));
      this->Triangle->Points->SetPoint(2,this->Points->GetPoint(0));
      if (this->Triangle->IntersectWithLine(p1, p2, tol, t, x, pcoords, subId) )
        {
        pcoords[0] = 1.0 - (pcoords[0]+pcoords[1]);
        pcoords[1] = 1.0 - pcoords[1];
        return 1;
        }
      return 0;

    case 1:
      this->Triangle->Points->SetPoint(0,this->Points->GetPoint(0));
      this->Triangle->Points->SetPoint(1,this->Points->GetPoint(1));
      this->Triangle->Points->SetPoint(2,this->Points->GetPoint(3));
      if (this->Triangle->IntersectWithLine(p1, p2, tol, t, x, pcoords, subId) )
        {
        return 1;
        }
      this->Triangle->Points->SetPoint(0,this->Points->GetPoint(2));
      this->Triangle->Points->SetPoint(1,this->Points->GetPoint(3));
      this->Triangle->Points->SetPoint(2,this->Points->GetPoint(1));
      if (this->Triangle->IntersectWithLine(p1, p2, tol, t, x, pcoords, subId) )
        {
        pcoords[0] = 1.0 - pcoords[0];
        pcoords[1] = 1.0 - pcoords[1];
        return 1;
        }
      return 0;

      break;
    }

  return 0;
}

