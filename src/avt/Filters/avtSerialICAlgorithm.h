/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                            avtSerialICAlgorithm.h                         //
// ************************************************************************* //

#ifndef AVT_SERIAL_IC_ALGORITHM_H
#define AVT_SERIAL_IC_ALGORITHM_H

#include "avtICAlgorithm.h"

// ****************************************************************************
// Class: avtSerialICAlgorithm
//
// Purpose:
//    A class for performing serial streamline integration.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change the names of several methods to reflect the new emphasis in
//   particle advection, as opposed to streamlines.  Also change reference
//   from avtStreamlineFilter to avtPICSFilter.
//
//   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
//   Rename class "IC" from "SL", to reflect the emphasis on integral curves,
//   as opposed to streamlines.
//
// ****************************************************************************

class avtSerialICAlgorithm : public avtICAlgorithm
{
  public:
    avtSerialICAlgorithm( avtPICSFilter *picsFilter );
    virtual ~avtSerialICAlgorithm();
    
    virtual const char*       AlgoName() const {return "Serial";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              ResetIntegralCurvesForContinueExecute();
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);

  protected:
    virtual void              RunAlgorithm();
    std::list<avtIntegralCurve *> activeICs, oobICs;
    
};

#endif
