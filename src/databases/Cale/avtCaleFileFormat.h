/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtCaleFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CALE_FILE_FORMAT_H
#define AVT_CALE_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <visit-config.h>

#ifdef HAVE_PDB_PROPER
#include <pdb.h>
#define PDBLIB_ERRORSTRING PD_err
#else
#include <lite_pdb.h>
#ifdef _WIN32
#define PDBLIB_ERRORSTRING "(don't have the PD_err error string)"
#else
#define PDBLIB_ERRORSTRING PD_err
#endif
#endif

// ****************************************************************************
//  Class: avtCaleFileFormat
//
//  Purpose:
//      Reads in cale_visit files as a plugin to VisIt.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
// ****************************************************************************

class avtCaleFileFormat : public avtSTSDFileFormat
{
  public:
                       avtCaleFileFormat(const char *filename);
    virtual           ~avtCaleFileFormat();

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual bool      ReturnsValidCycle() const { return true; };
    virtual int       GetCycle(void);
    virtual int       GetCycleFromFilename(const char *f) const;
    virtual bool      ReturnsValidTime() const { return true; };
    virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "Cale"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);
    virtual void          *GetAuxiliaryData(const char *var, 
                                            const char *type,
                                            void *, 
                                            DestructorFunction &df) ;

    static void Identify(const char *filename);

  protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                   GetUsedMeshLimits (void);
    PDBfile                *GetPDBFile();
    int  removezoneghost_index(int in,int zk,int zl, int lp);
    PDBfile *pdbfile ;
    int kminmesh, kmaxmesh, lminmesh, lmaxmesh, nnallsmesh ;
};


#endif
