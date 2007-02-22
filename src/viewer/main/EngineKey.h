/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
