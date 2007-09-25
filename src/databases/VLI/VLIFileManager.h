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

#ifndef VLI_FILE_MANAGER_H
#define VLI_FILE_MANAGER_H
#include <ConfigManager.h>
#include <string>

class DataNode;
class VLIAttribute;
class VLIDataset;

// ****************************************************************************
// Class: VLIFileManager
//
// Purpose:
//   Reads and writes ColorControlPointLists to/from files.
//
// Notes:      
//
// Programmer: Markus Glatter <glatter@cs.utk.edu>
// Creation:   Fri Aug 10 11:48:00 EDT 2007 
//
// Modifications:
//
// ****************************************************************************

class STATE_API VLIFileManager : public ConfigManager
{
public:
                      VLIFileManager();
    virtual          ~VLIFileManager();

    virtual bool      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
    VLIDataset       *getDataset();
    void              setDataset(VLIDataset *);
    std::string       getName();
    void              setName(std::string);
    const int*        getAxis();
    void              setAxis(const int*);
    int               getNoDataServers();
    void              setNoDataServers(int n);
    std::string       getSyscall();
    void              setSyscall(std::string);
    std::string       getServer();
    void              setServer(std::string);
    std::string       getDatafile();
    void              setDatafile(std::string);

protected:
    DataNode         *Import(DataNode *node);
    DataNode         *Export();

private:
    VLIDataset       *dataset;
    int               axis[4];
    std::string       name;
    std::string       syscall;
    std::string       server;
    std::string       datafile;
    int               nservers;
};
   
class STATE_API VLIAttribute
{
public:
                        VLIAttribute();
                        VLIAttribute(std::string name, double min, double max, bool isScaled);
                        VLIAttribute(DataNode *node);
    virtual            ~VLIAttribute();
    
    virtual std::string GetName();
    virtual double      GetMin();
    virtual double      GetMax();
    virtual bool        GetIsScaled();
    virtual DataNode   *Export();
    virtual bool        Import(DataNode *node);
    virtual void        Set(std::string name, double min, double max, bool isScaled = true);

private:
    std::string         name;
    double              min;
    double              max;
    bool                isScaled;
};

class STATE_API VLIDataset
{
public:
                          VLIDataset();
                          VLIDataset(const int nattr, const std::string format, const long nitems);
                          VLIDataset(DataNode *node);
    virtual              ~VLIDataset();
    
    virtual std::string   getFormat();
    virtual long          getNItems();
    virtual int           getNAttributes();
    virtual VLIAttribute *getAttribute(const int i);
    virtual DataNode     *Export(bool withAttributes = true);
    virtual bool          Import(DataNode *node);
    virtual void          Set(const int nattr, const std::string format, const long nitems);
    
private:
    std::string           format;
    long                  nitems;
    int                   nattr;
    VLIAttribute         *attributes;
};


#endif
