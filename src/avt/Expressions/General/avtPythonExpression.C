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
//                              avtPythonExpression.C                        //
// ************************************************************************* //
#include <avtPythonExpression.h>
#include <Python.h>

#include <avtExprNode.h>
#include <avtExpressionEvaluatorFilter.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <Utility.h>

#include <vtkDataSet.h>

#include <avtPythonFilterEnvironment.h>
#include <PyContract.h>

// Helper for throwing an Exception Expression when things go wrong.
#define PYEXPR_ERROR(msg) \
{  \
  string emsg(msg); \
  string pyerr=""; \
  if(pyEnv->FetchPythonError(pyerr))\
     emsg += "\nPython Environment Error:\n" + pyerr;\
  CleanUp(); \
  EXCEPTION2(ExpressionException, outputVariableName,emsg);\
}


// ****************************************************************************
//  Method: avtPythonExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
//  Modifications:
//
// ****************************************************************************

avtPythonExpression::avtPythonExpression()
: pyEnv(NULL)
{
    pyEnv = new avtPythonFilterEnvironment();
}


// ****************************************************************************
//  Method: avtPythonExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************

avtPythonExpression::~avtPythonExpression()
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
avtPythonExpression::CleanUp()
{
    if(pyEnv)
    {
        delete pyEnv;
        pyEnv = NULL;
    }
}

// ****************************************************************************
//  Method: avtPythonExpression::GetType
//
//  Purpose:
//      Provide Expression Type Name.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************
const char *
avtPythonExpression::GetType()
{
    string res = "";

    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::GetType Error - "
                    "Python filter not initialized.")

    if(py_filter && py_filter->GetAttribute("name",res))
    {
        if(res != "avtPythonExpression")
            res = "avtPythonExpression(" + res + ")";
    }
    else
        PYEXPR_ERROR("avtPythonExpression::GetType Error - "
                     "fetch of python filter attribute 'name' failed");

    return res.c_str();
}

// ****************************************************************************
//  Method: avtPythonExpression::GetDescription
//
//  Purpose:
//      Provide Expression Description.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************
const char *
avtPythonExpression::GetDescription()
{
    // hook to pyavt filter
    string res = "";

    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::GetDescription Error - "
                     "Python filter not initialized.");


    if(py_filter && !py_filter->GetAttribute("description",res))
        PYEXPR_ERROR("avtPythonExpression::GetDescription Error - "
                      "fetch of python filter attribute 'description' failed");

    return res.c_str();
}

// ****************************************************************************
//  Method: avtPythonExpression::NumVariableArguments
//
//  Purpose:
//      Provides the number of arguments that are variables.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************
int
avtPythonExpression::NumVariableArguments()
{
    int res = 0;
    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::NumVariableArguments Error - "
                     "Python filter not initialized.");

    if(py_filter && !py_filter->GetAttribute("input_num_vars",res))
        PYEXPR_ERROR("avtPythonExpression::NumVariableArguments Error - "
                     "fetch of python filter attribute 'input_num_vars' failed");

    return res;
}

// ****************************************************************************
//  Method: avtPythonExpression::GetVariableDimension
//
//  Purpose:
//      Returns the number of components in the output array.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************

int
avtPythonExpression::GetVariableDimension()
{
    // hook to pyavt filter
    int res = 1;
    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::GetVariableDimension Error - "
                     "Python filter not initialized.");

    if(py_filter && !py_filter->GetAttribute("output_dimension",res))
        PYEXPR_ERROR("avtPythonExpression::GetVariableDimension Error - "
                     "fetch of python filter attribute 'output_dimension' failed");

    return res;
}


// ****************************************************************************
//  Method: avtPythonExpression::IsPointVariable
//
//  Purpose:
//      Returns true if the expression result is nodal, false if zonal.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************

bool
avtPythonExpression::IsPointVariable()
{
    // hook to pyavt filter

    bool res = false;
    // hook to pyavt filter
    avtPythonFilter *py_filter = pyEnv->Filter();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::IsPointVariable Error - "
                     "Python filter not initialized.");

    if(py_filter && !py_filter->GetAttribute("output_is_point_var",res))
        PYEXPR_ERROR("avtPythonExpression::IsPointVariable Error - "
                     "fetch of python filter attribute 'output_is_point_var' failed");

    return false;
}


// ****************************************************************************
//  Method: avtPythonExpression::ProcessArguments
//
//  Purpose:
//      Parses expression arguments.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
// ****************************************************************************

void
avtPythonExpression::ProcessArguments(ArgsExpr *args,
                                      ExprPipelineState *state)
{

    // get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // check for call with improper # of arguments
    if (nargs < 2)
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Python Filter Expression requires at least to input "
                     "arguments:\n"
                     "A variable and a string providing the python filter "
                     "source");

    // the last argument should be the python script defining the filter.
    // attempt to load it first:
    pyScript = "";

    ExprParseTreeNode *last_node =  ((*arguments)[nargs-1])->GetExpr();
    string last_node_type = last_node ->GetTypeName();


    if(last_node_type != "StringConst")
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Last expression argument must be a string constant"
                     "containing a python filter source.");

    pyScript = dynamic_cast<StringConstExpr*>(last_node)->GetValue();
    if(!pyEnv->Initialize())
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Failed to initialize the python filter environment.");

    if(!pyEnv->LoadFilter(pyScript))
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Failed to load python filter script.");

    PyObject *py_vars_list = pyEnv->Filter()->FetchAttribute("input_var_names");
    if(py_vars_list == NULL || !PyList_Check(py_vars_list))
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Failed to fetch 'input_var_names' "
                     "list from python filter");

    PyObject *py_args_list = pyEnv->Filter()->FetchAttribute("arguments");
    if(py_args_list == NULL || !PyList_Check(py_args_list))
        PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Failed to fetch 'arguments' list "
                     "from python filter.");

    // process the remaining arguments
    for(int i=0;i< nargs-1;i++)
    {
        ExprParseTreeNode *node =  ((*arguments)[i])->GetExpr();
        string node_type = node->GetTypeName();
        PyObject *py_arg = NULL;

        if(node_type == "IntegerConst")
        {
            long ival = (long)dynamic_cast<IntegerConstExpr*>(node)->GetValue();
            // create PyInt
            py_arg = PyInt_FromLong(ival);
        }
        else if(node_type == "FloatConst")
        {
            double fval = dynamic_cast<FloatConstExpr*>(node)->GetValue();
            // create PyFloat
            py_arg = PyFloat_FromDouble(fval);
        }
        else if(node_type == "StringConst")
        {
            string sval = dynamic_cast<StringConstExpr*>(node)->GetValue();
            // create Py_STRING
            py_arg = PyString_FromString(sval.c_str());
        }
        else if(node_type == "BooleanConst")
        {
            bool bval = dynamic_cast<BooleanConstExpr*>(node)->GetValue();
            // select either Py_True or Py_False
            if(bval)
               py_arg = Py_True;
            else
               py_arg = Py_False;

            Py_INCREF(py_arg);
        }


        if(py_arg)
        {
            // if it is a constant add it to the filter's arguments list
            // append to the filter's arguments list
            if(PyList_Append(py_args_list,py_arg) == -1)
                PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                     "Unable to add argument value to Python filter "
                     "'arguments' list.");

        }
        else
        {
            // we have a variable or expression, let it build its own filters.
            // add add its name to the variables list

            avtExprNode *expr_node= dynamic_cast<avtExprNode*>(node);
            expr_node->CreateFilters(state);

            if(node_type == "Var")
            {
                PathExpr *path_expr= dynamic_cast<VarExpr*>(node)->GetVar();
                string text =path_expr->GetFullpath();
                PyObject *py_txt = PyString_FromString(text.c_str());
                if(PyList_Append(py_vars_list,py_txt) == -1)
                    PYEXPR_ERROR("avtPythonExpression::ProcessArguments Error - "
                                 "Unable to add variable name to Python filter "
                                 "'input_var_names' list.");
            }
        }
    }
}




// ****************************************************************************
//  Method: avtPythonExpression::Execute
//
//  Purpose:
//      Executes the python filter.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:50:10 PST 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtPythonExpression::Execute()
{
    //
    // the real outputVariableName is not avalaible until well after
    // ProcessArguments is called - set it here so the filter can use the 
    // the proper output name.
    //
    pyEnv->Filter()->SetAttribute("output_var_name",string(outputVariableName));

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
            PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                         "Unable to create execution input lists");

    // array to hold output leaves
    avtDataTree_p *leaves = new avtDataTree_p[nsets];

    // hand data_sets and domain_ids to the python expression
    for(int i = 0; i < nsets ; i++)
    {
        if(PyTuple_SetItem(py_dsets,i,pyEnv->WrapVTKObject(data_sets[i],"vtkDataSet")) != 0)
            PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                         "Unable to add data set to execution input list");
        if(PyTuple_SetItem(py_domids,i,PyInt_FromLong(domain_ids[i])) != 0)
            PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                         "Unable to add domain id to execution input list");

    }

    // call execute on Filter
    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Python filter not initialized.");

    // get the execute method
    PyObject *py_exe= PyString_FromString("execute");
    if(py_exe == NULL)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Error preparing for call of 'execute' method.");
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,
                                                      py_exe,
                                                      py_dsets,py_domids,
                                                      NULL);
    if(py_exe_res == NULL)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Python Expression 'execute' method failed");
    if(PySequence_Check(py_exe_res) == 0)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Python Expression 'execute' method must return a sequence of "
                     "data sets & a sequence of domain_ids");

    Py_DECREF(py_dsets);
    Py_DECREF(py_domids);
    Py_DECREF(py_exe);


    // result should be a list with datasets and domain ids

    PyObject *py_result_dsets  = PySequence_GetItem(py_exe_res,0);
    PyObject *py_result_domids = PySequence_GetItem(py_exe_res,1);

    

    if(py_result_dsets == NULL || PySequence_Check(py_result_dsets) == 0)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Python Expression execute method must return a sequence of "
                     "data sets & a sequence of domain_ids");

    if(py_result_domids == NULL || PySequence_Check(py_result_domids) == 0)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Python Expression execute method must return a sequence of "
                     "data sets & a sequence of domain_ids");

    PyObject *py_r_dsets  = PySequence_Fast(py_result_dsets,"Expected Sequence");
    PyObject *py_r_domids = PySequence_Fast(py_result_domids,"Expected Sequence");

    if(py_r_dsets == NULL || py_r_domids == NULL)
        PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                     "Unable to obtain result sequences.");


    int n_r_dsets  = PySequence_Size(py_r_dsets);
    int n_r_domids = PySequence_Size(py_r_domids);

    // check that the lists are the correct size?

    vector<vtkDataSet*> res_dsets;
    vector<int>         res_domids;
    
    // process all local sets
    for(int i = 0; i < nsets ; i++)
    {
        // get current set
        vtkDataSet *res_dset = NULL; // fetch each set from vtk output
        int         res_domid = -1;

        PyObject *py_dset  = PySequence_Fast_GET_ITEM(py_r_dsets,i);  // borrowed
        PyObject *py_domid = PySequence_Fast_GET_ITEM(py_r_domids,i); // borrowed
        
        if(py_dset == NULL)
            PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                         "Unable fetch result data set.");

        if(py_dset != Py_None) // allow 'None' as return
        {
            res_dset = (vtkDataSet*) pyEnv->UnwrapVTKObject(py_dset,"vtkDataSet");
            if(res_dset == NULL)
                PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                             "Error unwraping vtkDataSet result.");
             if(py_domid == NULL || PyInt_Check(py_domid) == 0)
                 PYEXPR_ERROR("avtPythonExpression::Execute Error - "
                              "Unable fetch result domain id.");
             res_domid = (int) PyInt_AsLong(py_domid);
        }
        
        res_dsets.push_back(res_dset);
        res_domids.push_back(res_domid);
    }
    
    // if we have no errors, register the datasets & create the output 
    // data tree
    for(int i = 0; i < nsets ; i++)
    {
        if(res_dsets[i]) // add result as new leaf
        {
            res_dsets[i]->Register(NULL);
            leaves[i] = new avtDataTree(res_dsets[i],res_domids[i]);
        }
        else // if the dataset only contained ghost zones we could end up here
            leaves[i] = NULL;
        
    }

    Py_DECREF(py_result_dsets);
    Py_DECREF(py_result_domids);
    Py_DECREF(py_r_dsets);
    Py_DECREF(py_r_domids);
    Py_DECREF(py_exe_res);

    // create output data tree
    avtDataTree_p result_tree = new avtDataTree(nsets,leaves);
    // set output data tree
    SetOutputDataTree(result_tree);

    // cleanup leaves array
    delete [] leaves;

}

// ****************************************************************************
//  Method: avtPythonExpression::ModifyContract
//
//  Purpose:
//      Provides access to the contract to a python expression filter.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:45:21 PST 2010
//
// ****************************************************************************

avtContract_p
avtPythonExpression::ModifyContract(avtContract_p in_contract)
{
    avtContract_p contract =  avtExpressionFilter::ModifyContract(in_contract);

    // hand contract over to the python filter
    PyObject *py_filter = pyEnv->Filter()->PythonObject();
    if(py_filter == NULL)
          PYEXPR_ERROR("avtPythonExpression::ModifyContract Error - "
                       "Python filter not initialized.");
    PyObject *py_contract = PyContract_Wrap(contract);


    // get the modify_contract method
    PyObject *py_exe= PyString_FromString("modify_contract");
    if(py_exe == NULL)
           PYEXPR_ERROR("avtPythonExpression::ModifyContract Error - "
                        "Error preparing for call of 'modify_contract' method.");
    // call with py_dsets, py_domids
    PyObject *py_exe_res = PyObject_CallMethodObjArgs(py_filter,
                                                      py_exe,
                                                      py_contract,
                                                      NULL);
    if(py_exe_res == NULL)
      PYEXPR_ERROR("avtPythonExpression::ModifyContract Error - "
                   "Python Expression 'modify_contract' method failed");
    Py_DECREF(py_exe);
    Py_DECREF(py_exe_res);
    Py_DECREF(py_contract);


    return contract;
}

