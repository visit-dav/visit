/*
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2008 Scientific Computing and Imaging Institute,
  University of Utah.

  
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/


/*
 *  Transform.cc: ?
 *
 *  Written by:
 *   Author ?
 *   Department of Computer Science
 *   University of Utah
 *   Date ?
 *
 */

#include <cstdio>

#include <slivr/Transform.h>
#include <slivr/Point.h>
#include <slivr/Plane.h>

#include <string.h>
#include <iostream>

using namespace SLIVR;
using namespace std;


Transform::Transform()
{
  load_identity();
  inverse_valid=0;
}

Transform::Transform(const Transform& copy)
{
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      mat[i][j]=copy.mat[i][j];
      imat[i][j]=copy.imat[i][j];
    }
  }
  inverse_valid=copy.inverse_valid;
}

Transform::Transform(const Point& p, const Vector& i, 
                     const Vector& j, const Vector& k){
  load_basis(p, i, j, k);
}

Transform::~Transform()
{
}

void
Transform::load_basis(const Point &p,
                      const Vector &x,
                      const Vector &y,
                      const Vector &z)
{
  load_frame(x,y,z);
  pre_translate(p.asVector());
  compute_imat();
}

void
Transform::load_frame(const Vector& x, 
                      const Vector& y, 
                      const Vector& z)
{
  mat[3][3] = imat[3][3] = 1.0;
  mat[0][3] = mat[1][3] = mat[2][3] = 0.0; // no perspective
  imat[0][3] = imat[1][3] = imat[2][3] = 0.0; // no perspective

  mat[3][0] = mat[3][1] = mat[3][2] = 0.0;
  imat[3][0] = imat[3][1] = imat[3][2] = 0.0;

  mat[0][0] = x.x();
  mat[1][0] = x.y();
  mat[2][0] = x.z();

  mat[0][1] = y.x();
  mat[1][1] = y.y();
  mat[2][1] = y.z();

  mat[0][2] = z.x();
  mat[1][2] = z.y();
  mat[2][2] = z.z();

  imat[0][0] = x.x();
  imat[0][1] = x.y();
  imat[0][2] = x.z();

  imat[1][0] = y.x();
  imat[1][1] = y.y();
  imat[1][2] = y.z();

  imat[2][0] = z.x();
  imat[2][1] = z.y();
  imat[2][2] = z.z();

  inverse_valid = 1;
}

void
Transform::change_basis(Transform& T)
{
  T.compute_imat();
  pre_mulmat(T.imat);
}

void
Transform::post_trans(const Transform& T)
{
  post_mulmat(T.mat);
  inverse_valid = 0;
}

void
Transform::pre_trans(const Transform& T)
{
  pre_mulmat(T.mat);
  inverse_valid = 0;
}

void
Transform::print(void)
{
  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++)
      printf("%f ",mat[i][j]); 
    printf("\n");
  }
  printf("\n");
        
}

void
Transform::printi(void)
{
  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++)
      printf("%f ",imat[i][j]); 
    printf("\n");
  }
  printf("\n");
        
}

void
Transform::build_scale(double m[4][4], const Vector& v)
{
  load_identity(m);
  m[0][0]=v.x();
  m[1][1]=v.y();
  m[2][2]=v.z();
  inverse_valid = 0;
}
    
void
Transform::pre_scale(const Vector& v)
{
  double m[4][4];
  build_scale(m,v);
  pre_mulmat(m);
  inverse_valid=0;
}

void
Transform::post_scale(const Vector& v)
{
  double m[4][4];
  build_scale(m,v);
  post_mulmat(m);
  inverse_valid=0;
}

// rotate into a new frame (z=shear-fixed-plane, y=projected shear vector),
//    shear in y (based on value of z), rotate back to original frame
void
Transform::build_shear(double mat[4][4], const Vector& s, const Plane& p) 
{    
  load_identity(mat);
  Vector sv(p.project(s));      // s projected onto p
  Vector dn(s-sv);      // difference (in normal direction) btwn s and sv
  double d=Dot(dn,p.normal());
  if (fabs(d)<0.00001) {
    cerr << "Transform - shear vector lies in shear fixed plane.  Returning identity." << endl;
    return;
  }
  double yshear=sv.length()/d; // compute the length of the shear vector,
  // after the normal-to-shear-plane component
  // has been made unit-length.
  Vector svn(sv);
  svn.normalize();      // normalized vector for building orthonormal basis
  //    Vector su(Cross(sv,p.normal()));
  Vector su(Cross(p.normal(),sv));
  Transform r;  // the rotation to take the z-axis to the shear normal
  // and the y-axis to the projected shear vector
  r.load_frame(su, sv, p.normal());
  //    r.invert();
  Transform sh;
  double a[16];
  sh.get(a);
  a[6]=yshear;
  a[7]=-yshear*p.eval_point(Point(0,0,0));  // this last piece is "d" from the plane
  sh.set(a);
  sh.change_basis(r);
  sh.get(a);
  double *ptr=&(a[0]);
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      mat[i][j]=*ptr++;
  inverse_valid = 0;
}

void
Transform::pre_shear(const Vector& s, const Plane& p)
{
  double m[4][4];
  build_shear(m,s,p);
  pre_mulmat(m);
  inverse_valid=0;
}

void
Transform::post_shear(const Vector& s, const Plane& p)
{
  double m[4][4];
  build_shear(m,s,p);
  post_mulmat(m);
  inverse_valid=0;
}

void
Transform::build_translate(double m[4][4], const Vector& v)
{
  load_identity(m);
  m[0][3]=v.x();
  m[1][3]=v.y();
  m[2][3]=v.z();
  inverse_valid = 0;
}

void
Transform::pre_translate(const Vector& v)
{
  double m[4][4];
  build_translate(m,v);
  pre_mulmat(m);
  inverse_valid=0;
}

void
Transform::post_translate(const Vector& v)
{
  double m[4][4];
  build_translate(m,v);    
  post_mulmat(m);
  inverse_valid=0;
}

void
Transform::build_rotate(double m[4][4], double angle, const Vector& axis)
{
  // From Foley and Van Dam, Pg 227
  // NOTE: Element 0,1 is wrong in the text!
  double sintheta=sin(angle);
  double costheta=cos(angle);
  double ux=axis.x();
  double uy=axis.y();
  double uz=axis.z();
  m[0][0]=ux*ux+costheta*(1-ux*ux);
  m[0][1]=ux*uy*(1-costheta)-uz*sintheta;
  m[0][2]=uz*ux*(1-costheta)+uy*sintheta;
  m[0][3]=0;

  m[1][0]=ux*uy*(1-costheta)+uz*sintheta;
  m[1][1]=uy*uy+costheta*(1-uy*uy);
  m[1][2]=uy*uz*(1-costheta)-ux*sintheta;
  m[1][3]=0;

  m[2][0]=uz*ux*(1-costheta)-uy*sintheta;
  m[2][1]=uy*uz*(1-costheta)+ux*sintheta;
  m[2][2]=uz*uz+costheta*(1-uz*uz);
  m[2][3]=0;

  m[3][0]=0;
  m[3][1]=0;
  m[3][2]=0;
  m[3][3]=1;
  
  inverse_valid=0;
}

void
Transform::pre_rotate(double angle, const Vector& axis)
{
  double m[4][4];
  build_rotate(m, angle, axis);
  pre_mulmat(m);
  inverse_valid=0;
}       

void
Transform::post_rotate(double angle, const Vector& axis)
{
  double m[4][4];
  build_rotate(m, angle, axis);
  post_mulmat(m);
  inverse_valid=0;
}       

bool
Transform::rotate(const Vector& from, const Vector& to)
{
  Vector t(to); t.normalize();
  Vector f(from); f.normalize();
  Vector axis(Cross(f,t));
  if (axis.length2() < 1e-8) {
    // Vectors are too close to each other to get a stable axis of
    // rotation, so return.
    return false;
  }
  double sinth=axis.length();
  double costh=Dot(f,t);
  if(fabs(sinth) < 1.e-9){
    if(costh > 0)
      return false; // no rotate;
    else {
      // from and to are in opposite directions, find an axis of rotation
      // Try the Z axis first.  This will fail if from is along Z, so try
      // Y next.  Then rotate 180 degrees.
      axis = Cross(from, Vector(0,0,1));
      if(axis.length2() < 1.e-9)
        axis = Cross(from, Vector(0,1,0));
      axis.normalize();
      pre_rotate(M_PI, axis);
    }
  } else {
    pre_rotate(atan2(sinth, costh), axis.normal());
  }
  return true;
}

void
Transform::build_permute(double m[4][4],int xmap, int ymap, int zmap, 
                         int pre){
  load_zero(m);
  m[3][3]=1;
  if (pre) {    // for each row, set the mapped column
    if (xmap<0) m[0][-1-xmap]=-1; else m[0][xmap-1]=1;
    if (ymap<0) m[1][-1-ymap]=-1; else m[1][ymap-1]=1;
    if (zmap<0) m[2][-1-zmap]=-1; else m[2][zmap-1]=1;
  } else {      // for each column, set the mapped row
    if (xmap<0) m[-1-xmap][0]=-1; else m[xmap-1][0]=1;
    if (ymap<0) m[-1-ymap][1]=-1; else m[ymap-1][1]=1;
    if (zmap<0) m[-1-zmap][2]=-1; else m[zmap-1][2]=1;
  }
  inverse_valid = 0;
}

void
Transform::pre_permute(int xmap, int ymap, int zmap)
{
  double m[4][4];
  build_permute(m, xmap, ymap, zmap, 1);
  pre_mulmat(m);
  inverse_valid=0;
}

void
Transform::post_permute(int xmap, int ymap, int zmap)
{
  double m[4][4];
  build_permute(m, xmap, ymap, zmap, 0);
  post_mulmat(m);
  inverse_valid=0;
}

Point
Transform::project(const Point& p) const
{
  return Point(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z()+mat[0][3],
               mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z()+mat[1][3],
               mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z()+mat[2][3],
               mat[3][0]*p.x()+mat[3][1]*p.y()+mat[3][2]*p.z()+mat[3][3]);
}

void
Transform::project(const Vector& p, Vector& res) const
{
  res.x(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z());
  res.y(mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z());
  res.z(mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z());
}

void
Transform::project_inplace(Vector& p) const
{
  Vector t = p;
  t.x(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z());
  t.y(mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z());
  t.z(mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z());
  p = t;
}

Vector
Transform::project(const Vector& p) const
{
  return Vector(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z(),
                mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z(),
                mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z());
}
void
Transform::project(const Point& p, Point& res) const
{
  double invw=1./(mat[3][0]*p.x()+mat[3][1]*p.y()+mat[3][2]*p.z()+mat[3][3]);
  res.x(invw*(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z()+mat[0][3]));
  res.y(invw*(mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z()+mat[1][3]));
  res.z(invw*(mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z()+mat[2][3]));
}

void
Transform::project_inplace(Point& p) const
{
 
  Point t = p;
  double invw=1./(mat[3][0]*p.x()+mat[3][1]*p.y()+mat[3][2]*p.z()+mat[3][3]);
  t.x(invw*(mat[0][0]*p.x()+mat[0][1]*p.y()+mat[0][2]*p.z()+mat[0][3]));
  t.y(invw*(mat[1][0]*p.x()+mat[1][1]*p.y()+mat[1][2]*p.z()+mat[1][3]));
  t.z(invw*(mat[2][0]*p.x()+mat[2][1]*p.y()+mat[2][2]*p.z()+mat[2][3]));
  p = t;
}

Point
Transform::unproject(const Point& p) const
{
  if(!inverse_valid) compute_imat();
  return Point(imat[0][0]*p.x()+imat[0][1]*p.y()+imat[0][2]*p.z()+imat[0][3],
               imat[1][0]*p.x()+imat[1][1]*p.y()+imat[1][2]*p.z()+imat[1][3],
               imat[2][0]*p.x()+imat[2][1]*p.y()+imat[2][2]*p.z()+imat[2][3],
               imat[3][0]*p.x()+imat[3][1]*p.y()+imat[3][2]*p.z()+imat[3][3]);
}

void
Transform::unproject(const Point& p, Point& res) const
{
      
  if(!inverse_valid) compute_imat();
  double invw=
    1./(imat[3][0]*p.x()+imat[3][1]*p.y()+imat[3][2]*p.z()+imat[3][3]);
  res.x(invw*
        (imat[0][0]*p.x()+imat[0][1]*p.y()+imat[0][2]*p.z()+imat[0][3]));
  res.y(invw*
        (imat[1][0]*p.x()+imat[1][1]*p.y()+imat[1][2]*p.z()+imat[1][3]));
  res.z(invw*
        (imat[2][0]*p.x()+imat[2][1]*p.y()+imat[2][2]*p.z()+imat[2][3]));
}

void
Transform::unproject_inplace(Point& p) const
{
  Point t = p;

  if(!inverse_valid) compute_imat();
  double invw=
    1./(imat[3][0]*p.x()+imat[3][1]*p.y()+imat[3][2]*p.z()+imat[3][3]);
  t.x(invw*
      (imat[0][0]*p.x()+imat[0][1]*p.y()+imat[0][2]*p.z()+imat[0][3]));
  t.y(invw*
      (imat[1][0]*p.x()+imat[1][1]*p.y()+imat[1][2]*p.z()+imat[1][3]));
  t.z(invw*
      (imat[2][0]*p.x()+imat[2][1]*p.y()+imat[2][2]*p.z()+imat[2][3]));
  p = t;
}

Vector
Transform::unproject(const Vector& p) const
{
  if(!inverse_valid) compute_imat();
  return Vector(imat[0][0]*p.x()+imat[0][1]*p.y()+imat[0][2]*p.z(),
                imat[1][0]*p.x()+imat[1][1]*p.y()+imat[1][2]*p.z(),
                imat[2][0]*p.x()+imat[2][1]*p.y()+imat[2][2]*p.z());
}

void
Transform::unproject(const Vector& v, Vector& res) const
{
  if(!inverse_valid) compute_imat();
  res.x(imat[0][0]*v.x()+imat[0][1]*v.y()+imat[0][2]*v.z());
  res.y(imat[1][0]*v.x()+imat[1][1]*v.y()+imat[1][2]*v.z());
  res.z(imat[2][0]*v.x()+imat[2][1]*v.y()+imat[2][2]*v.z());
}

void
Transform::unproject_inplace(Vector& v) const
{
  Vector t = v;
  if(!inverse_valid) compute_imat();
  t.x(imat[0][0]*v.x()+imat[0][1]*v.y()+imat[0][2]*v.z());
  t.y(imat[1][0]*v.x()+imat[1][1]*v.y()+imat[1][2]*v.z());
  t.z(imat[2][0]*v.x()+imat[2][1]*v.y()+imat[2][2]*v.z());
  v = t;
}

Vector
Transform::project_normal(const Vector& p) const
{
  if(!inverse_valid) compute_imat();
  double x=imat[0][0]*p.x()+imat[1][0]*p.y()+imat[2][0]*p.z();
  double y=imat[0][1]*p.x()+imat[1][1]*p.y()+imat[2][1]*p.z();
  double z=imat[0][2]*p.x()+imat[1][2]*p.y()+imat[2][2]*p.z();
  return Vector(x, y, z);
}

void
Transform::project_normal(const Vector& p, Vector& res)  const
{
  if(!inverse_valid) compute_imat();
  res.x(imat[0][0]*p.x()+imat[1][0]*p.y()+imat[2][0]*p.z());
  res.y(imat[0][1]*p.x()+imat[1][1]*p.y()+imat[2][1]*p.z());
  res.z(imat[0][2]*p.x()+imat[1][2]*p.y()+imat[2][2]*p.z());
}

void
Transform::project_normal_inplace(Vector& p) const
{
  if(!inverse_valid) compute_imat();
  Vector res;
  res.x(imat[0][0]*p.x()+imat[1][0]*p.y()+imat[2][0]*p.z());
  res.y(imat[0][1]*p.x()+imat[1][1]*p.y()+imat[2][1]*p.z());
  res.z(imat[0][2]*p.x()+imat[1][2]*p.y()+imat[2][2]*p.z());
  p = res;
}

void
Transform::get(double* gmat) const
{
  double* p=gmat;
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      *p++=mat[i][j];
    }
  }
}

// GL stores its matrices column-major.  Need to take the transpose...
void
Transform::get_trans(double* gmat) const
{
  double* p=gmat;
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      *p++=mat[j][i];
    }
  }
}

void
Transform::set(double* pmat)
{
  double* p=pmat;
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      mat[i][j]= *p++;
    }
  }
  inverse_valid=0;
}

void
Transform::set_trans(double* pmat)
{
  double* p=pmat;
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      mat[j][i]= *p++;
    }
  }
  inverse_valid=0;
}

void
Transform::load_zero(double m[4][4])
{
  m[0][0] = 0.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
  m[1][0] = 0.0; m[1][1] = 0.0; m[1][2] = 0.0; m[1][3] = 0.0;
  m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 0.0; m[2][3] = 0.0;
  m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 0.0;
}

void
Transform::load_identity()
{
  mat[0][0] = 1.0; mat[0][1] = 0.0; mat[0][2] = 0.0; mat[0][3] = 0.0;
  mat[1][0] = 0.0; mat[1][1] = 1.0; mat[1][2] = 0.0; mat[1][3] = 0.0;
  mat[2][0] = 0.0; mat[2][1] = 0.0; mat[2][2] = 1.0; mat[2][3] = 0.0;
  mat[3][0] = 0.0; mat[3][1] = 0.0; mat[3][2] = 0.0; mat[3][3] = 1.0;
  inverse_valid=0;
}

void
Transform::install_mat(double m[4][4])
{
  memcpy(mat, m, sizeof(double) * 16);
}

void
Transform::load_identity(double m[4][4]) 
{
  m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
  m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
  m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
  m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
}

void
Transform::invert()
{
  double tmp;
  compute_imat();
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
    {
      tmp=mat[i][j];
      mat[i][j]=imat[i][j];
      imat[i][j]=tmp;
    }
}

void
Transform::compute_imat() const
{
  double a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
  a=mat[0][0]; b=mat[0][1]; c=mat[0][2]; d=mat[0][3];
  e=mat[1][0]; f=mat[1][1]; g=mat[1][2]; h=mat[1][3];
  i=mat[2][0]; j=mat[2][1]; k=mat[2][2]; l=mat[2][3];
  m=mat[3][0]; n=mat[3][1]; o=mat[3][2]; p=mat[3][3];

  double q=a*f*k*p - a*f*l*o - a*j*g*p + a*j*h*o + a*n*g*l - a*n*h*k
    - e*b*k*p + e*b*l*o + e*j*c*p - e*j*d*o - e*n*c*l + e*n*d*k
    + i*b*g*p - i*b*h*o - i*f*c*p + i*f*d*o + i*n*c*h - i*n*d*g
    - m*b*g*l + m*b*h*k + m*f*c*l - m*f*d*k - m*j*c*h + m*j*d*g;


  // This test is imperfect. The condition number may be a good indicator,
  // however this is not a perfect indicator neither.
  
  if (fabs(q)==0.0) 
  {
    imat[0][0]=imat[1][1]=imat[2][2]=imat[3][3]=1;
    imat[1][0]=imat[1][2]=imat[1][3]=imat[0][1]=0;
    imat[2][0]=imat[2][1]=imat[2][3]=imat[0][2]=0;
    imat[3][0]=imat[3][1]=imat[3][2]=imat[0][3]=0;
    return;
  }
  
  imat[0][0]=(f*k*p - f*l*o - j*g*p + j*h*o + n*g*l - n*h*k)/q;
  imat[0][1]=-(b*k*p - b*l*o - j*c*p + j*d*o + n*c*l - n*d*k)/q;
  imat[0][2]=(b*g*p - b*h*o - f*c*p + f*d*o + n*c*h - n*d*g)/q;
  imat[0][3]=-(b*g*l - b*h*k - f*c*l + f*d*k + j*c*h - j*d*g)/q;

  imat[1][0]=-(e*k*p - e*l*o - i*g*p + i*h*o + m*g*l - m*h*k)/q;
  imat[1][1]=(a*k*p - a*l*o - i*c*p + i*d*o + m*c*l - m*d*k)/q;
  imat[1][2]=-(a*g*p - a*h*o - e*c*p + e*d*o + m*c*h - m*d*g)/q;
  imat[1][3]=(a*g*l - a*h*k - e*c*l + e*d*k + i*c*h - i*d*g)/q;

  imat[2][0]=(e*j*p - e*l*n - i*f*p + i*h*n + m*f*l - m*h*j)/q;
  imat[2][1]=-(a*j*p - a*l*n - i*b*p + i*d*n + m*b*l - m*d*j)/q;
  imat[2][2]=(a*f*p - a*h*n - e*b*p + e*d*n + m*b*h - m*d*f)/q;
  imat[2][3]=-(a*f*l - a*h*j - e*b*l + e*d*j + i*b*h - i*d*f)/q;

  imat[3][0]=-(e*j*o - e*k*n - i*f*o + i*g*n + m*f*k - m*g*j)/q;
  imat[3][1]=(a*j*o - a*k*n - i*b*o + i*c*n + m*b*k - m*c*j)/q;
  imat[3][2]=-(a*f*o - a*g*n - e*b*o + e*c*n + m*b*g - m*c*f)/q;
  imat[3][3]=(a*f*k - a*g*j - e*b*k + e*c*j + i*b*g - i*c*f)/q;
  
  inverse_valid=1;
}

void
Transform::post_mulmat(const double mmat[4][4])
{
  double newmat[4][4];
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      newmat[i][j]=0.0;
      for(int k=0;k<4;k++){
        newmat[i][j]+=mat[i][k]*mmat[k][j];
      }
    }
  }
  install_mat(newmat);
}

void
Transform::pre_mulmat(const double mmat[4][4])
{
  double newmat[4][4];
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      newmat[i][j]=0.0;
      for(int k=0;k<4;k++){
        newmat[i][j]+=mmat[i][k]*mat[k][j];
      }
    }
  }
  install_mat(newmat);
}

inline double DtoR(double d)
{
  return d*(M_PI/180.);
}

void
Transform::perspective(const Point& eyep, const Point& lookat,
                       const Vector& up, double fov,
                       double znear, double zfar,
                       int xres, int yres)
{
  Vector lookdir(lookat-eyep);
  Vector z(lookdir);
  z.normalize();
  Vector x(Cross(z, up));
  x.normalize();
  Vector y(Cross(x, z));
  double xviewsize=tan(DtoR(fov/2.))*2.;
  double yviewsize=xviewsize*yres/xres;
  double zscale=-znear;
  double xscale=xviewsize*0.5;
  double yscale=yviewsize*0.5;
  x*=xscale;
  y*=yscale;
  z*=zscale;
  //    pre_translate(Point(0,0,0)-eyep);
  double m[4][4];
  // Viewing...
  m[0][0]=x.x(); m[0][1]=y.x(); m[0][2]=z.x(); m[0][3]=eyep.x();
  m[1][0]=x.y(); m[1][1]=y.y(); m[1][2]=z.y(); m[1][3]=eyep.y();
  m[2][0]=x.z(); m[2][1]=y.z(); m[2][2]=z.z(); m[2][3]=eyep.z();
  m[3][0]=0;     m[3][1]=0; m[3][2]=0.0;   m[3][3]=1.0;
  invmat(m);
  pre_mulmat(m);
    
  // Perspective...
  m[0][0]=1.0; m[0][1]=0.0; m[0][2]=0.0; m[0][3]=0.0;
  m[1][0]=0.0; m[1][1]=1.0; m[1][2]=0.0; m[1][3]=0.0;
  m[2][0]=0.0; m[2][1]=0.0; m[2][2]=-(zfar-1)/(1+zfar); m[2][3]=-2*zfar/(1+zfar);
  m[3][0]=0.0; m[3][1]=0.0; m[3][2]=-1.0; m[3][3]=0.0;
  pre_mulmat(m);

  pre_scale(Vector(1,-1,1)); // X starts at the top...
  pre_translate(Vector(1,1,0));
  pre_scale(Vector(xres/2., yres/2., 1.0));     
  m[3][3]+=1.0; // hack
}

void
Transform::invmat(double m[4][4])
{

  double imat[4][4];
  int i;
  for(i=0;i<4;i++){
    for(int j=0;j<4;j++){
      imat[i][j]=0.0;
    }
    imat[i][i]=1.0;
  }

  // Gauss-Jordan with partial pivoting
  for(i=0;i<4;i++){
    double max=fabs(m[i][i]);
    int row=i;
    int j;
    for(j=i+i;j<4;j++){
      if(fabs(m[j][i]) > max){
        max=fabs(m[j][i]);
        row=j;
      }
    }
    assert(max!=0);
    if(row!=i){
      switch_rows(m, i, row);
      switch_rows(imat, i, row);
    }
    double denom=1./m[i][i];
    for(j=i+1;j<4;j++){
      double factor=m[j][i]*denom;
      sub_rows(m, j, i, factor);
      sub_rows(imat, j, i, factor);
    }
  }

  // Jordan
  for(i=1;i<4;i++){
    assert(m[i][i]!=0);
    double denom=1./m[i][i];
    for(int j=0;j<i;j++){
      double factor=m[j][i]*denom;
      sub_rows(m, j, i, factor);
      sub_rows(imat, j, i, factor);
    }
  }

  // Normalize
  for(i=0;i<4;i++){
    assert(m[i][i]!=0);
    double factor=1./m[i][i];
    for(int j=0;j<4;j++){
      imat[i][j] *= factor;
      m[i][j]=imat[i][j];
    }
  }
}

void
Transform::switch_rows(double m[4][4], int r1, int r2) const
{
  for(int i=0;i<4;i++){
    double tmp=m[r1][i];
    m[r1][i]=m[r2][i];
    m[r2][i]=tmp;
  }
}


void
Transform::sub_rows(double m[4][4], int r1, int r2, double mul) const
{
  for(int i=0;i<4;i++)
    m[r1][i] -= m[r2][i]*mul;
}

Transform&
Transform::operator=(const Transform& copy)
{
  for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
      mat[i][j]=copy.mat[i][j];
  inverse_valid=0;
  return *this;
}

Point
SLIVR::operator*(Transform &t, const Point &d)
{
  float result[4], tmp[4];
  result[0] = result[1] = result[2] = result[3] = 0;
  tmp[0] = d(0);
  tmp[1] = d(1);
  tmp[2] = d(2);
  tmp[3] = 1.0;

  double mat[16];
  t.get(mat);

  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++) {
      result[i] += mat[4*i + j] * tmp[j];
    }
  }

  return Point(result[0], result[1], result[2]);
}

Vector
SLIVR::operator*(Transform &t, const Vector &d)
{
  float result[4], tmp[4];
  result[0] = result[1] = result[2] = result[3] = 0;
  tmp[0] = d.x();
  tmp[1] = d.y();
  tmp[2] = d.z();
  tmp[3] = 0.0;

  double mat[16];
  t.get(mat);

  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++) {
      result[i] += mat[4*i + j] * tmp[j];
    }
  }

  return Vector(result[0], result[1], result[2]);
}
