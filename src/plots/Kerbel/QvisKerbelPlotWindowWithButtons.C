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
