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
// ****************************************************************************

ReadRPC::ReadRPC() : BlockingRPC("ssiaa")
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
// ****************************************************************************

void
ReadRPC::operator()(const string &f, const string &v, int t,
                    const CompactSILRestrictionAttributes &s,
                    const MaterialAttributes &m)
{
    debug3 << "Executing read RPC" 
           << "\n\t file='" << f.c_str() << "'"
           << "\n\t var ='" << v.c_str() << "'"
           << "\n\t time='" << t << "'"
           << endl;

    SetFile(f);
    SetVar(v);
    SetTime(t);
    SetCSRAttributes(s);
    SetMaterialAttributes(m);

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
// ****************************************************************************

void
ReadRPC::SelectAll()
{
    Select(0, (void*)&file);
    Select(1, (void*)&var);
    Select(2, (void*)&time);
    Select(3, (void*)&silr_atts);
    Select(4, (void*)&materialAtts);
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
