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
//                            avtJMFileFormat.h                              //
// ************************************************************************* //

#ifndef AVT_JM_FILE_FORMAT_H
#define AVT_JM_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>

#include <vectortypes.h>
#include <map>
#include <set>

#include <PDBFileObject.h>

class avtFileFormatInterface;

// ****************************************************************************
//  Class: avtJMFileFormat
//
//  Purpose:
//      Reads in JM files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

class JMFileFormat : public avtMTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList);

                       JMFileFormat(const char * const*);
                       JMFileFormat(const char * const*, PDBFileObject *);
    virtual           ~JMFileFormat();

    bool                   Identify();
    void                   OwnsPDB();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void          *GetAuxiliaryData(const char *var, int timestep, 
                                            const char *type, void *args, 
                                            DestructorFunction &);

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual void           ActivateTimestep(int);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Jose Milovich"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    struct VarItem
    {
        VarItem();
        ~VarItem();
        void *DataForTime(int);

        TypeEnum  type; 
        int      *dims;
        int       ndims;
        int       nTotalElements;
        void     *data;
    };

    PDBFileObject                   *pdb;
    bool                             ownsPDB;
    std::map<std::string, VarItem *> variableCache;
    std::set<std::string>            curveNames;

    void                   GetMaterialNames(stringVector &materialNames);
    VarItem *              ReadVariable(const std::string &var);
    VarItem *              ReadIREG();
    intVector              GetSize(const char *varName);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
