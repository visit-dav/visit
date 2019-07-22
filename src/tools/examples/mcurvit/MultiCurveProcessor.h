// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MULTI_CURVE_PROCESSOR_H
#define MULTI_CURVE_PROCESSOR_H

#include <QObject>

class Synchronizer;
class VisItViewer;

#include <string>
#include <vector>

#include <stdio.h>

// ****************************************************************************
// Class: MultiCurveProcessor
//
// Purpose:
//   This application creates a multi curve command processor.  It reads a
//   command file, processes the commands, and exits.  It only handles files
//   with 2d rectilinear node centered variables.  It applies an index select
//   operator to the data to allow the user to subselect portions of the
//   data.  It also allows the user to control many of the viewing attributes
//   of the plot.
//
// Notes:
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:41:27 PST 2009
//
// Modifications:
//
// ****************************************************************************

class MultiCurveProcessor : public QObject
{
    Q_OBJECT
public:
    MultiCurveProcessor(VisItViewer *);
    virtual ~MultiCurveProcessor();

    void                   ProcessBatchFile(const char *, const char *);

    void                   OpenDatabase(const std::string &);
    void                   CreatePlot(const std::string &);
    void                   FinishPlot();
    void                   SaveWindow(const int, const std::string &);

private slots:
    void execCommands();
    void execCommand();
private:
    void                   ReadInt(FILE *, int &, int &);
    void                   ReadDouble(FILE *, int &, double &);
    void                   ReadString(FILE *, int &, std::string &);
    void                   ReadQuotedString(FILE *, int &, std::string &);
    void                   ReadFile(const char *);

    VisItViewer           *viewer;

    Synchronizer          *sync;

    bool                   save;
    int                    saveFormat;
    std::string            openFile;
    std::string            saveFile;
    std::vector<std::string> plotVars;

    std::vector<int>       commandList;
    int                    iCommand;
    int                    iVar;
};

#endif
