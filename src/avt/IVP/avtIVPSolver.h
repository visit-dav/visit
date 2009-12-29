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

// ************************************************************************* //
//                               avtIVPSolver.h                              //
// ************************************************************************* //

#ifndef AVT_IVPSOLVER_H
#define AVT_IVPSOLVER_H

#include <avtVector.h>
#include <avtIVPField.h>
#include <avtBezierSegment.h>
#include <MemStream.h>
#include <string>
#include <vector>

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
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Store the velocity with each step.
//
//    Hank Childs, Tue Aug 19 15:34:14 PDT 2008
//    Make sure that velStart and velEnd are appropriately sized.
//    
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion.
//
//    Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//    Added ComputeSpeed, ComputeScalarVariable and associated member data.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Added a copy constructor.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
// ****************************************************************************

class IVP_API avtIVPStep: public avtBezierSegment
{
public:
    avtIVPStep() : avtBezierSegment()
    { tStart = tEnd = scalarValue = 0.0; speed = 0.0; vorticity = 0.0;
      velStart = avtVector(0.,0.,0.); velEnd = avtVector(0.,0.,0.); }

    avtIVPStep(const avtIVPStep &s) : avtBezierSegment(s),
                                      tStart(s.tStart),tEnd(s.tEnd), scalarValue(s.scalarValue),
                                      speed(s.speed), vorticity(s.vorticity),
                                      velStart(s.velStart), velEnd(s.velEnd)
    {
    }
    
    void   ComputeSpeed(const avtIVPField *field)
    {
        speed = velEnd.length();
    }
    void   ComputeScalarVariables(const std::vector<std::string> &s, const avtIVPField *field)
    {
        for (int i = 0; i < s.size(); i++)
            scalarValues.push_back(field->ComputeScalarVariable(s[i], tEnd, lastV()));
    }
                
    void   ComputeVorticity(const avtIVPField *field)
    {
        double tMid = tStart + (tEnd-tStart)/2.0;
        
        avtVector pt = evaluate( tMid );
        if ( field->IsInside( tMid, pt ) )
            vorticity = field->ComputeVorticity( tMid, pt );
    }
    
    virtual void Serialize(MemStream::Mode mode, MemStream &buff)
    {
        //debug1 << "avtIVPStep::Serialize()\n";
        buff.io( mode, tStart );
        buff.io( mode, tEnd );
        buff.io( mode, speed );
        buff.io( mode, vorticity );
        buff.io( mode, scalarValue );
        buff.io( mode, velStart );
        buff.io( mode, velEnd );
        
        avtBezierSegment::Serialize(mode, buff);
        //buff.io( mode, _dim );
        //buff.io( mode, _data );
    }

    double   Length(double eps=1e-6)
    {
        return avtBezierSegment::length( 0.0, 1.0, eps );
    }
    
    double tStart, tEnd;
    avtVector velStart, velEnd;
    double speed, vorticity, scalarValue;
    std::vector<double> scalarValues;
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
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changned how distanced based termination is computed.
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Add operator<< for enums.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

class avtIVPSolver
{
  public:
    enum Result
    {
        OK,
        TERMINATE,
        OUTSIDE_DOMAIN,
        STEPSIZE_UNDERFLOW,
        STIFFNESS_DETECTED,
        UNSPECIFIED_ERROR,
    };
    enum TerminateType
    {
        TIME,
        DISTANCE,
        STEPS,
        INTERSECTIONS
    };
    
    virtual void    Reset(const double& t_start, const avtVector& y_start) = 0;

    virtual Result  Step(const avtIVPField* field,
                         const TerminateType &termType,
                         const double &end,
                         avtIVPStep* ivpstep = 0 ) = 0;

    virtual void    OnExitDomain() {}
    virtual avtVector  GetCurrentY() const = 0;
    virtual double  GetCurrentT() const = 0;

    virtual void    SetCurrentY(const avtVector &newY) = 0;
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


inline std::ostream& operator<<( std::ostream& out, const avtIVPSolver::Result &res )
{
    switch (res)
    {
      case avtIVPSolver::OK:  out<<"OK"; break;
      case avtIVPSolver::TERMINATE: out<<"TERMINATE"; break;
      case avtIVPSolver::OUTSIDE_DOMAIN: out<<"OUTSIDE_DOMAIN"; break;
      case avtIVPSolver::STEPSIZE_UNDERFLOW: out<<"STEPSIZE_UNDERFLOW"; break;
      case avtIVPSolver::STIFFNESS_DETECTED: out<<"STIFFNESS_DETECTED"; break;
      case avtIVPSolver::UNSPECIFIED_ERROR: out<<"UNSPECIFIED_ERROR"; break;
      default:
        out<<"UNKNOWN_RESULT"; break;
    }
    return out;
}

inline std::ostream& operator<<( std::ostream& out, const avtIVPSolver::TerminateType &term )
{
    switch (term)
    {
      case avtIVPSolver::TIME:  out<<"TIME"; break;
      case avtIVPSolver::DISTANCE: out<<"DISTANCE"; break;
      case avtIVPSolver::STEPS: out<<"STEPS"; break;
      case avtIVPSolver::INTERSECTIONS: out<<"INTERSECTIONS"; break;
      default:
        out<<"UNKNOWN_TERMINATION"; break;
    }
    return out;
}

#endif


