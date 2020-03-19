// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Wed Sep 11 09:33:55 PDT 2013
//    Remove 'STATE_API'.  Added stream versions of Read/WriteConfigFile.
//
// ****************************************************************************

class VLIFileManager : public ConfigManager
{
public:
                      VLIFileManager();
    virtual          ~VLIFileManager();

    virtual bool      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
    virtual bool      WriteConfigFile(std::ostream &out);
    virtual DataNode *ReadConfigFile(std::istream &in);

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
   
class VLIAttribute
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

class VLIDataset
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
