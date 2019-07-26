// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CONNECTION_PRINTER_UI_H
#define VIEWER_CONNECTION_PRINTER_UI_H
#include <ViewerConnectionPrinter.h>
#include <QSocketNotifier>

// ****************************************************************************
// Class: ViewerConnectionPrinterImplementation
//
// Purpose:
//   Subclass of QSocketNotifier that we use for printing VCL console output.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:49:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerConnectionPrinterImplementation : public QSocketNotifier
{
    Q_OBJECT
public:
    ViewerConnectionPrinterImplementation(Connection *);
    virtual ~ViewerConnectionPrinterImplementation();
private slots:
    void HandleRead(int);
private:
    Connection *conn;
};

// ****************************************************************************
// Class: ViewerConnectionPrinterUI
//
// Purpose:
//   The visible part of the UI-based connection printer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:49:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerConnectionPrinterUI : public ViewerConnectionPrinter
{
public:
    ViewerConnectionPrinterUI();
    virtual ~ViewerConnectionPrinterUI();
    virtual void SetConnection(Connection *);
private:
    ViewerConnectionPrinterImplementation *impl;
};

#endif
