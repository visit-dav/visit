
#ifndef EOS_HPP
#define EOS_HPP

#include <math.h>
#include <string.h>
#include <map>
#include <string>

const int  IdealEOS_type      = 1;
const int  JwlEOS_type        = 2;
const int  SesameEOS_type     = 3;
const int  GenMixEOS_type     = 4;
const int  IdealTilEOS_type   = 5;
const int  MultiIdealEOS_type = 6;
const int  JwlJwlEOS_type     = 7;
const int  JwlBEOS_type       = 8;

/* const char IdealEOS_type[] ="IdealEOS"; */
/* const char JwlEOS_type[]   ="JwlEOS"; */
/* const char SesameEOS_type[]="SesameEOS"; */


class EOS
{
public:
    EOS() { }
    virtual ~EOS() { }
    /*virtual const char* EOStype() const=0;*/
    virtual int   EOStype() const=0;
    virtual float p_from_r_e( float r, float e )=0;
    virtual float T_from_r_e( float r, float e )=0;
    virtual float a_from_r_e( float r, float e )=0;
    virtual int compute_mixture_props( float ri[] ){
	return 0;
    }

    virtual bool p_from_av(){ return false; }
    virtual bool T_from_av(){ return false; }
    virtual bool a_from_av(){ return false; }

    std::map<std::string, int> avmap = {};
};



class IdealEOS : public EOS
{
public:
    IdealEOS() : gamma(0.f), gasR(0.f) { }
    IdealEOS( float gm, float R )
    {
        gamma=gm;
        gasR =R;
        //strcpy(type, IdealEOS_type );
    }
    virtual ~IdealEOS() { }

    /* virtual const char* EOStype() const { */
    /*   return type; */
    /* } */
    virtual int   EOStype() const
    {
        return IdealEOS_type;
    }
    virtual float p_from_r_e( float r, float e )
    {
        return ( gamma - 1. )*r*e;
    }
    virtual float T_from_r_e( float r, float e )
    {
        return ( gamma - 1. )*e / gasR;
    }
    virtual float a_from_r_e( float r, float e )
    {
        return sqrt( gamma*( gamma - 1. )*e );
    }

protected:
    float gamma;
    float gasR;
    //char type[20];
};




class JwlEOS : public EOS
{
public:
    JwlEOS( float Ai, float Bi, float R1i, float R2i,
            float omgi, float rho0i, float Cvi );
    virtual ~JwlEOS() { }

    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return JwlEOS_type;
    }
    virtual float p_from_r_e( float r, float e );
    virtual float T_from_r_e( float r, float e );
    virtual float a_from_r_e( float r, float e );

protected:
    float A,B,R1,R2,omg,rho0,Cv;
    float gm1,or1,or2,rho_cf;
    //char type[20];
};

class JwlBEOS : public EOS
{
public:
    JwlBEOS(float A[], float R[], float Alam[], float Blam[], float Rlam[], float C, float omega, float rho0, float Cv);
    virtual ~JwlBEOS() { }
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return JwlBEOS_type;
    }
    virtual float p_from_r_e( float r, float e );
    virtual float T_from_r_e( float r, float e );
    virtual float a_from_r_e( float r, float e );

protected:
    //char type[20];

    float A_[5];
    float R_[5];
    float Alam_[5];
    float Blam_[5];
    float Rlam_[5];
    float C_;
    float omega_;
    float rho0_;
    float Cv_;

    virtual float comp_v(float r){ return rho0_/r; }
    virtual float comp_lambda(float v);
    virtual float comp_dlambda_dv(float v);
    virtual float comp_alpha(float lambda, float v);
    virtual float comp_dalpha_dv(float lambda, float dlambda_dv, float v);
};

class SesameEOS : public EOS
{
public:
    SesameEOS()
    {
        /*strcpy(type, SesameEOS_type ); */
    }
    virtual ~SesameEOS() { }
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return SesameEOS_type;
    }
    virtual float p_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float T_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float a_from_r_e( float r, float e )
    {
        return 0.;
    }

protected:
    //char type[20];
};

class GenMixEOS : public EOS
{
public:
    GenMixEOS(int nspec)
    {
        /*strcpy(type, GenMixEOS_type ); */
	avmap = {};

	//for( int i=0; i<nspec; i++ )
	//{
	//    std::string name = "species density " + std::to_string(i+1);
	//    avmap.insert({name, 6+i});
	//}

	for( int i=0; i<nspec; i++ )
	{
	    std::string name = "volume fraction " + std::to_string(i+1);
	    avmap.insert({name, 6+nspec + i});
	}
    }
    virtual ~GenMixEOS() { }
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return GenMixEOS_type;
    }
    virtual float p_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float T_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float a_from_r_e( float r, float e )
    {
        return 0.;
    }

    virtual bool a_from_av(){ return true; }

protected:
    //char type[20];
};

class IdealTilEOS : public EOS
{
public:
    IdealTilEOS()
    {
        /*strcpy(type, GenMixEOS_type ); */
	avmap = {{"species internal energy gas", 6}, \
	         {"sound speed gas", 7}, \
	         {"species internal energy Til", 10}, \
	         {"sound speed Til", 11}};
    }
    virtual ~IdealTilEOS() { }
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return IdealTilEOS_type;
    }
    virtual float p_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float T_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float a_from_r_e( float r, float e )
    {
        return 0.;
    }

    virtual bool p_from_av(){ return true; }
    virtual bool T_from_av(){ return true; }
    virtual bool a_from_av(){ return true; }

protected:
    //char type[20];
};

class MultiIdealEOS : public EOS
{
public:
    MultiIdealEOS(int nspec, float gms[], float rs[]);
    virtual ~MultiIdealEOS();
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return MultiIdealEOS_type;
    }
    virtual float p_from_r_e( float r, float e );
    virtual float T_from_r_e( float r, float e );
    virtual float a_from_r_e( float r, float e );
    virtual int   compute_mixture_props(float ri[]);

protected:
    int nspec;
    float* gammas;
    float* gasRs;
    bool computed_mixture_props;
    IdealEOS* singleEOS;
    //char type[20];
};

class JwlJwlEOS : public EOS
{
public:
    JwlJwlEOS()
    {
        /*strcpy(type, GenMixEOS_type ); */
	avmap = {{"species internal energy 1", 6}, \
	         {"sound speed 1", 7}, \
	         {"species internal energy 2", 10}, \
	         {"sound speed 2", 11}};
    }
    virtual ~JwlJwlEOS() { }
    //virtual const char* EOStype() const { return type; }
    virtual int   EOStype() const
    {
        return JwlJwlEOS_type;
    }
    virtual float p_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float T_from_r_e( float r, float e )
    {
        return 0.;
    }
    virtual float a_from_r_e( float r, float e )
    {
        return 0.;
    }

    virtual bool p_from_av(){ return true; }
    virtual bool T_from_av(){ return true; }
    virtual bool a_from_av(){ return true; }


protected:
    //char type[20];
};

#endif
