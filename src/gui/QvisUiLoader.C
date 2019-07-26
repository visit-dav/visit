// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisColorButton.h>
#include <QvisColorGridWidget.h>
#include <QvisColorTableButton.h>
#include <QvisElementButton.h>
#include <QvisDialogLineEdit.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <QvisPeriodicTableWidget.h>
#include <QvisPointControl.h>
#include <QvisScreenPositionEdit.h>
#include <QvisSpectrumBar.h>
#include <QvisVariableButton.h>
#include <QvisUiLoader.h>

// ****************************************************************************
// Method: QvisUiLoader::QvisUiLoader
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 14:35:43 PST 2006
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 10:47:24 PDT 2008
//   Changed the base class.
//
// ****************************************************************************

QvisUiLoader::QvisUiLoader() : QUiLoader()
{
}

// ****************************************************************************
// Method: QvisUiLoader::~QvisUiLoader
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 14:35:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisUiLoader::~QvisUiLoader()
{
}

// ****************************************************************************
// Method: QvisUiLoader::createWidget
//
// Purpose: 
//   Instantiates a class.
//
// Arguments:
//   className : The name of the class to instantiate.
//   parent    : The parent for the new widget.
//   name      : The name of the widget (not used anymore).
//
// Returns:    A pointer to the new widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 14:36:08 PST 2006
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 10:43:32 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QWidget *
QvisUiLoader::createWidget(const QString &className, QWidget *parent,
    const QString &name)
{
    QWidget *w = 0;

    if(className == "QvisColorButton")
        w = new QvisColorButton(parent);
    else if(className == "QvisColorGridWidget")
        w = new QvisColorGridWidget(parent);
    else if(className == "QvisColorTableButton")
        w = new QvisColorTableButton(parent);
    else if(className == "QvisElementButton")
        w = new QvisElementButton(parent);
    else if(className == "QvisDialogLineEdit")
        w = new QvisDialogLineEdit(parent);
    else if(className == "QvisLineWidthWidget")
        w = new QvisLineWidthWidget(0, parent);
    else if(className == "QvisOpacitySlider")
        w = new QvisOpacitySlider(parent);
    else if(className == "QvisPeriodicTableWidget")
        w = new QvisPeriodicTableWidget(parent);
    else if(className == "QvisPointControl")
        w = new QvisPointControl(parent);
    else if(className == "QvisScreenPositionEdit")
        w = new QvisScreenPositionEdit(parent);
    else if(className == "QvisSpectrumBar")
        w = new QvisSpectrumBar(parent);
    else if(className == "QvisVariableButton")
        w = new QvisVariableButton(parent);
    else
        w = QUiLoader::createWidget(className, parent, name);

    return w;
}
