#ifndef AVT_BEZIERSEGMENT_H
#define AVT_BEZIERSEGMENT_H

#include <avtVecArray.h>

class avtBezierSegment: public avtVecArray
{
public:

    avtBezierSegment()
    {
    }

    avtBezierSegment( unsigned int dim, unsigned int order ) :
        avtVecArray( dim, order )
    {
    }

    unsigned int degree() const 
    { 
        return size()-1; 
    }

    unsigned int order() const
    {
        return size();
    }

    avtVec evaluate( const double& param )
    {
        // BezierSegment evaluation using deCasteljau's scheme
        avtVecArray tmp( *this );
        
        for( int l=1; l<order(); ++l )
        {
            for( int i=degree(); i>=l; --i )
                tmp[i] = lerp( param, tmp[i-1], tmp[i] );
        }
         
        return tmp[degree()];
    }

    avtVec derivative( const double& param, unsigned int order = 1 )
    {
        avtVecArray tmp( *this );
        
        for( int i=0; i<order; ++i )
            tmp[i] = tmp[i+1] - tmp[i];

        order = degree() - order + 1;

        const double a = (1.0-param);
        const double b = param;

        for( int l=1; l<order; ++l )
            for( int i=order-1; i>=l; --i )
                tmp[i] = lerp( param, tmp[i-1], tmp[i] );
                
        return tmp[degree()];
    }

    double length( double t0=0.0, double t1=1.0, double eps=1e-6 )
    {
        double len = 0.0;
        int N = 10;
        double dt = (t1-t0)/(double)(N-1);
        avtVec p0 = evaluate( t0 );
        double t = t0+dt;
        for ( int i = 1; i < N; i++ )
        {
            avtVec p1 = evaluate( t );
            len += (p1-p0).length();
            p0 = p1;
            t += dt;
        }
        
        return len;
    }
};

#endif // AVT_BEZIERSEGMENT_H


