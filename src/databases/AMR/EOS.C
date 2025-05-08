
#include <EOS.h>

#include <cmath>
#include <limits>

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
    Cv  = Cvi/rho0i;
    gm1 = omgi/rho0i;
    or1 = omgi/(R1i*rho0i);
    or2 = omgi/(R2i*rho0i);
    rho_cf=0.;
    //strcpy(type, JwlEOS_type );

    avmap = {};
}


float JwlEOS::p_from_r_e( float r, float e )
{
  float ri,r1i,r2i,c1,c2,p;
  ri = 1./r;
  r1i= R1 * ri;
  r2i= R2 * ri;
  c1 = A * expf(-r1i);
  c2 = B * expf(-r2i);
  p = c1*( 1. - or1 * r ) + c2*( 1. - or2 * r ) + omg * r * e;
  return p;
}


float JwlEOS::T_from_r_e( float r, float e )
{
  float ri,r1i,r2i,c1,c2,t;
  ri = 1./r;
  r1i= R1 * ri;
  r2i= R2 * ri;
  c1 = A * expf(-r1i);
  c2 = B * expf(-r2i);
  t = ( e - c1/R1 - c2/R2 ) / Cv;
  return t;
}


float JwlEOS::a_from_r_e( float r, float e )
{
  float ri,r1i,r2i,c1,c2,d1,d2,d3,a; 
  ri = 1./r;
  r1i= R1 * ri;
  r2i= R2 * ri;
  c1 = A * expf(-r1i);
  c2 = B * expf(-r2i);
  d1 = c1*( r1i*ri - or1*(1.+omg) );
  d2 = c2*( r2i*ri - or2*(1.+omg) );
  d3 = omg*(1.+omg)*e;
  a  = sqrtf( d1+d2+d3 );
  return a;
}

JwlBEOS::JwlBEOS(float* A, float* R, float* Alam, float* Blam, float* Rlam, float C, float omega, float rho0, float Cv)
{
    /*strcpy(type, GenMixEOS_type ); */

    for ( int i=0; i<5; i++ )
    {
	A_[i] = *(A+i);
	R_[i] = *(R+i);
	Alam_[i] = *(Alam+i);
	Blam_[i] = *(Blam+i);
	Rlam_[i] = *(Rlam+i);
    }

    C_ = C;
    omega_ = omega;
    rho0_ = rho0;
    Cv_ = Cv;

    // define map for additional variables
    avmap = {{"burn fraction", 5}, \
	     {"specific total energy", 6}, \
	     {"specific internal energy", 7}};
}

float JwlBEOS::comp_lambda( float v ){
    float lambda = omega_;

    for( int i=0; i<5; i++ ){
	lambda += (Alam_[i]*v + Blam_[i])*exp(-Rlam_[i]*v);
    }

    return lambda;
};

float JwlBEOS::comp_dlambda_dv( float v ){
    float dlambda_dv = 0.0;

    for( int i=0; i<5; i++ ){
	dlambda_dv += (Alam_[i] - Rlam_[i]*(Alam_[i]*v + Blam_[i]))*expf(-Rlam_[i]*v);
    }

    return dlambda_dv;
};

float JwlBEOS::comp_alpha( float lambda, float v ){
    float alpha = C_*(1.0 - lambda/omega_)*pow(v, -1.0-omega_);

    float eps = std::numeric_limits<float>::epsilon();

    for( int i=0; i<5; i++ ){
	if( fabs(R_[i]) > eps ){
	    alpha += A_[i]*(1.0 - lambda/(R_[i]*v))*expf(-R_[i]*v);
	}
    }

    return alpha;
};

float JwlBEOS::comp_dalpha_dv( float lambda, float dlambda_dv, float v ){
    float dalpha_dv = (-1.0 - omega_)*C_*(1.0 - lambda/omega_)*pow(v, -2.0-omega_) - C_/omega_*dlambda_dv*pow(v, -1.0-omega_);

    float eps = std::numeric_limits<float>::epsilon();

    for( int i=0; i<5; i++ ){
        if( fabs(R_[i]) > eps ){
            float eRv = expf(-R_[i]*v);
            dalpha_dv += A_[i]*(lambda/v - dlambda_dv)*eRv/(R_[i]*v) - A_[i]*R_[i]*(1.0 - lambda/(R_[i]*v))*eRv;
        }
    }

    return dalpha_dv;
};

float JwlBEOS::p_from_r_e( float r, float e ){
    float v = comp_v(r);
    float lambda = comp_lambda(v);
    float alpha = comp_alpha(lambda, v);


    float p = alpha + r*lambda*e;

    if (p < 0.0)
	p = std::numeric_limits<float>::epsilon();

    return p;
};

float JwlBEOS::T_from_r_e( float r, float e ){
    return e / (r * Cv_);
};
float JwlBEOS::a_from_r_e( float r, float e ){
    float eps = std::numeric_limits<float>::epsilon();

    float v = comp_v(r);

    float lambda = comp_lambda(v);
    float dlambda_dv = comp_dlambda_dv(v);
    float alpha = comp_alpha(lambda, v);
    float dalpha_dv = comp_dalpha_dv(lambda, dlambda_dv, v);

    float p = alpha + r*lambda*e;
    if (p < 0.0)
	p = eps;

    float dv_dr = -v*v/rho0_;
    float de_dr = rho0_*p/(r*r);
    float dp_dv = dalpha_dv + e*r*(dlambda_dv - lambda/v);
    float dp_de = lambda/v;

    return sqrt(std::max(dp_dv*dv_dr + dp_de*de_dr, eps));
};

MultiIdealEOS::MultiIdealEOS(int nsp, float gms[], float rs[])
{
        nspec = nsp;

	gammas = new float[nspec];
	gasRs  = new float[nspec];
	
	for( int i=0; i<nspec; i++ )
	{
	    gammas[i] = gms[i];
	    gasRs[i] = rs[i];
	}

	computed_mixture_props = false;

	singleEOS = nullptr;
}

MultiIdealEOS::~MultiIdealEOS()
{
    delete[] gammas;
    delete[] gasRs;
    delete singleEOS;
}

int MultiIdealEOS::compute_mixture_props(float ri[])
{
    computed_mixture_props = false;

    float rmix  = 0.0;
    float rr    = 0.0;
    float Cvmix = 0.0;
    float Cvi   = 0.0;

    for( int i=0; i<nspec; i++ )
    {
        rmix += ri[i]*gasRs[i];

        Cvi = gasRs[i]/(gammas[i] - 1.0);
        Cvmix += ri[i]*Cvi;

        rr += ri[i];
    }
    rmix = rmix / rr;
    Cvmix = Cvmix / rr;

    float gmix = rmix / Cvmix + 1.0;

    delete singleEOS;
    singleEOS = new IdealEOS(gmix, rmix);
    computed_mixture_props = true;

    return 1;
}

float MultiIdealEOS::p_from_r_e(float r, float e)
{
    if (computed_mixture_props)
    {
	computed_mixture_props = false;
	return singleEOS->p_from_r_e(r, e);
    }
    else
    {
	return 0.0;
    }
}

float MultiIdealEOS::T_from_r_e(float r, float e)
{
    if (computed_mixture_props)
    {
	computed_mixture_props = false;
	return singleEOS->T_from_r_e(r, e);
    }
    else
    {
	return 0.0;
    }
}

float MultiIdealEOS::a_from_r_e(float r, float e)
{
    if (computed_mixture_props)
    {
	computed_mixture_props = false;
	return singleEOS->a_from_r_e(r, e);
    }
    else
    {
	return 0;
    }
}
