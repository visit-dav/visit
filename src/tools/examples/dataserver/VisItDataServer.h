// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DATA_SERVER_H
#define VISIT_DATA_SERVER_H
#include <AttributeSubject.h>
#include <avtDatabaseMetaData.h>

#include <string>

class VisItDataServerPrivate;
class vtkDataSet;

// ****************************************************************************
// Class: VisItDataServer
//
// Purpose:
//   Provide a simple interface that lets you use the compute engine to return
//   VTK datasets.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 16:58:59 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class VisItDataServer
{
public:
    VisItDataServer();
    ~VisItDataServer();

    void SetPluginDir(const std::string &pluginDir);
    void AddArgument(const std::string &arg);
    void DebugEngine();
    void Open(int argc, char *argv[]);
    void Close();

    std::string                ExpandPath(const std::string &path);
    const avtDatabaseMetaData *GetMetaData(const std::string &filename);

    void OpenDatabase(const std::string &filename, int timeState);
    void ReadData(const std::string &var);

    void DefineScalarExpression(const std::string &name, const std::string &def);
    void DefineVectorExpression(const std::string &name, const std::string &def);

    AttributeSubject *CreateOperatorAttributes(const std::string id);
    void              AddOperator(const std::string &id);
    void              AddOperator(const std::string &id, const AttributeSubject *atts);

    AttributeSubject *CreatePlotAttributes(const std::string id);
    void              AddPlot(const std::string &id);
    void              AddPlot(const std::string &id, const AttributeSubject *atts);

    vtkDataSet      **Execute(int &n);
private:
    VisItDataServerPrivate *d;
};

#endif
