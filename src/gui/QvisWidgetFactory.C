/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
#include <QvisColorButton.h>
#include <QvisColorGridWidget.h>
#include <QvisColorTableButton.h>
#include <QvisElementButton.h>
#include <QvisFileLineEdit.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisOpacitySlider.h>
#include <QvisPeriodicTableWidget.h>
#include <QvisPointControl.h>
#include <QvisScreenPositionEdit.h>
#include <QvisSpectrumBar.h>
#include <QvisVariableButton.h>
#include <QvisWidgetFactory.h>

// ****************************************************************************
// Method: QvisWidgetFactory::QvisWidgetFactory
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 14:35:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisWidgetFactory::QvisWidgetFactory() : QWidgetFactory()
{
}

// ****************************************************************************
// Method: QvisWidgetFactory::~QvisWidgetFactory
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

QvisWidgetFactory::~QvisWidgetFactory()
{
}

// ****************************************************************************
// Method: QvisWidgetFactory::createWidget
//
// Purpose: 
//   Instantiates a class.
//
// Arguments:
//   className : The name of the class to instantiate.
//   parent    : The parent for the new widget.
//   name      : The name for the new widget.
//
// Returns:    A pointer to the new widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 14:36:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisWidgetFactory::createWidget(const QString &className, QWidget *parent,
    const char *name) const
{
    QWidget *w = 0;

    if(className == "QvisColorButton")
        w = new QvisColorButton(parent, name);
    else if(className == "QvisColorGridWidget")
        w = new QvisColorGridWidget(parent, name);
    else if(className == "QvisColorTableButton")
        w = new QvisColorTableButton(parent, name);
    else if(className == "QvisElementButton")
        w = new QvisElementButton(parent, name);
    else if(className == "QvisFileLineEdit")
        w = new QvisFileLineEdit(parent, name);
    else if(className == "QvisLineStyleWidget")
        w = new QvisLineStyleWidget(0, parent, name);
    else if(className == "QvisLineWidthWidget")
        w = new QvisLineWidthWidget(0, parent, name);
    else if(className == "QvisOpacitySlider")
        w = new QvisOpacitySlider(parent, name);
    else if(className == "QvisPeriodicTableWidget")
        w = new QvisPeriodicTableWidget(parent, name);
    else if(className == "QvisPointControl")
        w = new QvisPointControl(parent, name);
    else if(className == "QvisScreenPositionEdit")
        w = new QvisScreenPositionEdit(parent, name);
    else if(className == "QvisSpectrumBar")
        w = new QvisSpectrumBar(parent, name);
    else if(className == "QvisVariableButton")
        w = new QvisVariableButton(parent, name);

    return w;
}
