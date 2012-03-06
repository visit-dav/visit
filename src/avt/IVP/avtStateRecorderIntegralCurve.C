/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                      avtStateRecorderIntegralCurve.C                      //
// ************************************************************************* //

#include <avtStateRecorderIntegralCurve.h>

#include <list>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <avtVector.h>
#include <algorithm>

const double avtStateRecorderIntegralCurve::epsilon = 100.0*(std::numeric_limits<float>::epsilon() *
                                                            std::numeric_limits<float>::epsilon());


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve constructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::avtStateRecorderIntegralCurve(
    unsigned char mask,
    const avtIVPSolver* model, 
    Direction dir,
    const double& t_start,
    const avtVector &p_start,
    const avtVector &v_start,
    int ID) :
    avtIntegralCurve(model, dir, t_start, p_start, v_start, ID), historyMask(mask)
{
    distance = 0.0;
    sequenceCnt = 0;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve constructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::avtStateRecorderIntegralCurve()
{
    distance = 0.0;
    sequenceCnt = 0;
    serializeFlags = 0;
    historyMask = 0;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve destructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::~avtStateRecorderIntegralCurve()
{
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::RecordStep
//
//  Purpose:
//      Records a sample from the current step at the specified time.
//
//  Programmer: Christoph Garth
//  Creation:   July 22, 2010
//
//  Modifications:
//
//   Dave Pugmire, Tue Sep 28 10:39:11 EDT 2010
//   If step is with tolerance of previous step, just overwrite the previous step.
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add scalar2
//
// ****************************************************************************

void avtStateRecorderIntegralCurve::RecordStep(const avtIVPField* field,
                                               const avtIVPStep& step,
                                               double t)
{
    avtVector p = step.GetP(t);

    /*
    //If the step is within tolerance of the previous step, just overwrite the last step
    //with this step.
    size_t nSamp = GetNumberOfSamples();
    if (nSamp > 1)
    {
        Sample prevSamp = GetSample(nSamp-1);
        if ((p-prevSamp.position).length2() < epsilon)
        {
            std::vector<float>::iterator m = history.begin() + (nSamp-1)*GetSampleStride();
            history.erase(m, history.end());
        }
    }
    */

    if( historyMask & SAMPLE_TIME )
        history.push_back( t );

    if( historyMask & SAMPLE_POSITION )
    {
        history.push_back( p.x );
        history.push_back( p.y );
        history.push_back( p.z );
    }
        
    if( historyMask & SAMPLE_VELOCITY )
    {
        avtVector v = step.GetV( t );
        history.push_back( v.x );
        history.push_back( v.y );
        history.push_back( v.z );
    }
        
    if( historyMask & SAMPLE_VORTICITY )
        history.push_back( field->ComputeVorticity( t, p ) );
        
    if( historyMask & SAMPLE_ARCLENGTH )
        history.push_back( distance );
        
    if( historyMask & SAMPLE_SCALAR0 )
        history.push_back( field->ComputeScalarVariable( 0, t, p ) );
        
    if( historyMask & SAMPLE_SCALAR1 )
        history.push_back( field->ComputeScalarVariable( 1, t, p ) );

    if( historyMask & SAMPLE_SCALAR2 )
        history.push_back( field->ComputeScalarVariable( 2, t, p ) );
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::AnalyzeStep
//
//  Purpose:
//      Analyzes the current step.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
//  Modifications:
//
//    Hank Childs, Fri Oct  8 23:30:27 PDT 2010
//    Move termination criteria into derived types.
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Pass the avtIVPField to CheckForTermination.
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::AnalyzeStep( avtIVPStep& step, 
                                            avtIVPField* field )
{
    // Record the first position of the step.
    RecordStep( field, step, step.GetT0() );

    // Possibly need this for state we record.
    distance += step.GetLength();

    if (CheckForTermination(step, field))
    {
        status = STATUS_FINISHED;
        RecordStep( field, step, step.GetT1() );
    }
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::GetSampleStride()
//
//  Purpose:
//      Returns the stride between consecutive samples.
//
//  Programmer: Christoph Garth
//  Creation:   July 14, 2010
//
//  Modifications:
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add scalar2
//
// ****************************************************************************

size_t avtStateRecorderIntegralCurve::GetSampleStride() const
{
    size_t stride = 0;

#define TEST_AND_INCREMENT( f, n ) \
    if( historyMask & f )          \
        stride += n;

    TEST_AND_INCREMENT( SAMPLE_TIME, 1 );
    TEST_AND_INCREMENT( SAMPLE_POSITION, 3 );
    TEST_AND_INCREMENT( SAMPLE_VELOCITY, 3 );
    TEST_AND_INCREMENT( SAMPLE_VORTICITY, 1 );
    TEST_AND_INCREMENT( SAMPLE_ARCLENGTH, 1 );
    TEST_AND_INCREMENT( SAMPLE_SCALAR0, 1 );
    TEST_AND_INCREMENT( SAMPLE_SCALAR1, 1 );
    TEST_AND_INCREMENT( SAMPLE_SCALAR2, 1 );

#undef TEST_AND_INCREMENT

    return stride;
};

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::GetSample()
//
//  Purpose:
//      Returns a sample from the streamline.
//
//  Programmer: Christoph Garth
//  Creation:   July 14, 2010
//
//  Modifications:
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add scalar2
//
// ****************************************************************************

avtStateRecorderIntegralCurve::Sample
avtStateRecorderIntegralCurve::GetSample( size_t n ) const
{
    std::vector<float>::const_iterator m = history.begin() + n*GetSampleStride();
    Sample s;

    if( historyMask & SAMPLE_TIME )
        s.time = *(m++);

    if( historyMask & SAMPLE_POSITION )
    {
        s.position.x = *(m++);
        s.position.y = *(m++);
        s.position.z = *(m++);
    }
    
    if( historyMask & SAMPLE_VELOCITY )
    {
        s.velocity.x = *(m++);
        s.velocity.y = *(m++);
        s.velocity.z = *(m++);
    }

    if( historyMask & SAMPLE_VORTICITY )
        s.vorticity = *(m++);

    if( historyMask & SAMPLE_ARCLENGTH )
        s.arclength = *(m++);

    if( historyMask & SAMPLE_SCALAR0 ) 
        s.scalar0 = *(m++);

    if( historyMask & SAMPLE_SCALAR1 )
        s.scalar1 = *(m++);

    if( historyMask & SAMPLE_SCALAR2 )
        s.scalar2 = *(m++);

    return s;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::GetNumberOfSamples()
//
//  Purpose:
//      Returns the number of history samples.
//
//  Programmer: Christoph Garth
//  Creation:   July 14, 2010
//
// ****************************************************************************

size_t avtStateRecorderIntegralCurve::GetNumberOfSamples() const
{
    return history.size() / GetSampleStride();
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::Serialize
//
//  Purpose:
//      Serializes a streamline so it can be sent to another processor.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
//  Modifications:
//
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Add portions for sequence tracking, which were previously in the base
//    class.
//
//   Dave Pugmire, Mon Sep 20 14:51:50 EDT 2010
//   Serialize the distance field.
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::Serialize(MemStream::Mode mode, MemStream &buff, 
                         avtIVPSolver *solver)
{
    // Have the base class serialize its part
    avtIntegralCurve::Serialize(mode, buff, solver);

    buff.io(mode, distance);

    buff.io(mode, historyMask);
    buff.io(mode, serializeFlags);

    bool serializeSteps = serializeFlags&SERIALIZE_STEPS;

    if (DebugStream::Level5())
        debug5<<"  avtStateRecorderIntegralCurve::Serialize "<<(mode==MemStream::READ?"READ":"WRITE")<<" serSteps= "<<serializeSteps<<endl;
    
    // R/W the steps.
    if( serializeSteps )
        buff.io(mode, history);

    if ((serializeFlags & SERIALIZE_INC_SEQ) && mode == MemStream::WRITE)
    {
        long seqCnt = sequenceCnt+1;
        buff.io(mode, seqCnt);
    }
    else
        buff.io(mode, sequenceCnt);

    serializeFlags = 0;

    if (DebugStream::Level5())
        debug5 << "DONE: avtStateRecorderIntegralCurve::Serialize. sz= "<<buff.len() << endl;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::MergeIntegralCurveSequence
//
//  Purpose:
//      Merge a vector of streamline sequences into a single streamline.
//      This is destructive, extra streamlines are deleted.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Sorting can be done independant of streamline direction. Changed streamline
//   step from list to vector.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Move this method from avtStreamlineWrapper.
//
//   David Camp, Fri Jul 29 06:55:39 PDT 2011
//   Added code to send the ending setting, IC status, domain, ivp.
//   The pathlines need this information.
//
// ****************************************************************************

avtIntegralCurve *
avtStateRecorderIntegralCurve::MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v2)
{
    if( v2.empty() )
        return NULL;
    size_t vSize = v2.size();
    if( vSize == 1 )
        return v2[0];

    std::vector<avtStateRecorderIntegralCurve *> v( vSize );
    
    for( size_t i=0 ; i<vSize; ++i )
    {
        v[i] = dynamic_cast<avtStateRecorderIntegralCurve*>( v2[i] );
        assert( v[i] != NULL );
    }

    // sort the streamlines by id and sequence number, in ascending order
    std::sort( v.begin(), v.end(), 
               avtStateRecorderIntegralCurve::IdSeqCompare );

    // find the combined history size
    size_t combinedHistorySize = 0;

    vSize = v.size(); // This should be the same size as v2
    for( size_t i=0; i < vSize; ++i )
    {
        combinedHistorySize += v[i]->history.size();

        // sanity check: make sure all ids are the same
        assert( v[i]->id == v[0]->id );
    }
        
    // now curve pieces are in sorted order and we can simply merge the histories
    // in sequence order; we merge by appending to the first (v[0]'s) history
    v[0]->history.reserve( combinedHistorySize );

    // Need to get the ending setting transfered.
    v[0]->status = v[vSize-1]->status;
    v[0]->domain = v[vSize-1]->domain;
    avtIVPSolver *tmpSolver = v[0]->ivp;
    v[0]->ivp = v[vSize-1]->ivp;
    v[vSize-1]->ivp = tmpSolver;

    for( size_t i=1; i < vSize; i++ )
    {
        v[0]->history.insert( v[0]->history.end(), 
                              v[i]->history.begin(), v[i]->history.end() );

        delete v[i];
    }

    v2.clear();
    return v[0];
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::LessThan
//
//  Purpose:
//      Performs a LessThan operation, used when doing parallel communication
//      and needing to sort curves.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2011
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::LessThan(const avtIntegralCurve *ic) const
{
    return IdSeqCompare(this, ic);
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::IdSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::IdSeqCompare(const avtIntegralCurve *icA,
                                            const avtIntegralCurve *icB)
{
    avtStateRecorderIntegralCurve *sicA = (avtStateRecorderIntegralCurve *) icA;
    avtStateRecorderIntegralCurve *sicB = (avtStateRecorderIntegralCurve *) icB;

    if (sicA->id == sicB->id)
        return sicA->sequenceCnt < sicB->sequenceCnt;

    return sicA->id < sicB->id;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::IdRevSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then reverse sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::IdRevSeqCompare(const avtIntegralCurve *icA,
                                               const avtIntegralCurve *icB)
{
    avtStateRecorderIntegralCurve *sicA = (avtStateRecorderIntegralCurve *) icA;
    avtStateRecorderIntegralCurve *sicB = (avtStateRecorderIntegralCurve *) icB;

    if (sicA->id == sicB->id)
        return sicA->sequenceCnt > sicB->sequenceCnt;

    return sicA->id < sicB->id;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::SameCurve
//
//  Purpose:
//      Checks to see if two curves are the same.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2010
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::SameCurve(avtIntegralCurve *ic)
{
    avtStateRecorderIntegralCurve *sic = (avtStateRecorderIntegralCurve *) ic;
    return (id == sic->id) && (sequenceCnt == sic->sequenceCnt);
}

