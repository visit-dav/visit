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

// ************************************************************************* //
//                        avtApplyMapExpression.C                            //
// ************************************************************************* //

#include <avtApplyMapExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>

#include <sstream>

using namespace std;


// ****************************************************************************
//  Method: avtApplyMapExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
// ****************************************************************************

avtApplyMapExpression::avtApplyMapExpression()
: mapType(0), maxStringLength(0), 
  numericDefault(-1.0), stringDefault("<undefined>")
{}


// ****************************************************************************
//  Method: avtMapExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
// ****************************************************************************

avtApplyMapExpression::~avtApplyMapExpression()
{}


// ****************************************************************************
//  Method: avtApplyMapExpression::DeriveVariable
//
//  Purpose:
//      Transforms input array values using the user defined mapping function.
//      If an input array value does not exist in the map the output is
//      assigned a default value.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Notes: Adapted from Jeremy Meredith's avtApplyEnumerationExpression.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtApplyMapExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int    i;
    ostringstream oss;

    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    vtkDataArray *var = in_ds->GetPointData()->GetArray(varnames[0]);
    if (var == NULL)
        var = in_ds->GetCellData()->GetArray(varnames[0]);

    oss << "Could not find input variable \"" << varnames[0] << "\"";
    if (var == NULL)
        ThrowError(oss.str());

    if (var->GetNumberOfComponents() != 1)
        ThrowError("The map expression only supports scalar inputs.");

    vtkDataArray *res = NULL;

    if(mapType == 0) // numeric map
    {
        vtkFloatArray *rv = vtkFloatArray::New();
        int nvals = var->GetNumberOfTuples();
        rv->SetNumberOfComponents(1);
        rv->SetNumberOfTuples(nvals);

        double val = 0.0;

        for (i = 0 ; i < nvals ; i++)
        {
            val = var->GetTuple1(i);

            map<double,double>::iterator itr = numericMap.find(val);

            if(itr == numericMap.end())
                rv->SetComponent(i, 0, numericDefault);
            else
                rv->SetComponent(i, 0, itr->second);

        }
        res = rv;

    }
    else if(mapType == 1) // string map
    {
        vtkUnsignedCharArray *rv = vtkUnsignedCharArray::New();
        int nvals = var->GetNumberOfTuples();
        // set the # of components to max string length + 1
        rv->SetNumberOfComponents(maxStringLength+1);
        rv->SetNumberOfTuples(nvals);
        double val = 0.0;

        unsigned char *ptr = rv->GetPointer(0); 
        for (i = 0 ; i < nvals ; i++)
        {
            val = var->GetTuple1(i);
            map<double,string>::iterator itr = stringMap.find(val);

            // copy char values from string into the vtk char array.
            if(itr == stringMap.end())
                strcpy((char*)ptr,(const char*)stringDefault.c_str());
            else
                strcpy((char*)ptr,(const char*)itr->second.c_str());;
            ptr+= maxStringLength+1;
        }
        res = rv;
    }

    return res;
}


// ****************************************************************************
//  Method: avtApplyMapExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.
//      Parses 'to' and 'from' lists to define a mapping function.
//
//  Arguments:
//      args      Expression arguments.
//      state     Pipeline state.
//
//  Programmer:   Cyrus Harrison
//  Creation:     August 11, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    //
    // support the following use cases:
    //  2 args:
    //   map(var,[TO])
    //  3 args:
    //   map(var,[TO],default)
    //   map(var,[FRO],[TO])
    //  4 args:
    //   map(var,[FRO],[TO],default)
    //

    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

     // first argument should be the var name, let it do its own magic
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>((*arguments)[0]->GetExpr());
    first_tree->CreateFilters(state);

    // first arg should be the var and we need to build
    vector<double> from_values;

    if ( nargs < 2)
        ThrowError("the map expression requries at least two arguments");

    ExprParseTreeNode *second_node = (*arguments)[1]->GetExpr();
    if (second_node->GetTypeName() != "List")
        ThrowError("second argument must be a numeric list.");

    if(nargs == 2)
    {
        ListExpr *to_list = dynamic_cast<ListExpr*>(second_node);
        BuildMap(to_list);
    }
    else if ( nargs == 3)
    {
        ExprParseTreeNode *third_node = (*arguments)[2]->GetExpr();
        if (third_node->GetTypeName() == "List")
        {
            // we have two lists
            // second node contains the "from" values.
            ListExpr *from_list = dynamic_cast<ListExpr*>(second_node);
            if(!from_list->ExtractNumericElements(from_values))
                ThrowError("map 'from' argument contain a numeric list.");

            // third node contains the "to" values.
            ListExpr *to_list = dynamic_cast<ListExpr*>(third_node);
            BuildMap(to_list,from_values);
        }
        else
        {
            // we have one list & a default value
            // second node contains the "to" values.
            ListExpr *to_list = dynamic_cast<ListExpr*>(second_node);
            BuildMap(to_list);
            SetMapDefaultValues(third_node);
        }
    }
    else if ( nargs == 4)
    {
        // we should have two lists and a default.
        ExprParseTreeNode *third_node = (*arguments)[2]->GetExpr();
        if (third_node->GetTypeName() != "List")
            ThrowError("third argument must be a numeric list.");

        ListExpr *from_list = dynamic_cast<ListExpr*>(second_node);
        if(!from_list->ExtractNumericElements(from_values))
            ThrowError("map 'from' argument contain a numeric list.");

        ListExpr *to_list = dynamic_cast<ListExpr*>(third_node);

        ExprParseTreeNode *forth_node = (*arguments)[3]->GetExpr();
        BuildMap(to_list,from_values);
        // set default value
        SetMapDefaultValues(forth_node);
    }
}


// ****************************************************************************
//  Method: avtApplyMapExpression::UpdateDataObjectInfo
//
//  Purpose:
//      Set Treat as ASCII if the output array holds string labels.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Fri Aug 14 11:36:35 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::UpdateDataObjectInfo()
{
    avtMultipleInputExpressionFilter::UpdateDataObjectInfo();

    if(mapType == 1)
    {
        avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
        atts.SetTreatAsASCII(true);
    }
}

// ****************************************************************************
//  Method: avtApplyMapExpression::BuildMap
//
//  Purpose:
//      Sets up the mapping function from expression argument arrays.
//      This variant of BuildMap creates a default 'from' list.
//
//  Arguments:
//      to_list  List Expression containing map 'to' values.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::BuildMap(ListExpr *to_list)
{
    // 'from' list was not providied:
    // create a simple one enumerating [0 <-> (n-1)]
    // where n is the size of the 'to' list.
    int to_size = to_list->GetElems()->size();
    vector<double> from_vals(to_size,0.0);
    for(int i=0; i < to_size;i++)
        from_vals[i] = (double)i;

    BuildMap(to_list,from_vals);
}


// ****************************************************************************
//  Method: avtApplyMapExpression::BuildMap
//
//  Purpose:
//      Sets up the mapping function from expression argument arrays.
//
//  Arguments:
//      to_list     List Expression containing map 'to' values.
//      from_vals   A vector holding the 'from' values.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::BuildMap(ListExpr *to_list,
                                const vector<double> &from_vals)
{
    int from_size = from_vals.size();
    vector<double> numeric_vals;
    vector<string> string_vals;
    maxStringLength = 0;
    mapType = 0;

    if(to_list->ExtractNumericElements(numeric_vals))
    {
        mapType = 0;
        if( from_size  != numeric_vals.size())
            ThrowError("map function 'from' and 'to' arrays must "
                       "contain the same # of elements.");

        for(unsigned int i=0; i < from_size; i++)
            numericMap[from_vals[i]] = numeric_vals[i];
    }
    else if(to_list->ExtractStringElements(string_vals))
    {
        mapType = 1;
        if(from_size != string_vals.size())
            ThrowError("map function 'from' and 'to' arrays must "
                       "contain the same # of elements.");

        for(unsigned int i=0; i < from_size; i++)
        {
            unsigned slen = string_vals[i].size();
            if(slen > maxStringLength) 
                maxStringLength = slen;
            stringMap[from_vals[i]] = string_vals[i];
        }
        maxStringLength++;

        // make sure we can hold the default string if necessary
        if(maxStringLength < stringDefault.size())
            maxStringLength= stringDefault.size() + 1;
    }
    else
    {
        // error
        ThrowError("map function 'to' array must be a "
                   "homogeneous list of numeric or string values");
    }

}

// ****************************************************************************
//  Method: avtApplyMapExpression::SetMapDefaultValues
//
//  Purpose:
//      Parses default map values from an expression node.
//
//  Arguments:
//      node    Expression node containing the map default value.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::SetMapDefaultValues(ExprParseTreeNode *node)
{
    numericDefault = -1.0;
    stringDefault = "<undefined>";

    // extract the default value from the node.
    if(mapType == 0)
    {

        if(node->GetTypeName() == "FloatConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(node);
            numericDefault = (double) dynamic_cast<FloatConstExpr*>(c)->GetValue(); 
        }
        else if(node->GetTypeName() == "IntegerConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(node);
            numericDefault = (double) dynamic_cast<IntegerConstExpr*>(c)->GetValue(); 
        }
        else
            ThrowError("a numeric map requires a numeric default value.");
    }
    else if (mapType == 1)
    {
        if(node->GetTypeName() == "StringConst")
        {
            ConstExpr *c = dynamic_cast<ConstExpr*>(node);
            stringDefault =  dynamic_cast<StringConstExpr*>(c)->GetValue();
        }
        else
            ThrowError("a string map requires a string default value.");
    }
}

// ****************************************************************************
//  Method: avtApplyMapExpression::ThrowError
//
//  Purpose:
//      Helper that throws an exception with given error message.
//
//  Arguments:
//      msg    Error message.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtApplyMapExpression::ThrowError(const string &msg)
{
    // note this undermines the exception line # reporting
    // but it cleans up the code above significantly.
    EXCEPTION2(ExpressionException, outputVariableName,
               msg + "\n'map' usage:\n"
                     "  map(value,[TO list])\n"
                     "  map(value,[TO list],default)\n"
                     "  map(value,[FROM list],[TO list])\n"
                     "  map(value,[FROM list],[TO list],default)\n");
}



