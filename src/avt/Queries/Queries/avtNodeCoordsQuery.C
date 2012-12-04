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
//                            avtNodeCoordsQuery.C                           //
// ************************************************************************* //

#include <avtNodeCoordsQuery.h>
#include <avtOriginatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <vector>
#include <snprintf.h>
#include <avtParallel.h>
#include <QueryArgumentException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtNodeCoordsQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:21:52 PDT 2011
//    Added domain, element, useGlobalId.
//
// ****************************************************************************

avtNodeCoordsQuery::avtNodeCoordsQuery() : avtDatasetQuery() 
{
    domain = 0;
    element = 0;
    useGlobalId = false;
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNodeCoordsQuery::~avtNodeCoordsQuery() 
{
}

// ****************************************************************************
//  Method: avtNodeCoordsQuery::SetInputParams
//
//  Purpose:  Allows this query to read input parameters set by user.
//    
//  Arguments:
//    params:   MapNode containing input.
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 20, 2011 
//
//  Modifications:
//    Kathleen Biagas, Mon Dec  3 17:32:31 PST 2012
//    Remove 'domain' requirement.  Single-domain problems shouldn't need to 
//    have a domain specified.
//
// ****************************************************************************

void
avtNodeCoordsQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("use_global_id"))
        useGlobalId = params.GetEntry("use_global_id")->AsInt();

    if (params.HasEntry("domain"))
        domain = params.GetEntry("domain")->AsInt();

    if (params.HasEntry("element"))
        element = params.GetEntry("element")->AsInt();
    else
        EXCEPTION1(QueryArgumentException, "element");
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::GetDefaultInputParams
//
//  Purpose:  Retrieve default input values.
//    
//  Arguments:
//    params:   MapNode to store default input values.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
// ****************************************************************************

void
avtNodeCoordsQuery::GetDefaultInputParams(MapNode &params)
{
    params["domain"] = 0;
    params["use_global_id"] = 0;
    params["element"] =  0;
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Kathleen Bonnell, Tue Jul  6 15:20:56 PDT 2004 
//    Removed MPI calls, use GetFloatArrayToRootProc. 
//
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
//    Moved code that actually finds zone center to FindLocalCenter and
//    FindGlobalCenter. 
//
//    Kathleen Bonnell, Tue Dec 28 14:52:22 PST 2004 
//    Add 'global' to output string as necessary. 
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Tue Jun 21 10:24:16 PDT 2011
//    Domain, element, useGlobalId retrieved in SetInputParams.
//
//    Kathleen Biagas, Mon Dec  3 17:34:56 PST 2012
//    Remove use of nodeOrigin to set output message. Element is the correct
//    id, nodeOrigin taken into acount in the 'Find???Coord' methods.
//
// ****************************************************************************

void
avtNodeCoordsQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 
    
    string floatFormat = queryAtts.GetFloatFormat();
    string format ="";

    UpdateProgress(0, 0);

    bool singleDomain = false;

    if (!useGlobalId)
    {
        intVector dlist;
        avtDataRequest_p dataRequest = 
            GetInput()->GetOriginatingSource()->GetFullDataRequest();
        dataRequest->GetSIL().GetDomainList(dlist);

        if (dlist.size() == 1 && dataRequest->UsesAllDomains())
        {
            singleDomain = true;
        }
    }
    else
    {
        singleDomain = true;
    }


    double coord[3] = {0., 0., 0.};

    bool success;
    if (useGlobalId)
    {
        success = FindGlobalCoord(coord);
    }
    else 
    {
        success = FindLocalCoord(coord);
    }
    
    GetDoubleArrayToRootProc(coord, 3, success);

    if (PAR_Rank() != 0)
        return;

    char msg[120];

    if (success)
    {
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        if (singleDomain)
        {
            string global;
            if (useGlobalId)
                global = "global";
            if (dim == 2)
            {
                format = "The coords of %s node %d are (" + floatFormat + ", " 
                                                          + floatFormat + ").";
                SNPRINTF(msg, 120, format.c_str(), 
                         global.c_str(), element, coord[0], coord[1]);
            }
            else 
            {
                format = "The coords of %s node %d are (" + floatFormat + ", " 
                                                          + floatFormat + ", " 
                                                          + floatFormat + ").";
                SNPRINTF(msg, 120, format.c_str(), 
                         global.c_str(), element, 
                         coord[0], coord[1], coord[2]);
            }
        }
        else
        {
            avtOriginatingSource *src = GetInput()->GetOriginatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int dom         = domain  - blockOrigin;
            int ts          = qA->GetTimeStep();
            string var      = qA->GetVariables()[0];
            string domainName;
            src->GetDomainName(var, ts, dom, domainName);
            if (dim == 2)
            {
                format = "The coords of node %d (%s) are (" + floatFormat +", " 
                                                            + floatFormat +").";
                SNPRINTF(msg, 120, format.c_str(), 
                         element, domainName.c_str(),
                         coord[0], coord[1]);
            }
            else 
            {
                format = "The coords of node %d (%s) are (" + floatFormat +", " 
                                                            + floatFormat +", " 
                                                            + floatFormat +").";

                SNPRINTF(msg, 120, format.c_str(), 
                         element, domainName.c_str(),
                         coord[0], coord[1], coord[2]);
            }
        }
  
        doubleVector c;
        c.push_back((double)coord[0]);
        c.push_back((double)coord[1]);
        c.push_back((double)coord[2]);
        qA->SetResultsValue(c);
    }
    else
    {
        if (singleDomain)
        {
            SNPRINTF(msg, 120, "The coords of node %d could not be determined.",
                     element);
        }
        else
        {
            avtOriginatingSource *src = GetInput()->GetOriginatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int dom         = domain  - blockOrigin;
            int ts          = qA->GetTimeStep();
            string var      = qA->GetVariables()[0];
            string domainName;
            src->GetDomainName(var, ts, dom, domainName);
            SNPRINTF(msg, 120, "The coords of node %d (%s) could not be determined.",
                     element, domainName.c_str());
        }
    }

    qA->SetResultsMessage(msg);
    UpdateProgress(1, 0);
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::FindLocalCoord
//
//  Purpose:
//    Find the coordinats of node specified in queryAtts in domain specified
//    by queryAtts. 
//
//  Returns:
//    true upon succesful location of node and determination of its 
//    coordinates, false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 (moved from method PerformQuery)
//
//  Modifications:
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Kathleen Biagas, Tue Jun 21 10:24:16 PDT 2011
//    Domain, element retrieved in SetInputParams.
//
// ****************************************************************************

bool
avtNodeCoordsQuery::FindLocalCoord(double coord[3])
{
    intVector dlist;

    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    int nodeOrigin  = GetInput()->GetInfo().GetAttributes().GetNodeOrigin();
    int dom         = domain  - blockOrigin;
    int node        = element - nodeOrigin; 
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    dom = (dom < 0 ? 0 : dom);

    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    //
    //  See if any processor is working with this domain.
    //
    intVector dAllProc;
    trav.GetDomainListAllProcs(dAllProc);
    bool domainUsed = false;
    for (int j = 0; j < dAllProc.size() && !domainUsed; j++)
    {
        if (dAllProc[j] == dom)
            domainUsed = true;
    }

    avtOriginatingSource *src = GetInput()->GetOriginatingSource();
    if (domainUsed)
    {
        for (int i = 0; i < dlist.size() && !success; ++i) 
        {
            if (dlist[i] == dom)
            {
                success = src->QueryCoords(var, dom, node, ts, coord, false, false);
            }
        }
    }
    else if (PAR_Rank() == 0)
    {
        success = src->QueryCoords(var, dom, node, ts, coord, false, false);
    }
    return success; 
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::FindGlobalCoord
//
//  Purpose:
//    Find the coordinates of global node specified in queryAtts.  Must search 
//    all domains.
//
//  Returns:
//    true upon succesful location of node and determination of its 
//    coordinates, false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 
//
//  Modifications:
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Kathleen Biagas, Tue Jun 21 10:24:16 PDT 2011
//    Element retrieved in SetInputParams.
//
// ****************************************************************************

bool
avtNodeCoordsQuery::FindGlobalCoord(double coord[3])
{
    int nodeOrigin  = GetInput()->GetInfo().GetAttributes().GetNodeOrigin();
    int node        = element - nodeOrigin; 
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    intVector dlist;
    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    avtOriginatingSource *src = GetInput()->GetOriginatingSource();
    for (int i = 0; i < dlist.size() && !success; ++i) 
    {
        success = src->QueryCoords(var, dlist[i], node, ts, coord, false, true);
    }
    return success;
}

