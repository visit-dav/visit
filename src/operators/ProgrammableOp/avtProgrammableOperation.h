/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//  File: avtProgrammableOperation.h
// ************************************************************************* //

#ifndef AVT_SCRIPT_OPERATION_H
#define AVT_SCRIPT_OPERATION_H

#include <ProgrammableOperation.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkImageData.h>
/// class to provide default script operations..
class avtProgrammableOperation : public ProgrammableOpController
{
    class avtVisItForEachLocation : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments&, vtkShapedDataArray &result);

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };

#ifdef HAVE_LIB_R
    class avtVisItForEachLocationR : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments&, vtkShapedDataArray& result);

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };

    class avtVisItGetRSupportDirectory : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments&, Variant&);

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };
#endif

    class avtVisItForEachLocationPython : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments&, vtkShapedDataArray& result);

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };



    class avtVisItForEachFile : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments&, vtkShapedDataArray&)
        {
            return false;
        }

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes)
        {
            name = "visit_foreach_file";
            return ProgrammableOperation::VTK_DATA_ARRAY;
        }
    };

    class avtVisItGetVarInfo : public ProgrammableOperation
    {
    public:
        /// we are always returning true unless the script
        /// itself is failing not the inquiry..
        virtual bool func(ProgrammableOpArguments& args, Variant& result);
        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };

    class avtVisItGetMeshInfo : public ProgrammableOperation
    {
    public:
        /// we are always returning true unless the script
        /// itself is failing not the inquiry..
        virtual bool func(ProgrammableOpArguments& args, Variant& result);
        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };

    class avtVisItWriteData : public ProgrammableOperation
    {
    public:
        virtual bool func(ProgrammableOpArguments &args,
                          Variant& result);

        virtual ResponseType getSignature(std::string& name,
                                  stringVector& argnames,
                                  std::vector<ScriptType>& argtypes);
    };
    
    class avtVisItMaxAcrossTime : public ProgrammableOperation
    {
        virtual bool func(ProgrammableOpArguments&, vtkShapedDataArray&);

        virtual ResponseType getSignature(std::string& name,
                             stringVector& argnames,
                             std::vector<ScriptType>& argtypes);
    };

public:
    avtProgrammableOperation();
    void RegisterOperations(ProgrammableOpManager* manager);
private:
    /// visit foreach location variations..
    avtVisItForEachLocation vfel;
#ifdef HAVE_LIB_R
    avtVisItForEachLocationR vfelr;
    avtVisItGetRSupportDirectory avag;
#endif
    avtVisItForEachLocationPython vfelp;

    avtVisItForEachFile vfef;

    avtVisItGetVarInfo vgvi;
    avtVisItGetMeshInfo vgmi;
    avtVisItWriteData vmax;
    avtVisItWriteData avwd;
};

#endif
