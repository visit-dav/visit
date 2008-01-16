/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                      avtDatasetToSamplePointsFilter.C                     //
// ************************************************************************* //

#include <avtDatasetToSamplePointsFilter.h>

#include <avtDatasetExaminer.h>
#include <avtParallel.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToSamplePointsFilter::avtDatasetToSamplePointsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToSamplePointsFilter::~avtDatasetToSamplePointsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToSamplePointsFilter::PreExecute
//
//  Purpose:
//      Executes before the avtFilter calls Execute.  The sample points need
//      to know how many variables they will be sampling over before they get
//      going.  This is a chance to set that up.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 09:17:12 PST 2002
//    Add better support for datasets with no variables.
//
//    Hank Childs, Fri Mar 15 17:40:02 PST 2002
//    Account for new avtDatasetExaminer.
//
//    Hank Childs, Wed Jul 23 14:59:55 PDT 2003
//    Account for not sampling VTK and AVT variables.
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
//    Hank Childs, Thu May 31 13:47:16 PDT 2007
//    Add support for non-scalar variables.
//
// ****************************************************************************

void
avtDatasetToSamplePointsFilter::PreExecute(void)
{
    avtDatasetToDataObjectFilter::PreExecute();

    avtDataset_p ds = GetTypedInput();
    VarList vl;
    vl.nvars = -1;
    avtDatasetExaminer::GetVariableList(ds, vl);

    avtSamplePoints_p sp = GetTypedOutput();
    if (vl.nvars <= 0)
    {
        debug1 << "!!! Converting a dataset that has no variables to sample "
               << "points" << endl;
        vl.nvars = 0;
    }

    bool leaveAsIs = false;
    if (vl.nvars == 0 && sp->GetNumberOfVariables() > 0)
    {
        //
        // Someone came in and set the output so that it had more variables --
        // this is common practice if we are executing in parallel and we
        // have more processors than domains.
        //
        debug1 << "!!! The sample points already believed that it had "
               << "variables -- leaving as is." << endl;
        leaveAsIs = true;
    }

    if (!leaveAsIs)
    {
        vector<string> varnames;
        vector<int>    varsize;
        int realNVars = 0;
        for (int i = 0 ; i < vl.nvars ; i++)
        {
            const char *vname = vl.varnames[i].c_str();
            if (strstr(vname, "vtk") != NULL)
                continue;
            if (strstr(vname, "avt") != NULL)
                continue;
            varnames.push_back(vl.varnames[i]);
            varsize.push_back(vl.varsizes[i]);
            realNVars++;
        }

        // Some contortions here to use existing calls in avtParallel.
        int nvars = UnifyMaximumValue(varnames.size());
        GetListToRootProc(varnames, nvars);
        BroadcastStringVector(varnames, PAR_Rank());

        while (varsize.size() < nvars)
            varsize.push_back(0);
        std::vector<int> varsize2(nvars);
        UnifyMaximumValue(varsize, varsize2);

        sp->SetNumberOfVariables(varsize2, varnames);
    }
}


