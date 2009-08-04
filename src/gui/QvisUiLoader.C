/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QvisColorButton.h>
#include <QvisColorGridWidget.h>
#include <QvisColorTableButton.h>
#include <QvisElementButton.h>
#include <QvisDialogLineEdit.h>
#include <QvisLineStyleWidget.h>
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
    else if(className == "QvisLineStyleWidget")
        w = new QvisLineStyleWidget(0, parent);
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
