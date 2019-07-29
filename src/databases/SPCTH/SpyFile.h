// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
