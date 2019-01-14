/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                             SpyFile.h                                     //
// ************************************************************************* //

#ifndef SPY_FILE_H
#define SPY_FILE_H

#include <fstream>
#include <vector>

#include <Spy.h>


// ****************************************************************************
//  Class: SpyFile
//
//  Purpose:    To parse and store the contents of an SPCTH (spy) file.
//
//  Programmer: Kevin Griffin 
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

class SpyFile
{
  public:
    SpyFile(std::ifstream * const);
    virtual       ~SpyFile();
    
    bool           ReadFileHeader();
    void           ReadTimestepData(const int);
    
    char          *GetMagicString();
    const char    *GetFileTitle();
    int            GetFileVersion();
    int            GetFilePtrSize();
    int            GetCompression();
    int            GetProcessorId();
    int            GetNumberOfProcs();
    int            GetIGM();
    int            GetNumberOfCellFields();
    int            GetNumberOfDims();
    int            GetNumberOfMats();
    int            GetNumberOfMatFields();
    int            GetMaxNumMaterials();
    const double         *GetGmin();
    const double         *GetGmax();
    int            GetNumberOfAMRBlocks();
    int            GetNumberOfDumps();
    const std::vector<Dump *> * const   GetDumps();
    const DumpHeader * const            GetDumpHeader(const int);
    const Field *  GetCellFields();
    const Field *  GetMatsFields();
    bool           IsMarkOn();
    int            GetMaxAMRLevels();
    DataBlock      *GetDataBlock(const int);
    double          ***GetField(DataBlock *const, const int, const int);
    int            GetFieldId(const char *);
    char           *GetFieldName(const int);
    char           *GetFieldComment(const int);
    int             GetMat(const int);  // TODO
    double         *GetFieldVal(DataBlock *const, const int, const int, const int, const int, const int, const int);
    void            GetBlockCellSize(const int, double *);

  private:
    std::ifstream   *m_in;
    double          *scratch;
    int              m_timestep;
    
    FileHeaderInfo  *spyFileHeader;
    
    double          m_last_group;
    GroupHeader     *m_group_header;
    
    TimestepData    *tsData;
    bool            m_bigendian;
    
    void            FreeUpFileHeader();
    void            FreeUpTimestep();
    void            ReadDumpHeader();
    void            ReadTracer();
    void            ReadHistogramData();
    void            ReadBlockGeometries();
    void            ReadVariableData(const int);
    void            ReadMarkersVars();
    
    void            ReadMarkerHeader();
    
    bool            IsBigEndian();
    void            FlipInt(int *, const int);
    void            FlipDouble(double *, const int);
    int             ReadInt();
    void            ReadInt(int *, const int);
    void            ReadDouble(double *, const int);
    void            ReadGroups();
    void            ReadGroupHeader();
    void            RunLengthDecode(double *, const int, void *, const int);
    void            RunLengthDecodeInt(int *, const int, void *, const int);
    void            RunLengthDecodeTrend(double *, const int, void *, const int);
    
    double          Float2Double(unsigned char *);
    int             Int4ToInt(unsigned char *);
    
    void            ComputeCenterOfMass(DataBlock *const, double *);
};


#endif
