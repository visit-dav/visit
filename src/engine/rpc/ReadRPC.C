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

#include <ReadRPC.h>
#include <string>
#include <DebugStream.h>

using std::string;

// ****************************************************************************
//  Constructor: ReadRPC::ReadRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Added file format type.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bool to support to treat all databases as time varying
// ****************************************************************************

ReadRPC::ReadRPC() : BlockingRPC("ssiaasabb")
{
}

// ****************************************************************************
//  Destructor: ReadRPC::~ReadRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

ReadRPC::~ReadRPC()
{
}

// ****************************************************************************
//  Method: ReadRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    f         the filename
//    v         the variable name
//    t         the time step
//    s         the sil restriction attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:41:39 PST 2000
//    Made output go to log file instead of cout or cerr.
//
//    Hank Childs, Wed Jun 13 10:40:58 PDT 2001
//    Added SIL restriction argument.
//
//    Jeremy Meredith, Thu Oct 24 11:21:04 PDT 2002
//    Added material interface reconstruction attributes.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Added file format type.
//
//    Mark C. Miller Sun Nov  6 07:07:53 PST 2005
//    Added mesh management attributes
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bool to support to treat all databases as time varying
// ****************************************************************************

void
ReadRPC::operator()(const string &ft, const string &f, const string &v, int t,
                    const CompactSILRestrictionAttributes &s,
                    const MaterialAttributes &m,
                    const MeshManagementAttributes &mm,
                    bool treatAllDBsAsTimeVarying,
                    bool ignoreExtents)
{
    debug3 << "Executing read RPC" 
           << "\n\t file format='" << ft.c_str() << "'"
           << "\n\t file='" << f.c_str() << "'"
           << "\n\t var ='" << v.c_str() << "'"
           << "\n\t time='" << t << "'"
           << "\n\t treatAllDBsAsTimeVarying ='" << treatAllDBsAsTimeVarying << "'"
           << "\n\t ignoreExtents ='" << ignoreExtents << "'"
           << endl;

    SetFormat(ft);
    SetFile(f);
    SetVar(v);
    SetTime(t);
    SetCSRAttributes(s);
    SetMaterialAttributes(m);
    SetMeshManagementAttributes(mm);
    SetTreatAllDBsAsTimeVarying(treatAllDBsAsTimeVarying);
    SetIgnoreExtents(ignoreExtents);

    Execute();
}

// ****************************************************************************
//  Method: ReadRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Added file format type.
//
//    Mark C. Miller Sun Nov  6 07:07:53 PST 2005
//    Added mesh management attributes
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bool to support to treat all databases as time varying
// ****************************************************************************

void
ReadRPC::SelectAll()
{
    Select(0, (void*)&file);
    Select(1, (void*)&var);
    Select(2, (void*)&time);
    Select(3, (void*)&silr_atts);
    Select(4, (void*)&materialAtts);
    Select(5, (void*)&format);
    Select(6, (void*)&meshManagementAtts);
    Select(7, (void*)&treatAllDBsAsTimeVarying);
    Select(8, (void*)&ignoreExtents);
}


// ****************************************************************************
//  Method: ReadRPC::SetFile
//
//  Purpose: 
//    This sets the file name parameter.
//
//  Arguments:
//    f         the filename
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

void
ReadRPC::SetFile(const string &f)
{
    file = f;
    Select(0, (void*)&file);
}

// ****************************************************************************
//  Method: ReadRPC::SetFormat
//
//  Purpose: 
//    This sets the file format type.
//
//  Arguments:
//    f         the file format
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2004
//
// ****************************************************************************

void
ReadRPC::SetFormat(const string &f)
{
    format = f;
    Select(5, (void*)&format);
}

// ****************************************************************************
//  Method: ReadRPC::SetVar
//
//  Purpose: 
//    This sets the variable parameter.
//
//  Arguments:
//    v         the variable name
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

void
ReadRPC::SetVar(const string &v)
{
    var = v;
    Select(1, (void*)&var);
}

// ****************************************************************************
//  Method: ReadRPC::SetTime
//
//  Purpose: 
//    This sets the time step parameter.
//
//  Arguments:
//    t         the time step
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

void
ReadRPC::SetTime(int t)
{
    time = t;
    Select(2, (void*)&time);
}


// ****************************************************************************
//  Method: ReadRPC::SetCSRAttributes
//
//  Purpose:
//    This sets the SIL restriction.
//
//  Arguments:
//    s         the sil restriction
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2001
//
// ****************************************************************************

void
ReadRPC::SetCSRAttributes(const CompactSILRestrictionAttributes &s)
{
    silr_atts = s;
    Select(3, (void*)&silr_atts);
}

// ****************************************************************************
//  Method:  ReadRPC::SetMaterialAttributes
//
//  Purpose:
//    Sets the material interface reconstruction attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

void
ReadRPC::SetMaterialAttributes(const MaterialAttributes &m)
{
    materialAtts = m;
    Select(4, (void*)&materialAtts);
}

// ****************************************************************************
//  Method:  ReadRPC::SetMeshManagementAttributes
//
//  Purpose: Sets the mesh management attributes.
//
//  Programmer:  Mark C. Miller 
//  Creation:    November 6, 2005
//
// ****************************************************************************

void
ReadRPC::SetMeshManagementAttributes(const MeshManagementAttributes &mm)
{
    meshManagementAtts = mm;
    Select(6, (void*)&meshManagementAtts);
}

// ****************************************************************************
//  Method:  ReadRPC::SetTreatAllDBsAsTimeVarying
//
//  Purpose: Sets flag to treat all databases as time varying
//
//  Programmer:  Mark C. Miller 
//  Creation:    June 12, 2007 
//
// ****************************************************************************

void
ReadRPC::SetTreatAllDBsAsTimeVarying(bool set)
{
    treatAllDBsAsTimeVarying = set;
    Select(7, (void*)&treatAllDBsAsTimeVarying);
}

void
ReadRPC::SetIgnoreExtents(bool set)
{
    ignoreExtents = set;
    Select(8, (void*)&ignoreExtents);
}

// ****************************************************************************
//  Method: ReadRPC::GetFile
//
//  Purpose: 
//    This returns the file name.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

string
ReadRPC::GetFile() const
{
    return file;
}

// ****************************************************************************
//  Method: ReadRPC::GetFile
//
//  Purpose: 
//    This returns the file format type.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2004
//
// ****************************************************************************

string
ReadRPC::GetFormat() const
{
    return format;
}

// ****************************************************************************
//  Method: ReadRPC::GetVar
//
//  Purpose: 
//    This returns the variable name.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

string
ReadRPC::GetVar() const
{
    return var;
}

// ****************************************************************************
//  Method: ReadRPC::GetTime
//
//  Purpose: 
//    This returns the time step.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

int
ReadRPC::GetTime() const
{
    return time;
}

// ****************************************************************************
//  Method: ReadRPC::GetCSRAttributes
//
//  Purpose: 
//    This returns the SIL restriction attributes
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2001
//
// ****************************************************************************

const CompactSILRestrictionAttributes &
ReadRPC::GetCSRAttributes() const
{
    return silr_atts;
}

// ****************************************************************************
//  Method:  ReadRPC::GetMaterialAttributes
//
//  Purpose:
//    Returns the material interface reconstruction attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

const MaterialAttributes &
ReadRPC::GetMaterialAttributes() const
{
    return materialAtts;
}

// ****************************************************************************
//  Method:  ReadRPC::GetMeshManagementAttributes
//
//  Purpose: Returns the mesh management attributes.
//
//  Programmer:  Mark C. Miller 
//  Creation:    November 6, 2005 
//
// ****************************************************************************

const MeshManagementAttributes &
ReadRPC::GetMeshManagementAttributes() const
{
    return meshManagementAtts;
}

// ****************************************************************************
//  Method:  ReadRPC::GetTreatAllDBsAsTimeVarying
//
//  Purpose: Returns flag indicating if all databases should be treated as time
//  varying
//
//  Programmer:  Mark C. Miller 
//  Creation:    June 12, 2007 
//
// ****************************************************************************

bool
ReadRPC::GetTreatAllDBsAsTimeVarying() const
{
    return treatAllDBsAsTimeVarying;
}

bool
ReadRPC::GetIgnoreExtents() const
{
    return ignoreExtents;
}
