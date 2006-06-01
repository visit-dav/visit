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

#include <QvisKerbelPlotWindowWithButtons.h>
#include <KerbelAttributes.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <DataNode.h>
#include <stdio.h>

QvisKerbelPlotWindowWithButtons::QvisKerbelPlotWindowWithButtons(
    const int type,
    KerbelAttributes *subj,
    const char *caption,
    const char *shortName,
    QvisNotepadArea *notepad)
        : QvisKerbelPlotWindow(type, subj, caption, shortName, notepad)
{
    atts = subj;
}

void
QvisKerbelPlotWindowWithButtons::CreateWindowContents()
{
    QvisKerbelPlotWindow::CreateWindowContents();

    QGridLayout *buttonLayout = new QGridLayout(topLayout, 6, 2, 10,
                                                "buttonLayout");

    QPushButton *load = new QPushButton("Load", central, "load");
    buttonLayout->addWidget(load, 0, 0);
    connect(load, SIGNAL(clicked()), this, SLOT(LoadSlot()));
    QPushButton *save = new QPushButton("Save", central, "save");
    buttonLayout->addWidget(save, 0, 1);
    connect(save, SIGNAL(clicked()), this, SLOT(SaveSlot()));
}

void
QvisKerbelPlotWindowWithButtons::LoadSlot()
{
    // Get the current values from the window
    GetCurrentValues(-1);

    // Try to open up the input file.
    const char *name = atts->GetFilename().c_str();
    fp = fopen(name, "r");
    if (fp == NULL)
    {
        Error(QString().sprintf("KerbelPlot: Unable to read from filename \"%s\".\n",name));
        return;
    }

    // Read the XML tag and ignore it.
    FinishTag();

    // Create a root node
    DataNode node("KerbelPlot");
    ReadObject(&node);
    fclose(fp);
    fp = NULL;

    // Set the attributes from this node.
    atts->SetFromNode(&node);

    // Update the window with the new values.
    UpdateWindow(true);

    // Tell the user that we were successful.
    Message(QString().sprintf("KerbelPlot: Successfully loaded data from \"%s\".  Press Apply to see the effect of the new data on the plot.\n",name));
}

void
QvisKerbelPlotWindowWithButtons::SaveSlot()
{
    // Get the current values from the window
    GetCurrentValues(-1);

    // Try to open up an output file.
    const char *name = atts->GetFilename().c_str();
    fp = fopen(name, "w");
    if (fp == NULL)
    {
        Error(QString().sprintf("KerbelPlot: Unable to save to filename \"%s\".\n",name));
        return;
    }

    // Create a root node, put the KerbelPlot attributes in it, then save
    // it out.
    DataNode node("KerbelPlot");
    atts->CreateNode(&node);

    fprintf(fp, "<?xml version\"1.0\"?>\n");
    WriteObject(&node);
    fclose(fp);
    fp = NULL;

    // Tell the user that we were successful.
    Message(QString().sprintf("KerbelPlot: Successfully saved data to \"%s\".\n",name));
}
