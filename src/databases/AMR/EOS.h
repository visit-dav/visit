
#ifndef EOS_HPP
#define EOS_HPP

#include <math.h>
#include <string.h>

const int  IdealEOS_type = 1;
const int  JwlEOS_type   = 2;
const int  SesameEOS_type= 3;

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



#endif
