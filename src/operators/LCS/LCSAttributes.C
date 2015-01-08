/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <LCSAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for LCSAttributes::SourceType
//

static const char *SourceType_strings[] = {
"NativeMesh", "RegularGrid"};

std::string
LCSAttributes::SourceType_ToString(LCSAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SourceType_strings[index];
}

std::string
LCSAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SourceType_strings[index];
}

bool
LCSAttributes::SourceType_FromString(const std::string &s, LCSAttributes::SourceType &val)
{
    val = LCSAttributes::NativeMesh;
    for(int i = 0; i < 2; ++i)
    {
        if(s == SourceType_strings[i])
        {
            val = (SourceType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::Extents
//

static const char *Extents_strings[] = {
"Full", "Subset"};

std::string
LCSAttributes::Extents_ToString(LCSAttributes::Extents t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return Extents_strings[index];
}

std::string
LCSAttributes::Extents_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return Extents_strings[index];
}

bool
LCSAttributes::Extents_FromString(const std::string &s, LCSAttributes::Extents &val)
{
    val = LCSAttributes::Full;
    for(int i = 0; i < 2; ++i)
    {
        if(s == Extents_strings[i])
        {
            val = (Extents)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::IntegrationDirection
//

static const char *IntegrationDirection_strings[] = {
"Forward", "Backward", "Both"
};

std::string
LCSAttributes::IntegrationDirection_ToString(LCSAttributes::IntegrationDirection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return IntegrationDirection_strings[index];
}

std::string
LCSAttributes::IntegrationDirection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return IntegrationDirection_strings[index];
}

bool
LCSAttributes::IntegrationDirection_FromString(const std::string &s, LCSAttributes::IntegrationDirection &val)
{
    val = LCSAttributes::Forward;
    for(int i = 0; i < 3; ++i)
    {
        if(s == IntegrationDirection_strings[i])
        {
            val = (IntegrationDirection)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::FieldType
//

static const char *FieldType_strings[] = {
"Default", "FlashField", "M3DC12DField", 
"M3DC13DField", "Nek5000Field", "NektarPPField", 
"NIMRODField"};

std::string
LCSAttributes::FieldType_ToString(LCSAttributes::FieldType t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return FieldType_strings[index];
}

std::string
LCSAttributes::FieldType_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return FieldType_strings[index];
}

bool
LCSAttributes::FieldType_FromString(const std::string &s, LCSAttributes::FieldType &val)
{
    val = LCSAttributes::Default;
    for(int i = 0; i < 7; ++i)
    {
        if(s == FieldType_strings[i])
        {
            val = (FieldType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"Euler", "Leapfrog", "DormandPrince", 
"AdamsBashforth", "RK4", "M3DC12DIntegrator"
};

std::string
LCSAttributes::IntegrationType_ToString(LCSAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return IntegrationType_strings[index];
}

std::string
LCSAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
LCSAttributes::IntegrationType_FromString(const std::string &s, LCSAttributes::IntegrationType &val)
{
    val = LCSAttributes::Euler;
    for(int i = 0; i < 6; ++i)
    {
        if(s == IntegrationType_strings[i])
        {
            val = (IntegrationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::SizeType
//

static const char *SizeType_strings[] = {
"Absolute", "FractionOfBBox"};

std::string
LCSAttributes::SizeType_ToString(LCSAttributes::SizeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SizeType_strings[index];
}

std::string
LCSAttributes::SizeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SizeType_strings[index];
}

bool
LCSAttributes::SizeType_FromString(const std::string &s, LCSAttributes::SizeType &val)
{
    val = LCSAttributes::Absolute;
    for(int i = 0; i < 2; ++i)
    {
        if(s == SizeType_strings[i])
        {
            val = (SizeType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::ParallelizationAlgorithmType
//

static const char *ParallelizationAlgorithmType_strings[] = {
"LoadOnDemand", "ParallelStaticDomains", "MasterSlave", 
"VisItSelects"};

std::string
LCSAttributes::ParallelizationAlgorithmType_ToString(LCSAttributes::ParallelizationAlgorithmType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return ParallelizationAlgorithmType_strings[index];
}

std::string
LCSAttributes::ParallelizationAlgorithmType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return ParallelizationAlgorithmType_strings[index];
}

bool
LCSAttributes::ParallelizationAlgorithmType_FromString(const std::string &s, LCSAttributes::ParallelizationAlgorithmType &val)
{
    val = LCSAttributes::LoadOnDemand;
    for(int i = 0; i < 4; ++i)
    {
        if(s == ParallelizationAlgorithmType_strings[i])
        {
            val = (ParallelizationAlgorithmType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::OperationType
//

static const char *OperationType_strings[] = {
"Lyapunov", "IntegrationTime", "ArcLength", 
"AverageDistanceFromSeed"};

std::string
LCSAttributes::OperationType_ToString(LCSAttributes::OperationType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return OperationType_strings[index];
}

std::string
LCSAttributes::OperationType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return OperationType_strings[index];
}

bool
LCSAttributes::OperationType_FromString(const std::string &s, LCSAttributes::OperationType &val)
{
    val = LCSAttributes::Lyapunov;
    for(int i = 0; i < 4; ++i)
    {
        if(s == OperationType_strings[i])
        {
            val = (OperationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::OperatorType
//

static const char *OperatorType_strings[] = {
"BaseValue", "Gradient"};

std::string
LCSAttributes::OperatorType_ToString(LCSAttributes::OperatorType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OperatorType_strings[index];
}

std::string
LCSAttributes::OperatorType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OperatorType_strings[index];
}

bool
LCSAttributes::OperatorType_FromString(const std::string &s, LCSAttributes::OperatorType &val)
{
    val = LCSAttributes::BaseValue;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OperatorType_strings[i])
        {
            val = (OperatorType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::TerminationType
//

static const char *TerminationType_strings[] = {
"Time", "Distance", "Size"
};

std::string
LCSAttributes::TerminationType_ToString(LCSAttributes::TerminationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TerminationType_strings[index];
}

std::string
LCSAttributes::TerminationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TerminationType_strings[index];
}

bool
LCSAttributes::TerminationType_FromString(const std::string &s, LCSAttributes::TerminationType &val)
{
    val = LCSAttributes::Time;
    for(int i = 0; i < 3; ++i)
    {
        if(s == TerminationType_strings[i])
        {
            val = (TerminationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LCSAttributes::PathlinesCMFE
//

static const char *PathlinesCMFE_strings[] = {
"CONN_CMFE", "POS_CMFE"};

std::string
LCSAttributes::PathlinesCMFE_ToString(LCSAttributes::PathlinesCMFE t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return PathlinesCMFE_strings[index];
}

std::string
LCSAttributes::PathlinesCMFE_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return PathlinesCMFE_strings[index];
}

bool
LCSAttributes::PathlinesCMFE_FromString(const std::string &s, LCSAttributes::PathlinesCMFE &val)
{
    val = LCSAttributes::CONN_CMFE;
    for(int i = 0; i < 2; ++i)
    {
        if(s == PathlinesCMFE_strings[i])
        {
            val = (PathlinesCMFE)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Init utility for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LCSAttributes::Init()
{
    sourceType = NativeMesh;
    Resolution[0] = 10;
    Resolution[1] = 10;
    Resolution[2] = 10;
    UseDataSetStart = Full;
    StartPosition[0] = 0;
    StartPosition[1] = 0;
    StartPosition[2] = 0;
    UseDataSetEnd = Full;
    EndPosition[0] = 1;
    EndPosition[1] = 1;
    EndPosition[2] = 1;
    integrationDirection = Forward;
    maxSteps = 1000;
    operationType = Lyapunov;
    operatorType = BaseValue;
    terminationType = Time;
    terminateBySize = false;
    termSize = 10;
    terminateByDistance = false;
    termDistance = 10;
    terminateByTime = false;
    termTime = 10;
    maxStepLength = 0.1;
    limitMaximumTimestep = false;
    maxTimeStep = 0.1;
    relTol = 0.0001;
    absTolSizeType = FractionOfBBox;
    absTolAbsolute = 1e-06;
    absTolBBox = 1e-06;
    fieldType = Default;
    fieldConstant = 1;
    velocitySource[0] = 0;
    velocitySource[1] = 0;
    velocitySource[2] = 0;
    integrationType = DormandPrince;
    clampLogValues = true;
    parallelizationAlgorithmType = VisItSelects;
    maxProcessCount = 10;
    maxDomainCacheSize = 3;
    workGroupSize = 32;
    pathlines = false;
    pathlinesOverrideStartingTimeFlag = false;
    pathlinesOverrideStartingTime = 0;
    pathlinesPeriod = 0;
    pathlinesCMFE = POS_CMFE;
    forceNodeCenteredData = false;
    issueTerminationWarnings = true;
    issueStiffnessWarnings = true;
    issueCriticalPointsWarnings = true;
    criticalPointThreshold = 0.001;

    LCSAttributes::SelectAll();
}

// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Copy utility for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LCSAttributes::Copy(const LCSAttributes &obj)
{
    sourceType = obj.sourceType;
    Resolution[0] = obj.Resolution[0];
    Resolution[1] = obj.Resolution[1];
    Resolution[2] = obj.Resolution[2];

    UseDataSetStart = obj.UseDataSetStart;
    StartPosition[0] = obj.StartPosition[0];
    StartPosition[1] = obj.StartPosition[1];
    StartPosition[2] = obj.StartPosition[2];

    UseDataSetEnd = obj.UseDataSetEnd;
    EndPosition[0] = obj.EndPosition[0];
    EndPosition[1] = obj.EndPosition[1];
    EndPosition[2] = obj.EndPosition[2];

    integrationDirection = obj.integrationDirection;
    maxSteps = obj.maxSteps;
    operationType = obj.operationType;
    operatorType = obj.operatorType;
    terminationType = obj.terminationType;
    terminateBySize = obj.terminateBySize;
    termSize = obj.termSize;
    terminateByDistance = obj.terminateByDistance;
    termDistance = obj.termDistance;
    terminateByTime = obj.terminateByTime;
    termTime = obj.termTime;
    maxStepLength = obj.maxStepLength;
    limitMaximumTimestep = obj.limitMaximumTimestep;
    maxTimeStep = obj.maxTimeStep;
    relTol = obj.relTol;
    absTolSizeType = obj.absTolSizeType;
    absTolAbsolute = obj.absTolAbsolute;
    absTolBBox = obj.absTolBBox;
    fieldType = obj.fieldType;
    fieldConstant = obj.fieldConstant;
    velocitySource[0] = obj.velocitySource[0];
    velocitySource[1] = obj.velocitySource[1];
    velocitySource[2] = obj.velocitySource[2];

    integrationType = obj.integrationType;
    clampLogValues = obj.clampLogValues;
    parallelizationAlgorithmType = obj.parallelizationAlgorithmType;
    maxProcessCount = obj.maxProcessCount;
    maxDomainCacheSize = obj.maxDomainCacheSize;
    workGroupSize = obj.workGroupSize;
    pathlines = obj.pathlines;
    pathlinesOverrideStartingTimeFlag = obj.pathlinesOverrideStartingTimeFlag;
    pathlinesOverrideStartingTime = obj.pathlinesOverrideStartingTime;
    pathlinesPeriod = obj.pathlinesPeriod;
    pathlinesCMFE = obj.pathlinesCMFE;
    forceNodeCenteredData = obj.forceNodeCenteredData;
    issueTerminationWarnings = obj.issueTerminationWarnings;
    issueStiffnessWarnings = obj.issueStiffnessWarnings;
    issueCriticalPointsWarnings = obj.issueCriticalPointsWarnings;
    criticalPointThreshold = obj.criticalPointThreshold;

    LCSAttributes::SelectAll();
}

// Type map format string
const char *LCSAttributes::TypeMapFormatString = LCSATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t LCSAttributes::TmfsStruct = {LCSATTRIBUTES_TMFS};


// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Default constructor for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes::LCSAttributes() : 
    AttributeSubject(LCSAttributes::TypeMapFormatString)
{
    LCSAttributes::Init();
}

// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Constructor for the derived classes of LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes::LCSAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LCSAttributes::Init();
}

// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Copy constructor for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes::LCSAttributes(const LCSAttributes &obj) : 
    AttributeSubject(LCSAttributes::TypeMapFormatString)
{
    LCSAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LCSAttributes::LCSAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes::LCSAttributes(const LCSAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LCSAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LCSAttributes::~LCSAttributes
//
// Purpose: 
//   Destructor for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes::~LCSAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: LCSAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LCSAttributes& 
LCSAttributes::operator = (const LCSAttributes &obj)
{
    if (this == &obj) return *this;

    LCSAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: LCSAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LCSAttributes::operator == (const LCSAttributes &obj) const
{
    // Compare the Resolution arrays.
    bool Resolution_equal = true;
    for(int i = 0; i < 3 && Resolution_equal; ++i)
        Resolution_equal = (Resolution[i] == obj.Resolution[i]);

    // Compare the StartPosition arrays.
    bool StartPosition_equal = true;
    for(int i = 0; i < 3 && StartPosition_equal; ++i)
        StartPosition_equal = (StartPosition[i] == obj.StartPosition[i]);

    // Compare the EndPosition arrays.
    bool EndPosition_equal = true;
    for(int i = 0; i < 3 && EndPosition_equal; ++i)
        EndPosition_equal = (EndPosition[i] == obj.EndPosition[i]);

    // Compare the velocitySource arrays.
    bool velocitySource_equal = true;
    for(int i = 0; i < 3 && velocitySource_equal; ++i)
        velocitySource_equal = (velocitySource[i] == obj.velocitySource[i]);

    // Create the return value
    return ((sourceType == obj.sourceType) &&
            Resolution_equal &&
            (UseDataSetStart == obj.UseDataSetStart) &&
            StartPosition_equal &&
            (UseDataSetEnd == obj.UseDataSetEnd) &&
            EndPosition_equal &&
            (integrationDirection == obj.integrationDirection) &&
            (maxSteps == obj.maxSteps) &&
            (operationType == obj.operationType) &&
            (operatorType == obj.operatorType) &&
            (terminationType == obj.terminationType) &&
            (terminateBySize == obj.terminateBySize) &&
            (termSize == obj.termSize) &&
            (terminateByDistance == obj.terminateByDistance) &&
            (termDistance == obj.termDistance) &&
            (terminateByTime == obj.terminateByTime) &&
            (termTime == obj.termTime) &&
            (maxStepLength == obj.maxStepLength) &&
            (limitMaximumTimestep == obj.limitMaximumTimestep) &&
            (maxTimeStep == obj.maxTimeStep) &&
            (relTol == obj.relTol) &&
            (absTolSizeType == obj.absTolSizeType) &&
            (absTolAbsolute == obj.absTolAbsolute) &&
            (absTolBBox == obj.absTolBBox) &&
            (fieldType == obj.fieldType) &&
            (fieldConstant == obj.fieldConstant) &&
            velocitySource_equal &&
            (integrationType == obj.integrationType) &&
            (clampLogValues == obj.clampLogValues) &&
            (parallelizationAlgorithmType == obj.parallelizationAlgorithmType) &&
            (maxProcessCount == obj.maxProcessCount) &&
            (maxDomainCacheSize == obj.maxDomainCacheSize) &&
            (workGroupSize == obj.workGroupSize) &&
            (pathlines == obj.pathlines) &&
            (pathlinesOverrideStartingTimeFlag == obj.pathlinesOverrideStartingTimeFlag) &&
            (pathlinesOverrideStartingTime == obj.pathlinesOverrideStartingTime) &&
            (pathlinesPeriod == obj.pathlinesPeriod) &&
            (pathlinesCMFE == obj.pathlinesCMFE) &&
            (forceNodeCenteredData == obj.forceNodeCenteredData) &&
            (issueTerminationWarnings == obj.issueTerminationWarnings) &&
            (issueStiffnessWarnings == obj.issueStiffnessWarnings) &&
            (issueCriticalPointsWarnings == obj.issueCriticalPointsWarnings) &&
            (criticalPointThreshold == obj.criticalPointThreshold));
}

// ****************************************************************************
// Method: LCSAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LCSAttributes::operator != (const LCSAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: LCSAttributes::TypeName
//
// Purpose: 
//   Type name method for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const std::string
LCSAttributes::TypeName() const
{
    return "LCSAttributes";
}

// ****************************************************************************
// Method: LCSAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the LCSAttributes class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 4 15:22:57 PST 2002
//
// Modifications:
//    Brad Whitlock, Wed Dec 22 12:57:53 PDT 2004
//    I added code to support the point tool.
//
//    Hank Childs, Sat Mar  3 09:00:12 PST 2007
//    Disable useWholeBox if we are copying box extents.
//
//    Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//    New seed sources.
//
// ****************************************************************************

bool
LCSAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const LCSAttributes *tmp = (const LCSAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    return retval;
}

// ****************************************************************************
//  Method: LCSAttributes::CreateCompatible
//
//  Purpose:
//     Creates a new state object of the desired type.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Tue Jan 21 12:33:04 PDT 2003
//    I added code to set the "have radius" flag to true so the plane tool
//    resizes properly when resizing the plane radius.
//
//    Brad Whitlock, Wed Dec 22 12:54:43 PDT 2004
//    I added code to support the point tool.
//
// ****************************************************************************

AttributeSubject *
LCSAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new LCSAttributes(*this);
    }

    return retval;
}

// ****************************************************************************
// Method: LCSAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the LCSAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LCSAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new LCSAttributes(*this);
    else
        retval = new LCSAttributes;

    return retval;
}

// ****************************************************************************
// Method: LCSAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
LCSAttributes::SelectAll()
{
    Select(ID_sourceType,                        (void *)&sourceType);
    Select(ID_Resolution,                        (void *)Resolution, 3);
    Select(ID_UseDataSetStart,                   (void *)&UseDataSetStart);
    Select(ID_StartPosition,                     (void *)StartPosition, 3);
    Select(ID_UseDataSetEnd,                     (void *)&UseDataSetEnd);
    Select(ID_EndPosition,                       (void *)EndPosition, 3);
    Select(ID_integrationDirection,              (void *)&integrationDirection);
    Select(ID_maxSteps,                          (void *)&maxSteps);
    Select(ID_operationType,                     (void *)&operationType);
    Select(ID_operatorType,                      (void *)&operatorType);
    Select(ID_terminationType,                   (void *)&terminationType);
    Select(ID_terminateBySize,                   (void *)&terminateBySize);
    Select(ID_termSize,                          (void *)&termSize);
    Select(ID_terminateByDistance,               (void *)&terminateByDistance);
    Select(ID_termDistance,                      (void *)&termDistance);
    Select(ID_terminateByTime,                   (void *)&terminateByTime);
    Select(ID_termTime,                          (void *)&termTime);
    Select(ID_maxStepLength,                     (void *)&maxStepLength);
    Select(ID_limitMaximumTimestep,              (void *)&limitMaximumTimestep);
    Select(ID_maxTimeStep,                       (void *)&maxTimeStep);
    Select(ID_relTol,                            (void *)&relTol);
    Select(ID_absTolSizeType,                    (void *)&absTolSizeType);
    Select(ID_absTolAbsolute,                    (void *)&absTolAbsolute);
    Select(ID_absTolBBox,                        (void *)&absTolBBox);
    Select(ID_fieldType,                         (void *)&fieldType);
    Select(ID_fieldConstant,                     (void *)&fieldConstant);
    Select(ID_velocitySource,                    (void *)velocitySource, 3);
    Select(ID_integrationType,                   (void *)&integrationType);
    Select(ID_clampLogValues,                    (void *)&clampLogValues);
    Select(ID_parallelizationAlgorithmType,      (void *)&parallelizationAlgorithmType);
    Select(ID_maxProcessCount,                   (void *)&maxProcessCount);
    Select(ID_maxDomainCacheSize,                (void *)&maxDomainCacheSize);
    Select(ID_workGroupSize,                     (void *)&workGroupSize);
    Select(ID_pathlines,                         (void *)&pathlines);
    Select(ID_pathlinesOverrideStartingTimeFlag, (void *)&pathlinesOverrideStartingTimeFlag);
    Select(ID_pathlinesOverrideStartingTime,     (void *)&pathlinesOverrideStartingTime);
    Select(ID_pathlinesPeriod,                   (void *)&pathlinesPeriod);
    Select(ID_pathlinesCMFE,                     (void *)&pathlinesCMFE);
    Select(ID_forceNodeCenteredData,             (void *)&forceNodeCenteredData);
    Select(ID_issueTerminationWarnings,          (void *)&issueTerminationWarnings);
    Select(ID_issueStiffnessWarnings,            (void *)&issueStiffnessWarnings);
    Select(ID_issueCriticalPointsWarnings,       (void *)&issueCriticalPointsWarnings);
    Select(ID_criticalPointThreshold,            (void *)&criticalPointThreshold);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LCSAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LCSAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    LCSAttributes defaultObject;
    bool addToParent = false;
    // Create a node for LCSAttributes.
    DataNode *node = new DataNode("LCSAttributes");

    if(completeSave || !FieldsEqual(ID_sourceType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sourceType", SourceType_ToString(sourceType)));
    }

    if(completeSave || !FieldsEqual(ID_Resolution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("Resolution", Resolution, 3));
    }

    if(completeSave || !FieldsEqual(ID_UseDataSetStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("UseDataSetStart", Extents_ToString(UseDataSetStart)));
    }

    if(completeSave || !FieldsEqual(ID_StartPosition, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("StartPosition", StartPosition, 3));
    }

    if(completeSave || !FieldsEqual(ID_UseDataSetEnd, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("UseDataSetEnd", Extents_ToString(UseDataSetEnd)));
    }

    if(completeSave || !FieldsEqual(ID_EndPosition, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("EndPosition", EndPosition, 3));
    }

    if(completeSave || !FieldsEqual(ID_integrationDirection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationDirection", IntegrationDirection_ToString(integrationDirection)));
    }

    if(completeSave || !FieldsEqual(ID_maxSteps, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxSteps", maxSteps));
    }

    if(completeSave || !FieldsEqual(ID_operationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("operationType", OperationType_ToString(operationType)));
    }

    if(completeSave || !FieldsEqual(ID_operatorType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("operatorType", OperatorType_ToString(operatorType)));
    }

    if(completeSave || !FieldsEqual(ID_terminationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminationType", TerminationType_ToString(terminationType)));
    }

    if(completeSave || !FieldsEqual(ID_terminateBySize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminateBySize", terminateBySize));
    }

    if(completeSave || !FieldsEqual(ID_termSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termSize", termSize));
    }

    if(completeSave || !FieldsEqual(ID_terminateByDistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminateByDistance", terminateByDistance));
    }

    if(completeSave || !FieldsEqual(ID_termDistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termDistance", termDistance));
    }

    if(completeSave || !FieldsEqual(ID_terminateByTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminateByTime", terminateByTime));
    }

    if(completeSave || !FieldsEqual(ID_termTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termTime", termTime));
    }

    if(completeSave || !FieldsEqual(ID_maxStepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStepLength", maxStepLength));
    }

    if(completeSave || !FieldsEqual(ID_limitMaximumTimestep, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("limitMaximumTimestep", limitMaximumTimestep));
    }

    if(completeSave || !FieldsEqual(ID_maxTimeStep, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxTimeStep", maxTimeStep));
    }

    if(completeSave || !FieldsEqual(ID_relTol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("relTol", relTol));
    }

    if(completeSave || !FieldsEqual(ID_absTolSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolSizeType", SizeType_ToString(absTolSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_absTolAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolAbsolute", absTolAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_absTolBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolBBox", absTolBBox));
    }

    if(completeSave || !FieldsEqual(ID_fieldType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fieldType", FieldType_ToString(fieldType)));
    }

    if(completeSave || !FieldsEqual(ID_fieldConstant, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fieldConstant", fieldConstant));
    }

    if(completeSave || !FieldsEqual(ID_velocitySource, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("velocitySource", velocitySource, 3));
    }

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_clampLogValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("clampLogValues", clampLogValues));
    }

    if(completeSave || !FieldsEqual(ID_parallelizationAlgorithmType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("parallelizationAlgorithmType", ParallelizationAlgorithmType_ToString(parallelizationAlgorithmType)));
    }

    if(completeSave || !FieldsEqual(ID_maxProcessCount, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxProcessCount", maxProcessCount));
    }

    if(completeSave || !FieldsEqual(ID_maxDomainCacheSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxDomainCacheSize", maxDomainCacheSize));
    }

    if(completeSave || !FieldsEqual(ID_workGroupSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("workGroupSize", workGroupSize));
    }

    if(completeSave || !FieldsEqual(ID_pathlines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlines", pathlines));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesOverrideStartingTimeFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesOverrideStartingTimeFlag", pathlinesOverrideStartingTimeFlag));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesOverrideStartingTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesOverrideStartingTime", pathlinesOverrideStartingTime));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesPeriod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesPeriod", pathlinesPeriod));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesCMFE, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesCMFE", PathlinesCMFE_ToString(pathlinesCMFE)));
    }

    if(completeSave || !FieldsEqual(ID_forceNodeCenteredData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("forceNodeCenteredData", forceNodeCenteredData));
    }

    if(completeSave || !FieldsEqual(ID_issueTerminationWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueTerminationWarnings", issueTerminationWarnings));
    }

    if(completeSave || !FieldsEqual(ID_issueStiffnessWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueStiffnessWarnings", issueStiffnessWarnings));
    }

    if(completeSave || !FieldsEqual(ID_issueCriticalPointsWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueCriticalPointsWarnings", issueCriticalPointsWarnings));
    }

    if(completeSave || !FieldsEqual(ID_criticalPointThreshold, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("criticalPointThreshold", criticalPointThreshold));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: LCSAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
LCSAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("LCSAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
    if((node = searchNode->GetNode("Resolution")) != 0)
        SetResolution(node->AsIntArray());
    if((node = searchNode->GetNode("UseDataSetStart")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetUseDataSetStart(Extents(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Extents value;
            if(Extents_FromString(node->AsString(), value))
                SetUseDataSetStart(value);
        }
    }
    if((node = searchNode->GetNode("StartPosition")) != 0)
        SetStartPosition(node->AsDoubleArray());
    if((node = searchNode->GetNode("UseDataSetEnd")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetUseDataSetEnd(Extents(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Extents value;
            if(Extents_FromString(node->AsString(), value))
                SetUseDataSetEnd(value);
        }
    }
    if((node = searchNode->GetNode("EndPosition")) != 0)
        SetEndPosition(node->AsDoubleArray());
    if((node = searchNode->GetNode("integrationDirection")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetIntegrationDirection(IntegrationDirection(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationDirection value;
            if(IntegrationDirection_FromString(node->AsString(), value))
                SetIntegrationDirection(value);
        }
    }
    if((node = searchNode->GetNode("maxSteps")) != 0)
        SetMaxSteps(node->AsInt());
    if((node = searchNode->GetNode("operationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetOperationType(OperationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OperationType value;
            if(OperationType_FromString(node->AsString(), value))
                SetOperationType(value);
        }
    }
    if((node = searchNode->GetNode("operatorType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOperatorType(OperatorType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OperatorType value;
            if(OperatorType_FromString(node->AsString(), value))
                SetOperatorType(value);
        }
    }
    if((node = searchNode->GetNode("terminationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetTerminationType(TerminationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TerminationType value;
            if(TerminationType_FromString(node->AsString(), value))
                SetTerminationType(value);
        }
    }
    if((node = searchNode->GetNode("terminateBySize")) != 0)
        SetTerminateBySize(node->AsBool());
    if((node = searchNode->GetNode("termSize")) != 0)
        SetTermSize(node->AsDouble());
    if((node = searchNode->GetNode("terminateByDistance")) != 0)
        SetTerminateByDistance(node->AsBool());
    if((node = searchNode->GetNode("termDistance")) != 0)
        SetTermDistance(node->AsDouble());
    if((node = searchNode->GetNode("terminateByTime")) != 0)
        SetTerminateByTime(node->AsBool());
    if((node = searchNode->GetNode("termTime")) != 0)
        SetTermTime(node->AsDouble());
    if((node = searchNode->GetNode("maxStepLength")) != 0)
        SetMaxStepLength(node->AsDouble());
    if((node = searchNode->GetNode("limitMaximumTimestep")) != 0)
        SetLimitMaximumTimestep(node->AsBool());
    if((node = searchNode->GetNode("maxTimeStep")) != 0)
        SetMaxTimeStep(node->AsDouble());
    if((node = searchNode->GetNode("relTol")) != 0)
        SetRelTol(node->AsDouble());
    if((node = searchNode->GetNode("absTolSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetAbsTolSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetAbsTolSizeType(value);
        }
    }
    if((node = searchNode->GetNode("absTolAbsolute")) != 0)
        SetAbsTolAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("absTolBBox")) != 0)
        SetAbsTolBBox(node->AsDouble());
    if((node = searchNode->GetNode("fieldType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 7)
                SetFieldType(FieldType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            FieldType value;
            if(FieldType_FromString(node->AsString(), value))
                SetFieldType(value);
        }
    }
    if((node = searchNode->GetNode("fieldConstant")) != 0)
        SetFieldConstant(node->AsDouble());
    if((node = searchNode->GetNode("velocitySource")) != 0)
        SetVelocitySource(node->AsDoubleArray());
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 6)
                SetIntegrationType(IntegrationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationType value;
            if(IntegrationType_FromString(node->AsString(), value))
                SetIntegrationType(value);
        }
    }
    if((node = searchNode->GetNode("clampLogValues")) != 0)
        SetClampLogValues(node->AsBool());
    if((node = searchNode->GetNode("parallelizationAlgorithmType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetParallelizationAlgorithmType(ParallelizationAlgorithmType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ParallelizationAlgorithmType value;
            if(ParallelizationAlgorithmType_FromString(node->AsString(), value))
                SetParallelizationAlgorithmType(value);
        }
    }
    if((node = searchNode->GetNode("maxProcessCount")) != 0)
        SetMaxProcessCount(node->AsInt());
    if((node = searchNode->GetNode("maxDomainCacheSize")) != 0)
        SetMaxDomainCacheSize(node->AsInt());
    if((node = searchNode->GetNode("workGroupSize")) != 0)
        SetWorkGroupSize(node->AsInt());
    if((node = searchNode->GetNode("pathlines")) != 0)
        SetPathlines(node->AsBool());
    if((node = searchNode->GetNode("pathlinesOverrideStartingTimeFlag")) != 0)
        SetPathlinesOverrideStartingTimeFlag(node->AsBool());
    if((node = searchNode->GetNode("pathlinesOverrideStartingTime")) != 0)
        SetPathlinesOverrideStartingTime(node->AsDouble());
    if((node = searchNode->GetNode("pathlinesPeriod")) != 0)
        SetPathlinesPeriod(node->AsDouble());
    if((node = searchNode->GetNode("pathlinesCMFE")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetPathlinesCMFE(PathlinesCMFE(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PathlinesCMFE value;
            if(PathlinesCMFE_FromString(node->AsString(), value))
                SetPathlinesCMFE(value);
        }
    }
    if((node = searchNode->GetNode("forceNodeCenteredData")) != 0)
        SetForceNodeCenteredData(node->AsBool());
    if((node = searchNode->GetNode("issueTerminationWarnings")) != 0)
        SetIssueTerminationWarnings(node->AsBool());
    if((node = searchNode->GetNode("issueStiffnessWarnings")) != 0)
        SetIssueStiffnessWarnings(node->AsBool());
    if((node = searchNode->GetNode("issueCriticalPointsWarnings")) != 0)
        SetIssueCriticalPointsWarnings(node->AsBool());
    if((node = searchNode->GetNode("criticalPointThreshold")) != 0)
        SetCriticalPointThreshold(node->AsDouble());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LCSAttributes::SetSourceType(LCSAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
LCSAttributes::SetResolution(const int *Resolution_)
{
    Resolution[0] = Resolution_[0];
    Resolution[1] = Resolution_[1];
    Resolution[2] = Resolution_[2];
    Select(ID_Resolution, (void *)Resolution, 3);
}

void
LCSAttributes::SetUseDataSetStart(LCSAttributes::Extents UseDataSetStart_)
{
    UseDataSetStart = UseDataSetStart_;
    Select(ID_UseDataSetStart, (void *)&UseDataSetStart);
}

void
LCSAttributes::SetStartPosition(const double *StartPosition_)
{
    StartPosition[0] = StartPosition_[0];
    StartPosition[1] = StartPosition_[1];
    StartPosition[2] = StartPosition_[2];
    Select(ID_StartPosition, (void *)StartPosition, 3);
}

void
LCSAttributes::SetUseDataSetEnd(LCSAttributes::Extents UseDataSetEnd_)
{
    UseDataSetEnd = UseDataSetEnd_;
    Select(ID_UseDataSetEnd, (void *)&UseDataSetEnd);
}

void
LCSAttributes::SetEndPosition(const double *EndPosition_)
{
    EndPosition[0] = EndPosition_[0];
    EndPosition[1] = EndPosition_[1];
    EndPosition[2] = EndPosition_[2];
    Select(ID_EndPosition, (void *)EndPosition, 3);
}

void
LCSAttributes::SetIntegrationDirection(LCSAttributes::IntegrationDirection integrationDirection_)
{
    integrationDirection = integrationDirection_;
    Select(ID_integrationDirection, (void *)&integrationDirection);
}

void
LCSAttributes::SetMaxSteps(int maxSteps_)
{
    maxSteps = maxSteps_;
    Select(ID_maxSteps, (void *)&maxSteps);
}

void
LCSAttributes::SetOperationType(LCSAttributes::OperationType operationType_)
{
    operationType = operationType_;
    Select(ID_operationType, (void *)&operationType);
}

void
LCSAttributes::SetOperatorType(LCSAttributes::OperatorType operatorType_)
{
    operatorType = operatorType_;
    Select(ID_operatorType, (void *)&operatorType);
}

void
LCSAttributes::SetTerminationType(LCSAttributes::TerminationType terminationType_)
{
    terminationType = terminationType_;
    Select(ID_terminationType, (void *)&terminationType);
}

void
LCSAttributes::SetTerminateBySize(bool terminateBySize_)
{
    terminateBySize = terminateBySize_;
    Select(ID_terminateBySize, (void *)&terminateBySize);
}

void
LCSAttributes::SetTermSize(double termSize_)
{
    termSize = termSize_;
    Select(ID_termSize, (void *)&termSize);
}

void
LCSAttributes::SetTerminateByDistance(bool terminateByDistance_)
{
    terminateByDistance = terminateByDistance_;
    Select(ID_terminateByDistance, (void *)&terminateByDistance);
}

void
LCSAttributes::SetTermDistance(double termDistance_)
{
    termDistance = termDistance_;
    Select(ID_termDistance, (void *)&termDistance);
}

void
LCSAttributes::SetTerminateByTime(bool terminateByTime_)
{
    terminateByTime = terminateByTime_;
    Select(ID_terminateByTime, (void *)&terminateByTime);
}

void
LCSAttributes::SetTermTime(double termTime_)
{
    termTime = termTime_;
    Select(ID_termTime, (void *)&termTime);
}

void
LCSAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
LCSAttributes::SetLimitMaximumTimestep(bool limitMaximumTimestep_)
{
    limitMaximumTimestep = limitMaximumTimestep_;
    Select(ID_limitMaximumTimestep, (void *)&limitMaximumTimestep);
}

void
LCSAttributes::SetMaxTimeStep(double maxTimeStep_)
{
    maxTimeStep = maxTimeStep_;
    Select(ID_maxTimeStep, (void *)&maxTimeStep);
}

void
LCSAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
LCSAttributes::SetAbsTolSizeType(LCSAttributes::SizeType absTolSizeType_)
{
    absTolSizeType = absTolSizeType_;
    Select(ID_absTolSizeType, (void *)&absTolSizeType);
}

void
LCSAttributes::SetAbsTolAbsolute(double absTolAbsolute_)
{
    absTolAbsolute = absTolAbsolute_;
    Select(ID_absTolAbsolute, (void *)&absTolAbsolute);
}

void
LCSAttributes::SetAbsTolBBox(double absTolBBox_)
{
    absTolBBox = absTolBBox_;
    Select(ID_absTolBBox, (void *)&absTolBBox);
}

void
LCSAttributes::SetFieldType(LCSAttributes::FieldType fieldType_)
{
    fieldType = fieldType_;
    Select(ID_fieldType, (void *)&fieldType);
}

void
LCSAttributes::SetFieldConstant(double fieldConstant_)
{
    fieldConstant = fieldConstant_;
    Select(ID_fieldConstant, (void *)&fieldConstant);
}

void
LCSAttributes::SetVelocitySource(const double *velocitySource_)
{
    velocitySource[0] = velocitySource_[0];
    velocitySource[1] = velocitySource_[1];
    velocitySource[2] = velocitySource_[2];
    Select(ID_velocitySource, (void *)velocitySource, 3);
}

void
LCSAttributes::SetIntegrationType(LCSAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
LCSAttributes::SetClampLogValues(bool clampLogValues_)
{
    clampLogValues = clampLogValues_;
    Select(ID_clampLogValues, (void *)&clampLogValues);
}

void
LCSAttributes::SetParallelizationAlgorithmType(LCSAttributes::ParallelizationAlgorithmType parallelizationAlgorithmType_)
{
    parallelizationAlgorithmType = parallelizationAlgorithmType_;
    Select(ID_parallelizationAlgorithmType, (void *)&parallelizationAlgorithmType);
}

void
LCSAttributes::SetMaxProcessCount(int maxProcessCount_)
{
    maxProcessCount = maxProcessCount_;
    Select(ID_maxProcessCount, (void *)&maxProcessCount);
}

void
LCSAttributes::SetMaxDomainCacheSize(int maxDomainCacheSize_)
{
    maxDomainCacheSize = maxDomainCacheSize_;
    Select(ID_maxDomainCacheSize, (void *)&maxDomainCacheSize);
}

void
LCSAttributes::SetWorkGroupSize(int workGroupSize_)
{
    workGroupSize = workGroupSize_;
    Select(ID_workGroupSize, (void *)&workGroupSize);
}

void
LCSAttributes::SetPathlines(bool pathlines_)
{
    pathlines = pathlines_;
    Select(ID_pathlines, (void *)&pathlines);
}

void
LCSAttributes::SetPathlinesOverrideStartingTimeFlag(bool pathlinesOverrideStartingTimeFlag_)
{
    pathlinesOverrideStartingTimeFlag = pathlinesOverrideStartingTimeFlag_;
    Select(ID_pathlinesOverrideStartingTimeFlag, (void *)&pathlinesOverrideStartingTimeFlag);
}

void
LCSAttributes::SetPathlinesOverrideStartingTime(double pathlinesOverrideStartingTime_)
{
    pathlinesOverrideStartingTime = pathlinesOverrideStartingTime_;
    Select(ID_pathlinesOverrideStartingTime, (void *)&pathlinesOverrideStartingTime);
}

void
LCSAttributes::SetPathlinesPeriod(double pathlinesPeriod_)
{
    pathlinesPeriod = pathlinesPeriod_;
    Select(ID_pathlinesPeriod, (void *)&pathlinesPeriod);
}

void
LCSAttributes::SetPathlinesCMFE(LCSAttributes::PathlinesCMFE pathlinesCMFE_)
{
    pathlinesCMFE = pathlinesCMFE_;
    Select(ID_pathlinesCMFE, (void *)&pathlinesCMFE);
}

void
LCSAttributes::SetForceNodeCenteredData(bool forceNodeCenteredData_)
{
    forceNodeCenteredData = forceNodeCenteredData_;
    Select(ID_forceNodeCenteredData, (void *)&forceNodeCenteredData);
}

void
LCSAttributes::SetIssueTerminationWarnings(bool issueTerminationWarnings_)
{
    issueTerminationWarnings = issueTerminationWarnings_;
    Select(ID_issueTerminationWarnings, (void *)&issueTerminationWarnings);
}

void
LCSAttributes::SetIssueStiffnessWarnings(bool issueStiffnessWarnings_)
{
    issueStiffnessWarnings = issueStiffnessWarnings_;
    Select(ID_issueStiffnessWarnings, (void *)&issueStiffnessWarnings);
}

void
LCSAttributes::SetIssueCriticalPointsWarnings(bool issueCriticalPointsWarnings_)
{
    issueCriticalPointsWarnings = issueCriticalPointsWarnings_;
    Select(ID_issueCriticalPointsWarnings, (void *)&issueCriticalPointsWarnings);
}

void
LCSAttributes::SetCriticalPointThreshold(double criticalPointThreshold_)
{
    criticalPointThreshold = criticalPointThreshold_;
    Select(ID_criticalPointThreshold, (void *)&criticalPointThreshold);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LCSAttributes::SourceType
LCSAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

const int *
LCSAttributes::GetResolution() const
{
    return Resolution;
}

int *
LCSAttributes::GetResolution()
{
    return Resolution;
}

LCSAttributes::Extents
LCSAttributes::GetUseDataSetStart() const
{
    return Extents(UseDataSetStart);
}

const double *
LCSAttributes::GetStartPosition() const
{
    return StartPosition;
}

double *
LCSAttributes::GetStartPosition()
{
    return StartPosition;
}

LCSAttributes::Extents
LCSAttributes::GetUseDataSetEnd() const
{
    return Extents(UseDataSetEnd);
}

const double *
LCSAttributes::GetEndPosition() const
{
    return EndPosition;
}

double *
LCSAttributes::GetEndPosition()
{
    return EndPosition;
}

LCSAttributes::IntegrationDirection
LCSAttributes::GetIntegrationDirection() const
{
    return IntegrationDirection(integrationDirection);
}

int
LCSAttributes::GetMaxSteps() const
{
    return maxSteps;
}

LCSAttributes::OperationType
LCSAttributes::GetOperationType() const
{
    return OperationType(operationType);
}

LCSAttributes::OperatorType
LCSAttributes::GetOperatorType() const
{
    return OperatorType(operatorType);
}

LCSAttributes::TerminationType
LCSAttributes::GetTerminationType() const
{
    return TerminationType(terminationType);
}

bool
LCSAttributes::GetTerminateBySize() const
{
    return terminateBySize;
}

double
LCSAttributes::GetTermSize() const
{
    return termSize;
}

bool
LCSAttributes::GetTerminateByDistance() const
{
    return terminateByDistance;
}

double
LCSAttributes::GetTermDistance() const
{
    return termDistance;
}

bool
LCSAttributes::GetTerminateByTime() const
{
    return terminateByTime;
}

double
LCSAttributes::GetTermTime() const
{
    return termTime;
}

double
LCSAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

bool
LCSAttributes::GetLimitMaximumTimestep() const
{
    return limitMaximumTimestep;
}

double
LCSAttributes::GetMaxTimeStep() const
{
    return maxTimeStep;
}

double
LCSAttributes::GetRelTol() const
{
    return relTol;
}

LCSAttributes::SizeType
LCSAttributes::GetAbsTolSizeType() const
{
    return SizeType(absTolSizeType);
}

double
LCSAttributes::GetAbsTolAbsolute() const
{
    return absTolAbsolute;
}

double
LCSAttributes::GetAbsTolBBox() const
{
    return absTolBBox;
}

LCSAttributes::FieldType
LCSAttributes::GetFieldType() const
{
    return FieldType(fieldType);
}

double
LCSAttributes::GetFieldConstant() const
{
    return fieldConstant;
}

const double *
LCSAttributes::GetVelocitySource() const
{
    return velocitySource;
}

double *
LCSAttributes::GetVelocitySource()
{
    return velocitySource;
}

LCSAttributes::IntegrationType
LCSAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

bool
LCSAttributes::GetClampLogValues() const
{
    return clampLogValues;
}

LCSAttributes::ParallelizationAlgorithmType
LCSAttributes::GetParallelizationAlgorithmType() const
{
    return ParallelizationAlgorithmType(parallelizationAlgorithmType);
}

int
LCSAttributes::GetMaxProcessCount() const
{
    return maxProcessCount;
}

int
LCSAttributes::GetMaxDomainCacheSize() const
{
    return maxDomainCacheSize;
}

int
LCSAttributes::GetWorkGroupSize() const
{
    return workGroupSize;
}

bool
LCSAttributes::GetPathlines() const
{
    return pathlines;
}

bool
LCSAttributes::GetPathlinesOverrideStartingTimeFlag() const
{
    return pathlinesOverrideStartingTimeFlag;
}

double
LCSAttributes::GetPathlinesOverrideStartingTime() const
{
    return pathlinesOverrideStartingTime;
}

double
LCSAttributes::GetPathlinesPeriod() const
{
    return pathlinesPeriod;
}

LCSAttributes::PathlinesCMFE
LCSAttributes::GetPathlinesCMFE() const
{
    return PathlinesCMFE(pathlinesCMFE);
}

bool
LCSAttributes::GetForceNodeCenteredData() const
{
    return forceNodeCenteredData;
}

bool
LCSAttributes::GetIssueTerminationWarnings() const
{
    return issueTerminationWarnings;
}

bool
LCSAttributes::GetIssueStiffnessWarnings() const
{
    return issueStiffnessWarnings;
}

bool
LCSAttributes::GetIssueCriticalPointsWarnings() const
{
    return issueCriticalPointsWarnings;
}

double
LCSAttributes::GetCriticalPointThreshold() const
{
    return criticalPointThreshold;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
LCSAttributes::SelectResolution()
{
    Select(ID_Resolution, (void *)Resolution, 3);
}

void
LCSAttributes::SelectStartPosition()
{
    Select(ID_StartPosition, (void *)StartPosition, 3);
}

void
LCSAttributes::SelectEndPosition()
{
    Select(ID_EndPosition, (void *)EndPosition, 3);
}

void
LCSAttributes::SelectVelocitySource()
{
    Select(ID_velocitySource, (void *)velocitySource, 3);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LCSAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
LCSAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                        return "sourceType";
    case ID_Resolution:                        return "Resolution";
    case ID_UseDataSetStart:                   return "UseDataSetStart";
    case ID_StartPosition:                     return "StartPosition";
    case ID_UseDataSetEnd:                     return "UseDataSetEnd";
    case ID_EndPosition:                       return "EndPosition";
    case ID_integrationDirection:              return "integrationDirection";
    case ID_maxSteps:                          return "maxSteps";
    case ID_operationType:                     return "operationType";
    case ID_operatorType:                      return "operatorType";
    case ID_terminationType:                   return "terminationType";
    case ID_terminateBySize:                   return "terminateBySize";
    case ID_termSize:                          return "termSize";
    case ID_terminateByDistance:               return "terminateByDistance";
    case ID_termDistance:                      return "termDistance";
    case ID_terminateByTime:                   return "terminateByTime";
    case ID_termTime:                          return "termTime";
    case ID_maxStepLength:                     return "maxStepLength";
    case ID_limitMaximumTimestep:              return "limitMaximumTimestep";
    case ID_maxTimeStep:                       return "maxTimeStep";
    case ID_relTol:                            return "relTol";
    case ID_absTolSizeType:                    return "absTolSizeType";
    case ID_absTolAbsolute:                    return "absTolAbsolute";
    case ID_absTolBBox:                        return "absTolBBox";
    case ID_fieldType:                         return "fieldType";
    case ID_fieldConstant:                     return "fieldConstant";
    case ID_velocitySource:                    return "velocitySource";
    case ID_integrationType:                   return "integrationType";
    case ID_clampLogValues:                    return "clampLogValues";
    case ID_parallelizationAlgorithmType:      return "parallelizationAlgorithmType";
    case ID_maxProcessCount:                   return "maxProcessCount";
    case ID_maxDomainCacheSize:                return "maxDomainCacheSize";
    case ID_workGroupSize:                     return "workGroupSize";
    case ID_pathlines:                         return "pathlines";
    case ID_pathlinesOverrideStartingTimeFlag: return "pathlinesOverrideStartingTimeFlag";
    case ID_pathlinesOverrideStartingTime:     return "pathlinesOverrideStartingTime";
    case ID_pathlinesPeriod:                   return "pathlinesPeriod";
    case ID_pathlinesCMFE:                     return "pathlinesCMFE";
    case ID_forceNodeCenteredData:             return "forceNodeCenteredData";
    case ID_issueTerminationWarnings:          return "issueTerminationWarnings";
    case ID_issueStiffnessWarnings:            return "issueStiffnessWarnings";
    case ID_issueCriticalPointsWarnings:       return "issueCriticalPointsWarnings";
    case ID_criticalPointThreshold:            return "criticalPointThreshold";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LCSAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
LCSAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_sourceType:                        return FieldType_enum;
    case ID_Resolution:                        return FieldType_intArray;
    case ID_UseDataSetStart:                   return FieldType_enum;
    case ID_StartPosition:                     return FieldType_doubleArray;
    case ID_UseDataSetEnd:                     return FieldType_enum;
    case ID_EndPosition:                       return FieldType_doubleArray;
    case ID_integrationDirection:              return FieldType_enum;
    case ID_maxSteps:                          return FieldType_int;
    case ID_operationType:                     return FieldType_enum;
    case ID_operatorType:                      return FieldType_enum;
    case ID_terminationType:                   return FieldType_enum;
    case ID_terminateBySize:                   return FieldType_bool;
    case ID_termSize:                          return FieldType_double;
    case ID_terminateByDistance:               return FieldType_bool;
    case ID_termDistance:                      return FieldType_double;
    case ID_terminateByTime:                   return FieldType_bool;
    case ID_termTime:                          return FieldType_double;
    case ID_maxStepLength:                     return FieldType_double;
    case ID_limitMaximumTimestep:              return FieldType_bool;
    case ID_maxTimeStep:                       return FieldType_double;
    case ID_relTol:                            return FieldType_double;
    case ID_absTolSizeType:                    return FieldType_enum;
    case ID_absTolAbsolute:                    return FieldType_double;
    case ID_absTolBBox:                        return FieldType_double;
    case ID_fieldType:                         return FieldType_enum;
    case ID_fieldConstant:                     return FieldType_double;
    case ID_velocitySource:                    return FieldType_doubleArray;
    case ID_integrationType:                   return FieldType_enum;
    case ID_clampLogValues:                    return FieldType_bool;
    case ID_parallelizationAlgorithmType:      return FieldType_enum;
    case ID_maxProcessCount:                   return FieldType_int;
    case ID_maxDomainCacheSize:                return FieldType_int;
    case ID_workGroupSize:                     return FieldType_int;
    case ID_pathlines:                         return FieldType_bool;
    case ID_pathlinesOverrideStartingTimeFlag: return FieldType_bool;
    case ID_pathlinesOverrideStartingTime:     return FieldType_double;
    case ID_pathlinesPeriod:                   return FieldType_double;
    case ID_pathlinesCMFE:                     return FieldType_enum;
    case ID_forceNodeCenteredData:             return FieldType_bool;
    case ID_issueTerminationWarnings:          return FieldType_bool;
    case ID_issueStiffnessWarnings:            return FieldType_bool;
    case ID_issueCriticalPointsWarnings:       return FieldType_bool;
    case ID_criticalPointThreshold:            return FieldType_double;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: LCSAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
LCSAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                        return "enum";
    case ID_Resolution:                        return "intArray";
    case ID_UseDataSetStart:                   return "enum";
    case ID_StartPosition:                     return "doubleArray";
    case ID_UseDataSetEnd:                     return "enum";
    case ID_EndPosition:                       return "doubleArray";
    case ID_integrationDirection:              return "enum";
    case ID_maxSteps:                          return "int";
    case ID_operationType:                     return "enum";
    case ID_operatorType:                      return "enum";
    case ID_terminationType:                   return "enum";
    case ID_terminateBySize:                   return "bool";
    case ID_termSize:                          return "double";
    case ID_terminateByDistance:               return "bool";
    case ID_termDistance:                      return "double";
    case ID_terminateByTime:                   return "bool";
    case ID_termTime:                          return "double";
    case ID_maxStepLength:                     return "double";
    case ID_limitMaximumTimestep:              return "bool";
    case ID_maxTimeStep:                       return "double";
    case ID_relTol:                            return "double";
    case ID_absTolSizeType:                    return "enum";
    case ID_absTolAbsolute:                    return "double";
    case ID_absTolBBox:                        return "double";
    case ID_fieldType:                         return "enum";
    case ID_fieldConstant:                     return "double";
    case ID_velocitySource:                    return "doubleArray";
    case ID_integrationType:                   return "enum";
    case ID_clampLogValues:                    return "bool";
    case ID_parallelizationAlgorithmType:      return "enum";
    case ID_maxProcessCount:                   return "int";
    case ID_maxDomainCacheSize:                return "int";
    case ID_workGroupSize:                     return "int";
    case ID_pathlines:                         return "bool";
    case ID_pathlinesOverrideStartingTimeFlag: return "bool";
    case ID_pathlinesOverrideStartingTime:     return "double";
    case ID_pathlinesPeriod:                   return "double";
    case ID_pathlinesCMFE:                     return "enum";
    case ID_forceNodeCenteredData:             return "bool";
    case ID_issueTerminationWarnings:          return "bool";
    case ID_issueStiffnessWarnings:            return "bool";
    case ID_issueCriticalPointsWarnings:       return "bool";
    case ID_criticalPointThreshold:            return "double";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LCSAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
LCSAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const LCSAttributes &obj = *((const LCSAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_sourceType:
        {  // new scope
        retval = (sourceType == obj.sourceType);
        }
        break;
    case ID_Resolution:
        {  // new scope
        // Compare the Resolution arrays.
        bool Resolution_equal = true;
        for(int i = 0; i < 3 && Resolution_equal; ++i)
            Resolution_equal = (Resolution[i] == obj.Resolution[i]);

        retval = Resolution_equal;
        }
        break;
    case ID_UseDataSetStart:
        {  // new scope
        retval = (UseDataSetStart == obj.UseDataSetStart);
        }
        break;
    case ID_StartPosition:
        {  // new scope
        // Compare the StartPosition arrays.
        bool StartPosition_equal = true;
        for(int i = 0; i < 3 && StartPosition_equal; ++i)
            StartPosition_equal = (StartPosition[i] == obj.StartPosition[i]);

        retval = StartPosition_equal;
        }
        break;
    case ID_UseDataSetEnd:
        {  // new scope
        retval = (UseDataSetEnd == obj.UseDataSetEnd);
        }
        break;
    case ID_EndPosition:
        {  // new scope
        // Compare the EndPosition arrays.
        bool EndPosition_equal = true;
        for(int i = 0; i < 3 && EndPosition_equal; ++i)
            EndPosition_equal = (EndPosition[i] == obj.EndPosition[i]);

        retval = EndPosition_equal;
        }
        break;
    case ID_integrationDirection:
        {  // new scope
        retval = (integrationDirection == obj.integrationDirection);
        }
        break;
    case ID_maxSteps:
        {  // new scope
        retval = (maxSteps == obj.maxSteps);
        }
        break;
    case ID_operationType:
        {  // new scope
        retval = (operationType == obj.operationType);
        }
        break;
    case ID_operatorType:
        {  // new scope
        retval = (operatorType == obj.operatorType);
        }
        break;
    case ID_terminationType:
        {  // new scope
        retval = (terminationType == obj.terminationType);
        }
        break;
    case ID_terminateBySize:
        {  // new scope
        retval = (terminateBySize == obj.terminateBySize);
        }
        break;
    case ID_termSize:
        {  // new scope
        retval = (termSize == obj.termSize);
        }
        break;
    case ID_terminateByDistance:
        {  // new scope
        retval = (terminateByDistance == obj.terminateByDistance);
        }
        break;
    case ID_termDistance:
        {  // new scope
        retval = (termDistance == obj.termDistance);
        }
        break;
    case ID_terminateByTime:
        {  // new scope
        retval = (terminateByTime == obj.terminateByTime);
        }
        break;
    case ID_termTime:
        {  // new scope
        retval = (termTime == obj.termTime);
        }
        break;
    case ID_maxStepLength:
        {  // new scope
        retval = (maxStepLength == obj.maxStepLength);
        }
        break;
    case ID_limitMaximumTimestep:
        {  // new scope
        retval = (limitMaximumTimestep == obj.limitMaximumTimestep);
        }
        break;
    case ID_maxTimeStep:
        {  // new scope
        retval = (maxTimeStep == obj.maxTimeStep);
        }
        break;
    case ID_relTol:
        {  // new scope
        retval = (relTol == obj.relTol);
        }
        break;
    case ID_absTolSizeType:
        {  // new scope
        retval = (absTolSizeType == obj.absTolSizeType);
        }
        break;
    case ID_absTolAbsolute:
        {  // new scope
        retval = (absTolAbsolute == obj.absTolAbsolute);
        }
        break;
    case ID_absTolBBox:
        {  // new scope
        retval = (absTolBBox == obj.absTolBBox);
        }
        break;
    case ID_fieldType:
        {  // new scope
        retval = (fieldType == obj.fieldType);
        }
        break;
    case ID_fieldConstant:
        {  // new scope
        retval = (fieldConstant == obj.fieldConstant);
        }
        break;
    case ID_velocitySource:
        {  // new scope
        // Compare the velocitySource arrays.
        bool velocitySource_equal = true;
        for(int i = 0; i < 3 && velocitySource_equal; ++i)
            velocitySource_equal = (velocitySource[i] == obj.velocitySource[i]);

        retval = velocitySource_equal;
        }
        break;
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_clampLogValues:
        {  // new scope
        retval = (clampLogValues == obj.clampLogValues);
        }
        break;
    case ID_parallelizationAlgorithmType:
        {  // new scope
        retval = (parallelizationAlgorithmType == obj.parallelizationAlgorithmType);
        }
        break;
    case ID_maxProcessCount:
        {  // new scope
        retval = (maxProcessCount == obj.maxProcessCount);
        }
        break;
    case ID_maxDomainCacheSize:
        {  // new scope
        retval = (maxDomainCacheSize == obj.maxDomainCacheSize);
        }
        break;
    case ID_workGroupSize:
        {  // new scope
        retval = (workGroupSize == obj.workGroupSize);
        }
        break;
    case ID_pathlines:
        {  // new scope
        retval = (pathlines == obj.pathlines);
        }
        break;
    case ID_pathlinesOverrideStartingTimeFlag:
        {  // new scope
        retval = (pathlinesOverrideStartingTimeFlag == obj.pathlinesOverrideStartingTimeFlag);
        }
        break;
    case ID_pathlinesOverrideStartingTime:
        {  // new scope
        retval = (pathlinesOverrideStartingTime == obj.pathlinesOverrideStartingTime);
        }
        break;
    case ID_pathlinesPeriod:
        {  // new scope
        retval = (pathlinesPeriod == obj.pathlinesPeriod);
        }
        break;
    case ID_pathlinesCMFE:
        {  // new scope
        retval = (pathlinesCMFE == obj.pathlinesCMFE);
        }
        break;
    case ID_forceNodeCenteredData:
        {  // new scope
        retval = (forceNodeCenteredData == obj.forceNodeCenteredData);
        }
        break;
    case ID_issueTerminationWarnings:
        {  // new scope
        retval = (issueTerminationWarnings == obj.issueTerminationWarnings);
        }
        break;
    case ID_issueStiffnessWarnings:
        {  // new scope
        retval = (issueStiffnessWarnings == obj.issueStiffnessWarnings);
        }
        break;
    case ID_issueCriticalPointsWarnings:
        {  // new scope
        retval = (issueCriticalPointsWarnings == obj.issueCriticalPointsWarnings);
        }
        break;
    case ID_criticalPointThreshold:
        {  // new scope
        retval = (criticalPointThreshold == obj.criticalPointThreshold);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//  Method: LCSAttributes::ChangesRequireRecalculation
//
//  Purpose:
//     Determines whether or not the plot must be recalculated based on the
//     new attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Notes:  Most attributes cause the LCS to change.
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:52:45 PDT 2004
//    I made the coloring method matter when comparing LCS attributes
//    and I added support for ribbons.
//
//    Hank Childs, Sat Mar  3 09:00:12 PST 2007
//    Add support for useWholeBox.
//
//    Hank Childs, Sun May  3 11:49:31 CDT 2009
//    Add support for point lists.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the LCSs plots.
//
//   Christoph Garth, Wed Jan 13 17:14:21 PST 2010 
//   Add support for circle source.
//
//   Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//   Add support for absTol that is fraction of the bounding box.
//
//   Hank Childs, Mon Oct  4 14:32:06 PDT 2010
//   Add support for having multiple termination criterias.
//
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
LCSAttributes::ChangesRequireRecalculation(const LCSAttributes &obj) const
{
    //Check the general stuff first...

    if( sourceType != obj.sourceType ||
        integrationDirection != obj.integrationDirection ||
        maxSteps != obj.maxSteps ||
        terminationType != obj.terminationType ||
        terminateBySize != obj.terminateBySize ||
        termSize != obj.termSize ||
        terminateByDistance != obj.terminateByDistance ||
        termDistance != obj.termDistance ||
        terminateByTime != obj.terminateByTime ||
        termTime != obj.termTime ||
        maxStepLength != obj.maxStepLength ||
        limitMaximumTimestep != obj.limitMaximumTimestep ||
        maxTimeStep != obj.maxTimeStep ||
        relTol != obj.relTol ||
        absTolSizeType != obj.absTolSizeType ||
        absTolAbsolute != obj.absTolAbsolute ||
        absTolBBox != obj.absTolBBox ||
        fieldType != obj.fieldType ||
        fieldConstant != obj. fieldConstant||
        integrationType != obj.integrationType ||
        parallelizationAlgorithmType != obj.parallelizationAlgorithmType ||
        maxProcessCount != obj.maxProcessCount ||
        maxDomainCacheSize != obj.maxDomainCacheSize ||
        workGroupSize != obj.workGroupSize ||
        pathlines != obj.pathlines ||
        pathlinesOverrideStartingTimeFlag != obj.pathlinesOverrideStartingTimeFlag ||
        pathlinesOverrideStartingTime != obj.pathlinesOverrideStartingTime ||
        pathlinesCMFE != obj.pathlinesCMFE ||
        forceNodeCenteredData != obj.forceNodeCenteredData ||
        issueTerminationWarnings != obj.issueTerminationWarnings ||
        issueStiffnessWarnings != obj.issueStiffnessWarnings ||
        issueCriticalPointsWarnings != obj.issueCriticalPointsWarnings ||
        criticalPointThreshold != obj.criticalPointThreshold )
    {
        return true;
    }
         
    // If they say they don't want warnings, then don't re-execute. If
    // they do, then re-execute so we can give them that warning.
    if (issueTerminationWarnings != obj.issueTerminationWarnings &&
        obj.issueTerminationWarnings == true)
        return true;

    if (fieldType == FlashField &&
        POINT_DIFFERS(velocitySource, obj.velocitySource))
    {
        return true;
    }

    //Check by source type.
    if ((sourceType == RegularGrid) &&
         POINT_DIFFERS(Resolution, obj.Resolution))
    {
        return true;
    }

    if ((sourceType == RegularGrid) &&
         UseDataSetStart == Subset &&
         POINT_DIFFERS(StartPosition, obj.StartPosition))
    {
        return true;
    }

    if ((sourceType == RegularGrid) &&
         UseDataSetEnd == Subset &&
         POINT_DIFFERS(EndPosition, obj.EndPosition))
    {
        return true;
    }

    return false;
}

