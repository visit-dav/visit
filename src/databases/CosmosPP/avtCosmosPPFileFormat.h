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
//                          avtCosmosPPFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_COSMOS_PP_FILE_FORMAT_H
#define AVT_COSMOS_PP_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>

class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtCosmosPPFileFormat
//
//  Purpose:
//      A file format reader for the CosmosPP file format.
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jul 19 16:57:59 PDT 2004
//    Added haveIssuedWarning.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to PopulateDatabaseMetaData satisfy new interface
// ****************************************************************************

class avtCosmosPPFileFormat : public avtMTMDFileFormat
{
  public:
                          avtCosmosPPFileFormat(const char *);
    virtual              ~avtCosmosPPFileFormat();
    
    virtual const char   *GetType(void) { return "Cosmos++ File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual void          GetTimes(std::vector<double> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

  protected:
    std::string                         dirname;
    
    std::vector<int>                    cycles;
    std::vector<double>                 times;
    int                                 rank;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nscalars;
    int                                 nvectors;
    bool                                haveIssuedWarning;

    std::vector<std::string>            scalarVarNames;
    std::vector<std::string>            vectorVarNames;
    std::vector<std::string>            proc_names;
    std::vector<std::string>            dump_names;

    std::vector<std::vector<bool> >                   readDataset;
    std::vector<std::vector<vtkUnstructuredGrid *> >  dataset;

    void ReadDataset(int, int);

    void ReadString(ifstream &ifile, std::string &str);
};

#endif
