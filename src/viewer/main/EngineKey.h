// ************************************************************************* //
//                                 EngineKey.h                               //
// ************************************************************************* //

#ifndef ENGINE_KEY_H
#define ENGINE_KEY_H

#include <string>

#include <viewer_exports.h>

// ****************************************************************************
//  Class:  EngineKey
//
//  Purpose:
//    Contains the information needed to choose an item. At the time
//    of this writing, that information includes a host name and a
//    (possibly blank) simulation identifier, and the simulation
//    identifier is currently implemented as the database name of the
//    ".sim" file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 26, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 31 10:16:38 PDT 2004
//    Added != operator so it builds on the SGI.
//
//    Brad Whitlock, Thu Apr 8 12:25:35 PDT 2004
//    Made a small change to IsSimulation because somehow I was able to
//    get a plot with <unset> for the simName, which is possible if the
//    default constructor is used. Anyway, session files don't get saved
//    for simulations and ViewerPlotList thought my plots were from a
//    simulation even though they were not.
//
//    Brad Whitlock, Tue May 4 10:01:33 PDT 2004
//    Added a method to return the original hostname.
//
// ****************************************************************************

class VIEWER_API EngineKey
{
  private:
    std::string origHostName;
    std::string realHostName;
    std::string simName;
    static std::string localhost;
  public:

    EngineKey()
    {
        origHostName = "<unset>";
        realHostName = "<unset>";
        simName      = "<unset>";
    }

    EngineKey(const std::string &h, const std::string &s)
        : origHostName(h), simName(s)
    {
        if (origHostName == "localhost")
            realHostName = localhost;
        else
            realHostName = origHostName;
    }

    EngineKey(const EngineKey &ek)
    {
        origHostName = ek.origHostName;
        simName      = ek.simName;
        realHostName = ek.realHostName;
    }

    const EngineKey &operator=(const EngineKey &ek)
    {
        origHostName = ek.origHostName;
        simName      = ek.simName;
        realHostName = ek.realHostName;
        return *this;
    }

    std::string ID() const
    {
        if (simName == "")
        {
            return realHostName;
        }
        else
        {
            return realHostName + ":" + simName;
        }
    }

    const std::string &HostName() const      { return realHostName; }
    const std::string &OriginalHostName() const  { return origHostName; }

    const std::string &SimName()  const      { return simName; }

    bool IsSimulation() const
    {
        return !simName.empty() && simName != "<unset>";
    }

    bool operator==(const EngineKey &ek) const
    {
        return (realHostName == ek.realHostName &&
                simName      == ek.simName);
    }

    bool operator!=(const EngineKey &ek) const
    {
        return (realHostName != ek.realHostName ||
                simName      != ek.simName);
    }

    bool operator<(const EngineKey &ek) const
    {
        if (realHostName < ek.realHostName)
            return true;
        else if (realHostName > ek.realHostName)
            return false;
        else if (simName < ek.simName)
            return true;
        else
            return false;
    }

    static void SetLocalHost(const std::string &lh)
    {
        localhost = lh;
    }
};

#endif
