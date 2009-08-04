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

#ifndef PF3DFILEFORMAT_H
#define PF3DFILEFORMAT_H
#include <PDBReader.h>
#include <avtSTMDFileFormat.h>
#include <visitstream.h>
#include <vectortypes.h>

class avtFileFormatInterface;
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
// Class: PF3DFileFormat
//
// Purpose:
//   Reads the PF3D file format.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 10:28:16 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 20 10:50:54 PDT 2004
//   Rewrote for new PF3D file format.
//
//   Brad Whitlock, Thu Dec 1 14:53:29 PST 2005
//   Added ability to dynamically determine the size of visnams.
//
//   Brad Whitlock, Thu Jun 22 16:43:21 PST 2006
//   Added support for PF3D files that contain more than 1 domain per file.
//
// ****************************************************************************

class PF3DFileFormat : public PDBReader, public avtSTMDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList, int nBlocks);

    PF3DFileFormat(const char *filename);
    virtual ~PF3DFileFormat();

    // Methods overrides for an STMD file format.
    virtual const char   *GetType();
    virtual void          ActivateTimestep(void);
    virtual int           GetCycle(void);
    virtual double        GetTime(void);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          PopulateIOInformation(avtIOInformation &);

    virtual void         *GetAuxiliaryData(const char *var, int dom,
                                           const char *type, void *,
                                           DestructorFunction &df);
    virtual void          FreeUpResources();

    virtual vtkDataSet   *GetMesh(int, const char *var);
    virtual vtkDataArray *GetVar(int, const char *var);

protected:
    virtual void CloseFile(int);

    virtual bool IdentifyFormat();
    static  bool StaticIdentifyFormat(PDBFileObject *);

    //
    // This class contains the structured information from the
    // master file.
    //
    class MasterInformation
    {
    public:
        struct MemberData
        {
            MemberData();
            ~MemberData();
            void Print(ostream &) const;

            std::string name;
            TypeEnum    dataType;
            int         ndims;
            int         dims[3];
            void       *data;
        };

        typedef std::vector<MemberData *> MemberDataVector;
    public:
        MasterInformation();
        virtual ~MasterInformation();
        bool Read(PDBFileObject *pdb);
        int  GetNDomains() const;

        //
        // Methods to get pointers to the fields of the variable
        // sized "struct".
        //
        double        Get_tnowps() const;
        const long *  Get_domloc() const;
        const double *Get_xyzloc() const;
        const char *  Get_visnams() const;
        const char *  Get_visname_for_domain(int dom, int comp) const;
        const double *GetMinArray(const std::string &varName) const;
        const double *GetMaxArray(const std::string &varName) const;
        std::string   Get_dom_prefix_for_domain(int dim) const;
        int           Get_num_grp_size() const;
        int           Get_grp_size(int grp) const;
        const long   *Get_grp_members(int grp) const;

        ostream &operator << (ostream &os);
    private:
        const MemberData *FindMember(const std::string &name) const;


        int              nDomains;
        MemberDataVector members;
    };

    class BOF
    {
    public:
        BOF();
        BOF(const BOF &);
        virtual ~BOF();
        static void Destruct(void *);
    
        int    size[3];
        float *data;
    };

private:
    void Initialize();

    std::string FilenameForDomain(int realDomain);
    PDBFileObject *GetDomainFileObject(int realDomain);

    void ReadVariableInformation();
    void SetUpDomainConnectivity();
    int  GetNDomains() const;
    void GetExtents(int dom, double *extents);
    void GetLogicalExtents(int dom, int *globalorigin, int *size);
    void GetVarMinMaxArrays(const std::string &name,
                            const double **minvals,
                            const double **maxvals);
    int  GetVariableIndex(const std::string &name) const;
    BOF *GetBOF(int realDomain, const char *varName);
    int  GetRealDomainIndex(int dom) const;
    std::string GetBOFKey(int realDomain, const char *varName) const;
    bool ReadStringVector(const char *name, stringVector &output);
    bool CanAccessFile(const std::string &) const;

    bool initialized;

    // Values that are read in when the file is opened.
    int               cycle;
    int               nx;          // .5 Number of cells in X 
    int               ny;          // .5 Number of cells in Y
    int               nz;          // Number of cells -1 in Z
    double            lenx;        // Size of the whole problem in X.
    double            leny;        // Size of the whole problem in Y.
    double            lenz;        // Size of the whole problem in Z.
    MasterInformation master;      // Contains information about all of the
                                   // domains in the problem.
    stringVector      glob_nams;   // Contains variable names.
    stringVector      int_nams;    // Contains variable names in file.
    stringVector      glob_units;  // Contains variable units.
    intVector         apply_exp;   // Contains whether we should apply exp().
    std::string       databaseComment; // Contains the database information.
    bool              compression; // Determines whether data is compressed.
};

#endif
