/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                              avtMinMaxQuery.C                             //
// ************************************************************************* //

#include <avtMinMaxQuery.h>

#include <iomanip>
#include <snprintf.h>
#include <string>
#include <vector>
#include <float.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItUtility.h>

#include <avtCommonDataFunctions.h>
#include <avtMaterial.h>
#include <avtMatrix.h>
#include <avtMetaData.h>
#include <avtMixedVariable.h>
#include <avtParallel.h>
#include <avtQueryableSource.h>
#include <avtTerminatingSource.h>

#include <NonQueryableInputException.h>
#include <DebugStream.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtMinMaxQuery::avtMinMaxQuery
//
//  Purpose:
//      Construct an avtMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004
//    Added args, which control whether we do the Min or Max (or both).
//
//    Kathleen Bonnell, Tue Jul  6 17:05:42 PDT 2004 
//    Init minInfo1/2, maxInfo1/2, nodeMsg1/2 and zoneMsg1/2.
//
// ****************************************************************************

avtMinMaxQuery::avtMinMaxQuery(bool domin, bool domax)
{
    dimension = 3;
    topoDim = 2;
    blockOrigin = 0;
    cellOrigin = 0;
    invTransform = NULL;
    singleDomain = true;
    doMin = domin;
    doMax = domax;

    nodeMsg1 = "(over all nodes, even those not incident to a zone on the mesh)";
    nodeMsg2 = "(over only those nodes incident to a zone on the mesh)";
    zoneMsg1 = "(using only per-zone quantities)";
    zoneMsg2 = "(using per-material zonal quantities)";

    minInfo1.Initialize(FLT_MAX, "Min");
    minInfo2.Initialize(FLT_MAX, "Min");
    maxInfo1.Initialize(-FLT_MAX, "Max");
    maxInfo2.Initialize(-FLT_MAX, "Max");
}

// ****************************************************************************
//  Method: avtMinMaxQuery::~avtMinMaxQuery
//
//  Purpose:
//      Destruct an avtMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004
//    Delete invTransform.
//
// ****************************************************************************

avtMinMaxQuery::~avtMinMaxQuery()
{
    if (invTransform != NULL)
    {
        delete invTransform;
        invTransform = NULL;
    }
}


// ****************************************************************************
//  Method: avtMinMaxQuery::VerifyInput
//
//  Purpose:
//    Verify a new input.  Overrides base class in order to allow vectors
//    (topo dim == 0) to be queried.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 19, 2003 
//
// ****************************************************************************

void
avtMinMaxQuery::VerifyInput()
{
    if (!GetInput()->GetInfo().GetValidity().GetQueryable())
    {
        EXCEPTION0(NonQueryableInputException);
    }
}

// ****************************************************************************
//  Method: avtMinMaxQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//    Retrieves the correct spatial dimension, and resets certain values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  6 17:05:42 PDT 2004 
//    Init minInfo1/2, maxInfo1/2.
//
// ****************************************************************************

void
avtMinMaxQuery::PreExecute()
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    dimension = data.GetSpatialDimension();
    topoDim = data.GetTopologicalDimension();
    blockOrigin = data.GetBlockOrigin();
    cellOrigin = data.GetCellOrigin();

    minMsg = "No Information Found";
    maxMsg = "No Information Found";
    elementName = "";

    minInfo1.Initialize(FLT_MAX, "Min");
    minInfo2.Initialize(FLT_MAX, "Min");

    maxInfo1.Initialize(-FLT_MAX, "Max");
    maxInfo2.Initialize(-FLT_MAX, "Max");
}


// ****************************************************************************
//  Method: avtMinMaxQuery::Execute
//
//  Purpose:
//    Retrieves var information from the dataset, based on a domain and zone.
//
//  Arguments:
//    ds          The input dataset.
//    dom         The domain number.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003
//    Test for ghost zones. Changed min/max val check to <= or >= so that
//    serial and parallel versions will always return the same results.
//
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004 
//    Only check min/or max if they are set to be done. 
//
//    Kathleen Bonnell, Thu May  6 17:36:43 PDT 2004 
//    If working with OriginalData, or zones have been preserved, use the
//    zone number found here, rather than querying the database for it.
//
//    Kathleen Bonnell, Tue Jul  6 17:05:42 PDT 2004 
//    Reworked to store results in MinMaxInfo members.  Added ability
//    to return multiple min/max results:  
//        nodal:  connected geometry only, entire mesh
//        zonal:  per-zone quantities, per-material zonal quantities.
//    
//    Kathleen Bonnell, Tue Jul 27 09:53:01 PDT 2004 
//    Store the value per material, even if not mixed. 
//
//    Kathleen Bonnell, Thu Aug 26 10:22:00 PDT 2004 
//    Changed min/max val check to <= or >= so that serial and parallel 
//    versions will always return the same results. 
//    (ThisProcessorHasMinimum/MaximumValue has changed).
//
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Fri Jan  7 15:15:32 PST 2005 
//    Fix memory leak -- delete cellIds. 
//
//    Hank Childs, Thu Mar 10 11:53:28 PST 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Wed Apr 27 08:29:52 PDT 2005 
//    Modified ghost tests to account for ghost nodes. 
//
//    Hank Childs, Tue Aug 30 15:24:00 PDT 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Mon Jul 31 08:19:38 PDT 2006 
//    Curves now respresented as 1D RectilinearGrids. 
//
// ****************************************************************************

void 
avtMinMaxQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }
    int i;
    vtkUnsignedCharArray *ghostZones = 
           (vtkUnsignedCharArray*)ds->GetCellData()->GetArray("avtGhostZones");
    vtkUnsignedCharArray *ghostNodes = 
           (vtkUnsignedCharArray*)ds->GetPointData()->GetArray("avtGhostNodes");
    vtkDataArray *data = NULL;
    string var = queryAtts.GetVariables()[0];
    int varType = queryAtts.GetVarTypes()[0];
    int ts = queryAtts.GetTimeStep();
    scalarCurve = false;
    bool checkGhost = (ghostZones != NULL || ghostNodes != NULL);
    bool haveMin1 = false;
    bool haveMin2 = false;
    bool haveMax1 = false;
    bool haveMax2 = false;

    int domain = (dom < 0 ? 0 : dom);
    if ((data = ds->GetPointData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = true;
        elementName = "node";
    }
    else if ((data = ds->GetCellData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = false;
        elementName = "zone";
    }
    else if (varType == QueryAttributes::Curve) 
    {
        data = ds->GetPointData()->GetScalars();
        nodeCentered = true;
        elementName = "node";
    }
    else if ((varType == QueryAttributes::Scalar) && (topoDim == 1))
    {
        //
        //  This allows Lineouts to be queried for minMax.
        //
        data = ds->GetPointData()->GetScalars();
        nodeCentered = true;
        elementName = "node";
        scalarCurve = true;
    }
    else 
    {
        debug5 << "avtMinMaxQuery could not find a vtkDataArray"
               << " associated with var " << var.c_str() << endl;
        return;    
    }

    double val;
    double x[3], x9[9];
    bool zonesPreserved = GetInput()->GetInfo().GetValidity().GetZonesPreserved();

    avtMaterial *mat = NULL;
    vector<CellMatInfo> matInfo;
    avtMixedVariable *mv = NULL;
    if (OriginalData() && !nodeCentered)
    {
        avtMetaData *md = GetInput()->GetTerminatingSource()->GetMetaData();
        mv = md->GetMixedVar(domain, ts);
        if (mv != NULL)
        {
            mat = md->GetMaterial(domain, ts);
        } 
    }

    stringVector matNames;
    floatVector matValues;
    vtkIdList *cellIds = vtkIdList::New();
    vtkIdList *ids = vtkIdList::New();

    for (int elNum = 0; elNum < data->GetNumberOfTuples(); elNum++)
    {
        switch(varType)
        {
            case QueryAttributes::Vector :
                data->GetTuple(elNum, x);
                val = sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
                break; 
            case QueryAttributes::Tensor :
            case QueryAttributes::Symmetric_Tensor :
                data->GetTuple(elNum, x9);
                val = MajorEigenvalue(x9);
                break; 
            case QueryAttributes::Scalar :
            case QueryAttributes::Curve :
            default:
                val = data->GetComponent(elNum, 0);
                break; 
        }
        bool ghost = false;
        if (nodeCentered)
        {
            if (checkGhost)
            {
                if (ghostNodes != NULL)
                {
                    ghost = (ghostNodes->GetValue(elNum) > 0);
                }
                else 
                {
                    ds->GetPointCells(elNum, ids);
                    int numGhostCells = 0; 
                    for (int i = 0; i < ids->GetNumberOfIds(); i++)
                        numGhostCells += 
                          ghostZones->GetValue(ids->GetId(i)) > 0 ?  1 : 0;
                    ghost = numGhostCells == ids->GetNumberOfIds();
                }
            }
 
            if (doMin && !ghost) 
            {
                if (val < minInfo1.GetValue())
                {
                    haveMin1 = true;
                    minInfo1.SetElementNum(elNum);
                    minInfo1.SetValue(val);
                    minInfo1.SetDomain(domain);
                }
                ds->GetPointCells(elNum, cellIds);
                if (cellIds->GetNumberOfIds() > 0  && 
                    val < minInfo2.GetValue())
                {
                    haveMin2 = true;
                    minInfo2.SetElementNum(elNum);
                    minInfo2.SetValue(val);
                    minInfo2.SetDomain(domain);
                }
                cellIds->Reset();
            }
            if (doMax && !ghost) 
            {
                if (val > maxInfo1.GetValue())
                {
                    haveMax1 = true;
                    maxInfo1.SetElementNum(elNum);
                    maxInfo1.SetValue(val);
                    maxInfo1.SetDomain(domain);
                }
                ds->GetPointCells(elNum, cellIds);
                if (cellIds->GetNumberOfIds() > 0  && 
                    val > maxInfo2.GetValue())
                {
                    haveMax2 = true;
                    maxInfo2.SetElementNum(elNum);
                    maxInfo2.SetValue(val);
                    maxInfo2.SetDomain(domain);
                }
                cellIds->Reset();
            }
        }
        else // zoneCentered
        {
            if (checkGhost)
            {
                if (ghostZones != NULL)
                {
                    ghost = (ghostZones->GetValue(elNum) > 0);
                }
                else 
                {
                    ds->GetCellPoints(elNum, ids);
                    int numGhostNodes = 0; 
                    for (int i = 0; i < ids->GetNumberOfIds(); i++)
                        numGhostNodes += 
                          ghostNodes->GetValue(ids->GetId(i)) > 0 ?  1 : 0;
                    ghost = numGhostNodes > 0;
                }
            }
 
            if (!ghost && mat != NULL && elNum >= 0 && elNum < mat->GetNZones())
            {
                matInfo = mat->ExtractCellMatInfo(elNum);
                for (i = 0; i < matInfo.size(); ++i)
                {
                    matNames.push_back(matInfo[i].name);
                    if (matInfo[i].mix_index != -1)
                    {
                        matValues.push_back(mv->GetBuffer()[matInfo[i].mix_index]);
                    }
                    else 
                    {
                        matValues.push_back(val);
                    }
                }
                matInfo.clear();
            }

            if (doMin && !ghost)
            {
                if (val < minInfo1.GetValue())
                {
                    haveMin1 = true;
                    minInfo1.SetElementNum(elNum);
                    minInfo1.SetValue(val);
                    minInfo1.SetDomain(domain);
                }
                for (i = 0; i < matValues.size(); i++)
                {
                    if (matValues[i] < minInfo2.GetValue())
                    {
                        haveMin2 = true;
                        minInfo2.SetElementNum(elNum);
                        minInfo2.SetValue(matValues[i]);
                        minInfo2.SetDomain(domain);
                        minInfo2.SetMatName(matNames[i]);
                    }
                }
            }
            if (doMax && !ghost)
            {
                if (val > maxInfo1.GetValue()) 
                {
                    haveMax1 = true;
                    maxInfo1.SetElementNum(elNum);
                    maxInfo1.SetValue(val);
                    maxInfo1.SetDomain(domain);
                }
                for (i = 0; i < matValues.size(); i++)
                {
                    if (matValues[i] > maxInfo2.GetValue())
                    {
                        haveMax2 = true;
                        maxInfo2.SetElementNum(elNum);
                        maxInfo2.SetValue(matValues[i]);
                        maxInfo2.SetDomain(domain);
                        maxInfo2.SetMatName(matNames[i]);
                    }
                }
            }
            if (!matNames.empty())
                matNames.clear();
            if (!matValues.empty())
                matValues.clear();
        } 
    }
    cellIds->Delete();
    ids->Delete();

    if (nodeCentered)
    {
        if (haveMin1)
            FinalizeNodeCoord(ds, minInfo1); 
   
        if (haveMin2)
            FinalizeNodeCoord(ds, minInfo2); 

        if (haveMax1)
            FinalizeNodeCoord(ds, maxInfo1); 
   
        if (haveMax2)
            FinalizeNodeCoord(ds, maxInfo2); 
    }
    else // zoneCentered
    {
        vtkDataArray *origCells = 
                     ds->GetCellData()->GetArray("avtOriginalCellNumbers"); 

        if (haveMin1)
            FinalizeZoneCoord(ds, origCells, minInfo1, zonesPreserved);

        if (haveMin2)
            FinalizeZoneCoord(ds, origCells, minInfo2, zonesPreserved);

        if (haveMax1)
            FinalizeZoneCoord(ds, origCells, maxInfo1, zonesPreserved);

        if (haveMax2)
            FinalizeZoneCoord(ds, origCells, maxInfo2, zonesPreserved);
    }
}



// ****************************************************************************
//  Method: avtMinMaxQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 19 13:48:53 PST 2003
//    Change the order of the hasMin/hasMax tests so that it works correctly in
//    parallel when one of the processors hsas no data.
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Kathleen Bonnell, Tue Jul  6 17:05:42 PDT 2004 
//    Removed MPI calls, use methods from avtParallel instead.  Reworked as
//    result values now stored in MinMaxInfo objects. 
//
// ****************************************************************************

void
avtMinMaxQuery::PostExecute(void)
{
    int hasMin1 = 0, hasMax1 = 0; 
    int hasMin2 = 0, hasMax2 = 0; 

    hasMin1 = (ThisProcessorHasMinimumValue(minInfo1.GetValue()) && 
               minInfo1.GetValue() != FLT_MAX);
    if (hasMin1)
    {
        minInfo1.TransformCoord(invTransform);
        FindElement(minInfo1);
    }

    hasMax1 = (ThisProcessorHasMaximumValue(maxInfo1.GetValue()) && 
               maxInfo1.GetValue() != -FLT_MAX);
    if (hasMax1)
    {
        maxInfo1.TransformCoord(invTransform);
        FindElement(maxInfo1);
    }

    hasMin2 = (ThisProcessorHasMinimumValue(minInfo2.GetValue()) && 
               minInfo2.GetValue() != FLT_MAX);
    if (hasMin2)
    {
        minInfo2.TransformCoord(invTransform);
        FindElement(minInfo2);
    }

    hasMax2 = (ThisProcessorHasMaximumValue(maxInfo2.GetValue()) && 
               maxInfo2.GetValue() != -FLT_MAX);
    if (hasMax2)
    {
        maxInfo2.TransformCoord(invTransform);
        FindElement(maxInfo2);
    }

    GetAttToRootProc(minInfo1, hasMin1);
    GetAttToRootProc(minInfo2, hasMin2);
    GetAttToRootProc(maxInfo1, hasMax1);
    GetAttToRootProc(maxInfo2, hasMax2);
    if (PAR_Rank() == 0)
    {
        int nMin = 0, nMax = 0;
        if (minInfo1.GetElementNum() != -1)
            nMin++;
        if ((minInfo2.GetElementNum() != -1) && 
            (!minInfo1.EquivalentForOutput(minInfo2)))
            nMin++;
        if (maxInfo1.GetElementNum() != -1)
            nMax++;
        if ((maxInfo2.GetElementNum() != -1) && 
            (!maxInfo1.EquivalentForOutput(maxInfo2)))
            nMax++;

        nMin = (nMin == 0 ? nMin : (nMax > nMin ? nMax : nMin));
        nMax = (nMax == 0 ? nMax : (nMin > nMax ? nMin : nMax));
       
        doubleVector resVals;
        CreateMessage(nMin, minInfo1, minInfo2, minMsg, resVals);
        CreateMessage(nMax, maxInfo1, maxInfo2, maxMsg, resVals);
        CreateResultMessage((nMin > nMax ? nMin : nMax));
        SetResultValues(resVals);
    }
}


// ****************************************************************************
//  Method: avtMinMaxQuery::Preparation
//
//  Purpose:
//    Preforms preparation tasks common to all derived types. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  1 15:26:10 PDT 2004
//    avtDataAttributes now carries two transforms, use the InvTransform.
//
//    Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004 
//    Allocate storage for invTransform. 
//
// ****************************************************************************

void
avtMinMaxQuery::Preparation(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()
        ->GetGeneralPipelineSpecification()->GetDataSpecification();

    src = inData->GetQueryableSource();

    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);
    if (dlist.size() == 1 && dspec->UsesAllDomains())
        singleDomain = true;
    else 
        singleDomain = false;

    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    if (inAtts.HasInvTransform() && inAtts.GetCanUseInvTransform())
    {
        invTransform = new avtMatrix(*(inAtts.GetInvTransform()));
    }
    else 
    {
        invTransform = NULL;
    }
}


// ****************************************************************************
//  Method: avtMinMaxQuery::GetNodeCoord
//
//  Purpose:
//    Retrieves the coordinate for the specified node id. 
//
//  Arguments:
//    ds        The dataset from which to retrieve the coordinate.
//    id        The id for the node.
//    coord     A place to store the node's coordinates.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtMinMaxQuery::GetNodeCoord(vtkDataSet *ds, const int id, double coord[3])
{
    ds->GetPoint(id, coord);
}


// ****************************************************************************
//  Method: avtMinMaxQuery::GetCellCoord
//
//  Purpose:
//    Retrieves the coordinate (cell center) for the specified cell id.
//
//  Arguments:
//    ds        The dataset from which to retrieve the coordinate.
//    id        The id for the node.
//    coord     A place to store the cell's coordinates.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 20 17:10:21 PDT 2004
//    Use vtkVisItUtility method to compute cell center.
//
// ****************************************************************************

void
avtMinMaxQuery::GetCellCoord(vtkDataSet *ds, const int id, double coord[3])
{
    vtkVisItUtility::GetCellCenter(ds->GetCell(id), coord);
}


// ****************************************************************************
//  Method: avtMinMaxQuery::CreateResultMessage
//
//  Purpose:
//    Concatenates the Min and Max messages.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004
//    Modified so that either Min Or Max could be done separately.
//
// ****************************************************************************

void
avtMinMaxQuery::CreateResultMessage(const int n)
{
    string msg = "\n";
    if (doMin)
    {
        if (doMax)
        {
            if (n > 1)
                msg += minMsg + "\n\n" + maxMsg + "\n\n";
            else 
                msg += minMsg + "\n"   + maxMsg + "\n\n";

        }
        else 
            msg += minMsg + "\n\n";
    }
    else 
    {
        msg += maxMsg + "\n\n";
    }
    SetResultMessage(msg);
}


// ****************************************************************************
//  Method: avtMinMaxQuery::InfoToString
//
//  Purpose:
//    Creates a string from the passed info.
//
//  Arguments:
//    info      The MinMaxInfo to use in creating the string.
//   
//  Returns 
//    A string formatted for output to user.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 1, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri May 13 13:45:58 PDT 2005
//    Fix memory leak.
//
// ****************************************************************************

string 
avtMinMaxQuery::InfoToString(const MinMaxInfo &info)
{
    ostrstream os;
    int elNum = info.GetElementNum();

    if (!nodeCentered)
        elNum += cellOrigin;
  
    os.setf(ios::fixed);
    os.setf(ios::showpoint);
 
    os << info.GetValue() << " (" << elementName.c_str() << " " << elNum << " ";


    if (info.GetMatName() != "NO_MAT")
        os << "for material " << info.GetMatName().c_str() << " ";

    if (!singleDomain)
    {
        string domainName;
        src->GetDomainName(queryAtts.GetVariables()[0], queryAtts.GetTimeStep(), 
                           info.GetDomain(), domainName);
     
        if (domainName.size() > 0)
        { 
            os << "in " << domainName.c_str() << " " ;
        }
        else 
        { 
            os << "in domain " <<  info.GetDomain()+blockOrigin << " ";
        }
    }

    os << "at coord <";

    const double *c = info.GetCoord();
    if (queryAtts.GetVarTypes()[0] == QueryAttributes::Curve || scalarCurve)
    { 
        os << c[0];
    }
    else if (dimension == 2 && !invTransform)
    {
        os << c[0] << ", " << c[1];
    }
    else
    {
        os << c[0] << ", " << c[1] << ", " << c[2];
    }
    os << ">)" << ends;
    string str(os.str());
#if !defined(_WIN32)
    os.freeze(false);
#endif
    return str;
}


// ****************************************************************************
//  Method: avtMinMaxQuery::CreateMessage
//
//  Purpose:
//    Creates strings from passed info.
//
//  Arguments:
//    nMsg      The number of info's to use in creating the string.
//    info1     One MinMaxInfo.
//    info2     Another MinMaxInfo.
//    msg       A place to store the string.
//    vals      A place to store the values.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 1, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtMinMaxQuery::CreateMessage(const int nMsg, const MinMaxInfo &info1,
                              const MinMaxInfo &info2, string &msg,
                              doubleVector &vals) 
{
    if (nMsg == 0) 
        return;

    string var = queryAtts.GetVariables()[0]; 

    if (nMsg == 1)
    {
        msg =  var + " -- " + info1.GetType() + " = ";
        msg += InfoToString(info1);
        vals.push_back(info1.GetValue());
    }
    else 
    {
        if (nodeCentered)
        {
            msg =  var + " -- " + info1.GetType() + " " + nodeMsg1;
            msg +=  "\n           = ";
            msg += InfoToString(info1);
            msg =  msg + "\n" + var + " -- " + info2.GetType() + " " + nodeMsg2; 
            msg +=  "\n           = ";
            msg += InfoToString(info2);
            vals.push_back(info1.GetValue());
        }
        else 
        {
            msg =  var + " -- " + info1.GetType() + " " + zoneMsg1;
            msg +=  "\n           = ";
            msg += InfoToString(info1);
            msg =  msg + "\n" + var + " -- " + info2.GetType() + " " + zoneMsg2;
            msg +=  "\n           = ";
            msg += InfoToString(info2);
            vals.push_back(info2.GetValue());
        }
    }
}


// ****************************************************************************
//  Method: avtMinMaxQuery::FinalizeNodeCoord
//
//  Purpose:
//    Determines the node coordinate for the passed info.
//
//  Arguments:
//    info      The info that needs to be updated. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 1, 2004 
//
//  Modifications:
//
// ****************************************************************************

void  
avtMinMaxQuery::FinalizeNodeCoord(vtkDataSet *ds, MinMaxInfo &info)
{
    double *c = info.GetCoord();
    GetNodeCoord(ds, info.GetElementNum(), c);
    info.SetCoord(c);
    //
    // Indicate that the db needs to supply the correct
    // node number.
    //
    if (!scalarCurve && !OriginalData())
        info.SetElementNum(-1);
} 


// ****************************************************************************
//  Method: avtMinMaxQuery::FinalizeZoneCoord
//
//  Purpose:
//    Determines the zone coordinate, and resets the element number if 
//    appropriate. 
// 
//  Arguments: 
//    oCells    An array containinng original cell numbers (may be NULL)
//    info      The info that needs to be updated. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 1, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtMinMaxQuery::FinalizeZoneCoord(vtkDataSet *ds, vtkDataArray *oCells, 
                               MinMaxInfo &info, bool zonesPreserved)
{
    int comp = -1;

    if (oCells)
        comp  = oCells->GetNumberOfComponents() -1;

    int elNum = info.GetElementNum();
    double *c = info.GetCoord();
    GetCellCoord(ds, elNum, c);

    if (oCells)
    {
        elNum =(int)oCells->GetComponent(elNum, comp);
    }
    else 
    {
        //
        // Indicate that the db needs to supply the correct
        // cell number.
        //
        if (!scalarCurve && !OriginalData() && !zonesPreserved) 
            elNum = -1;
    }
    info.SetElementNum(elNum);
} 


// ****************************************************************************
//  Method: avtMinMaxQuery::FindElement
//
//  Purpose:
//    Queries the database for the element number associated with
//    the coord of MinMaxInfo.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 1, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtMinMaxQuery::FindElement(MinMaxInfo &info)
{
    int elNum = info.GetElementNum();
    if (elNum == -1)
    {
        string var = queryAtts.GetVariables()[0];
        int ts = queryAtts.GetTimeStep();
        double *c = info.GetCoord(); 
        src->FindElementForPoint(var.c_str(), ts, info.GetDomain(), 
                                 elementName.c_str(), c, elNum); 
        info.SetElementNum(elNum);
        info.SetCoord(c);
    }
}
