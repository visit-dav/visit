// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              SpyFile.C                                    //
// ************************************************************************* //

#include <SpyFile.h>

#include <VisItException.h>
#include <DebugStream.h>

#include <float.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstring>

using std::ifstream;
using std::ios;

// ****************************************************************************
//  Method: SpyFile constructor
//
//  Arguments:
//      fin    SPCTH (Spy) input file stream
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
// ****************************************************************************

SpyFile::SpyFile(ifstream * const fin) :
        m_in(fin), scratch(NULL), m_timestep(-1), spyFileHeader(NULL),
        m_last_group(0), m_group_header(NULL), tsData(NULL)
{
    m_bigendian = IsBigEndian();
}

// ****************************************************************************
//  Method: SpyFile destructor
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

SpyFile::~SpyFile()
{
    // Scratch
    if(scratch != NULL)
    {
        delete [] scratch;
    }
    
    // Free timestep data
    FreeUpTimestep();
    
    // Free SPCTH (Spy) File Header
    FreeUpFileHeader();

    // Group Header
    if(m_group_header != NULL)
    {
        delete [] m_group_header->dumpCycles;
        delete [] m_group_header->dumpTimes;
        delete [] m_group_header->dumpOffsets;
        
        if(m_group_header->dumpDT != NULL)
        {
            delete [] m_group_header->dumpDT;
        }
        
        delete m_group_header;
    }
}

// ****************************************************************************
//  Method: SpyFile::FreeUpFileHeader
//
//  Purpose:
//      Free up the memory allocated for storing the file header info.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void SpyFile::FreeUpFileHeader()
{
    if(spyFileHeader != NULL)
    {
        // Magic String
        if(spyFileHeader->magicStr != NULL)
        {
            delete [] spyFileHeader->magicStr;
        }
        
        // File Title
        if(spyFileHeader->title != NULL)
        {
            delete [] spyFileHeader->title;
        }
        
        // Gmin
        if(spyFileHeader->gMin != NULL)
        {
            delete [] spyFileHeader->gMin;
        }
        
        // Gmax
        if(spyFileHeader->gMax != NULL)
        {
            delete [] spyFileHeader->gMax;
        }
        
        // Marker Headers
        MarkerHeader *m_markerheaders = spyFileHeader->markerHeaders;
        
        if(m_markerheaders != NULL)
        {
            for(int m=0; m<spyFileHeader->nmat; m++)
            {
                delete [] m_markerheaders[m].junk;
                
                if(m_markerheaders[m].numMarkers > 0)
                {
                    for(int i=0; i<m_markerheaders[m].numVars; i++)
                    {
                        delete [] m_markerheaders[m].varnames[i];
                        delete [] m_markerheaders[m].varlabels[i];
                    }
                    
                    delete [] m_markerheaders[m].varnames;
                    delete [] m_markerheaders[m].varlabels;
                }
            }
            
            delete [] m_markerheaders;
        }
        
        // File Header Junk
        if(spyFileHeader->headerJunk != NULL)
        {
            delete [] spyFileHeader->headerJunk;
        }
        
        // Cell Fields
        Field *m_cellfields = spyFileHeader->cellFields;
        
        if(m_cellfields != NULL)
        {
            for(int i=0; i<spyFileHeader->nCellFields; i++)
            {
                delete [] m_cellfields[i].fieldID;
                delete [] m_cellfields[i].fieldComment;
            }
            
            delete [] m_cellfields;
        }
        
        // Material Fields
        Field *m_matfields = spyFileHeader->matFields;
        
        if(m_matfields != NULL)
        {
            for(int i=0; i<spyFileHeader->nMatFields; i++)
            {
                delete [] m_matfields[i].fieldID;
                delete [] m_matfields[i].fieldComment;
            }
            
            delete [] m_matfields;
        }
        
        // Dumps
        if(spyFileHeader->dumps != NULL)
        {
            spyFileHeader->dumps->clear();
            delete spyFileHeader->dumps;
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::FreeUpTimestep
//
//  Purpose:
//      Free the memory allocated to store the timestep data.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::FreeUpTimestep()
{
    if(tsData != NULL)
    {
        // Tracer
        if(tsData->tracer != NULL)
        {
            
            delete [] tsData->tracer->xTracer;
            delete [] tsData->tracer->yTracer;
            delete [] tsData->tracer->zTracer;
            
            delete [] tsData->tracer->lTracer;
            delete [] tsData->tracer->iTracer;
            delete [] tsData->tracer->jTracer;
            delete [] tsData->tracer->kTracer;
            
            delete  tsData->tracer;
        }
        
        // Histograms
        if(tsData->histogram != NULL)
        {
            
            if(tsData->histogram->numIndicators > 0)
            {
                for(int j=0; j<tsData->histogram->numIndicators; j++)
                {
                    for(int k=0; k<tsData->histogram->nBins[j]; k++)
                    {
                        delete [] tsData->histogram->histogram[k];
                    }
                    
                    delete [] tsData->histogram->histogram;
                    delete [] tsData->histogram->histMin;
                    delete [] tsData->histogram->histMax;
                    delete [] tsData->histogram->histType;
                    delete [] tsData->histogram->refAbove;
                    delete [] tsData->histogram->refBelow;
                    delete [] tsData->histogram->unrAbove;
                    delete [] tsData->histogram->unrBelow;
                    delete [] tsData->histogram->nBins;
                }
            }
            
            delete tsData->histogram;
        }
        
        // Data Blocks
        if(tsData->dataBlock != NULL)
        {
            int ny, nz;
            const DumpHeader * const dumpHeader = tsData->dumpHeader;
            
            for(int n=0; n<tsData->dataBlock->nBlocks; n++)
            {
                ny = tsData->dataBlock->ny[n];
                nz = tsData->dataBlock->nz[n];
                
                for(int l=0; l<dumpHeader->numVars; l++)
                {
                    double ***field = GetField(tsData->dataBlock, dumpHeader->vars[l], n);
                    if(field != NULL)
                    {
                        for(int k=0; k<nz; k++)
                        {
                            if(field[k][0] != NULL)
                            {
                                delete [] field[k][0];
                            }
                        }
                    }
                }
            }
            
            if(tsData->dataBlock->cField != NULL)
            {
                delete [] tsData->dataBlock->cField;
            }
            
            if(tsData->dataBlock->mField != NULL)
            {
                delete [] tsData->dataBlock->mField;
            }
            
            
            delete tsData->dataBlock;
        }
        
        // MarkerBlocks
        if(tsData->markerBlock != NULL)
        {
            if(tsData->markerBlock->xloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->xloc[m];
                }
                
                delete [] tsData->markerBlock->xloc;
            }
            
            if(tsData->markerBlock->yloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->yloc[m];
                }
                
                delete [] tsData->markerBlock->yloc;
            }
            
            if(tsData->markerBlock->zloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->zloc[m];
                }
                
                delete [] tsData->markerBlock->zloc;
            }
            
            if(tsData->markerBlock->iloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->iloc[m];
                }
                
                delete [] tsData->markerBlock->iloc;
            }
            
            if(tsData->markerBlock->jloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->jloc[m];
                }
                
                delete [] tsData->markerBlock->jloc;
            }
            
            if(tsData->markerBlock->kloc != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->kloc[m];
                }
                
                delete [] tsData->markerBlock->kloc;
            }
            
            if(tsData->markerBlock->bmark != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    delete [] tsData->markerBlock->bmark[m];
                }
                
                delete []  tsData->markerBlock->bmark;
            }
            
            if(tsData->markerBlock->vars != NULL)
            {
                for(int m=0; m<spyFileHeader->nmat; m++)
                {
                    for(int i=0; i<spyFileHeader->markerHeaders[m].numVars; i++)
                    {
                        delete [] tsData->markerBlock->vars[m][i];
                    }
                    
                    delete [] tsData->markerBlock->vars[m];
                }
                
                delete [] tsData->markerBlock->vars;
            }
            
            delete tsData->markerBlock;
        }
        
        // Dump Header
        if(tsData->dumpHeader != NULL)
        {
            delete [] tsData->dumpHeader->vars;
            delete [] tsData->dumpHeader->varsOffset;
            delete tsData->dumpHeader;
        }
        
        spyFileHeader->dumps->at(m_timestep)->loaded = false;
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadMarkersVars
//
//  Purpose:
//      Read the marker variables if the flag for MPM particles is on and
//      the SPCTH (Spy) file versions is >= 105.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadMarkersVars()
{
    tsData->markerBlock = new MarkerBlock();
    char *buffer = NULL;
    MarkerHeader *markerHeader = NULL;
    int numBytes = 0;
    int m_nmat = spyFileHeader->nmat;
    
    for(int n=0; n<m_nmat; n++)
    {
        markerHeader = &spyFileHeader->markerHeaders[n];
        
        if(markerHeader->numMarkers > 0)
        {
            // Allocate Resources if needed
            // xloc
            if(tsData->markerBlock->xloc == NULL)
            {
                tsData->markerBlock->xloc = new double *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->xloc[i] = new double[markerHeader->numMarkers];
                }
            }
            
            // yloc
            if(tsData->markerBlock->yloc == NULL)
            {
                tsData->markerBlock->yloc = new double *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->yloc[i] = new double[markerHeader->numMarkers];
                }
            }
            
            // zloc
            if(tsData->markerBlock->zloc == NULL)
            {
                tsData->markerBlock->zloc = new double *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->zloc[i] = new double[markerHeader->numMarkers];
                }
            }
            
            // iloc
            if(tsData->markerBlock->iloc == NULL)
            {
                tsData->markerBlock->iloc = new int *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->iloc[i] = new int[markerHeader->numMarkers];
                }
            }
            
            // jloc
            if(tsData->markerBlock->jloc == NULL)
            {
                tsData->markerBlock->jloc = new int *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->jloc[i] = new int[markerHeader->numMarkers];
                }
            }
            
            // kloc
            if(tsData->markerBlock->kloc == NULL)
            {
                tsData->markerBlock->kloc = new int *[m_nmat];
                for(int i=0; i<m_nmat; i++)
                {
                    tsData->markerBlock->kloc[i] = new int[markerHeader->numMarkers];
                }
            }
            
            buffer = new char[5*markerHeader->numRealMarkers+8];
            
            double *xloc = tsData->markerBlock->xloc[n];
            numBytes = ReadInt();
            m_in->read(buffer, numBytes);
            RunLengthDecode(xloc, markerHeader->numRealMarkers, buffer, numBytes);
            
            int *iloc = tsData->markerBlock->iloc[n];
            numBytes = ReadInt();
            m_in->read(buffer, numBytes);
            RunLengthDecodeInt(iloc, markerHeader->numRealMarkers, buffer, numBytes);
            
            int *jloc = tsData->markerBlock->jloc[n];
            double *yloc = tsData->markerBlock->yloc[n];
            
            if(spyFileHeader->ndim > 1)
            {
                numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecode(yloc, markerHeader->numRealMarkers, buffer, numBytes);
                
                numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecodeInt(jloc, markerHeader->numRealMarkers, buffer, numBytes);
            }
            else
            {
                for(int i=0; i<markerHeader->numRealMarkers; i++)
                {
                    jloc[i] = 0;
                    yloc[i] = 0.0;
                }
            }
            
            double *zloc = tsData->markerBlock->zloc[n];
            int *kloc = tsData->markerBlock->kloc[n];
            
            if(spyFileHeader->ndim > 2)
            {
                numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecode(zloc, markerHeader->numRealMarkers, buffer, numBytes);
                
                numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecodeInt(kloc, markerHeader->numRealMarkers, buffer, numBytes);
            }
            else
            {
                for(int i=0; i<markerHeader->numRealMarkers; i++)
                {
                    kloc[i] = 0;
                    zloc[i] = 0.0;
                }
            }
            
            // bmark
            if(tsData->markerBlock->bmark == NULL)
            {
                tsData->markerBlock->bmark = new int *[m_nmat];
            }
            
            int *bmark = tsData->markerBlock->bmark[n];
            numBytes = ReadInt();
            m_in->read(buffer, numBytes);
            RunLengthDecodeInt(bmark, markerHeader->numRealMarkers, buffer, numBytes);
            
            // Read in marker variables
            if(tsData->markerBlock->vars == NULL)
            {
                tsData->markerBlock->vars = new REAL**[m_nmat];
            }
            
            tsData->markerBlock->vars[n] = new REAL*[markerHeader->numVars];
            REAL **vars = tsData->markerBlock->vars[n];
            
            for(int i=0; i<markerHeader->numVars; i++)
            {
                vars[i] = new REAL[markerHeader->numMarkers];
                
                numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecode(vars[i], markerHeader->numRealMarkers, buffer, numBytes);
            }
            
            delete [] buffer;
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadVariableData
//
//  Purpose:
//      Reads variable data for all blocks and kplanes. It is assumed that the
//      file pointer is pointing to the beginning of the compressed data in
//      file: m_in.
//
//
//  Arguments:
//      varId   Saved Variable ID
//
//  Returns:
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadVariableData(const int varId)
{
    int tmp, max = 0;
    
    // Loop over the blocks and find the maximum required buffer size to hold
    // the compressed variable data.
    for(int n=0; n<tsData->dataBlock->nBlocks; n++)
    {
        if(tsData->dataBlock->allocated[n] > 0)
        {
            tmp = 5 * tsData->dataBlock->nx[n] * tsData->dataBlock->ny[n] + 8;
            if(tmp > max)
            {
                max = tmp;
            }
        }
    }
    
    if(max == 0)
    {
        return;
    }
    
    double ***field;
    int nBytes;
    char *buffer = new char[max];
    
    // Loop over blocks, read and uncompress the data
    for(int n=0; n<tsData->dataBlock->nBlocks; n++)
    {
        if(tsData->dataBlock->allocated[n] > 0)
        {
            field = GetField(tsData->dataBlock, varId, n);
            
            if(field != NULL)
            {
                for(int k=0; k<tsData->dataBlock->nz[n]; k++)
                {
                    if(field[k][0] == NULL)
                    {
                        field[k][0] = new double[tsData->dataBlock->nx[n]*tsData->dataBlock->ny[n]];
                        for(int j=1; j<tsData->dataBlock->ny[n]; j++)
                        {
                            field[k][j] = field[k][0] + j * tsData->dataBlock->nx[n];
                        }
                    }
                    
                    nBytes = ReadInt();
                    m_in->read(buffer, nBytes);
                    RunLengthDecode(&field[k][0][0], tsData->dataBlock->nx[n]*tsData->dataBlock->ny[n], buffer, nBytes);
                }
            }
        }
    }
    
    delete [] buffer;
}

// ****************************************************************************
//  Method: SpyFile::GetFieldVal
//
//  Purpose:
//     Get the field value(s) of field with field ID = fieldId.
//
//
//  Arguments:
//      block   the current timestep's data
//      fieldId the field ID
//      k       z starting index
//      j       y starting index
//      is      x starting index
//      ie      x ending index
//      domain  the current block/domain
//
//  Returns: The field values or NULL if no data exists
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

double *
SpyFile::GetFieldVal(DataBlock *const block, const int fieldId,
                     const int k, const int j,
                     const int is, const int ie,
                     const int domain)
{
    int m_nmat = spyFileHeader->nmat;
    
    if(fieldId >= 0 && fieldId != 7 && fieldId != 13 && (fieldId == 0 || fieldId % STM_VOLM != 0))
    {
        double ***field = GetField(block, fieldId, domain);
        if(field != NULL)
        {
            return &field[k][j][is];
        }
        else
        {
            return NULL;
        }
    }
    
    // Derived Quantity
    double *x = block->x[domain];
    double *y = block->y[domain];
    double *z = block->z[domain];
    int caseId, matId;
    
    // Set case ID for derived material variable
    if((fieldId <= STM_TKM_1) && (fieldId > STM_TKM))
    {
        caseId = STM_TKM;
        matId = fieldId - (STM_TKM) - 1;
    }
    
    if((fieldId <= STM_DENSM_1) && (fieldId > STM_DENSM))
    {
        caseId = STM_DENSM;
        matId = fieldId - (STM_DENSM) - 1;
    }
    
    int jp1 = std::min(j+1, block->ny[domain]-1);
    int kp1 = std::min(k+1, block->nz[domain]-1);
    
    if(scratch != NULL)
    {
        delete [] scratch;
        scratch = new double[block->nx[domain]+1];
    }
    else
    {
        scratch = new double[block->nx[domain]+1];
    }
    
    for(int i=0; i<ie; i++)
    {
        scratch[i] = 0;
        
        switch (caseId)
        {
            // Velocity Magnitude
            case STM_VMAG: {
                int vx = block->cField[domain][STM_VX][k][j][i];
                
                if(spyFileHeader->ndim == 1)
                {
                    scratch[i] = std::abs((double)vx);
                }
                else if(spyFileHeader->ndim == 2)
                {
                    int vy = block->cField[domain][STM_VY][k][j][i];
                    scratch[i] = sqrt(vx*vx + vy*vy);
                }
                else
                {
                    int vy = block->cField[domain][STM_VY][k][j][i];
                    int vz = block->cField[domain][STM_VZ][k][j][i];
                    scratch[i] = sqrt(vx*vx + vy*vy + vz*vz);
                }
                break;
            }
                
            // Specific kinetic energy
            case STM_KE: {
                int vx = block->cField[domain][STM_VX][k][j][i];
                
                if(spyFileHeader->ndim == 1)
                {
                    scratch[i] = 0.5 * (vx*vx);
                }
                else if(spyFileHeader->ndim == 2)
                {
                    int vy = block->cField[domain][STM_VY][k][j][i];
                    scratch[i] = 0.5 * (vx*vx + vy*vy);
                }
                else
                {
                    int vy = block->cField[domain][STM_VY][k][j][i];
                    int vz = block->cField[domain][STM_VZ][k][j][i];
                    scratch[i] = 0.5*(vx*vx + vy*vy + vz*vz);
                }
                break;
            }
                
            // Density
            case STM_DENS: {
                // total cell mass
                double Mass = 0;
                
                for(int m=0; m<m_nmat; m++)
                {
                    Mass += block->mField[domain][1][m][k][j][i];
                }
                
                // Cell volume
                double Cvol = 0;
                
                if(spyFileHeader->igm == 11)
                {
                    Cvol = M_PI * (x[i+1]*x[i+1]-x[i]*x[i]);
                }
                else if(spyFileHeader->igm == 12)
                {
                    Cvol = 4 * M_PI/3 * (x[i+1]*x[i+1]*x[i+1] - x[i]*x[i]*x[i]);
                }
                else if(spyFileHeader->igm == 20)
                {
                    Cvol = (y[j+1]-y[j])*(x[i+1]-x[i]);
                }
                else if(spyFileHeader->igm == 21)
                {
                    Cvol = M_PI*(y[j+1]-y[j])*(x[i+1]*x[i+1]-x[i]*x[i]);
                }
                else
                {
                    Cvol = (z[k+1]-z[k])*(y[j+1]-y[j])*(x[i+1]-x[i]);
                }
                
                scratch[i] = Mass/Cvol;
                break;
            }
                
            // Material volume fraction
            case STM_MVOL:
                if(block->cField[domain][0][0][0] != NULL)
                {
                    scratch[i] = 1 - block->cField[domain][0][k][j][i];
                }
                else
                {
                    for(int m=0; m<m_nmat; m++)
                    {
                        scratch[i] += block->mField[domain][0][m][k][j][i];
                    }
                }
                break;
                
            // IE - total material specific energy
            case STM_IE: {
                // total cell mass
                double Mass = 0;
                for(int m = 0; m<m_nmat; m++)
                {
                    Mass += block->mField[domain][1][m][k][j][i];
                    scratch[i] += block->mField[domain][1][m][k][j][i] * block->mField[domain][4][m][k][j][i];
                }
                
                if(Mass > 0)
                {
                    scratch[i] /= Mass;
                }
                
                break;
            }
                
            // XX stress
            case STM_XXSTRESS:
                scratch[i] = block->cField[domain][STM_P][k][j][i] - block->cField[domain][STM_XXDEV][k][j][i];
                break;
                
            // YY stress
            case STM_YYSTRESS:
                scratch[i] = block->cField[domain][STM_P][k][j][i] - block->cField[domain][STM_YYDEV][k][j][i];
                break;
                
            // ZZ stress
            case STM_ZZSTRESS:
                scratch[i] = block->cField[domain][STM_P][k][j][i] + block->cField[domain][STM_XXDEV][k][j][i] + block->cField[domain][STM_YYDEV][k][j][i];
                break;
                
            // ZZ deviator
            case STM_ZZDEV:
                scratch[i] = -block->cField[domain][STM_XXDEV][k][j][i] - block->cField[domain][STM_YYDEV][k][j][i];
                break;
                
            // J2P
            case STM_J2P:
            case STM_VM:
                if(spyFileHeader->igm < 20)
                {
                    if(spyFileHeader->igm == 11)
                    {
                        scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_XXDEV][k][j][i] +
                                             block->cField[domain][STM_YYDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i] +
                                             block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i]));
                    }
                    else
                    {
                        scratch[i] = 3*fabs(block->cField[domain][STM_XXDEV][k][j][i])/2;
                    }
                }
                else if(spyFileHeader->igm < 30)
                {
                    scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i] * block->cField[domain][STM_XXDEV][k][j][i] +
                                         block->cField[domain][STM_YYDEV][k][j][i] * block->cField[domain][STM_YYDEV][k][j][i] +
                                         block->cField[domain][STM_XYDEV][k][j][i] * block->cField[domain][STM_XYDEV][k][j][i] +
                                         block->cField[domain][STM_XXDEV][k][j][i] * block->cField[domain][STM_YYDEV][k][j][i]));
                }
                else
                {
                    scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i] * block->cField[domain][STM_XXDEV][k][j][i] +
                                         block->cField[domain][STM_YYDEV][k][j][i] * block->cField[domain][STM_YYDEV][k][j][i] +
                                         block->cField[domain][STM_XYDEV][k][j][i] * block->cField[domain][STM_XYDEV][k][j][i] +
                                         block->cField[domain][STM_YZDEV][k][j][i] * block->cField[domain][STM_YZDEV][k][j][i] +
                                         block->cField[domain][STM_XZDEV][k][j][i] * block->cField[domain][STM_XZDEV][k][j][i] +
                                         block->cField[domain][STM_XXDEV][k][j][i] * block->cField[domain][STM_YYDEV][k][j][i]));
                }
                break;
                
            // J2PP
            case STM_J2PP:
                if (block->cField[domain][STM_P][k][j][i]>0) {
                    if(spyFileHeader->igm < 20)
                    {
                        if(spyFileHeader->igm == 11)
                        {
                            scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_XXDEV][k][j][i]
                                                 + block->cField[domain][STM_YYDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i]
                                                 + block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i])) / block->cField[domain][STM_P][k][j][i];
                        } else {
                            scratch[i] = 3*fabs(block->cField[domain][STM_XXDEV][k][j][i])/(2*block->cField[domain][STM_P][k][j][i]);
                        }
                    }
                    else if (spyFileHeader->igm < 30)
                    {
                        scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_XXDEV][k][j][i]
                                             + block->cField[domain][STM_YYDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i]
                                             + block->cField[domain][STM_XYDEV][k][j][i]*block->cField[domain][STM_XYDEV][k][j][i]
                                             + block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i])) / block->cField[domain][STM_P][k][j][i];
                    }
                    else
                    {
                        scratch[i] = sqrt(3*(block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_XXDEV][k][j][i]
                                             + block->cField[domain][STM_YYDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i]
                                             + block->cField[domain][STM_XYDEV][k][j][i]*block->cField[domain][STM_XYDEV][k][j][i]
                                             + block->cField[domain][STM_YZDEV][k][j][i]*block->cField[domain][STM_YZDEV][k][j][i]
                                             + block->cField[domain][STM_XZDEV][k][j][i]*block->cField[domain][STM_XZDEV][k][j][i]
                                             + block->cField[domain][STM_XXDEV][k][j][i]*block->cField[domain][STM_YYDEV][k][j][i])) / block->cField[domain][STM_P][k][j][i];
                    }
                }
                else
                {
                    scratch[i]=-1;
                }
                break;
                
            // CVMAG
            case STM_CVMAG: {
                int ip1 = std::min(i+1, block->nx[domain]-1);
                double vx = 0.5*(block->cField[domain][STM_VX][k][j][i]+block->cField[domain][STM_VX][k][j][ip1]);
                
                if (spyFileHeader->ndim==1)
                {
                    scratch[i] = fabs(vx);
                }
                else if(spyFileHeader->ndim==2)
                {
                    double vy = 0.5*(block->cField[domain][STM_VY][k][j][i]+block->cField[domain][STM_VY][k][jp1][i]);
                    scratch[i]= sqrt(vx*vx + vy*vy);
                }
                else
                {
                    double vy = 0.5*(block->cField[domain][STM_VY][k][j][i]+block->cField[domain][STM_VY][k][jp1][i]);
                    double vz = 0.5*(block->cField[domain][STM_VZ][k][j][i]+block->cField[domain][STM_VZ][kp1][j][i]);
                    scratch[i]= sqrt(vx*vx + vy*vy + vz*vz);
                }
                break;
            }
                
            // CVX
            case STM_CVX: {
                int ip1 = std::min(i+1, block->nx[domain]-1);
                double vx = 0.5*(block->cField[domain][STM_VX][k][j][i]+block->cField[domain][STM_VX][k][j][ip1]);
                scratch[i] = vx;
                break;
            }
                
            // CVY
            case STM_CVY: {
                if (spyFileHeader->ndim >= 2)
                {
                    double vy = 0.5*(block->cField[domain][STM_VY][k][j][i]+block->cField[domain][STM_VY][k][jp1][i]);
                    scratch[i]= vy;
                }
                else
                {
                    scratch[i]= 0.;
                }
                break;
            }
            
            // CVZ
            case STM_CVZ: {
                if (spyFileHeader->ndim==3)
                {
                    double vz = 0.5*(block->cField[domain][STM_VZ][k][j][i]+block->cField[domain][STM_VZ][kp1][j][i]);
                    scratch[i]= vz;
                }
                else
                {
                    scratch[i]= 0.;
                }
                break;
            }
                
            // DiVV
            case STM_DIVV: {
                int ip1 = std::min(i+1, block->nx[domain]-1);
                double vx  = block->cField[domain][STM_VX][k][j][i];
                double vxp = block->cField[domain][STM_VX][k][j][ip1];
                
                if (spyFileHeader->ndim==1)
                {
                    if(spyFileHeader->igm == 10)
                    {
                        scratch[i] = (vxp-vx)/(x[i+1]-x[i]);
                    }
                    else if (spyFileHeader->igm == 11)
                    {
                        scratch[i] = 2.0*(vxp*x[i+1]-vx*x[i])/((x[i+1]+x[i])*(x[i+1]-x[i]));
                    }
                    else if (spyFileHeader->igm == 12)
                    {
                        scratch[i] = 4.0*(vxp*x[i+1]*x[i+1]-vx*x[i]*x[i])/((x[i+1]+x[i])*(x[i+1]+x[i])*(x[i+1]-x[i]));
                    }
                }
                else if (spyFileHeader->ndim==2)
                {
                    double vy  = block->cField[domain][STM_VY][k][j][i];
                    double vyp = block->cField[domain][STM_VY][k][jp1][i];
                    
                    if(spyFileHeader->igm == 20)
                    {
                        scratch[i] = (vxp-vx)/(x[i+1]-x[i]) + (vyp-vy)/(y[j+1]-y[j]);
                    }
                    else
                    {
                        scratch[i] = 2.0*(vxp*x[i+1]-vx*x[i])/((x[i+1]+x[i])*(x[i+1]-x[i])) +
                        (vyp-vy)/(y[j+1]-y[j]);
                    }
                }
                else
                {
                    double vy  = block->cField[domain][STM_VY][k][j][i];
                    double vyp = block->cField[domain][STM_VY][k][jp1][i];
                    double vz  = block->cField[domain][STM_VZ][k][j][i];
                    double vzp = block->cField[domain][STM_VZ][kp1][j][i];
                    scratch[i] = (vxp-vx)/(x[i+1]-x[i]) + (vyp-vy)/(y[j+1]-y[j]) + (vzp-vz)/(z[i+1]-z[i]);
                }
                break;
            }
                
            // Cell Tempurature in K
            case STM_TK:
                scratch[i] = block->cField[domain][STM_T][k][j][i] * EV2K;
                break;
                
            // Material Tempurature in K
            case STM_TKM:
                scratch[i] = block->mField[domain][3][matId][k][j][i]*EV2K;
                break;
                
            // Material Density
            case STM_DENSM: {
                // Material mass
                double Mass = block->mField[domain][1][matId][k][j][i];
                double Cvol = 0;
                
                // Cell Volume
                if (spyFileHeader->igm==11)
                {
                    Cvol=M_PI*(x[i+1]*x[i+1]-x[i]*x[i]);
                }
                else if(spyFileHeader->igm==12)
                {
                    Cvol = 4*M_PI/3*(x[i+1]*x[i+1]*x[i+1]-x[i]*x[i]*x[i]);
                }
                else if (spyFileHeader->igm==20)
                {
                    Cvol = (y[j+1]-y[j])*(x[i+1]-x[i]);
                }
                else if (spyFileHeader->igm==21)
                {
                    Cvol = M_PI*(y[j+1]-y[j])*(x[i+1]*x[i+1]-x[i]*x[i]);
                }
                else
                {
                    Cvol = (z[k+1]-z[k])*(y[j+1]-y[j])*(x[i+1]-x[i]);
                }
                
                if(Mass>0.)
                {
                    scratch[i] = Mass/(Cvol*block->mField[domain][0][matId][k][j][i]);
                } else {
                    scratch[i] = 0.;
                }
                break;
            }
                
            // Angular momenta, 3D only
            case STM_LX: {
                double com[3];
                ComputeCenterOfMass(block, &com[0]);
                double yc=0.5*(y[j]+y[j+1])-com[1];
                double zc=0.5*(z[k]+z[k+1])-com[2];
                double vy  = block->cField[domain][STM_VY][k][j][i];
                double vyp = block->cField[domain][STM_VY][k][jp1][i];
                double vz  = block->cField[domain][STM_VZ][k][j][i];
                double vzp = block->cField[domain][STM_VZ][kp1][j][i];
                scratch[i] = 0.5*(yc*(vz+vzp)-zc*(vy+vyp));
                break;
            }
                
            case STM_LY: {
                int ip1 = std::min(i+1, block->nx[domain]-1);
                double com[3];
                ComputeCenterOfMass(block, &com[0]);
                double xc=0.5*(x[i]+x[i+1])-com[0];
                double zc=0.5*(z[k]+z[k+1])-com[2];
                double vx  = block->cField[domain][STM_VX][k][j][i];
                double vxp = block->cField[domain][STM_VX][k][j][ip1];
                double vz  = block->cField[domain][STM_VZ][k][j][i];
                double vzp = block->cField[domain][STM_VZ][kp1][j][i];
                scratch[i] = 0.5*(zc*(vx+vxp)-xc*(vz+vzp));
                break;
            }
                
            case STM_LZ: {
                int ip1 = std::min(i+1, block->nx[domain]-1);
                double com[3];
                ComputeCenterOfMass(block, &com[0]);

                double xc=0.5*(x[i]+x[i+1])-com[0];
                double yc=0.5*(y[j]+y[j+1])-com[1];
                double vx  = block->cField[domain][STM_VX][k][j][i];
                double vxp = block->cField[domain][STM_VX][k][j][ip1];
                double vy  = block->cField[domain][STM_VY][k][j][i];
                double vyp = block->cField[domain][STM_VY][k][jp1][i];
                scratch[i] = 0.5*(xc*(vy+vyp)-yc*(vx+vxp));
                break;
            }
                
                /* Cell locations */
            case STM_X:
                scratch[i] = x[i];
                break;
                
            case STM_Y:
                if (spyFileHeader->ndim>=2)
                {
                    scratch[i] = y[j];
                }
                else
                {
                    scratch[i]=0;
                }
                
                break;
                
            case STM_Z:
                if (spyFileHeader->ndim==3) scratch[i] = z[k]; else scratch[i]=0;
                break;
                
            case STM_XC:
                scratch[i] = 0.5*(x[i]+x[i+1]);
                break;
                
            case STM_YC:
                if (spyFileHeader->ndim>=2) scratch[i] = 0.5*(y[j]+y[j+1]); else scratch[i]=0;
                break;
                
            case STM_ZC:
                if (spyFileHeader->ndim==3) scratch[i] = 0.5*(z[k]+z[k+1]); else scratch[i]=0;
                break;
                
            case STM_DX:
                scratch[i] = x[i+1] - x[i];
                break;
                
            case STM_DY:
                if (spyFileHeader->ndim>=2) scratch[i] = y[j+1] - y[j]; else scratch[i]=0;
                break;
                
            case STM_DZ:
                if (spyFileHeader->ndim>=3) scratch[i] = z[k+1] - z[k]; else scratch[i]=0;
                break;
                
            default:
                debug5 << "No data for field = " << fieldId << "(" << GetFieldName(fieldId) << ")" << endl;
                return NULL;
        }
    }
    
    return &scratch[is];
}

// ****************************************************************************
//  Method: SpyFile::ComputeCenterOfMass
//
//  Purpose:
//      Compute the center of mass for the data.
//
//
//  Arguments:
//      data    The data for the current timestep
//      out     The variable used to store the calculated center of mass
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ComputeCenterOfMass(DataBlock *const data, double *out)
{
    out[0] = 0.0;
    out[1] = 0.0;
    out[2] = 0.0;
    double sum, CMx, CMy, CMz;
    int m_nmat = spyFileHeader->nmat;
    
    sum = CMx = CMy = CMz = 0;
    bool retVal = false;
    
    if(data != NULL)
    {
        for(int i=STM_M+1; i<=STM_M+m_nmat; i++)
        {
            double ***field = GetField(data, i, 0);
            if(field == NULL)
            {
                retVal = true;
                break;
            }
            else
            {
                for(int k=0; k<data->nz[0]; k++)
                {
                    for(int j=0; j<data->ny[0]; j++)
                    {
                        if(field[k][j] == NULL)
                        {
                            retVal = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if(retVal)
    {
        return;
    }
    
    for(int blkIdx=0; blkIdx<data->nBlocks; blkIdx++)
    {
        double ****mass = data->mField[blkIdx][1];
        double *x = data->x[blkIdx];
        double *y = data->y[blkIdx];
        double *z = data->z[blkIdx];
        for(int k=1; k<data->nz[blkIdx]-1; k++)
        {
            for(int j=1; j<data->ny[blkIdx]-1; j++)
            {
                for(int i=1; i<data->nx[blkIdx]-1; i++)
                {
                    double cmass = 0;
                    for(int m=0; m<m_nmat; m++)
                    {
                        cmass += mass[m][k][j][i];
                    }
                    
                    CMx += 0.5*cmass*(x[i]+x[i+1]);
                    CMy += 0.5*cmass*(y[j]+y[j+1]);
                    CMz += 0.5*cmass*(z[k]+z[k+1]);
                    sum += cmass;
                }
            }
        }
    }
    
    double tempSum = sum;   // TODO: If in parallel, there needs to be a sum reduction across all processes?? (global_dsum(sum))
    out[0] = CMx;   // global_dsum(CMx)??
    out[1] = CMy;   // global_dsum(CMy)??
    out[2] = CMz;   // global_dsum(CMz)??
    
    if(tempSum > 0)
    {
        out[0] /= tempSum;
        out[1] /= tempSum;
        out[2] /= tempSum;
    }
    else
    {
        out[0] = out[1] = out[2] = 0.0;
    }
}

// ****************************************************************************
//  Method: SpyFile::GetField
//
//  Purpose:
//      Get the cell or material field represented by fieldId.
//
//
//  Arguments:
//      block   the data for the current timestep
//      fieldId the field ID
//      idx     block/domain
//
//  Returns:    The cell or material field represented by fieldId or NULL if
//              the cell or material is not found.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

double ***
SpyFile::GetField(DataBlock *const block, const int fieldId, const int idx)
{
    if(block->allocated[idx] > 0)
    {
        // Material Field
        if(fieldId >= STM_VOLM)
        {
            if(block->mField[idx] == NULL)
            {
                return NULL;
            }
            
            int tmp = fieldId / STM_VOLM;
            tmp--;
            if(block->mField[idx][tmp] == NULL)
            {
                return NULL;
            }
            
            int tmp1 = fieldId - (tmp+1) * STM_VOLM;
            tmp1--;
            return block->mField[idx][tmp][tmp1];
        }
        else
        {
            // Cell Field
            if(block->cField[idx] == NULL) {
                return NULL;
            }
            
            return block->cField[idx][fieldId];
        }
    }
    
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetFieldId
//
//  Purpose:
//      Get the field ID associated with fieldname.
//
//
//  Arguments:
//      fieldname   the field name
//
//  Returns:    The field ID associated with fieldname
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetFieldId(const char *fieldname)
{
    int i, len, lenm, m;
    Field *m_cellfields = spyFileHeader->cellFields;
    Field *m_matfields = spyFileHeader->matFields;
    
    len = strlen(fieldname);
    
    for(i=0; i<spyFileHeader->nCellFields; i++)
    {
        if(std::string(fieldname).compare(m_cellfields[i].fieldID) == 0)
        {
            return m_cellfields[i].fieldInt;
        }
    }
    
    for (i=0; i<len; i++)
    {
        if (fieldname[i] == '+') break;
    }
    
    len=i;
    if (len < strlen(fieldname))
    {
        sscanf(&fieldname[len+1],"%d",&m);
    }
    else
    {
        m=0;
    }
    
    for (i=0; i<spyFileHeader->nMatFields; i++)
    {
        if(std::string(fieldname).compare(0, len, m_matfields[i].fieldID) == 0)
        {
            lenm = strlen(m_matfields[i].fieldID);
            if(len == lenm)
            {
                return m_matfields[i].fieldInt + m;
            }
        }
    }
    
    if (strncmp(fieldname,"TKM",3)==0) {
        return STM_TKM+m;
    } else if (strncmp(fieldname,"DENSM",5)==0) {
        return STM_DENSM+m;
    } else if (strcmp(fieldname,"VMAG")==0) {
        return STM_VMAG;
    } else if (strcmp(fieldname,"KE")==0) {
        return STM_KE;
    } else if (strcmp(fieldname,"DENS")==0) {
        return STM_DENS;
    } else if (strcmp(fieldname,"IE")==0) {
        return STM_IE;
    } else if (strcmp(fieldname,"XXSTRESS")==0) {
        return STM_XXSTRESS;
    } else if (strcmp(fieldname,"YYSTRESS")==0) {
        return STM_YYSTRESS;
    } else if (strcmp(fieldname,"ZZSTRESS")==0) {
        return STM_ZZSTRESS;
    } else if (strcmp(fieldname,"ZZDEV")==0) {
        return STM_ZZDEV;
    } else if (strcmp(fieldname,"TK")==0) {
        return STM_TK;
    } else if (strcmp(fieldname,"J2P")==0) {
        return STM_J2P;
    } else if (strcmp(fieldname,"VM")==0) {
        return STM_VM;
    } else if (strcmp(fieldname,"J2PP")==0) {
        return STM_J2PP;
    } else if (strcmp(fieldname,"CVMAG")==0) {
        return STM_CVMAG;
    } else if (strcmp(fieldname,"CVX")==0) {
        return STM_CVX;
    } else if (strcmp(fieldname,"CVY")==0) {
        return STM_CVY;
    } else if (strcmp(fieldname,"CVZ")==0) {
        return STM_CVZ;
    } else if (strcmp(fieldname,"DIVV")==0) {
        return STM_DIVV;
    } else if (strcmp(fieldname,"LX")==0) {
        return STM_LX;
    } else if (strcmp(fieldname,"LY")==0) {
        return STM_LY;
    } else if (strcmp(fieldname,"LZ")==0) {
        return STM_LZ;
    } else if (strcmp(fieldname,"X")==0) {
        return STM_X;
    } else if (strcmp(fieldname,"Y")==0) {
        return STM_Y;
    } else if (strcmp(fieldname,"Z")==0) {
        return STM_Z;
    } else if (strcmp(fieldname,"XC")==0) {
        return STM_XC;
    } else if (strcmp(fieldname,"YC")==0) {
        return STM_YC;
    } else if (strcmp(fieldname,"ZC")==0) {
        return STM_ZC;
    } else if (strcmp(fieldname,"DX")==0) {
        return STM_DX;
    } else if (strcmp(fieldname,"DY")==0) {
        return STM_DY;
    } else if (strcmp(fieldname,"DZ")==0) {
        return STM_DZ;
    } else if (strcmp(fieldname,"MVOL")==0) {
        return STM_MVOL;
    } else if (strcmp(fieldname,"ALL")==0) {
        return -9999;
    } else if (strcmp(fieldname,"GLOBAL")==0) {
        return -9998;
    } else if (strcmp(fieldname,"MAT_GLOBAL")==0) {
        return -9997;
    } else if (strcmp(fieldname,"POSITION")==0) {
        return -9996;
    }
    
    return -1;
}

// ****************************************************************************
//  Method: SpyFile::GetFieldName
//
//  Purpose:
//      To get the name of the field associated with the field ID.
//
//
//  Arguments:
//      fieldId The ID of the field
//
//  Returns:    The field name associated with the field ID
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
char *
SpyFile::GetFieldName(const int fieldId)
{
    char *tmpstr = new char[80];
    
    // Material Field
    if(fieldId >= STM_VOLM)
    {
        int tmp = fieldId / STM_VOLM;
        int tmp1 = fieldId - tmp * STM_VOLM;
        tmp--;
        
        if(tmp1 > 0)
        {
            snprintf(tmpstr, 80, "%s+%d", spyFileHeader->matFields[tmp].fieldID, tmp1);
            return tmpstr;
        }
        else
        {
            return spyFileHeader->matFields[tmp].fieldID;
        }
    }
    else if((fieldId >= 0) && (fieldId < STM_VOLM))
    {
        // Cell Field
        return spyFileHeader->cellFields[fieldId].fieldID;
    }
    else
    {
        // Derived Field
        int matId = -1;
        int caseId = fieldId;
        
        if((fieldId <= STM_TKM_1) && (fieldId >= STM_TKM))
        {
            caseId = STM_TKM;
            matId = fieldId - (STM_TKM);
        }
        
        if((fieldId <= STM_DENSM_1) && (fieldId >= STM_DENSM))
        {
            caseId = STM_DENSM;
            matId = fieldId - (STM_DENSM);
        }
        
        switch(caseId)
        {
            case STM_VMAG:
                snprintf(tmpstr, 80, "%s", "VMAG");
                break;
                
            case STM_KE:
                snprintf(tmpstr, 80, "%s", "KE");
                break;
                
            case STM_DENS:
                snprintf(tmpstr, 80, "%s", "DENS");
                break;
                
            case STM_DENSM:
                if(matId > 0)
                {
                    snprintf(tmpstr, 80, "%s+%d", "DENSM", matId);
                }
                else
                {
                    snprintf(tmpstr, 80, "%s", "DENSM");
                }
                break;
                
            case STM_IE:
                snprintf(tmpstr, 80, "%s", "IE");
                break;
                
            case STM_XXSTRESS:
                snprintf(tmpstr, 80, "%s", "XXSTRESS");
                break;
                
            case STM_YYSTRESS:
                snprintf(tmpstr, 80, "%s", "YYSTRESS");
                break;
                
            case STM_ZZSTRESS:
                snprintf(tmpstr, 80, "%s", "ZZSTRESS");
                break;
                
            case STM_ZZDEV:
                snprintf(tmpstr, 80, "%s", "ZZDEV");
                break;
                
            case STM_TK:
                snprintf(tmpstr, 80, "%s", "TK");
                break;
                
            case STM_TKM:
                if(matId > 0)
                {
                    snprintf(tmpstr, 80, "%s+%d", "TKM", matId);
                }
                else
                {
                    snprintf(tmpstr, 80, "%s", "TKM");
                }
                break;
                
            case STM_J2P:
                snprintf(tmpstr, 80, "%s","J2P");
                break;
                
            case STM_VM:
                snprintf(tmpstr, 80, "%s","von Mises");
                break;
                
            case STM_J2PP:
                snprintf(tmpstr, 80, "%s","J2PP");
                break;
                
            case STM_CVMAG:
                snprintf(tmpstr, 80, "%s","CVMAG");
                break;
                
            case STM_CVX:
                snprintf(tmpstr, 80, "%s","CVX");
                break;
                
            case STM_CVY:
                snprintf(tmpstr, 80, "%s","CVY");
                break;
                
            case STM_CVZ:
                snprintf(tmpstr, 80, "%s","CVZ");
                break;
                
            case STM_MVOL:
                snprintf(tmpstr, 80, "%s","MVOL");
                break;
                
            case STM_DIVV:
                snprintf(tmpstr, 80, "%s","DIVV");
                break;
                
            case STM_LX:
                snprintf(tmpstr, 80, "%s","LX");
                break;
                
            case STM_LY:
                snprintf(tmpstr, 80, "%s","LY");
                break;
                
            case STM_LZ:
                snprintf(tmpstr, 80, "%s","LZ");
                break;
                
            case STM_X:
                snprintf(tmpstr, 80, "%s","X");
                break;
                
            case STM_Y:
                snprintf(tmpstr, 80, "%s","Y");
                break;
                
            case STM_Z:
                snprintf(tmpstr, 80, "%s","Z");
                break;
                
            case STM_XC:
                snprintf(tmpstr, 80, "%s","XC");
                break;
                
            case STM_YC:
                snprintf(tmpstr, 80, "%s","YC");
                break;
                
            case STM_ZC:
                snprintf(tmpstr, 80, "%s","ZC");
                break;
                
            case STM_DX:
                snprintf(tmpstr, 80, "%s","DX");
                break;
                
            case STM_DY:
                snprintf(tmpstr, 80, "%s","DY");
                break;
                
            case STM_DZ:
                snprintf(tmpstr, 80, "%s","DZ");
                break;
                
            case -9999:
                snprintf(tmpstr, 80, "%s","ALL");
                break;
                
            case -9998:
                snprintf(tmpstr, 80, "%s","GLOBAL");
                break;
                
            case -9997:
                snprintf(tmpstr, 80, "%s","MAT_GLOBAL");
                break;
                
            case -9996:
                snprintf(tmpstr, 80, "%s","POSITION");
                break;
                
            default:
                snprintf(tmpstr, 80, "%s","DEFAULT-GFN");
        }
        
        return tmpstr;
    }
}

// ****************************************************************************
//  Method: SpyFile::GetFieldComment
//
//  Purpose:
//      To get the comment for the field associated with the field ID. The
//      field comment stores the units.//
//
//  Arguments:
//      fieldId The field ID
//
//  Returns:    The field name associated with the field ID
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

char *
SpyFile::GetFieldComment(const int field_id)
{
    int tmp,case_id;
    int size = 80;
    char * DerivedFieldComment = new char[size];
    
    /* Material field */
    
    if (field_id >= STM_VOLM)
    {
        tmp=field_id/STM_VOLM;
        tmp--;
        return spyFileHeader->matFields[tmp].fieldComment;
        
    }
    else if((field_id>=0) && (field_id<STM_VOLM))
    {
        
        /* Cell field */
        
        return spyFileHeader->cellFields[field_id].fieldComment;
        
    }
    else
    {
        /* Derived field */
        
        case_id = field_id;
        /* Set case_id for derived material variable */
        
        if((field_id<=STM_TKM_1) && (field_id>=STM_TKM)) {
            case_id = STM_TKM;
        }
        if((field_id<=STM_DENSM_1) && (field_id>=STM_DENSM)) {
            case_id = STM_DENS;
        }
        
        switch(case_id) {
                
            case STM_VMAG:
                sprintf(DerivedFieldComment,"%s","Vmag (cm/s)");
                break;
                
            case STM_KE:
                sprintf(DerivedFieldComment,"%s","Kinetic Energy (erg/g)");
                break;
                
            case STM_DENS:
                sprintf(DerivedFieldComment,"%s","Density (g/cm^3^)");
                break;
                
            case STM_IE:
                sprintf(DerivedFieldComment,"%s","Internal Energy (erg/g)");
                break;
                
            case STM_XXSTRESS:
                sprintf(DerivedFieldComment,"%s","XX stress (dyn/cm^2^)");
                break;
                
            case STM_YYSTRESS:
                sprintf(DerivedFieldComment,"%s","YY stress (dyn/cm^2^)");
                break;
                
            case STM_ZZSTRESS:
                sprintf(DerivedFieldComment,"%s","ZZ stress (dyn/cm^2^)");
                break;
                
            case STM_ZZDEV:
                sprintf(DerivedFieldComment,"%s","ZZ stress deviator (dyn/cm^2^)");
                break;
                
            case STM_TK:
                sprintf(DerivedFieldComment,"%s","Temperature (K)");
                break;
                
            case STM_TKM:
                sprintf(DerivedFieldComment,"%s","Temperature (K)");
                break;
                
            case STM_J2P:
                sprintf(DerivedFieldComment,"%s","J2P (dyn/cm^2^)");
                break;
                
            case STM_VM:
                sprintf(DerivedFieldComment,"%s","von Mises (dyn/cm^2^)");
                break;
                
            case STM_J2PP:
                sprintf(DerivedFieldComment,"%s","J2P/P");
                break;
                
            case STM_MVOL:
                sprintf(DerivedFieldComment,"%s","Material Volume Fraction");
                break;
                
            case STM_CVMAG:
                sprintf(DerivedFieldComment,"%s","Vmag (cm/s)");
                break;
                
            case STM_CVX:
                sprintf(DerivedFieldComment,"%s","CVX (cm/s)");
                break;
                
            case STM_CVY:
                sprintf(DerivedFieldComment,"%s","CVY (cm/s)");
                break;
                
            case STM_CVZ:
                sprintf(DerivedFieldComment,"%s","CVZ (cm/s)");
                break;
                
            case STM_DIVV:
                sprintf(DerivedFieldComment,"%s","Div V (s^-1^)");
                break;
                
            case STM_LX:
                sprintf(DerivedFieldComment,"%s","Specific Angular Momentum X (cm^2^/s)");
                break;
                
            case STM_LY:
                sprintf(DerivedFieldComment,"%s","Specific Angular Momentum Y (cm^2^/s)");
                break;
                
            case STM_LZ:
                sprintf(DerivedFieldComment,"%s","Specific Angular Momentum Z (cm^2^/s)");
                break;
                
            case STM_X:
                sprintf(DerivedFieldComment,"%s","X (cm)");
                break;
                
            case STM_Y:
                sprintf(DerivedFieldComment,"%s","Y (cm)");
                break;
                
            case STM_Z:
                sprintf(DerivedFieldComment,"%s","Z (cm)");
                break;
                
            case STM_XC:
                sprintf(DerivedFieldComment,"%s","X (cm)");
                break;
                
            case STM_YC:
                sprintf(DerivedFieldComment,"%s","Y (cm)");
                break;
                
            case STM_ZC:
                sprintf(DerivedFieldComment,"%s","Z (cm)");
                break;
                
            case STM_DX:
                sprintf(DerivedFieldComment,"%s","DX (cm)");
                break;
                
            case STM_DY:
                sprintf(DerivedFieldComment,"%s","DY (cm)");
                break;
                
            case STM_DZ:
                sprintf(DerivedFieldComment,"%s","DZ (cm)");
                break;
                
            default:
                sprintf(DerivedFieldComment,"%s","DEFAULT-DFC");
                break;
        }
        
        return DerivedFieldComment;
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadBlockGeometries
//
//  Purpose:
//      Read in the number of blocks and the block geometries.
//
//
//  Returns:    The number of tracers read in or -1 on failure.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadBlockGeometries()
{
    tsData->dataBlock = new DataBlock();
    
    int nBlocks = ReadInt();
    tsData->dataBlock->nBlocks = nBlocks;
    debug5 << "Number of Blocks: " << nBlocks << endl;
    
    // Alloc arrays
    tsData->dataBlock->nx = new int[nBlocks];
    tsData->dataBlock->ny = new int[nBlocks];
    tsData->dataBlock->nz = new int[nBlocks];
    
    tsData->dataBlock->allocated = new int[nBlocks];
    tsData->dataBlock->active = new int[nBlocks];
    tsData->dataBlock->level = new int[nBlocks];
    
    if(spyFileHeader->version >= 103)
    {
        tsData->dataBlock->bxBot = new int[nBlocks];
        tsData->dataBlock->bxTop = new int[nBlocks];
        tsData->dataBlock->byBot = new int[nBlocks];
        tsData->dataBlock->byTop = new int[nBlocks];
        tsData->dataBlock->bzBot = new int[nBlocks];
        tsData->dataBlock->bzTop = new int[nBlocks];
    }
    
    tsData->dataBlock->x = new double *[nBlocks];
    tsData->dataBlock->y = new double *[nBlocks];
    tsData->dataBlock->z = new double *[nBlocks];
    
    tsData->dataBlock->xBytes = new int[nBlocks];
    tsData->dataBlock->yBytes = new int[nBlocks];
    tsData->dataBlock->zBytes = new int[nBlocks];
    
    tsData->dataBlock->cField = new double ****[nBlocks];
    tsData->dataBlock->mField = new double *****[nBlocks];
    
    int max = 0;
    int tmp = 0;
    for(int n=0; n<nBlocks; n++)
    {
        
        tsData->dataBlock->nx[n] = ReadInt();
        tsData->dataBlock->ny[n] = ReadInt();
        tsData->dataBlock->nz[n] = ReadInt();
        
        tsData->dataBlock->allocated[n] = ReadInt();
        tsData->dataBlock->active[n] = ReadInt();
        tsData->dataBlock->level[n] = ReadInt();
        
        if(spyFileHeader->version >= 103)
        {
            tsData->dataBlock->bxBot[n] = ReadInt();
            tsData->dataBlock->bxTop[n] = ReadInt();
            tsData->dataBlock->byBot[n] = ReadInt();
            tsData->dataBlock->byTop[n] = ReadInt();
            tsData->dataBlock->bzBot[n] = ReadInt();
            tsData->dataBlock->bzTop[n] = ReadInt();
        }
        
        tmp = 5*std::max(tsData->dataBlock->nx[n]+1, std::max(tsData->dataBlock->ny[n]+1, tsData->dataBlock->nz[n]+1))+8;
        if(tmp > max)
        {
            max = tmp;
        }
        
        // Allocate space for the geometery arrays
        tsData->dataBlock->x[n] = new double[tsData->dataBlock->nx[n]+1];
        tsData->dataBlock->y[n] = new double[tsData->dataBlock->ny[n]+1];
        tsData->dataBlock->z[n] = new double[tsData->dataBlock->nz[n]+1];
    }
    
    char *buffer = new char[max];
    
    // Loop over the blocks read and uncompress the geometries of allocated blocks
    for(int n=0; n<nBlocks; n++)
    {
        if(tsData->dataBlock->allocated[n] > 0)
        {
            // x
            tsData->dataBlock->xBytes[n] = ReadInt();
            m_in->read(buffer, tsData->dataBlock->xBytes[n]);
            RunLengthDecodeTrend(tsData->dataBlock->x[n], tsData->dataBlock->nx[n]+1, buffer, tsData->dataBlock->xBytes[n]);
            
            // y
            tsData->dataBlock->yBytes[n] = ReadInt();
            m_in->read(buffer, tsData->dataBlock->yBytes[n]);
            RunLengthDecodeTrend(tsData->dataBlock->y[n], tsData->dataBlock->ny[n]+1, buffer, tsData->dataBlock->yBytes[n]);
            
            // z
            tsData->dataBlock->zBytes[n] = ReadInt();
            m_in->read(buffer, tsData->dataBlock->zBytes[n]);
            RunLengthDecodeTrend(tsData->dataBlock->z[n], tsData->dataBlock->nz[n]+1, buffer, tsData->dataBlock->zBytes[n]);
        }
    }
    
    delete [] buffer;
    
    // Allocate Variable Pointers
    int ny, nz;
    int m_nmat = spyFileHeader->nmat;
    
    for(int n=0; n<nBlocks; n++)
    {
        ny = tsData->dataBlock->ny[n];
        nz = tsData->dataBlock->nz[n];
        
        if(tsData->dataBlock->allocated[n] > 0)
        {
            // Cell Fields
            tsData->dataBlock->cField[n] = new double ***[spyFileHeader->nCellFields];
            for(int l=0; l< spyFileHeader->nCellFields; l++)
            {
                tsData->dataBlock->cField[n][l] = new double **[nz];
                double ***field = tsData->dataBlock->cField[n][l];
                for(int k=0; k<nz; k++)
                {
                    field[k] = new double *[ny];
                    for(int j=0; j<ny; j++)
                    {
                        field[k][j] = NULL;
                    }
                }
            }
            
            // Material Fields
            tsData->dataBlock->mField[n] = new double ****[spyFileHeader->nMatFields];
            for(int l=0; l<spyFileHeader->nMatFields; l++)
            {
                tsData->dataBlock->mField[n][l] = new double ***[m_nmat];
                for(int m=0; m<m_nmat; m++)
                {
                    tsData->dataBlock->mField[n][l][m] = new double **[nz];
                    double ***field = tsData->dataBlock->mField[n][l][m];
                    for(int k=0; k<nz; k++)
                    {
                        field[k] = new double *[ny];
                        for(int j=0; j<ny; j++)
                        {
                            field[k][j] = NULL;
                        }
                    }
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadHistogramData
//
//  Purpose:
//      Input the histogram data.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadHistogramData()
{
    tsData->histogram = new Histogram();
    int numIndicators = ReadInt();
    debug5 << "Number of Indicators: " << numIndicators << endl;
    
    if(numIndicators > 0)
    {
        tsData->histogram->numIndicators = numIndicators;
        tsData->histogram->maxBin = ReadInt();
        
        // Allocate storage
        tsData->histogram->histogram = new double *[numIndicators];
        tsData->histogram->histMin = new double[numIndicators];
        tsData->histogram->histMax = new double[numIndicators];
        tsData->histogram->histType = new int[numIndicators];
        tsData->histogram->refAbove = new double[numIndicators];
        tsData->histogram->refBelow = new double[numIndicators];
        tsData->histogram->unrAbove = new double[numIndicators];
        tsData->histogram->unrBelow = new double[numIndicators];
        tsData->histogram->nBins = new int[numIndicators];
        
        // Loop over the indicators and input
        for(int i=0; i<numIndicators; i++)
        {
            tsData->histogram->histType[i] = ReadInt();
            ReadDouble(&(tsData->histogram->refAbove[i]), 1);
            ReadDouble(&(tsData->histogram->refBelow[i]), 1);
            ReadDouble(&(tsData->histogram->unrAbove[i]), 1);
            ReadDouble(&(tsData->histogram->unrBelow[i]), 1);
            ReadDouble(&(tsData->histogram->histMin[i]), 1);
            ReadDouble(&(tsData->histogram->histMax[i]), 1);
            int nBins = ReadInt();
            tsData->histogram->nBins[i] = nBins;
            
            if(nBins > 0)
            {
                tsData->histogram->histogram[i] = new double[nBins];
                char *buffer = new char[5*nBins+8];
                
                int numBytes = ReadInt();
                m_in->read(buffer, numBytes);
                RunLengthDecode(tsData->histogram->histogram[i], nBins, buffer, numBytes);
                
                delete [] buffer;
            }
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadTracer
//
//  Purpose:
//      Input the number of tracers and tracer locations
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
void
SpyFile::ReadTracer()
{
    tsData->tracer = new Tracer();
    
    int size = ReadInt();
    debug5 << "Tracer count: " << size << endl;
    
    if(size > 0)
    {
        tsData->tracer->numTracers = size;
        char *buffer = new char[5*size+8];
        
        // X Tracer
        int numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->xTracer != NULL)
        {
            delete [] tsData->tracer->xTracer;
        }
        tsData->tracer->xTracer = new double[size];
        RunLengthDecode(tsData->tracer->xTracer, size, buffer, numBytes);
        
        // Y Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->yTracer != NULL)
        {
            delete [] tsData->tracer->yTracer;
        }
        tsData->tracer->yTracer = new double[size];
        RunLengthDecode(tsData->tracer->yTracer, size, buffer, numBytes);
        
        // Z Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->zTracer != NULL)
        {
            delete [] tsData->tracer->zTracer;
        }
        tsData->tracer->zTracer = new double[size];
        RunLengthDecode(tsData->tracer->zTracer, size, buffer, numBytes);
        
        // L Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->lTracer != NULL)
        {
            delete [] tsData->tracer->lTracer;
        }
        tsData->tracer->lTracer = reinterpret_cast<int *>(new double[size]);
        RunLengthDecodeInt(tsData->tracer->lTracer, size, buffer, numBytes);
        
        // I Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->iTracer != NULL)
        {
            delete [] tsData->tracer->iTracer;
        }
        tsData->tracer->iTracer = reinterpret_cast<int *>(new double[size]);
        RunLengthDecodeInt(tsData->tracer->iTracer, size, buffer, numBytes);
        
        // J Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->jTracer != NULL)
        {
            delete [] tsData->tracer->jTracer;
        }
        tsData->tracer->jTracer = reinterpret_cast<int *>(new double[size]);
        RunLengthDecodeInt(tsData->tracer->jTracer, size, buffer, numBytes);
        
        // K Tracer
        numBytes = ReadInt();
        m_in->read(buffer, numBytes);
        
        if(tsData->tracer->kTracer != NULL)
        {
            delete [] tsData->tracer->kTracer;
        }
        tsData->tracer->kTracer = reinterpret_cast<int *>(new double[size]);
        RunLengthDecodeInt(tsData->tracer->kTracer, size, buffer, numBytes);
        
        delete [] buffer;
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadDumpHeader
//
//  Purpose:
//      The dump header contains information specific to each dump.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadDumpHeader()
{
    tsData->dumpHeader = new DumpHeader();
    
    int cnt = ReadInt();
    debug5 << "Saved Vars count: " << cnt << endl;
    
    tsData->dumpHeader->numVars = cnt;
    tsData->dumpHeader->vars = new int[cnt];
    tsData->dumpHeader->varsOffset = new double[cnt];
    
    ReadInt(tsData->dumpHeader->vars, cnt);
    ReadDouble(tsData->dumpHeader->varsOffset, cnt);
    
    if(spyFileHeader->version <= 103)
    {
        for(int i=0; i<cnt; i++)
        {
            if(tsData->dumpHeader->vars[i] >= 100)
            {
                int tmp = tsData->dumpHeader->vars[i] / 100;
                tsData->dumpHeader->vars[i] = tmp * STM_VOLM + tsData->dumpHeader->vars[i] - 100 * tmp;
            }
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadTimestepData
//
//  Purpose:
//      Read the data for timestep ts.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadTimestepData(const int ts)
{
    if(!spyFileHeader->dumps->at(ts)->loaded)
    {
        if(tsData != NULL)
        {
            FreeUpTimestep();
        }
        
        tsData = new TimestepData();
        
        m_in->seekg(spyFileHeader->dumps->at(ts)->offset, m_in->beg);
        ReadDumpHeader();
        ReadTracer();
        ReadHistogramData();
        ReadBlockGeometries();
        
        // Input the varaiable data for all blocks and kplanes
        for(int j=0; j<tsData->dumpHeader->numVars; j++)
        {
            m_in->seekg(tsData->dumpHeader->varsOffset[j], m_in->beg);
            ReadVariableData(tsData->dumpHeader->vars[j]);
        }
        
        if(IsMarkOn() && spyFileHeader->version >= 105)
        {
            ReadMarkersVars();
        }
        
        spyFileHeader->dumps->at(ts)->loaded = true;
        m_timestep = ts;
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadFileHeader
//
//  Purpose:
//      Read the SPCTH (Spy) file header.
//
//  Returns:    true if the Spy file header was read successfully, otherwise false.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
bool
SpyFile::ReadFileHeader()
{
    if(m_in->is_open())
    {
        if(spyFileHeader != NULL)
        {
            FreeUpFileHeader();
        }
        
        spyFileHeader = new FileHeaderInfo();
        
        // Magic String
        spyFileHeader->magicStr = new char[MAGIC_STR_BYTES];
        m_in->read(spyFileHeader->magicStr, MAGIC_STR_BYTES);
        debug5 << "Magic Str: " << spyFileHeader->magicStr << endl;
        
        if(strncmp("spydata", spyFileHeader->magicStr, 7) != 0)
        {
            m_in->close();
            debug5 << "SpyFile::ReadFileHeader: Incorrect Magic String, received " << spyFileHeader->magicStr << ", but expecting spydata" << endl;
            return false;
        }
        
        // File Title
        spyFileHeader->title = new char[FILE_TITLE_BYTES];
        m_in->read(spyFileHeader->title, FILE_TITLE_BYTES);
        debug5 << "File Title: " << spyFileHeader->title << endl;
        
        // File Version
        spyFileHeader->version = ReadInt();
        debug5 << "File version: " <<  spyFileHeader->version << endl;
        
        if(spyFileHeader->version < 100)
        {
            debug5 << "SpyFile::ReadFileHeader: Error: Unsupported file version: " << spyFileHeader->version << endl;
            return false;
        }
        
        if(spyFileHeader->version != 105)
        {
            debug5 << "SpyFile::ReadFileHeader: Warning: Some features may not be supported for file version = " << spyFileHeader->version << endl;
        }
        
        // File Pointer Size
        if(spyFileHeader->version >= 102)
        {
            spyFileHeader->fileptrSize = ReadInt();
            debug5 << "File Ptr Size: " << spyFileHeader->fileptrSize << endl;
        }
        
        // Compression Flag
        spyFileHeader->compression = ReadInt();
        debug5 << "Compression: " <<  spyFileHeader->compression << endl;
        
        // Processor ID
        spyFileHeader->processorId = ReadInt();
        debug5 << "Processor ID: " <<  spyFileHeader->processorId << endl;
        
        // Number of processors
        spyFileHeader->nprocs = ReadInt();
        debug5 << "Number of processors: " <<  spyFileHeader->nprocs << endl;
        
        // IGM - Specifies problem dimensions (10=1DR, 11=1DC, 12=1DS, 20=2DR, 21=2DC, 30=3DR)
        spyFileHeader->igm = ReadInt();
        debug5 << "IGM: " <<  spyFileHeader->igm << endl;
        
        // Number of dimensions
        spyFileHeader->ndim = ReadInt();
        debug5 << "Number of Dimensions: " <<  spyFileHeader->ndim << endl;
        if(spyFileHeader->ndim == 1)
        {
            m_in->close();
            debug5 << "SpyFile::ReadFileHeader: Incorrect number of dimensions, received " << spyFileHeader->ndim << ", but VisIt currently only supports 2 or 3 for this type of file." << endl;
            return false;
        }
        
        // Number of materials
        spyFileHeader->nmat = ReadInt();
        debug5 << "Number of Materials: " <<  spyFileHeader->nmat << endl;
        
        // Maximum number of materials (98 for current CTH)
        spyFileHeader->maxMat = ReadInt();
        debug5 << "Max Materials: " <<  spyFileHeader->maxMat << endl;
        
        // Gmin
        spyFileHeader->gMin = new double[3];
        ReadDouble(spyFileHeader->gMin, 3);
        debug5 << "Gmin: " << spyFileHeader->gMin[0] << ", " << spyFileHeader->gMin[1] << ", " << spyFileHeader->gMin[2] << endl;
        
        // Gmax
        spyFileHeader->gMax = new double[3];
        ReadDouble(spyFileHeader->gMax, 3);
        debug5 << "Gmax: " << spyFileHeader->gMax[0] << ", " << spyFileHeader->gMax[1] << ", " << spyFileHeader->gMax[2] << endl;
        
        // Number of AMR blocks
        spyFileHeader->blocks = ReadInt();
        debug5 << "Number of AMR Blocks: " << spyFileHeader->blocks  << endl;
        
        // Marker Flag
        if(spyFileHeader->version >= 105)
        {
            spyFileHeader->markIsOn = ReadInt();
            
            debug5 << "Marker: " << spyFileHeader->markIsOn << endl;
        }
        
        if(IsMarkOn() && spyFileHeader->version >= 105)
        {
            ReadMarkerHeader();
        }
        
        // Maximum level of AMR refinement
        spyFileHeader->maxLevel = ReadInt();
        debug5 << "Max level of AMR refinement: " << spyFileHeader->maxLevel << endl;
        
        if(spyFileHeader->version >= 105)
        {
            spyFileHeader->mfIsOn = ReadInt();
            debug5 << "mf_is_on: " << spyFileHeader->mfIsOn  << endl;
            
            // Dummy Integer flags for future capabilities (SpyVersion >= 105)
            spyFileHeader->headerJunk = new int[4];
            ReadInt(spyFileHeader->headerJunk, 4);
            debug5 << "Junk: " << spyFileHeader->headerJunk[0] << ", " << spyFileHeader->headerJunk[1] << ", " << spyFileHeader->headerJunk[2] << ", " << spyFileHeader->headerJunk[3] << endl;
        }
        
        // Cell Fields
        spyFileHeader->nCellFields = ReadInt();
        debug5 << "Number of cell fields: " << spyFileHeader->nCellFields << endl;
        
        if(spyFileHeader->nCellFields > 0)
        {
            spyFileHeader->cellFields = new Field[spyFileHeader->nCellFields];
            
            for(int i=0; i<spyFileHeader->nCellFields; i++)
            {
                spyFileHeader->cellFields[i].fieldID = new char[30];
                m_in->read(spyFileHeader->cellFields[i].fieldID, 30);
                debug5 << "Cell Field ID: " << spyFileHeader->cellFields[i].fieldID << endl;
                
                spyFileHeader->cellFields[i].fieldComment = new char[80];
                m_in->read(spyFileHeader->cellFields[i].fieldComment, 80);
                debug5 << "Cell Field Comment: " << spyFileHeader->cellFields[i].fieldComment << endl;
                
                if(spyFileHeader->version >= 101)
                {
                    spyFileHeader->cellFields[i].fieldInt = ReadInt();
                }
                else
                {
                    spyFileHeader->cellFields[i].fieldInt = i;
                }
                debug5 << "Cell Field Int: " << spyFileHeader->cellFields[i].fieldInt << endl;
            }
        }
        
        // Material Field
        spyFileHeader->nMatFields = ReadInt();
        debug5 << "Number of material fields: " << spyFileHeader->nMatFields << endl;
        
        if(spyFileHeader->nMatFields > 0)
        {
            spyFileHeader->matFields = new Field[spyFileHeader->nMatFields];
            
            for(int i=0; i<spyFileHeader->nMatFields; i++)
            {
                spyFileHeader->matFields[i].fieldID = new char[30];
                m_in->read(spyFileHeader->matFields[i].fieldID, 30);
                debug5 << "Material Field ID: " << spyFileHeader->matFields[i].fieldID << endl;
                
                spyFileHeader->matFields[i].fieldComment = new char[80];
                m_in->read(spyFileHeader->matFields[i].fieldComment, 80);
                debug5 << "Material Field Comment: " << spyFileHeader->matFields[i].fieldComment << endl;
                
                if(spyFileHeader->version >= 101)
                {
                    spyFileHeader->matFields[i].fieldInt = ReadInt();
                }
                else
                {
                    spyFileHeader->matFields[i].fieldInt = (i+1)*100;
                }
                
                if(spyFileHeader->version <= 103)
                {
                    spyFileHeader->matFields[i].fieldInt *= 1000;
                }
                debug5 << "Material Field Int: " << spyFileHeader->matFields[i].fieldInt << endl;
            }
        }
        
        // Groups
        ReadDouble(&spyFileHeader->firstGroup, 1);
        debug5 << "first offset: " << spyFileHeader->firstGroup << endl;
        
        ReadGroups();
        debug5 << "Total Number of dumps: " << spyFileHeader->dumps->size() << endl;
    }
    else
    {
        debug5 << "SpyFile::ReadFileHeader: File not open for reading" << endl;
        return false;
    }
    
    return true;
}

// ****************************************************************************
//  Method: SpyFile::ReadGroups
//
//  Purpose:
//      Read the dump groups from the file.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadGroups()
{
    m_last_group = spyFileHeader->firstGroup;
    spyFileHeader->dumps = new std::vector<Dump *>();
    int localNdumps = 0;
    
    do {
        ReadGroupHeader();
        debug5 << "Number of dumps: " << spyFileHeader->ndumps << endl;

        for(int i=0; i<spyFileHeader->ndumps; i++)
        {
            double dt = m_group_header->dumpDT != NULL ? m_group_header->dumpDT[i] : 0;
            
            Dump *dump = new Dump(m_group_header->dumpCycles[i],
                                  m_group_header->dumpTimes[i],
                                  dt,
                                  m_group_header->dumpOffsets[i]);
            spyFileHeader->dumps->push_back(dump);
        }
        
        localNdumps += spyFileHeader->ndumps;
        
        if(spyFileHeader->ndumps == MAX_DUMPS)
        {
            m_last_group = m_group_header->lastOffset;
        }
    } while(spyFileHeader->ndumps == MAX_DUMPS);
    
    spyFileHeader->ndumps = localNdumps;
}

// ****************************************************************************
//  Method: SpyFile::ReadGroupHeader
//
//  Purpose:
//      The group header contains some meta-data about the all of the ‘dumps’
//      including time, cycles, and file pointer into the data block for each
//      time step.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadGroupHeader()
{
    if(m_in == NULL)
    {
        return;
    }
    
    m_in->seekg(m_last_group, m_in->beg);
    
    // Allocate Memory
    if(m_group_header == NULL)
    {
        m_group_header = new GroupHeader();
        
        m_group_header->dumpCycles = new int[MAX_DUMPS];
        m_group_header->dumpTimes = new double[MAX_DUMPS];
        m_group_header->dumpOffsets = new double[MAX_DUMPS];
        
        if(spyFileHeader->version >= 102)
        {
            m_group_header->dumpDT = new double[MAX_DUMPS];
        }
    }
    
    // Read in data
    spyFileHeader->ndumps = ReadInt();
    ReadInt(m_group_header->dumpCycles, MAX_DUMPS);
    ReadDouble(m_group_header->dumpTimes, MAX_DUMPS);
    
    if(spyFileHeader->version >= 102)
    {
        ReadDouble(m_group_header->dumpDT, MAX_DUMPS);
    }
    
    ReadDouble(m_group_header->dumpOffsets, MAX_DUMPS);
    double tmp;
    ReadDouble(&tmp, 1);
    m_group_header->lastOffset = tmp;
}

// ****************************************************************************
//  Method: SpyFile::ReadMarkerHeader
//
//  Purpose:
//      The marker header contains information specific to the material point
//      method.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadMarkerHeader()
{
    int m_nmat = spyFileHeader->nmat;
    MarkerHeader *m_markerheaders = spyFileHeader->markerHeaders;
    
    if(m_markerheaders == NULL)
    {
        m_markerheaders = new MarkerHeader[m_nmat];
    }
    
    // Read Marker Header
    for(int i=0; i<m_nmat; i++)
    {
        int numMarkers = ReadInt();
        m_markerheaders[i].numMarkers = numMarkers;
        m_markerheaders[i].numRealMarkers = ReadInt();
        
        m_markerheaders[i].junk = new int[3];
        ReadInt(m_markerheaders[i].junk, 3);
        
        if(numMarkers > 0)
        {
            int numVars = ReadInt();
            m_markerheaders[i].numVars = numVars;
            
            m_markerheaders[i].varnames = new char*[numVars];
            m_markerheaders[i].varlabels = new char*[numVars];
            
            for(int j=0; j<numVars; j++)
            {
                // Read in variable name
                m_markerheaders[i].varnames[j] = new char[30];
                m_in->read(m_markerheaders[i].varnames[j], 30);
                debug5 << "Marker Header Var Name: " << m_markerheaders[i].varnames[j] << endl;
                
                // Read in variable label
                m_markerheaders[i].varlabels[j] = new char[256];
                m_in->read(m_markerheaders[i].varlabels[j], 256);
                debug5 << "Marker Header Var Label: " << m_markerheaders[i].varlabels[j] << endl;
            }
        }
    }
}

// ****************************************************************************
//
//  Utility Methods
//
// ****************************************************************************

// ****************************************************************************
//  Method: SpyFile::Float2Double
//
//  Purpose:
//      Converts raw bytes representing a float value to double.
//
//  Arguments:
//      a       the encoded data
//
//  Returns:    double
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

double
SpyFile::Float2Double(unsigned char *a)
{
    unsigned char *ptmp;
    
    float tmp;
    ptmp = reinterpret_cast<unsigned char *>(&tmp);
    
    if(!m_bigendian)
    {
        ptmp[0] = a[3];
        ptmp[1] = a[2];
        ptmp[2] = a[1];
        ptmp[3] = a[0];
    }
    else
    {
        ptmp[0] = a[0];
        ptmp[1] = a[1];
        ptmp[2] = a[2];
        ptmp[3] = a[3];
    }
    
    return static_cast<double>(tmp);
}

// ****************************************************************************
//  Method: SpyFile::Int4ToInt
//
//  Purpose:
//      Converts raw bytes representing a float value to double.
//
//  Arguments:
//      a       the encoded data
//
//  Returns:    double
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::Int4ToInt(unsigned char *a)
{
    unsigned char *ptmp;
    
    int tmp;
    ptmp = reinterpret_cast<unsigned char *>(&tmp);
    
    if(!m_bigendian)
    {
        ptmp[0] = a[3];
        ptmp[1] = a[2];
        ptmp[2] = a[1];
        ptmp[3] = a[0];
    }
    else
    {
        ptmp[0] = a[0];
        ptmp[1] = a[1];
        ptmp[2] = a[2];
        ptmp[3] = a[3];
    }
    
    return static_cast<int>(tmp);
}

// ****************************************************************************
//  Method: SpyFile::FlipInt
//
//  Purpose:
//      Switch the endianess of the incoming integer.
//
//  Arguments:
//      a       the incoming integer
//      nint    the number of integers to change endianess.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::FlipInt(int *a, const int nint)
{
    int tmp;
    unsigned char *pa, *ptmp;
    
    ptmp = reinterpret_cast<unsigned char *>(&tmp);
    
    for(int i=0; i<nint; i++)
    {
        tmp = a[i];
        pa = reinterpret_cast<unsigned char *>(&a[i]);
        pa[0] = ptmp[3];
        pa[1] = ptmp[2];
        pa[2] = ptmp[1];
        pa[3] = ptmp[0];
    }
}

// ****************************************************************************
//  Method: SpyFile::FlipDouble
//
//  Purpose:
//      Switch the endianess of the incoming double.
//
//  Arguments:
//      a       the incoming doubles
//      count   the number of doubles to change endianess.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::FlipDouble(double *a, const int count)
{
    double tmp;
    unsigned char *pa, *ptmp;
    
    ptmp = reinterpret_cast<unsigned char *>(&tmp);
    
    for(int i=0; i<count; i++)
    {
        tmp = a[i];
        pa = reinterpret_cast<unsigned char *>(&a[i]);
        pa[0] = ptmp[7];
        pa[1] = ptmp[6];
        pa[2] = ptmp[5];
        pa[3] = ptmp[4];
        pa[4] = ptmp[3];
        pa[5] = ptmp[2];
        pa[6] = ptmp[1];
        pa[7] = ptmp[0];
    }
}

// ****************************************************************************
//  Method: SpyFile::ReadInt
//
//  Purpose:
//      Read an integer(s) from the Spy file in a machine-independent way.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::ReadInt()
{
    char *buffer = new char[INT_BYTES];
    
    m_in->read(buffer, INT_BYTES);
    int *valPtr = reinterpret_cast<int *>(buffer);
    
    if(!m_bigendian)
    {
        FlipInt(valPtr, 1);
    }
    
    int val = *valPtr;
    delete [] buffer;

    return val;
}

// ****************************************************************************
//  Method: SpyFile::ReadInt
//
//  Purpose:
//      Read a int(s) from the Spy file in a machine-independent way.
//
//  Arguments:
//      out_int the output parameter storing the read-in int value(s)
//      count   the number of ints to read in.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadInt(int *out_int, const int count)
{
    int size = count * INT_BYTES;
    char *buffer = new char[size];
    
    m_in->read(buffer, size);
    int *a = reinterpret_cast<int *>(buffer);
    
    if(!m_bigendian)
    {
        FlipInt(a, count);
    }
    
    for(int i=0; i<count; i++)
    {
        out_int[i] = a[i];
    }
    
    delete [] buffer;
}

// ****************************************************************************
//  Method: SpyFile::ReadDouble
//
//  Purpose:
//      Read a double(s) from the Spy file in a machine-independent way.
//
//  Arguments:
//      out_dbl the output parameter storing the read-in double value(s)
//      count   the number of doubles to read in.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::ReadDouble(double *out_dbl, const int count)
{
    int size = count * DOUBLE_BYTES;
    char *buffer = new char[size];
    
    m_in->read(buffer, size);
    double *a = reinterpret_cast<double *>(buffer);
    
    if(!m_bigendian)
    {
        FlipDouble(a, count);
    }
    
    for(int i=0; i<count; i++)
    {
        out_dbl[i] = a[i];
    }
    
    delete [] buffer;
}

// ****************************************************************************
//  Method: SpyFile::IsBigEndian
//
//  Purpose:
//      Determines if the current machine is Big or Little Endian.
//
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

bool
SpyFile::IsBigEndian()
{
    float a;
    unsigned char *pa;
    
    pa = (unsigned char *) &a;
    a = 1.23456e8;
    
    if(pa[0] == 0x4c)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ****************************************************************************
//  Method: SpyFile::RunLengthDecodeTrend
//
//  Purpose:
//
//  Arguments:
//      data    Allocated output space for the decoded data
//      n       The expected number of doubles (sizeof(data))
//      in      Encoded data
//      n_in    The number of bytes to decode from *in.
//
//  Returns:    a collection of doubles
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::RunLengthDecodeTrend(double *data, const int n, void *in, const int n_in)
{
    int i,j,k;
    unsigned char *ptmp, code;
    double x, dx;
    
    ptmp = static_cast<unsigned char *>(in);
    
    // Run-length decode
    i = 0;
    j = 0;
    x = Float2Double(ptmp);
    ptmp += 4;
    dx = Float2Double(ptmp);
    ptmp += 4;
    j += 8;
    
    while((i<n) && (j<n_in))
    {
        code = *ptmp;
        ptmp++;
        
        if(code < 128)
        {
            ptmp += 4;
            for(k=0; k<code; k++)
            {
                data[i] = x + i * dx;
                i++;
            }
            
            j += 5;
        }
        else
        {
            for(k=0; k<code-128; k++)
            {
                data[i] = Float2Double(ptmp) + i * dx;
                i++;
                ptmp += 4;
            }
            
            j += 4 * (code - 128) + 1;
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::RunLengthDecode
//
//  Purpose:
//      Run-length-decodes the data pointed to by *in and returns a collection
//      of doubles in *data.
//
//  Arguments:
//      data    Allocated output space for the decoded data
//      n       The expected number of doubles (sizeof(data))
//      in      Encoded data
//      n_in    The number of bytes to decode from *in.
//
//  Returns:    a collection of doubles
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::RunLengthDecode(double *data, const int n, void *in, const int n_in)
{
    int i,j,k;
    unsigned char *ptmp, code;
    double x;

    ptmp = static_cast<unsigned char *>(in);
    
    // Run-length decode
    i = 0;
    j = 0;
    
    while((i<n) && (j<n_in))
    {
        code = *ptmp;
        ptmp++;
        
        if(code < 128)
        {
            x = Float2Double(ptmp);
            ptmp += 4;
            for(k=0; k<code; k++)
            {
                data[i++] = x;
            }
            
            j += 5;
        }
        else
        {
            for(k=0; k<code-128; k++)
            {
                data[i++] = Float2Double(ptmp);
                ptmp += 4;
            }
            
            j += 4 * (code - 128) + 1;
        }
    }
}

// ****************************************************************************
//  Method: SpyFile::RunLengthDecodeInt
//
//  Purpose:
//      Run-length-decodes the data pointed to by *in and returns a collection
//      of ints in *data.
//
//  Arguments:
//      data    Allocated output space for the decoded data
//      n       The expected number of doubles (sizeof(data))
//      in      Encoded data
//      n_in    The number of bytes to decode from *in.
//
//  Returns:    a collection of doubles
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

void
SpyFile::RunLengthDecodeInt(int *data, const int n, void *in, const int n_in)
{
    int i,j,k;
    unsigned char *ptmp, code;
    int x;
    ptmp = static_cast<unsigned char *>(in);
    
    // Run-length decode
    i = 0;
    j = 0;
    
    while((i<n) && (j<n_in))
    {
        code = *ptmp;
        ptmp++;
        
        if(code < 128)
        {
            x = Int4ToInt(ptmp);
            ptmp += 4;
            
            for(k=0; k<code; k++)
            {
                data[i++] = x;
            }
            
            j += 5;
        }
        else
        {
            for(k=0; k<code-128; k++)
            {
                data[i++] = Int4ToInt(ptmp);
                ptmp += 4;
            }
            
            j += 4 * (code - 128) + 1;
        }
    }
}

// ****************************************************************************
//
//  Setters/Getters
//
// ****************************************************************************


// ****************************************************************************
//  Method: SpyFile::GetGmax
//
//  Purpose:
//      Return Gmax (Gmin and Gmax are the ‘bounds’ of the data).
//
//  Returns:    Gmax or NULL if the header has not been read in.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

const double *
SpyFile::GetGmax()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->gMax;
    }
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetGmin
//
//  Purpose:
//      Return Gmin (Gmin and Gmax are the ‘bounds’ of the data).
//
//  Returns:    Gmin
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

const double *
SpyFile::GetGmin()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->gMin;
    }
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberOfAMRBlocks
//
//  Purpose:
//      Get the number AMR blocks
//
//  Returns:    number of AMR blocks
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfAMRBlocks()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->blocks;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberOfCells
//
//  Purpose:
//      Get the number of cells
//
//  Returns:    number of cells
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
int
SpyFile::GetNumberOfCellFields()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->nCellFields;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberofMats
//
//  Purpose:
//      Get the number of materials
//
//  Returns:    number of materials
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfMatFields()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->nMatFields;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberofMats
//
//  Purpose:
//      Get the number of materials
//
//  Returns:    number of materials.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfMats()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->nmat;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetMaxNumMaterials
//
//  Purpose:
//      Get the maximum number of materials (98 for current CTH)
//
//  Returns:    max number of materials or -1 if the header has not been read.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetMaxNumMaterials()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->maxMat;
    }
    return -1;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberOfDims
//
//  Purpose:
//      Get the number of dimensions
//
//  Returns:    number of dimensions
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfDims()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->ndim;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetIGM
//
//  Purpose:
//      IGM – Specifies problem dimensions (10=1DR, 11=1DC, 12=1DS, 20=2DR, 21=2DC, 30=3DR).
//
//  Returns:    IGM
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetIGM()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->igm;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberOfProcs
//
//  Purpose:
//      Get the number of processors from the SPCTH file header.
//
//  Returns:    the number of processors
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfProcs()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->nprocs;
    }
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetProcessorId
//
//  Purpose:
//      Get the processor id from the SPCTH file header.
//
//  Returns:    the processor id.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetProcessorId()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->processorId;
    }
    
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetCompression
//
//  Purpose:
//      Get the compression flag from the SPCTH file header.
//
//  Returns:    the Spy compression flag or -1 if the header has not been
//              read.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetCompression()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->compression;
    }
    
    return -1;
}

// ****************************************************************************
//  Method: SpyFile::GetFileVersion
//
//  Purpose:
//      Get the file version from the SPCTH file header.
//
//  Returns:    the file version or -1 if the header has not been read.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetFileVersion()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->version;
    }
    
    return -1;
}

// ****************************************************************************
//  Method: SpyFile::GetFilePtrSize
//
//  Purpose:
//      Get the size of the file pointer.
//
//  Returns:    the file pointer size.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetFilePtrSize()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->fileptrSize;
    }
    
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetFileTitle
//
//  Purpose:
//      Get the file title from the SPCTH file header.
//
//  Returns:    the file title or NULL if the SPCTH file header has not been
//              read.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

const char *
SpyFile::GetFileTitle()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->title;
    }
    
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetMagicString
//
//  Purpose:
//      First 8 bytes is magic string “spydata” with \0 as string terminator at
//      byte 8.
//
//  Returns:    The magic string or NULL if the SPCTH file header has not been
//              read.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

char *
SpyFile::GetMagicString()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->magicStr;
    }
    
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::IsMarkOn
//
//  Purpose:
//      Flag for MPM Particles in problems (SpyVersion >= 105).
//
//  Returns:    true if Mark flag is on, otherwise false
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

bool
SpyFile::IsMarkOn()
{
    if(spyFileHeader != NULL)
    {
        if(spyFileHeader->markIsOn > 0)
        {
            return true;
        }
    }
    
    return false;
}

// ****************************************************************************
//  Method: SpyFile::GetDumps
//
//  Purpose:
//      Get the dumps
//
//  Returns:    list of Dumps
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

const std::vector<Dump *> * const
SpyFile::GetDumps()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->dumps;
    }
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetNumberOfDumps
//
//  Purpose:
//      Get the number of dumps
//
//  Returns:    number of dumps
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

int
SpyFile::GetNumberOfDumps()
{
    if(spyFileHeader != NULL)
    {
        if(spyFileHeader->dumps != NULL)
        {
            return spyFileHeader->dumps->size();
        }
    }
    
    return 0;
}

// ****************************************************************************
//  Method: SpyFile::GetDumpHeader
//
//  Purpose:
//      Get the Dump Header
//
//  Returns:    Dump Header or NULL if dump header doesn't exist for timestep.
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************

const DumpHeader * const
SpyFile::GetDumpHeader(const int ts)
{
    if(spyFileHeader != NULL)
    {
        if(ts < spyFileHeader->ndumps && spyFileHeader->dumps->at(ts)->loaded)
        {
             return tsData->dumpHeader;
        }
    }
    
    return NULL;
}

// ****************************************************************************
//  Method: SpyFile::GetMat
//
//  Purpose:
//      Get the material for the field represented by fieldId.
//
//  Arguments:
//      fieldId the field ID
//
//  Returns:    Returns -2 if cell field, -1 if mat field with no mat defined,
//              else returns 0..nmat-1
//
//  Programmer: Kevin Griffin
//  Creation:   Wed Jun 14 10:05:56 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
// Returns -2 if cell field, -1 if mat field with no mat defined, else returns 0..nmat-1
int
SpyFile::GetMat(const int fieldId)
{
    int tmp, tmp1;
    
    // Material Field
    if(fieldId >= STM_VOLM)
    {
        tmp = fieldId / STM_VOLM;
        tmp1 = fieldId - tmp * STM_VOLM - 1;
        return tmp1;
    }
    else if ((fieldId <= STM_DENSM_1) && (fieldId >= STM_DENSM))
    {
        return fieldId - (STM_DENSM) - 1;
    }
    else if ((fieldId<=STM_TKM_1) && (fieldId>=STM_TKM))
    {
        return fieldId-(STM_TKM)-1;
    } else {    // Cell Field
        return -2;
    }
}

const Field *
SpyFile::GetCellFields()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->cellFields;
    }
    return NULL;
}

const Field *
SpyFile::GetMatsFields()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->matFields;
    }
    return NULL;
}

int
SpyFile::GetMaxAMRLevels()
{
    if(spyFileHeader != NULL)
    {
        return spyFileHeader->maxLevel;
    }
    return -1;
}

DataBlock *
SpyFile::GetDataBlock(const int ts)
{
    if(spyFileHeader != NULL && spyFileHeader->dumps->at(ts)->loaded)
    {
        return tsData->dataBlock;
    }
    return NULL;
}

void
SpyFile::GetBlockCellSize(const int bid, double *dx)
{
    DataBlock *dataBlock = tsData->dataBlock;
    
    int dims[3] = {1, 1, 1};
    dims[0] = dataBlock->nx[bid];
    dims[1] = dataBlock->ny[bid];
    
    bool is3D = spyFileHeader->ndim == 3;
    if(is3D)
    {
        dims[2] = dataBlock->nz[bid];
    }
    
    double bounds[6] = {DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX};
    
    for(int i=0; i<dims[0]; i++)
    {
        bounds[0] = std::min(bounds[0], dataBlock->x[bid][i]);
        bounds[1] = std::max(bounds[1], dataBlock->x[bid][i]);
    }
    
    for(int i=0; i<dims[1]; i++)
    {
        bounds[2] = std::min(bounds[2], dataBlock->y[bid][i]);
        bounds[3] = std::max(bounds[3], dataBlock->y[bid][i]);
    }
    
    if(is3D)
    {
        for(int i=0; i<dims[2]; i++)
        {
            bounds[4] = std::min(bounds[4], dataBlock->z[bid][i]);
            bounds[5] = std::max(bounds[5], dataBlock->z[bid][i]);
        }
    }
    else
    {
        bounds[4] = 0;
        bounds[5] = 0;
    }
    
    dx[0] = (bounds[1] - bounds[0]) / dims[0];
    dx[1] = (bounds[3] - bounds[2]) / dims[1];
    dx[2] = (bounds[5] - bounds[4]) / dims[2];
}
