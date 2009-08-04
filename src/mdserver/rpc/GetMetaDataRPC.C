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

#include <GetMetaDataRPC.h>
#include <GetMetaDataException.h>
#include <DebugStream.h>
#include <string>
using std::string;

// ****************************************************************************
// Method: GetMetaDataRPC::GetMetaDataRPC
//
// Purpose: 
//   Constructor for the GetMetaDataRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:23:52 PST 2003
//   I added timeState.
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added forceReadAllCyclesAndTimes
//
//   Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//   Added ability to force using a specific plugin when reading
//   the metadata from a file (if it causes the file to be opened).
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added support for controlling creation of MeshQuality and TimeDerivative 
//   expressions.
//
//   Cyrus Harrison, Wed Nov 28 11:22:13 PST 2007
//   Added support for controlling creation of vector magnitude expressions
//
// ****************************************************************************

GetMetaDataRPC::GetMetaDataRPC() : BlockingRPC("sibsbbbb",&metaData)
{
    timeState = 0;
    forceReadAllCyclesAndTimes = false;
    forcedFileType="";
    treatAllDBsAsTimeVarying = false;
    createMeshQualityExpressions = true;
    createTimeDerivativeExpressions = true;
    createVectorMagnitudeExpressions = true;
}

// ****************************************************************************
// Method: GetMetaDataRPC::~GetMetaDataRPC
//
// Purpose: 
//   Destructor for the GetMetaDataRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//
// ****************************************************************************

GetMetaDataRPC::~GetMetaDataRPC()
{
}

// ****************************************************************************
// Method: GetMetaDataRPC::TypeName
//
// Purpose: 
//   Returns the name of the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:28:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const std::string
GetMetaDataRPC::TypeName() const
{
    return "GetMetaDataRPC";
}

// ****************************************************************************
// Method: GetMetaDataRPC::operator()
//
// Purpose: 
//   This is the () operator for the GetMetaDataRPC class. This method
//   makes the objects of this class function objects. This method
//   executes the RPC to get the metadata from a file and returns
//   a pointer to the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//    Jeremy Meredith, Fri Nov 17 16:29:43 PST 2000
//    Made output go to log file instead of cout or cerr.
//
//    Brad Whitlock, Thu Feb 7 11:11:27 PDT 2002
//    Added the error message to the new exception.
//
//    Brad Whitlock, Tue May 13 15:24:16 PST 2003
//    I added timeState.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added forceReadAllCyclesAndTimes
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added support for controlling creation of MeshQuality and TimeDerivative 
//    expressions.
//
//   Cyrus Harrison, Wed Nov 28 11:22:13 PST 2007
//   Added support for controlling creation of vector magnitude expressions
//
// ****************************************************************************

const avtDatabaseMetaData *
GetMetaDataRPC::operator()(const string &f, int ts, bool force,
                           const string &fft, bool tv, bool acmq, bool actd,
                           bool acvm)
{
    debug3 << "Executing GetMetaData RPC on file " << f.c_str()
           << ", timestate=" << ts
           << ", forceReadAllCyclesAndTimes = " << force
           << ", forcedFileType = " << fft
           << ", treatAllDBsAsTimeVarying = " << tv
           << ", createMeshQualityExpressions = " << acmq
           << ", createTimeDerivativeExpressions = " << actd
           << ", createVectorMagnitudeExpressions = " << acvm
           << endl;

    SetFile(f);
    SetTimeState(ts);
    SetForceReadAllCyclesAndTimes(force);
    SetForcedFileType(fft);
    SetTreatAllDBsAsTimeVarying(tv);
    SetCreateMeshQualityExpressions(acmq);
    SetCreateTimeDerivativeExpressions(actd);
    SetCreateVectorMagnitudeExpressions(acvm);
    
    // Try to execute the RPC.
    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(GetMetaDataException, GetReply()->Message());
    }

    return &metaData;
}

// ****************************************************************************
// Method: GetMetaDataRPC::SelectAll
//
// Purpose: 
//   This method selects all of the components in the GetMetaDataRPC
//   before the RPC is executed. This is so RPC function parameters
//   would be communicated.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//    Brad Whitlock, Tue May 13 15:25:11 PST 2003
//    I added timeState.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added forceReadAllCyclesAndTimes
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added support for controlling creation of MeshQuality and TimeDerivative 
//    expressions.
//
//    Cyrus Harrison, Wed Nov 28 11:22:13 PST 2007
//    Added support for controlling creation of vector magnitude expressions
//
// ****************************************************************************

void
GetMetaDataRPC::SelectAll()
{
    Select(0, (void*)&file);
    Select(1, (void*)&timeState);
    Select(2, (void*)&forceReadAllCyclesAndTimes);
    Select(3, (void*)&forcedFileType);
    Select(4, (void*)&treatAllDBsAsTimeVarying);
    Select(5, (void*)&createMeshQualityExpressions);
    Select(6, (void*)&createTimeDerivativeExpressions);
    Select(7, (void*)&createVectorMagnitudeExpressions);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetFile
//
// Purpose: 
//   This sets the file name from which to get the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// ****************************************************************************

void
GetMetaDataRPC::SetFile(const std::string &f)
{
    file = f;
    Select(0, (void*)&file);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetTimeState
//
// Purpose: 
//   This sets the timestate for which to get metadata.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:26:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
GetMetaDataRPC::SetTimeState(int ts)
{
    timeState = ts;
    Select(1, (void*)&timeState);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetForceReadAllCyclesAndTimes
//
// Purpose: This sets the bool for whether to read all cycles and times when
//          getting the metadata
//
// Programmer: Mark C. Miller 
// Creation:   Monday, May 16, 2005
//
// ****************************************************************************

void
GetMetaDataRPC::SetForceReadAllCyclesAndTimes(bool force)
{
    forceReadAllCyclesAndTimes = force;
    Select(2, (void*)&forceReadAllCyclesAndTimes);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetForcedFileType
//
// Purpose: 
//   This sets the file type to force using for opening the file.
//
// Programmer: Jeremy Meredith
// Creation:   August  9, 2006
//
// Modifications:
//   
// ****************************************************************************

void
GetMetaDataRPC::SetForcedFileType(const std::string &f)
{
    forcedFileType = f;
    Select(3, (void*)&forcedFileType);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetTreatAllDBsAsTimeVarying
//
// Purpose: This sets the bool for whether to treat all databases as time
//          varying
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007
//
// ****************************************************************************

void
GetMetaDataRPC::SetTreatAllDBsAsTimeVarying(bool set)
{
    treatAllDBsAsTimeVarying = set;
    Select(4, (void*)&treatAllDBsAsTimeVarying);
}


// ****************************************************************************
// Method: GetMetaDataRPC::SetCreateMeshQualityExpressions
//
// Purpose: This sets the bool for whether to automatically create MeshQuality 
//          expressions.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 8, 2007 
//
// ****************************************************************************

void
GetMetaDataRPC::SetCreateMeshQualityExpressions(bool set)
{
    createMeshQualityExpressions = set;
    Select(5, (void*)&createMeshQualityExpressions);
}


// ****************************************************************************
// Method: GetMetaDataRPC::SetCreateTimeDerivativeExpressions
//
// Purpose: This sets the bool for whether to automatically create 
//          TimeDerivative expressions.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 8, 2007 
//
// ****************************************************************************

void
GetMetaDataRPC::SetCreateTimeDerivativeExpressions(bool set)
{
    createTimeDerivativeExpressions = set;
    Select(6, (void*)&createTimeDerivativeExpressions);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetCreateVectorMagnitudeExpressions
//
// Purpose: Sets flag for auto creation of vector magnitude expressions.
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007 
//
// ****************************************************************************

void
GetMetaDataRPC::SetCreateVectorMagnitudeExpressions(bool set)
{
    createVectorMagnitudeExpressions = set;
    Select(7, (void*)&createVectorMagnitudeExpressions);
}



// ****************************************************************************
// Method: GetMetaDataRPC::GetFile
//
// Purpose: 
//   This gets the file name from which to get the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// ****************************************************************************

string
GetMetaDataRPC::GetFile() const
{
    return file;
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetTimeState
//
// Purpose: 
//   This gets the timestate for which to get metadata.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:26:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
GetMetaDataRPC::GetTimeState() const
{
    return timeState;
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetForceReadAllCyclesAndTimes
//
// Purpose: This gets the bool for whether to read all cycles and times when
//          getting metadata
//
// Programmer: Mark C. Miller 
// Creation:   May 16, 2005
//
// ****************************************************************************

bool
GetMetaDataRPC::GetForceReadAllCyclesAndTimes() const
{
    return forceReadAllCyclesAndTimes;
}


// ****************************************************************************
// Method: GetMetaDataRPC::GetForcedFileType
//
// Purpose: 
//   This gets the file type we are forced to use to open the file.
//
// Programmer: Jeremy Meredith
// Creation:   August  9, 2006
//
// Modifications:
//   
// ****************************************************************************

string
GetMetaDataRPC::GetForcedFileType() const
{
    return forcedFileType;
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetTreatAllDBsAsTimeVarying
//
// Purpose: This gets the bool for whether to treat all databases as time
//          varying
//
// Programmer: Mark C. Miller 
// Creation:   June 11, 2007 
//
// ****************************************************************************

bool
GetMetaDataRPC::GetTreatAllDBsAsTimeVarying() const
{
    return treatAllDBsAsTimeVarying;
}


// ****************************************************************************
// Method: GetMetaDataRPC::GetCreateMeshQualityExpressions
//
// Purpose: This gets the bool for whether to automatically create MeshQuality
//          expressions. 
//
// Programmer: Kathleen Bonnell
// Creation:   October 8, 2007 
//
// ****************************************************************************

bool
GetMetaDataRPC::GetCreateMeshQualityExpressions() const
{
    return createMeshQualityExpressions;
}


// ****************************************************************************
// Method: GetMetaDataRPC::GetCreateTimeDerivativeExpressions
//
// Purpose: This gets the bool for whether to automatically create
//          TimeDerivative expressions. 
//
// Programmer: Kathleen Bonnell
// Creation:   October 8, 2007 
//
// ****************************************************************************

bool
GetMetaDataRPC::GetCreateTimeDerivativeExpressions() const
{
    return createTimeDerivativeExpressions;
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetCreateVectorMagnitudeExpressions
//
// Purpose: Gets the flag for auto creating of vector magnitude expressions
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007 
//
// ****************************************************************************

bool
GetMetaDataRPC::GetCreateVectorMagnitudeExpressions() const
{
    return createVectorMagnitudeExpressions;
}
