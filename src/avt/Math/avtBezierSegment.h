// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_BEZIERSEGMENT_H
#define AVT_BEZIERSEGMENT_H

#include <avtVector.h>
#include <vector>
#include <MemStream.h>

// ****************************************************************************
//  Class avtBezierSegment
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:26:32 PDT 2008
//    Changed for loops to use size_t to eliminate signed/unsigned int 
//    comparison warnings.
//
//    Jeremy Meredith, Thu Aug  7 14:38:59 EDT 2008
//    Removed unused variables.
//
//    Dave Pugmire, Wed Jun 10 11:39:12 EDT 2009
//    Add firstV and lastV methods.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

class avtBezierSegment: public std::vector<avtVector>
{
public:

    avtBezierSegment()
    {
    }

    avtBezierSegment(unsigned int order)
    {
        resize(order);
    }

    unsigned int degree() const 
    { 
        return (unsigned int)size()-1; 
    }

    unsigned int order() const
    {
        return (unsigned int)size();
    }

    avtVector firstV() const
    {
        avtVector v;
        if (size() > 0)
            v = (*this)[0];
        return v;
    }

    avtVector lastV() const
    {
        avtVector v;
        if (size() > 0)
            v = (*this)[size()-1];
        return v;
    }

    avtVector evaluate( const double& param )
    {
        // BezierSegment evaluation using deCasteljau's scheme
        std::vector<avtVector> tmp(*this);
        
        for( size_t l=1; l<order(); ++l )
        {
            for( size_t i=degree(); i>=l; --i )
                tmp[i] = lerp( param, tmp[i-1], tmp[i] );
        }
         
        return tmp[degree()];
    }

    avtVector derivative( const double& param, unsigned int order = 1 )
    {
        std::vector<avtVector> tmp(*this);
        
        for( size_t i=0; i<order; ++i )
            tmp[i] = tmp[i+1] - tmp[i];

        order = degree() - order + 1;

        for( size_t l=1; l<order; ++l )
            for( size_t i=order-1; i>=l; --i )
                tmp[i] = lerp( param, tmp[i-1], tmp[i] );
                
        return tmp[degree()];
    }

    double length( double t0=0.0, double t1=1.0, double eps=1e-6 )
    {
        double len = 0.0;
        int N = 10;
        double dt = (t1-t0)/(double)(N-1);
        avtVector p0 = evaluate( t0 );
        double t = t0+dt;
        for ( int i = 1; i < N; i++ )
        {
            avtVector p1 = evaluate( t );
            len += (p1-p0).length();
            p0 = p1;
            t += dt;
        }
        
        return len;
    }

    virtual void Serialize(MemStream::Mode mode, MemStream &buff)
    {
        std::vector<avtVector> *vec = this;
        buff.io(mode, *vec);
    }
};

#endif // AVT_BEZIERSEGMENT_H


