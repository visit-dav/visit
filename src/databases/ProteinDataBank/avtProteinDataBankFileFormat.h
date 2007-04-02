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
//                            avtProteinDataBankFileFormat.h                 //
// ************************************************************************* //

#ifndef AVT_ProteinDataBank_FILE_FORMAT_H
#define AVT_ProteinDataBank_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <map>
#include <string>

class DBOptionsAttributes;


// ****************************************************************************
//  Struct:  Atom
//
//  Purpose:
//    Encapsulates an Atom for a PDB file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
struct Atom
{
    int   serial;
    char  name[5];
    char  altloc;
    char  resname[4];
    char  chainid;
    int   resseq;
    char  icode;
    float x;
    float y;
    float z;
    float occupancy;
    float tempfactor;
    char  segid[5];
    char  element[3];
    char  charge[3];

    int   atomicnumber;
    int   residuenumber;
    bool  backbone;

    Atom(const char *line);
    void Print(ostream &out);
};

// ****************************************************************************
//  Class: avtProteinDataBankFileFormat
//
//  Purpose:
//      Reads in ProteinDataBank files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Brad Whitlock, Thu Mar 23 11:45:32 PDT 2006
//    Added support for PDB title information.
//
// ****************************************************************************

class avtProteinDataBankFileFormat : public avtMTSDFileFormat
{
  public:
                       avtProteinDataBankFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtProteinDataBankFileFormat() {;};

    //
    // This is used to return unconventional data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ProteinDataBank"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    ifstream in;

    bool metadata_read;
    int  nmodels;
    std::vector< std::vector<Atom> > allatoms;
    std::vector<int>                 bonds[4];

    std::string filename;
    std::string dbTitle;
    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadAtomsForModel(int);
    void CreateBondsFromModel_Slow(int);
    void CreateBondsFromModel_Fast(int);
};


#endif
