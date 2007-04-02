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
//                             avtPointAttribute.C                           //
// ************************************************************************* //

#include <avtPointAttribute.h>

#include <float.h>
#include <stdio.h>

#include <vtkCell.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtParallel.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtPointAttribute constructor
//
//  Arguments:
//      p       The point attributes.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

avtPointAttribute::avtPointAttribute(Point &p) : attributes(p)
{
    point[0] = point[1] = point[2] = 0.;
    resolvedAttributes = false;
}


// ****************************************************************************
//  Method: avtPointAttribute destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPointAttribute::~avtPointAttribute()
{
    ;
}


// ****************************************************************************
//  Method: avtPointAttribute::GetPoint
//
//  Purpose:
//      Gets the resolved point.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtPointAttribute::GetPoint(double p[3])
{
    if (!resolvedAttributes)
    {
        EXCEPTION0(ImproperUseException);
    }

    p[0] = point[0];
    p[1] = point[1];
    p[2] = point[2];
}


// ****************************************************************************
//  Method: avtPointAttribute::PerformRestriction
//
//  Purpose:
//      Performs a restriction on the pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 13:12:59 PST 2002
//    I removed a semi-colon that should not have been there.
//
// ****************************************************************************

avtPipelineSpecification_p
avtPointAttribute::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = spec;

    int type = attributes.GetValueType();
    if (type == Point::VT_Min || type == Point::VT_Max)
    {
        char varname[1024];
        if (type == Point::VT_Min)
        {
            attributes.GetMinArgs(varname);
        }
        else
        {
            attributes.GetMaxArgs(varname);
        }
        if (strcmp(varname, "") != 0)
        {
            avtDataSpecification_p ds = rv->GetDataSpecification();
            const char *curVar = ds->GetVariable();
            if (strcmp(varname, curVar) != 0)
            {
                ds->AddSecondaryVariable(varname);
            }
            origVar = string(curVar);
        }
    }

    if (type == Point::VT_Zone || Point::VT_DomainZone)
    {
        rv->GetDataSpecification()->NeedZoneNumbers();
    }

    if (type != Point::VT_Tuple)
    {
        rv->NoDynamicLoadBalancing();
    }

    return rv;
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolveAttributes
//
//  Purpose:
//      Resolves the point attribute.
//
//  Arguments:
//      in_do      The input data object (assumed to be a dataset).
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtPointAttribute::ResolveAttributes(avtDataObject_p in_do)
{
    switch (attributes.GetValueType())
    {
      case Point::VT_Tuple:
        ResolvePoint();
        break;
      case Point::VT_Min:
        ResolveMin(in_do);
        break;
      case Point::VT_Max:
        ResolveMax(in_do);
        break;
      case Point::VT_Zone:
      case Point::VT_DomainZone:
        ResolveZone(in_do);
        break;
      case Point::VT_Node:
      case Point::VT_DomainNode:
        ResolveNode(in_do);
        break;
      case Point::VT_Unknown:
        EXCEPTION0(ImproperUseException);
    }

    resolvedAttributes = true;
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolvePoint
//
//  Purpose:
//      Resolves a point attribute when the parse type is a point.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtPointAttribute::ResolvePoint(void)
{
    attributes.GetTupleArgs(point[0], point[1], point[2]);
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolveMin
//
//  Purpose:
//      Resolves a point attribute when the parse type is for a minimum value.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 14:38:03 PST 2002
//    Changed CopyTo to a template function.
//
// ****************************************************************************

void
avtPointAttribute::ResolveMin(avtDataObject_p &in_do)
{
    avtDataset_p ds;
    CopyTo(ds, in_do);

    char varname[1024];
    attributes.GetMinArgs(varname);
    if (strcmp(varname, "") != 0)
    {
        ds->SetActiveVariable(varname);
    }

    double value;
    avtDatasetExaminer::FindMinimum(ds, point, value);

    if (strcmp(varname, "") != 0)
    {
        ds->SetActiveVariable(origVar.c_str());
    }

    //
    // Now do a parallel unification.
    //
    // This buffer is pretty much set up as a hack that uses existing functions
    // to unify minimum and maximum values -- we will only use the minimum
    // part as a way to broadcast a message.
    //
    double buff[6];
    buff[0] = DBL_MAX;
    buff[2] = DBL_MAX;
    buff[4] = DBL_MAX;
    if (ThisProcessorHasMinimumValue(value))
    {
        buff[0] = point[0];
        buff[2] = point[1];
        buff[4] = point[2];
    }
    UnifyMinMax(buff, 6);

    point[0] = buff[0];
    point[1] = buff[2];
    point[2] = buff[4];
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolveMax
//
//  Purpose:
//      Resolves a point attribute when the parse type is for a maximum value.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 14:38:03 PST 2002
//    Changed CopyTo to a template function.
//
// ****************************************************************************

void
avtPointAttribute::ResolveMax(avtDataObject_p &in_do)
{
    avtDataset_p ds;
    CopyTo(ds, in_do);

    char varname[1024];
    attributes.GetMaxArgs(varname);
    if (strcmp(varname, "") != 0)
    {
        ds->SetActiveVariable(varname);
    }

    double value;
    avtDatasetExaminer::FindMaximum(ds, point, value);

    if (strcmp(varname, "") != 0)
    {
        ds->SetActiveVariable(origVar.c_str());
    }

    //
    // Now do a parallel unification.
    //
    // This buffer is pretty much set up as a hack that uses existing functions
    // to unify minimum and maximum values -- we will only use the minimum
    // part as a way to broadcast a message.
    //
    double buff[6];
    buff[0] = DBL_MAX;
    buff[2] = DBL_MAX;
    buff[4] = DBL_MAX;
    if (ThisProcessorHasMinimumValue(-value))
    {
        buff[0] = point[0];
        buff[2] = point[1];
        buff[4] = point[2];
    }
    UnifyMinMax(buff, 6);

    point[0] = buff[0];
    point[1] = buff[2];
    point[2] = buff[4];
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolveZone
//
//  Purpose:
//      Determines the point corresponding to a zone.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 4 14:42:27 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Hank Childs, Sun Jun 16 21:08:50 PDT 2002
//    Account for non 0-origin cells and blocks.
//
// ****************************************************************************

void
avtPointAttribute::ResolveZone(avtDataObject_p &in_do)
{
    avtDataset_p ds;
    CopyTo(ds, in_do);

    int blockOrigin = ds->GetInfo().GetAttributes().GetBlockOrigin();
    int cellOrigin  = ds->GetInfo().GetAttributes().GetCellOrigin();
    int domain, zone;
    if (attributes.GetValueType() == Point::VT_Zone)
    {
        attributes.GetZoneArgs(zone);
        domain = 0;
        zone -= cellOrigin;
    }
    else
    {
        attributes.GetDomainZoneArgs(domain, zone);
        domain -= blockOrigin;
        zone   -= cellOrigin;
    }

    point[0] = DBL_MAX;
    point[1] = DBL_MAX;
    point[2] = DBL_MAX;
    bool success = avtDatasetExaminer::FindZone(ds, domain, zone, point);

    double buff[6];
    if (success)
    {
        buff[0] = point[0];
        buff[2] = point[1];
        buff[4] = point[2];
    }
    else
    {
        buff[0] = DBL_MAX;
        buff[2] = DBL_MAX;
        buff[4] = DBL_MAX;
    }
    
    // This buffer is pretty much set up as a hack that uses existing functions
    // to unify minimum and maximum values -- we will only use the minimum
    // part as a way to broadcast a message.
    //
    UnifyMinMax(buff, 6);

    if (buff[0] == DBL_MAX)
    {
        point[0] = 0.;
        point[1] = 0.;
        point[2] = 0.;
        char warning[1024];
        sprintf(warning, "Was not able to locate domain %d, zone %d, using "
                         "point (0., 0., 0.)", domain+blockOrigin, 
                                               zone+cellOrigin);
        avtCallback::IssueWarning(warning);
    }
    else
    {
        point[0] = buff[0];
        point[1] = buff[2];
        point[2] = buff[4];
    }
}


// ****************************************************************************
//  Method: avtPointAttribute::ResolveNode
//
//  Purpose:
//      Determines the point corresponding to a node.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Brad Whitlock, Thu Apr 4 14:43:49 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Hank Childs, Sun Jun 16 21:08:50 PDT 2002
//    Account for non 0-origin blocks.
//
// ****************************************************************************

void
avtPointAttribute::ResolveNode(avtDataObject_p &in_do)
{
    avtDataset_p ds;
    CopyTo(ds, in_do);

    int domain, node;
    int blockOrigin = ds->GetInfo().GetAttributes().GetBlockOrigin();
    if (attributes.GetValueType() == Point::VT_Node)
    {
        attributes.GetNodeArgs(node);
        domain = 0;
    }
    else
    {
        attributes.GetDomainNodeArgs(domain, node);
        domain -= blockOrigin;
    }

    point[0] = DBL_MAX;
    point[1] = DBL_MAX;
    point[2] = DBL_MAX;
    bool success = avtDatasetExaminer::FindNode(ds, domain, node, point);

    double buff[6];
    if (success)
    {
        buff[0] = point[0];
        buff[2] = point[1];
        buff[4] = point[2];
    }
    else
    {
        buff[0] = DBL_MAX;
        buff[2] = DBL_MAX;
        buff[4] = DBL_MAX;
    }
    
    // This buffer is pretty much set up as a hack that uses existing functions
    // to unify minimum and maximum values -- we will only use the minimum
    // part as a way to broadcast a message.
    //
    UnifyMinMax(buff, 6);

    if (buff[0] == DBL_MAX)
    {
        point[0] = 0.;
        point[1] = 0.;
        point[2] = 0.;
        char warning[1024];
        sprintf(warning, "Was not able to locate domain %d, node %d, using "
                         "point (0., 0., 0.)", domain+blockOrigin, node);
        avtCallback::IssueWarning(warning);
    }
    else
    {
        point[0] = buff[0];
        point[1] = buff[2];
        point[2] = buff[4];
    }
}


