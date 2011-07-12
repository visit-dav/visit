/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:        DataManagerAPI.h                                            //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _DATA_MANAGER_API_H_
#define _DATA_MANAGER_API_H_

#include <string>
#include <vector>

#include "StarObject.h"

/**
 *      The DataManagerAPI is the interface for all of the
 *      multiresolution data objects, including the
 *      readers @see MultiresMultifile and @see 
 *      MultiresMetadata, and any future metadata objects 
 *      that need to be used with the Visit plugin.
 *
 *      @author Andrew Foulks
 **/

class DataManagerAPI : public StarObject
{
public:  // 'structors
            DataManagerAPI() {};
    virtual ~DataManagerAPI() {};

public:  // api
    virtual const char*     filename            () const=0;
    virtual float           fileVersion         () const=0;
    virtual int             numFiles            () const=0;
    virtual int             numResolutions      () const=0;
    virtual std::string     gridFilename        () const=0;
    virtual std::vector<int> timeStepList        () const=0;

    // data rank: scalars, vectors, or tensors
    virtual int             numVariables        () const=0;
    virtual std::string     variableNameAt      (int index) const=0;
    virtual int             indexOfVariableName (const std::string& name) const=0;
    virtual bool            isScalar            (const std::string& name) const=0;
    virtual char            isVectorComponent   (const std::string& name) const=0;
    virtual bool            isVector            (const std::string& name) const=0;
    virtual bool            isTensor            (const std::string& name) const=0;
    virtual int             numVectorExpressions() const=0;
    virtual std::string     vectorExpressionAt  (int index) const=0;

    // data size and structure
    virtual int             width               (int resolution) const=0;
    virtual int             height              (int resolution) const=0;
    virtual int             depth               (int resolution) const=0;
    virtual int             numXchunks          (int resolution) const=0;
    virtual int             numYchunks          (int resolution) const=0;
    virtual int             numZchunks          (int resolution) const=0;
    virtual bool            hasMinVal           () const=0;
    virtual bool            hasMaxVal           () const=0;
    virtual float           minVal              () const=0;
    virtual float           maxVal              () const=0;
    virtual const float*    rawData             (const std::string& varName,
                                                 int resolution,
                                                 int fileindex,
                                                 int chunkIndex)=0;

    // convenience functions to get the chunksizes
    virtual int             chunkwidth          (int resolution) const
        { return width(resolution)/numXchunks(resolution); }
    virtual int             chunkheight         (int resolution) const
        { return height(resolution)/numYchunks(resolution); }
    virtual int             chunkdepth          (int resolution) const
        { return depth(resolution)/numZchunks(resolution); }

    // better descriptions of the attributes (for the ones we know about)
    virtual std::string     longScalarNameFor   (const std::string& scalar) const {
        if(scalar=="rr")  return "Plasma number density [cm**-3]";
        if(scalar=="pp")  return "Plasma pressure [pPa]";
        if(scalar=="vx")  return "Plasma velocity, x-component [km/s]";
        if(scalar=="vy")  return "Plasma velocity, y-component [km/s]";
        if(scalar=="vz")  return "Plasma velocity, z-component [km/s]";
        if(scalar=="bx")  return "Magnetic field, x-component [nT]";
        if(scalar=="by")  return "Magnetic field, y-component [nT]";
        if(scalar=="bz")  return "Magnetic field, x-component [nT]";
        if(scalar=="xjx") return "Current density, x-component [micro-A/m**2]";
        if(scalar=="xjy") return "Current density, y-component [micro-A/m**2]";
        if(scalar=="xjy") return "Current density, z-component [micro-A/m**2]";
        return "unknown scalar";
    }
    virtual std::string     longVectorNameFor(const std::string& vector) const {
        if(vector == "v")  return "Plasma velocity [km/s]";
        if(vector == "b")  return "Magnetic field [nT]";
        if(vector == "xj") return "Current density [micro-A/m**2]";
        return "unknown vector";
    }
    virtual std::string     longTensorNameFor   (const std::string& tensor) const {
        return "error: tensors not supported yet";
    }

private: // disable copy constructor and operator=
    DataManagerAPI(DataManagerAPI&);
    DataManagerAPI& operator=(DataManagerAPI&);
};

#endif // _DATA_MANAGER_API_H_

