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
#include <avtExpressionTypeConversions.h>

// ****************************************************************************
// Method: ExprType_To_avtVarType
//
// Purpose: 
//   Returns the avtVarType for the expression type.
//
// Arguments:
//   type : The expression type
//
// Returns:    The avtVarType of the expression or AVT_UNKNOWN_TYPE if the type
//             cannot be determined.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 18 14:45:25 PST 2002
//
// Modifications:
//      Sean Ahern, Wed Dec 11 16:29:52 PST 2002
//      Changed the interface so that it works with ExprTypes.
//
//      Sean Ahern, Mon Mar 17 23:01:02 America/Los_Angeles 2003
//      Changed the expression type names.
//   
//      Hank Childs, Tue Jul 19 13:39:12 PDT 2005
//      Added array expression.
//
//      Kathleen Bonnell, Wed Aug  2 17:20:11 PDT 2006 
//      Added curve expression.
//
// ****************************************************************************
avtVarType
ExprType_To_avtVarType(const Expression::ExprType type)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check to see if the variable is an expression.
    switch (type)
    {
    case Expression::ScalarMeshVar:
        retval = AVT_SCALAR_VAR;
        break;
    case Expression::VectorMeshVar:
        retval = AVT_VECTOR_VAR;
        break;
    case Expression::TensorMeshVar:
        retval = AVT_TENSOR_VAR;
        break;
    case Expression::SymmetricTensorMeshVar:
        retval = AVT_SYMMETRIC_TENSOR_VAR;
        break;
    case Expression::ArrayMeshVar:
        retval = AVT_ARRAY_VAR;
        break;
    case Expression::CurveMeshVar:
        retval = AVT_CURVE;
        break;
    case Expression::Mesh:
        retval = AVT_MESH;
        break;
    case Expression::Material:
        retval = AVT_MATERIAL;
        break;
    case Expression::Species:
        retval = AVT_MATSPECIES;
        break;
    case Expression::Unknown:
    default:
        retval = AVT_UNKNOWN_TYPE;
    }

    return retval;
}


// ****************************************************************************
// Method: avtVarType_To_ExprType
//
// Purpose: 
//   Returns the expression type for the AVT variable type.
//
// Arguments:
//   type : The AVT variable type
//
// Returns:    The expression type or UNKNOWN if the type cannot be determined.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   January 8, 2007
//
// ****************************************************************************

Expression::ExprType
avtVarType_To_ExprType(avtVarType type)
{
    Expression::ExprType retval = Expression::Unknown;

    // Check to see if the variable is an expression.
    switch (type)
    {
      case AVT_SCALAR_VAR:
        retval = Expression::ScalarMeshVar;
        break;
      case AVT_VECTOR_VAR:
        retval = Expression::VectorMeshVar;
        break;
      case AVT_TENSOR_VAR:
        retval =  Expression::TensorMeshVar;
        break;
      case AVT_SYMMETRIC_TENSOR_VAR:
        retval =  Expression::SymmetricTensorMeshVar;
        break;
      case AVT_ARRAY_VAR:
        retval =  Expression::ArrayMeshVar;
        break;
      case AVT_CURVE:
        retval =  Expression::CurveMeshVar;
        break;
      case AVT_MESH:
        retval =  Expression::Mesh;
        break;
      case AVT_MATERIAL:
        retval =  Expression::Material;
        break;
      case AVT_MATSPECIES:
        retval =  Expression::Species;
        break;
      case AVT_UNKNOWN_TYPE:
      default:
        retval =  Expression::Unknown;
    }

    return retval;
}
