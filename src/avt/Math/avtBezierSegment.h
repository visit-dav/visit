/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef AVT_BEZIERSEGMENT_H
#define AVT_BEZIERSEGMENT_H

#include <avtVecArray.h>

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
// ****************************************************************************

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

    avtVec firstV() const
    {
        avtVec v;
        if (size() > 0)
            v = (*this)[0];
        return v;
    }

    avtVec lastV() const
    {
        avtVec v;
        if (size() > 0)
            v = (*this)[size()-1];
        return v;
    }

    avtVec evaluate( const double& param )
    {
        // BezierSegment evaluation using deCasteljau's scheme
        avtVecArray tmp( *this );
        
        for( size_t l=1; l<order(); ++l )
        {
            for( size_t i=degree(); i>=l; --i )
                tmp[i] = lerp( param, tmp[i-1], tmp[i] );
        }
         
        return tmp[degree()];
    }

    avtVec derivative( const double& param, unsigned int order = 1 )
    {
        avtVecArray tmp( *this );
        
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


