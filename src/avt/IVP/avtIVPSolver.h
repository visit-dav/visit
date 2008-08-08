/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                               avtIVPSolver.h                              //
// ************************************************************************* //

#ifndef AVT_IVPSOLVER_H
#define AVT_IVPSOLVER_H

#include <avtVec.h>
#include <avtIVPField.h>
#include <avtBezierSegment.h>
#include <MemStream.h>

struct avtIVPStateHelper;


// ****************************************************************************
//  Class: avtIVPStep
//
//  Purpose:
//      avtIVPStep encapsulates the IVP solution obtained in a single step. The
//      global solution is obtained by the collection of steps an IVP solver 
//      has returned.
//
//      To accomodate many different kinds of numerical schemes, each step is 
//      treated as a polynomial and represented by in Bezier form as a Bezier 
//      segment (actually, avtIVPStep derives from avtBezierSegment). Hence, 
//      an avtIVPStep instance can be queried for any point of the solution on 
//      its corresponding parameter interval, and also supports evaluation of 
//      derivatives. This enables advanced applications such as direct 
//      intersection of solutions with a plane, or the Illuminated Streamlines 
//      technique, which all require solution derivatives for efficient 
//      implementation.
//
//      While polynomial representation is slightly inefficient for simpler 
//      schemes that do not produce dense output in the form of a polynomial 
//      but only a sequence of points, the polynomial step representation is 
//      still an acceptable cost since it drastically reduces implementation 
//      complexity.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7 08:50:13 PDT 2008
//    Changed return of ComputeVoriticity from double to void.
//
// ****************************************************************************

class IVP_API avtIVPStep: public avtBezierSegment
{
public:
    avtIVPStep() : avtBezierSegment() { tStart = tEnd = vorticity = 0.0; }
    
    void   ComputeVorticity(const avtIVPField *field)
    {
        double tMid = tStart + (tEnd-tStart)/2.0;
        
        avtVec pt = evaluate( tMid );
        if ( field->IsInside( tMid, pt ) )
            vorticity = field->ComputeVorticity( tMid, pt );
    }

    void     Serialize(MemStream::Mode mode, MemStream &buff)
    {
        //debug1 << "avtIVPStep::Serialize()\n";
        buff.io( mode, tStart );
        buff.io( mode, tEnd );
        buff.io( mode, vorticity );
        
        //TODO
        //avtBezierSegment::Serialize( mode, buff );
        buff.io( mode, _dim );
        buff.io( mode, _data );
    }

    double   Length(double eps=1e-6)
    {
        return avtBezierSegment::length( 0.0, 1.0, eps );
    }
    
    double tStart, tEnd;
    double vorticity;
};


// ****************************************************************************
//  Class: avtIVPState
//
//  Purpose:
//      A snapshot of the state of an avtIVPSolver, needed for transferring
//      solutions over a network.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

class avtIVPState
{
  public:
               avtIVPState() : _data(0), _size(0) { ; };
               avtIVPState(size_t sz, unsigned char *buff)
               {
                   _size = sz;
                   _data = new unsigned char[_size];
                   memcpy(_data,buff,_size);
               }

               ~avtIVPState() { delete [] _data; };

    size_t     size() const { return _size; };

    void*      data() const { return _data; }

    void       Serialize(MemStream::Mode mode, MemStream &buff)
               {
                   //debug1 << "avtIVPState::Serialize()\n";
                   if ( mode == MemStream::WRITE )
                   {
                       buff.io(mode, _size );
                       buff.io(mode, _data, _size);
                   }
                   else
                   {
                       size_t sz;
                       buff.io(mode, sz);
                       allocate(sz);
                       buff.io(mode, _data, sz);
                   }
                   //debug1 << "DONE: avtIVPState::Serialize() sz= " 
                   //       << size() << endl;
               }

  private:
    void       allocate( size_t size )
               {      
                   if ( _data )
                       delete[] _data;
           
                   _size = size;
                   _data = new unsigned char[size];
               }
 
               avtIVPState( const avtIVPState& );
               avtIVPState& operator=( const avtIVPState& );
       
    unsigned char* _data;
    size_t         _size;
    
    friend class avtIVPSolver;
};


// ****************************************************************************
//  Class: avtIVPSolver
//
//  Purpose:
//      avtIVPSolver is an abstract type. It embodies the low-level interface 
//      and represents a specific numerical scheme used for the solution of an 
//      Initial Value Problem (IVP) corresponding to a streamline. Any 
//      numerical scheme derives from avtIVPSolver and adheres to a basic 
//      common interface.
//
//      Setting up an IVP corresponds to instantiating a concrete, derived type 
//      of avtIVPSolver and setting various parameters. The solution is then 
//      computed in a step-by-step fashion using the step() method, which will 
//      optionally deliver the part of the IVP solution corresponding to this 
//      step in an avtIVPStep object. avtIVPSolver does not keep track of the 
//      global IVP solution, but only maintains an essential internal state 
//      that mirrors the solution history (this is important e.g. for adaptive 
//      stepsize control).
//
//      This state can be exported as an avtIVPState object using the method 
//      pair getState()/putState(). This latter mechanism should allow 
//      transferring IVP solver state over the network or into a file. Also, 
//      avtIVPSolver instances can be cloned using the the clone() method, and 
//      restarted (resetting basic state but not user-set parameters) for a new
//      IVP solution. This cloning facility allows a scheme to easily pass 
//      itself, together with its parameters, to other code that then uses it.
//
//      Finally, state that is common to all IVP schemes is exposed by means of
//      methods such as getCurrentT(). The IVP right-hand side is made
//      accessible to an IVP solver by means of the avtIVPField class, allowing 
//      the IVP solver to query points of the given vector field. 
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Added OnExitDomain method.
//
// ****************************************************************************

class avtIVPSolver
{
  public:
    enum Result
    {
        OK,
        OUTSIDE_DOMAIN,
        STEPSIZE_UNDERFLOW,
        STIFFNESS_DETECTED,
        UNSPECIFIED_ERROR,
    };
    
    virtual void    Reset(const double& t_start, const avtVecRef& y_start) = 0;

    virtual Result  Step( const avtIVPField* field, 
                         const double& t_max, 
                         avtIVPStep* ivpstep = 0 ) = 0;
    virtual void    OnExitDomain() {}
    virtual avtVec  GetCurrentY() const = 0;
    virtual double  GetCurrentT() const = 0;

    virtual void    SetCurrentY(const avtVec &newY) = 0;
    virtual void    SetCurrentT(double newT) = 0;

    virtual void    SetNextStepSize(const double& h)  = 0;
    virtual double  GetNextStepSize() const = 0;
    virtual void    SetMaximumStepSize(const double& h) = 0;

    virtual void    SetTolerances(const double& reltol, 
                                  const double& abstol) = 0;

    // state management
    virtual void    GetState(avtIVPState&);
    virtual void    PutState(const avtIVPState&);
            
    virtual avtIVPSolver* Clone() const = 0;
            
protected:
    virtual void    AcceptStateVisitor(avtIVPStateHelper& sv) = 0;
};

#endif


