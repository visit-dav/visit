/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                            avtPythonQuery.C                               //
// ************************************************************************* //

#include <avtPythonQuery.h>

#include <string>
#include <vector>
#include <float.h>
#include <vtkDataSet.h>
#include <avtParallel.h>
#include <avtPythonFilterEnvironment.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <Python.h>
#include <PyContract.h>

// Helper for throwing an exception when things go wrong.
#define PYQUERY_ERROR(msg) \
{  \
  string emsg(msg); \
  string pyerr=""; \
  if(pyEnv->FetchPythonError(pyerr))\
     emsg += "\nPython Environment Error:\n" + pyerr;\
  CleanUp(); \
  EXCEPTION1(VisItException, emsg); \
}

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtPythonQuery::avtPythonQuery
//
//  Purpose:
//      Construct an avtPythonQuery.C object.
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

avtPythonQuery::avtPythonQuery()
: avtDataObjectQuery(), avtDatasetSink(), pyEnv(NULL)
{
    pyEnv = new avtPythonFilterEnvironment();
}

// ****************************************************************************
//  Method: avtPythonQuery::~avtPythonQuery
//
//  Purpose:
//      Destruct an avtPythonQuery object.
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

avtPythonQuery::~avtPythonQuery()
{
    CleanUp();
}

// ****************************************************************************
//  Method: avtPythonExpression::CleanUp
//
//  Purpose:
//      Cleans up the python filter env.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************

void
avtPythonQuery::CleanUp()
{
    if(pyEnv)
    {
        delete pyEnv;
        pyEnv = NULL;
    }
}

// ****************************************************************************
//  Method: avtPythonQuery::GetType
//
//  Purpose:
//      Provide Query Type Name.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
// ****************************************************************************
const char*
avtPythonQuery::GetType()
{
    string res = "";

    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::GetType Error - "
                       "Python filter not initialized.");

    if(py_filter->GetAttribute("name",res))
    {
        if(res != "avtPythonQuery")
            res = "avtPythonQuery(" + res + ")";
    }
    else
        PYQUERY_ERROR("avtPythonQuery::GetType Error - "
                      "fetch of python filter attribute 'name' failed");

    return res.c_str();
}

// ****************************************************************************
//  Method: avtPythonQuery::GetDescription
//
//  Purpose:
//      Provide Query Description.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
// ****************************************************************************
const char*
avtPythonQuery::GetDescription()
{
    // hook to pyavt filter
    string res = "";

    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::GetDescription Error - "
                       "Python filter not initialized.");

    if(!py_filter->GetAttribute("description",res))
        PYQUERY_ERROR("avtPythonQuery::GetDescription Error - "
                      "fetch of python filter attribute 'description' failed");

    return res.c_str();
}

// ****************************************************************************
//  Method: avtPythonQuery::SetPythonScript
//
//  Purpose:
//    Sets the python script source used to setup the python query.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPythonQuery::SetPythonScript(const std::string &py_script)
{
    pyScript = py_script;
}

// ****************************************************************************
//  Method: avtPythonQuery::PerformQuery
//
//  Purpose:
//    Called to actually execute the query.
//
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
// ****************************************************************************
void
avtPythonQuery::PerformQuery(QueryAttributes *qatts)
{
    queryAtts = *qatts;

    if(!pyEnv->Initialize())
        PYQUERY_ERROR("avtPythonQuery::PerformQuery Error - "
                      "Failed to initialize the python filter environment.");

    if(!pyEnv->LoadFilter(pyScript))
        PYQUERY_ERROR("avtPythonQuery::PerformQuery Error - "
                      "Failed to load python filter script.");

    Init();

    UpdateContract();

    UpdateProgress(0, 0);

    avtDataTree_p tree = GetInputDataTree();

    int validInputTree = 0;
    if (*tree != NULL && !tree->IsEmpty())
        validInputTree = 1;
    else 
    {
        validInputTree |= 0;
        debug4 << "avtPythonQuery encountered EMPTY InputDataTree after ApplyFilters.  "
               << "This may be a valid state if running parallel and there "
               << "are more processors than domains." << endl;
    }

    bool hadError = false;
    PreExecute();
    TRY
    {
        Execute();
    }
    CATCH2(VisItException, e)
    {
        debug1 << "Exception occurred in " << GetType() << endl;
        debug1 << "Going to keep going to prevent a parallel hang." << endl;
        queryAtts.SetResultsMessage(e.Message());
        hadError = true;
    }
    ENDTRY

    PostExecute();

    validInputTree = UnifyMaximumValue(validInputTree);

    if (!hadError)
    {
        if (validInputTree)
        {
            //
            // Retrieve the query results and set the message in the atts. 
            //
            queryAtts.SetResultsMessage(resultMessage);
            queryAtts.SetResultsValue(resultValues);
            queryAtts.SetXmlResult(resultXml);
        }
        else
        {
            queryAtts.SetResultsMessage(std::string(GetType()) +
                    " was asked to execute on an empty data set.  The query "
                    "produced the following message: " + resultMessage);
            queryAtts.SetResultsValue(resultValues);
        }
    }

    UpdateProgress(1, 0);
    *qatts = queryAtts;
}


// ****************************************************************************
//  Method: avtPythonQuery::GetSecondaryVariables
//
//  Purpose:
//    Used to obtain the list of secondary variables.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

void 
avtPythonQuery::GetSecondaryVariables(std::vector<std::string> &res)
{
    // varNames contains all vars, fill res w/ varNames[1:]
    res.clear();
    int nvars = varNames.size();
    if(nvars >= 1)
        for(int i =1; i < nvars; i++)
            res.push_back(varNames[i]);

}

// ****************************************************************************
//  Method: avtPythonQuery::UpdateContract
//
//  Purpose:
//    Adds requested seconardy vars, and allows python query filter to
//    modify the contract.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Feb 17 09:30:46 PST 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtPythonQuery::UpdateContract()
{

    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Python filter not initialized.");

    avtDataObject_p dob = GetInput();

    avtContract_p orig_contract = dob->GetOriginatingSource()->
                                        GetGeneralContract();

    avtDataRequest_p orig_drequest = orig_contract->GetDataRequest();
    avtDataRequest_p drequest = new avtDataRequest(orig_drequest,
                                                   querySILR);

    // obtain the list of secondary vars so they can be
    // added to the contract data request.
    vector<string> secondary_vars;
    GetSecondaryVariables(secondary_vars);

    for(int i = 0 ; i < secondary_vars.size(); i++)
        drequest->AddSecondaryVariable(secondary_vars[i].c_str());

    avtContract_p contract = new avtContract(drequest,
                                             queryAtts.GetPipeIndex());

    // allow the python filter to modify the contract

    // hand contract over to the python filter
    PyObject *py_contract = PyContract_Wrap(contract);
    if(py_contract == NULL)
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Failed to wrap the contract for python access.");

    // get the modify_contract method
    PyObject *py_exe= PyString_FromString("modify_contract");
    if(py_exe == NULL)
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Error preparing for call of 'modify_contract' method.");
    
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,
                                                      py_exe,
                                                      py_contract,
                                                      NULL);
    if(py_exe_res == NULL)
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Python Query 'modify_contract' method failed");
    Py_DECREF(py_exe);
    Py_DECREF(py_exe_res);
    Py_DECREF(py_contract);


    avtDataObject_p res;
    CopyTo(res, dob);
    res->Update(contract);

    // Reset the input so that we have access to the data tree.
    SetTypedInput(res);

    // Add primary & secondary vars to the python filter's variable list.

    PyObject *py_vars_list =pyEnv->Filter()->FetchAttribute("input_var_names");
    if(py_vars_list == NULL || !PyList_Check(py_vars_list))
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Failed to fetch 'input_var_names' "
                      "list from python filter");

    // add primary var to input_var_names.
    PyObject *py_txt = PyString_FromString(drequest->GetVariable());
    if(py_txt == NULL || PyList_Append(py_vars_list,py_txt) == -1)
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Unable to add primary variable name to Python filter "
                      "'input_var_names' list.");

    // add secondary vars to input_var_names.
    for(int i = 0 ; i < secondary_vars.size(); i++)
    {
        py_txt = PyString_FromString(secondary_vars[i].c_str());
        if(py_txt == NULL || PyList_Append(py_vars_list,py_txt) == -1)
            PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                      "Unable to add secondary variable name to Python filter "
                      "'input_var_names' list.");
    }

    // set "float_format"
    if(!pyEnv->Filter()->SetAttribute("float_format",queryAtts.GetFloatFormat()))
        PYQUERY_ERROR("avtPythonQuery::UpdateContract Error - "
                  "Unable to set Python Query 'float_format' attribute.");

}


// ****************************************************************************
//  Method: avtPythonQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//    Calls analogous preexecute method in the python filter.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPythonQuery::PreExecute()
{
    // exec pre exe from python filter

    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::PreExecute Error - "
                      "Python filter not initialized.");

    // get the preexecute method
    PyObject *py_exe= PyString_FromString("pre_execute");
    if(py_exe == NULL)
        PYQUERY_ERROR("avtPythonQuery::PreExecute Error - "
                      "Error preparing for call of 'pre_execute' method.");
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,py_exe,NULL);
    if(py_exe_res == NULL)
         PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                      "'pre_execute' call on python filter failed.");

    Py_DECREF(py_exe);
    Py_DECREF(py_exe_res);
}


// ****************************************************************************
//  Method: avtPythonQuery::Execute
//
//  Purpose:
//    Calls python filter execute on datasets.
//
//  Arguments:
//    ds          The input dataset.
//    dom         The domain number.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
//
// ****************************************************************************

void
avtPythonQuery::Execute()
{
    // get input data tree to obtain datasets
    avtDataTree_p tree = GetInputDataTree();

    // holds number of datasets
    int nsets;
    // get datasets
    vtkDataSet **data_sets = tree->GetAllLeaves(nsets);

    // get dataset domain ids
    vector<int> domain_ids;
    tree->GetAllDomainIds(domain_ids);

    // create a tuple holding to hold the datasets
    PyObject *py_dsets  = PyTuple_New(nsets);
    PyObject *py_domids = PyTuple_New(nsets);

    if(py_dsets == NULL || py_domids == NULL)
        PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                       "Unable to create execution input lists");

    // hand data_sets and domain_ids to the python query
    for(int i = 0; i < nsets ; i++)
    {
        if(PyTuple_SetItem(py_dsets,i,pyEnv->WrapVTKObject(data_sets[i],"vtkDataSet")) != 0)
            PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                          "Unable to add data set to execution input list");
        if(PyTuple_SetItem(py_domids,i,PyInt_FromLong(domain_ids[i])) != 0)
            PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                          "Unable to add domain id to execution input list");
    }

    // call execute on Filter
    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                      "Python filter not initialized.");

    // get the execute method
    PyObject *py_exe= PyString_FromString("execute");
    if(py_exe == NULL)
        PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                     "Error preparing for call of 'execute' method.");
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,
                                                      py_exe,
                                                      py_dsets,py_domids,
                                                      NULL);
    if(py_exe_res == NULL)
        PYQUERY_ERROR("avtPythonQuery::Execute Error - "
                      "'execute' call on python query filter failed.");

    Py_DECREF(py_dsets);
    Py_DECREF(py_domids);
    Py_DECREF(py_exe);

}

// ****************************************************************************
//  Method: avtPythonQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    Calls analogous postexecute method in the python filter.
//
//
//  Programmer: Cyrus Harrison
//  Creation:   April 17, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPythonQuery::PostExecute(void)
{
    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
        PYQUERY_ERROR("avtPythonQuery::PostExecute Error - "
                      "Python filter not initialized.");

    // get the preexecute method
    PyObject *py_exe= PyString_FromString("post_execute");
    if(py_exe == NULL)
        PYQUERY_ERROR("avtPythonQuery::PostExecute Error - "
                      "Error preparing for call of 'post_execute' method.");
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,py_exe,NULL);
    if(py_exe_res == NULL)
        PYQUERY_ERROR("avtPythonQuery::PostExecute Error - "
                      "'post_execute' call on python filter failed.");

    Py_DECREF(py_exe);
    Py_DECREF(py_exe_res);

    // check for text result
    string msg;
    if(pyEnv->Filter()->GetAttribute("result_txt",msg))
        SetResultMessage(msg);

    double val;
    vector<double> vals;

    // for numeric results support a single value, or list containing number values.
    PyObject *py_vals = pyEnv->Filter()->FetchAttribute("result_val");
    if(py_vals != NULL && py_vals != Py_None)
    {
            if(PythonInterpreter::PyObjectToDouble(py_vals,val))
                vals.push_back(val);
            else if(PySequence_Check(py_vals) == 1)
            {
                PyObject *py_r_vals = PySequence_Fast(py_vals,"Expected Sequence");
                if(!py_r_vals)
                    PYQUERY_ERROR("avtPythonQuery::PostExecute Error - "
                                 "Unable to obtain result value sequence.");

                int n_r_vals  = PySequence_Size(py_r_vals);
                for(int i = 0; i < n_r_vals ; i++)
                {
                    PyObject *py_item = PySequence_Fast_GET_ITEM(py_r_vals,i); //borrowed
                    if(!py_item)
                        PYQUERY_ERROR("avtPythonQuery::PostExecute Error - "
                                      "Error getting result value entry.");
                    if(PythonInterpreter::PyObjectToDouble(py_item,val))
                        vals.push_back(val);
                }
                Py_DECREF(py_r_vals);
            }
            if(vals.size() > 0)
                SetResultValues(vals);
    }

}

