    
#include <EOS.h>

JwlEOS::
JwlEOS( float Ai, float Bi, float R1i, float R2i, 
      float omgi, float rho0i, float Cvi ) 
{
    A   = Ai;
    B   = Bi;
    R1  = R1i*rho0i;
    R2  = R2i*rho0i;
    omg = omgi;
    rho0= rho0i;
    Cv  = Cvi;
    gm1 = omgi/rho0i;
    or1 = omgi/(R1i*rho0i);
    or2 = omgi/(R2i*rho0i);
    rho_cf=0.;
    //strcpy(type, JwlEOS_type );
}


float JwlEOS::p_from_r_e( float r, float e )
{
  float ri,r1i,r2i,c1,c2,p;
  ri = 1./r;
  r1i= R1 * ri;
  r2i= R2 * ri;
  c1 = A * exp(-r1i);
  c2 = B * exp(-r2i);
  p = c1*( 1. - or1 * r ) + c2*( 1. - or2 * r ) + gm1 * r * e;
  return p;
}


float JwlEOS::T_from_r_e( float r, float e )
{
  float t;
  t = e / Cv;
  return t;
}


float JwlEOS::a_from_r_e( float r, float e )
{
  float ri,r1i,r2i,c1,c2,p,d1,d2,d3,a;
  ri = 1./r;
  r1i= R1 * ri;
  r2i= R2 * ri;
  c1 = A * exp(-r1i);
  c2 = B * exp(-r2i);
  p  = c1*( 1. - or1 * r ) + c2*( 1. - or2 * r ) + gm1 * r * e;
  d1 = c1*(( r1i - omg )*ri - or1 );
  d2 = c2*(( r2i - omg )*ri - or2 );
  d3 = gm1 * ( e + p*ri );
  a = sqrt( d1 + d2 + d3 );
  return a;
}

