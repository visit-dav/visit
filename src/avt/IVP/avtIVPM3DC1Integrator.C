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
//                              avtIVPM3DC1Integrator.C                      //
// ************************************************************************* //

#include <avtIVPM3DC1Integrator.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <avtIVPM3DC1Field.h>

#include <limits>
#include <cmath>

static const double epsilon = std::numeric_limits<double>::epsilon();

// helper function
// returns a with the same sign as b
static inline double sign( const double& a, const double& b )
{
    return (b > 0.0) ? std::abs(a) : -std::abs(a);
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator constructor
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

avtIVPM3DC1Integrator::avtIVPM3DC1Integrator()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    d = 0.0;
    numStep = 0;
    degenerate_iterations = 0;
    stiffness_eps = tol / 1000.0;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator destructor
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************


avtIVPM3DC1Integrator::~avtIVPM3DC1Integrator()
{
  cerr << "out of avtIVPM3DC1Integrator" << endl;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetCurrentT
//
//  Purpose:
//      Sets the current T.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::SetCurrentT(double newT)
{
    t = newT;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::GetCurrentT
//
//  Purpose:
//      Gets the current T.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

double 
avtIVPM3DC1Integrator::GetCurrentT() const 
{
    return t;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetCurrentY
//
//  Purpose:
//      Sets the current Y.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::SetCurrentY(const avtVector &newY)
{
    yCur = newY;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::GetCurrentY
//
//  Purpose:
//      Gets the current Y.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
// ****************************************************************************

avtVector 
avtIVPM3DC1Integrator::GetCurrentY() const
{
    return yCur;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetNextStepSize
//
//  Purpose:
//      Sets the step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void 
avtIVPM3DC1Integrator::SetNextStepSize(const double& newH)
{
    h = newH;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::GetNextStepSize
//
//  Purpose:
//      Gets the step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

double 
avtIVPM3DC1Integrator::GetNextStepSize() const
{
    return h;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetMaximumStepSize
//
//  Purpose:
//      Sets the maximum step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::SetMaximumStepSize(const double& maxH)
{
    h_max = maxH;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::GetMaximumStepSize
//
//  Purpose:
//      Gets the maximum step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

double
avtIVPM3DC1Integrator::GetMaximumStepSize() const
{
    return h_max;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetMaximumDegenerateIterations
//
//  Purpose:
//      Sets the maximum number of degenerate steps allowed
//
//  Creationist: Allen Sanderson
//  Creation:   Octobert 24, 2009
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::SetMaximumDegenerateIterations( const unsigned int& max )
{
    max_degenerate_iterations = max;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::SetTolerances
//
//  Purpose:
//      Sets the tolerance.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::SetTolerances(const double& relt, const double& abst)
{
    tol = abst;
    stiffness_eps = tol / 1000.0;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::Reset
//
//  Purpose:
//      Resets data members.  Called by the constructors.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void 
avtIVPM3DC1Integrator::Reset(const double& t_start, const avtVector& y_start)
{
    t = t_start;
    d = 0.0;
    numStep = 0;

    degenerate_iterations = 0;
    yCur = y_start;
    h = h_max;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::OnExitDomain
//
//  Purpose:
//      Post processing tasks after domain exit.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void
avtIVPM3DC1Integrator::OnExitDomain()
{
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Set the velStart/velEnd direction based on integration direction.
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPM3DC1Integrator::Step(const avtIVPField* field,
                            const TerminateType &termType,
                            const double &end,
                            avtIVPStep* ivpstep)
{
    double t_max;

    if (termType == TIME)
        t_max = end;
    else if (termType == DISTANCE || termType == STEPS || termType == INTERSECTIONS)
    {
        t_max = std::numeric_limits<double>::max();
        if (end < 0)
            t_max = -t_max;
    }

    const double direction = sign( 1.0, t_max - t );
    
    h = sign( h, direction );
    
    // do not run past integration end
    if( (t + 1.01*h - t_max) * direction > 0.0 ) 
    {
        h = t_max - t;
    }

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t)*epsilon )
    {
        return avtIVPSolver::STEPSIZE_UNDERFLOW;
    }

    avtIVPSolver::Result res;
    avtVector yNew = yCur;

    // This call begins the M3D code.
    res = vpstep(field, yCur, h, yNew);

    if ( res == avtIVPSolver::OK )
    {
        ivpstep->resize( 2 );

        avtVector yCurCart,  yNewCart;

        yCurCart[0] = yCur[0] * sin(yCur[1]);
        yCurCart[1] = yCur[0] * cos(yCur[1]);
        yCurCart[2] = yCur[2];

        yNewCart[0] = yNew[0] * sin(yNew[1]);
        yNewCart[1] = yNew[0] * cos(yNew[1]);
        yNewCart[2] = yNew[2];
        
        (*ivpstep)[0] = yCurCart;
        (*ivpstep)[1] = yNewCart;
        ivpstep->tStart = t;
        ivpstep->tEnd = t + h;
        numStep++;

        // Handle distanced based termination.
        if (termType == TIME)
        {
            if ((end > 0 && t >= end) ||
                (end < 0 && t <= end))
                return TERMINATE;
        }
        else if (termType == DISTANCE)
        {
            double len = ivpstep->length();
            
            //debug1<<"ABStep: "<<t<<" d: "<<d<<" => "<<(d+len)<<" h= "<<h<<" len= "<<len<<" sEps= "<<stiffness_eps<<endl;
            if (len < stiffness_eps)
            {
                degenerate_iterations++;
                if (degenerate_iterations > 15)
                {
                    //debug1<<"********** STIFFNESS ***************************\n";
                    return STIFFNESS_DETECTED;
                }
            }
            else
                degenerate_iterations = 0;

            if (d+len > fabs(end))
                throw avtIVPField::Undefined();
            else if (d+len >= fabs(end))
                return TERMINATE;

            d = d+len;
        }
        else if (termType == STEPS &&
                 numStep >= (int)fabs(end))
            return TERMINATE;

        if (end > 0.0)
        {
            ivpstep->velStart = getBfield(field, yCur);
            ivpstep->velEnd   = getBfield(field, yNew);
        }
        else
        {
            ivpstep->velStart = -getBfield(field, yCur);
            ivpstep->velEnd   = -getBfield(field, yNew);
        }

        yCur = yNew;
        t = t+h;
    }

    // Reset the step size on sucessful step.
    h = h_max;
    return res;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::vpstep
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************
avtIVPSolver::Result 
avtIVPM3DC1Integrator::vpstep(const avtIVPField* field,
                              avtVector &yCur, double h, avtVector &yNew)
{
  avtIVPSolver::Result res;

  double xin[3], xout[3];

  xin[0] = yCur[0];
  xin[1] = yCur[1];
  xin[2] = yCur[2];

  if (res = partial_step(field,  xin, 0, 0.5*h, xout)) return res;
  if (res = partial_step(field, xout, 1, 0.5*h, xout)) return res;
  if (res = partial_step(field, xout, 2,     h, xout)) return res;
  if (res = partial_step(field, xout, 1, 0.5*h, xout)) return res;
  if (res = partial_step(field, xout, 0, 0.5*h, xout)) return res;

  yNew[0] = xout[0];
  yNew[1] = xout[1];
  yNew[2] = xout[2];

  return avtIVPSolver::OK;
}

// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::partial_step
//
//  Purpose:
//      Take a partial step and return the result.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

#define NEWTACC 5.0e-7

const int flowtable[3][2] = {{1,2},{0,2},{0,1}};


avtIVPSolver::Result 
avtIVPM3DC1Integrator::partial_step(const avtIVPField* field,
                                    double *xin, int iflow, double h, double *xout)
{
  double Bval, dummy;

  xout[0] = xin[0];  xout[1] = xin[1];  xout[2] = xin[2];

  /* Q_i */
  if (advance(field, xout, iflow, 0, 0.5*h, NEWTACC))
    return avtIVPSolver::UNSPECIFIED_ERROR;

  /* P_e */
  if (getBfield(field, xout, iflow, 1, &Bval, 0, &dummy))
    return avtIVPSolver::UNSPECIFIED_ERROR;

  xout[flowtable[iflow][1]] += 0.5*h*Bval;

  /* P_i */
  if (advance(field, xout, iflow, 1, 0.5*h, NEWTACC))
    return avtIVPSolver::UNSPECIFIED_ERROR;

  /* Q_e */
  if (getBfield(field, xout, iflow, 0, &Bval, 0, &dummy))
    return avtIVPSolver::UNSPECIFIED_ERROR;

  xout[flowtable[iflow][0]] += 0.5*h*Bval;

  return avtIVPSolver::OK;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::advance
//
//  Purpose:
//      Advance and return the result.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

int
avtIVPM3DC1Integrator::advance(const avtIVPField* field,
                               double *x, int iflow, int icomp, double h, double xacc)
{
  double Bval, Bprime, xold, dx;
  int it;
  const int ITMAX=15;

  xold = x[flowtable[iflow][icomp]];

  if (getBfield(field, x, iflow, icomp, &Bval, 0, &Bprime)) return 1;
  x[flowtable[iflow][icomp]] += h*Bval;  /* Initial guess */

  /* Newton iteration algorithm from Numerical Recipes */
  for (it=1; it<=ITMAX; it++) {
    if (getBfield(field, x, iflow, icomp, &Bval, 1, &Bprime)) return it+1;
    dx = (x[flowtable[iflow][icomp]] - h*Bval - xold)/(1.0 - h*Bprime);
    x[flowtable[iflow][icomp]] -= dx;
    if (fabs(dx) < xacc*h) return 0;
  } /* end loop it */

  if (fabs(dx) > 1.0e-3*h)
    fprintf(stderr,
            "Newton method failed to converge in %d iterations (dx=%le, h=%le).\n",
            it, dx, h);
  return 0;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::getBfield
//
//  Purpose:
//      Gets the B field components directly
//
//  THIS CODE SHOULD NOT BE USED FOR FIELDLINE INTEGRATION!!!!
//      
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

avtVector
avtIVPM3DC1Integrator::getBfield(const avtIVPField* field, avtVector y)
{
  // FIX THIS CODE - It would be preferable to use a dynamic cast but
  // because the field is passd down as a const it can not be used.
  avtIVPM3DC1Field *m3dField = (avtIVPM3DC1Field *)(field);

  double pt[3];

  pt[0] = y[0];
  pt[1] = y[1];
  pt[2] = y[2];

  double xieta[2];
  int    element;

  /* Find the element containing the point; get local coords xi,eta */
  if ((element = m3dField->get_tri_coords2D(pt, element, xieta)) < 0) 
  {
    return avtVector(0,0,0);
  }
  else 
  {
    float B[3];

    m3dField->interpBcomps(B, pt, element, xieta);
    
    return avtVector(B);
  }
}
   
// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::getBfield
//
//  Purpose:
//      Returns the B field needed for fieldline integration
//      
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

int
avtIVPM3DC1Integrator::getBfield(const avtIVPField* field,
                                 double *x, int iflow, int icomp, double *Bout,
                                 int dflag, double *Bpout)
{
  // FIX THIS CODE - It would be preferable to use a dynamic cast but
  // because the field is passd down as a const it can not be used.
  avtIVPM3DC1Field *m3dField = (avtIVPM3DC1Field *)(field);

  if (m3dField->linflag)
    return getBfield2(field, x, iflow, icomp, Bout, dflag, Bpout);
  else
    return getBfield1(field, x, iflow, icomp, Bout, dflag, Bpout);
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::getBfield1
//
//  Purpose:
//      Axisymmetric equilibrium only
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************
int
avtIVPM3DC1Integrator::getBfield1(const avtIVPField* field,
                                  double *x, int iflow, int icomp, double *Bout,
                                  int dflag, double *Bpout)
{
  double xieta[2];
  int    element;

  // FIX THIS CODE - It would be preferable to use a dynamic cast but
  // because the field is passd down a const it can not be used.
  avtIVPM3DC1Field *m3dField = (avtIVPM3DC1Field *)(field);

  /* Find the element containing the point x; get local coords xi,eta */
  if ((element = m3dField->get_tri_coords2D(x, xieta)) < 0) return 1;

  /* Determine which field component to calculate */
  switch (iflow) {
  case 0: /* B1 = curl A_R */
    if (icomp) { /* z: -df'/dz */
      *Bout = 0.0;
      if (dflag) { /* dB1/dz = -d^2f'/dz^2 */
        *Bpout = 0.0;
      }
    } else { /* phi: d^2f/dz^2 */
      *Bout = m3dField->interpdz2(m3dField->f0, element, xieta);
      if (dflag) { /* dB1/dphi = d^2f'/dz^2 */
        *Bpout = 0.0;
      }
    }
    break;
  case 1: /* B2 = curl A_phi */
    if (icomp) { /* z: (1/R) d psi/dR */
      *Bout = m3dField->interpdR(m3dField->psi0, element, xieta) / x[0];
      if (dflag) { /* dB2/dz = (1/R) d^2 psi/dzdR */
        *Bpout = m3dField->interpdRdz(m3dField->psi0, element, xieta) / x[0];
      }
    } else { /* R: -(1/R) d psi/dz */
      *Bout = -m3dField->interpdz(m3dField->psi0, element, xieta) / x[0];
      if (dflag) { /* dB2/dR = (1/R^2) d psi/dz - (1/R) d^2 psi/dRdz */
        *Bpout = -(m3dField->interpdRdz(m3dField->psi0, element, xieta) + *Bout)/x[0];
      }
    }
    break;
  case 2: /* B3 = curl A_z */
    if (icomp) { /* phi: d^2f/dR^2 + (1/R)df/dR + F0/R^2 */
      *Bout = m3dField->interpdR2(m3dField->f0, element, xieta) +
        (m3dField->interpdR(m3dField->f0, element, xieta) + m3dField->F0/x[0])/ x[0];
      if (dflag) { /* dB3/dphi = d^2f'/dR^2 + (1/R)df'/dR */
        *Bpout = 0.0;
      }
    } else { /* R: -df'/dR */
      *Bout = 0.0;
      if (dflag) { /* dB3/dR = -d^2f'/dR^2 */
        *Bpout = 0.0;
      }
    }
    break;
  default:
    fprintf(stderr, "Unrecognized flow component %d in getBfield.\n", iflow);
    return 2;
  } /* end switch */

  return 0;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::getBfield2
//
//  Purpose:
//      Equilibrium plus mode n perturbation
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************
int
avtIVPM3DC1Integrator::getBfield2(const avtIVPField* field,
                                  double *x, int iflow, int icomp, double *Bout,
                                  int dflag, double *Bpout)
{
  double xieta[2];
  double co, sn, tmp1, tmp2, tmp3, tmp4;
  int    element;

  // FIX THIS CODE - It would be preferable to use a dynamic cast but
  // because the field is passd down a const it can not be used.
  avtIVPM3DC1Field *m3dField = (avtIVPM3DC1Field *)(field);

  /* Find the element containing the point x; get local coords xi,eta */
  if ((element = m3dField->get_tri_coords2D(x, xieta)) < 0) return 1;

  /* Compute sin, cosine */
  co = cos(m3dField->tmode * x[1]);  sn = sin(m3dField->tmode * x[1]);

  /* Determine which field component to calculate */
  switch (iflow) {
  case 0: /* B1 = curl A_R */
    if (icomp) { /* z: -df'/dz */
      *Bout = m3dField->tmode *
        (m3dField->interpdz(m3dField->fnr, element, xieta)*sn +
         m3dField->interpdz(m3dField->fni, element, xieta)*co);
      if (dflag) { /* dB1/dz = -d^2f'/dz^2 */
        *Bpout = m3dField->tmode *
          (m3dField->interpdz2(m3dField->fnr, element, xieta)*sn +
           m3dField->interpdz2(m3dField->fni, element, xieta)*co);
      }
    } else { /* phi: d^2f/dz^2 */
      tmp1 = m3dField->interpdz2(m3dField->fnr, element, xieta);
      tmp2 = m3dField->interpdz2(m3dField->fni, element, xieta);
      *Bout = m3dField->interpdz2(m3dField->f0, element, xieta) + tmp1*co - tmp2*sn;
      if (dflag) { /* dB1/dphi = d^2f'/dz^2 */
        *Bpout = -m3dField->tmode*(tmp1*sn + tmp2*co);
      }
    }
    break;
  case 1: /* B2 = curl A_phi */
    if (icomp) { /* z: (1/R) d psi/dR */
      *Bout = (m3dField->interpdR(m3dField->psi0, element, xieta) +
               m3dField->interpdR(m3dField->psinr, element, xieta)*co -
               m3dField->interpdR(m3dField->psini, element, xieta)*sn)/ x[0];
      if (dflag) { /* dB2/dz = (1/R) d^2 psi/dzdR */
        *Bpout = (m3dField->interpdRdz(m3dField->psi0, element, xieta) +
                  m3dField->interpdRdz(m3dField->psinr, element, xieta)*co -
                  m3dField->interpdRdz(m3dField->psini, element, xieta)*sn)/ x[0];
      }
    } else { /* R: -(1/R) d psi/dz */
      *Bout = -(m3dField->interpdz(m3dField->psi0, element, xieta) +
                m3dField->interpdz(m3dField->psinr, element, xieta)*co -
                m3dField->interpdz(m3dField->psini, element, xieta)*sn)/ x[0];
      if (dflag) { /* dB2/dR = (1/R^2) d psi/dz - (1/R) d^2 psi/dRdz */
        *Bpout = -(m3dField->interpdRdz(m3dField->psi0, element, xieta) +
                   m3dField->interpdRdz(m3dField->psinr, element, xieta)*co -
                   m3dField->interpdRdz(m3dField->psini, element, xieta)*sn +
                   *Bout) / x[0];
      }
    }
    break;
  case 2: /* B3 = curl A_z */
    tmp3 = m3dField->interpdR(m3dField->fnr, element, xieta);
    tmp4 = m3dField->interpdR(m3dField->fni, element, xieta);

    if (icomp) { /* phi: d^2f/dR^2 + (1/R)df/dR + F0/R^2 */
      tmp1 = m3dField->interpdR2(m3dField->fnr, element, xieta);
      tmp2 = m3dField->interpdR2(m3dField->fni, element, xieta);
      *Bout = m3dField->interpdR2(m3dField->f0, element, xieta) + tmp1*co - tmp2*sn +
        (m3dField->interpdR(m3dField->f0, element, xieta) + tmp3*co - tmp4*sn +
         m3dField->F0/x[0])/ x[0];
      if (dflag) { /* dB3/dphi = d^2f'/dR^2 + (1/R)df'/dR */
        *Bpout = -m3dField->tmode *
          (tmp1*sn + tmp2*co + (tmp3*sn + tmp4*co)/x[0]);
      }
    } else { /* R: -df'/dR */
      *Bout = m3dField->tmode*(tmp3*sn + tmp4*co);
      if (dflag) { /* dB3/dR = -d^2f'/dR^2 */
        *Bpout = m3dField->tmode *
          (m3dField->interpdR2(m3dField->fnr, element, xieta)*sn +
           m3dField->interpdR2(m3dField->fni, element, xieta)*co);
      }
    }
    break;
  default:
    fprintf(stderr, "Unrecognized flow component %d in getBfield.\n", iflow);
    return 2;
  } /* end switch */

  return 0;
}


// ****************************************************************************
//  Method: avtIVPM3DC1Integrator::AcceptStateVisitor
//
//  Purpose:
//      Loads the state into the state helper.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
//  Modifications:
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
// ****************************************************************************
void
avtIVPM3DC1Integrator::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    aiss.Accept(numStep)
        .Accept(tol)
        .Accept(degenerate_iterations)
        .Accept(max_degenerate_iterations)
        .Accept(stiffness_eps)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(d)
        .Accept(yCur)
        .Accept(ys[0])
        .Accept(ys[1]);
}
