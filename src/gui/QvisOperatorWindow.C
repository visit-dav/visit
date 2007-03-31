#include <QvisOperatorWindow.h>
#include <QvisNotepadArea.h>
#include <qmessagebox.h>

#include <GlobalAttributes.h>
#include <OperatorPluginManager.h>
#include <Plot.h>
#include <PlotList.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisOperatorWindow::QvisOperatorWindow
//
// Purpose: 
//   This is the constructor for the QvisOperatorWindow class.
//
// Arguments:
//   type      : The integer id that is associated with the operator plugin.
//   subj      : The AsliceAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:34:59 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Mar 6 11:29:17 PDT 2003
//   I added stretch.
//
// ****************************************************************************

QvisOperatorWindow::QvisOperatorWindow(const int type, Subject *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad,
    bool stretch) : QvisPostableWindowObserver(subj, caption, shortName,
    notepad, QvisPostableWindowObserver::AllExtraButtons, stretch)
{
    operatorType = type;
}

// ****************************************************************************
// Method: QvisOperatorWindow::~QvisOperatorWindow
//
// Purpose: 
//   This is the destructor for the QvisOperatorWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:35:55 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisOperatorWindow::~QvisOperatorWindow()
{
}

// ****************************************************************************
// Method: QvisOperatorWindow::Apply
//
// Purpose: 
//   This method applies the operator attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            aslice attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:38:36 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        subject->Notify();

        // Tell the viewer to set the operator attributes.
        SetOperatorOptions();
    }
    else
        subject->Notify();
}

// ****************************************************************************
// Method: QvisOperatorWindow::GetCurrentValues
//
// Purpose: 
//   This method gets the current values for widgets in the window. This is
//   a default implementation that does nothing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:41:57 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::GetCurrentValues(int)
{
    // override in derived classes.
}

// ****************************************************************************
// Method: QvisOperatorWindow::SetOperatorOptions
//
// Purpose: 
//   This looks to see if the operator is applied to any plot in the plot list
//   and ask the user if the operator should be added if it is not found.
//
// Arguments:
//   operatorType : The operator type.
//
// Programmer: Brad Whitlock
// Creation:   
//
// Modifications:
//   Brad Whitlock, Wed Jun 26 15:28:08 PST 2002
//   Made the operator prompt window only pop up if there are plots.
//
//   Jeremy Meredith, Mon Jun 23 16:18:29 PDT 2003
//   Changed GetAllID to GetEnabledID.
//
// ****************************************************************************

void
QvisOperatorWindow::SetOperatorOptions()
{
    PlotList *plots = viewer->GetPlotList();

    // If there are plots, see if the operator should be applied.
    if(plots->GetNumPlots() > 0)
    {
        bool applyToAll = viewer->GetGlobalAttributes()->GetApplyOperator();
        int i, j;

        // Look to see if the operator exists in the plots
        bool found = false;
        for(i = 0; i < plots->GetNumPlots() && !found; ++i)
        {
            const Plot &plot = plots->operator[](i);
            if(plot.GetActiveFlag() || applyToAll)
            {
                for(j = 0; j < plot.GetNumOperators(); ++j)
                {
                     if(operatorType == plot.GetOperator(j))
                     {
                         found = true;
                         break;
                     }
                }
            }
        }

        // If the operator was not found in the plot list, ask the user whether
        // the operator should be added to the plots.
        if(!found)
        {
            OperatorPluginManager *opMgr = OperatorPluginManager::Instance();

            // Create a prompt for the user.
            std::string opName(opMgr->GetPluginName(
                               opMgr->GetEnabledID(operatorType)));
            QString msg;
            msg.sprintf("No %s operator was found for the selected plots.\n"
                        "Do you want to apply the %s operator?\n\n",
                        opName.c_str(), opName.c_str());

            // Ask the user if he really wants to close the engine.
            if(QMessageBox::warning(this, "VisIt",
                                    msg.latin1(),
                                    "Yes", "No", 0,
                                    0, 1 ) == 0)
            {
                // Set the default attributes so when we apply the operator,
                // it only causes the plot to be modified one time. Otherwise
                // we have to add the operator and then set its options 
                // which would cause the plot to be generated twice.
                viewer->SetDefaultOperatorOptions(operatorType);
                viewer->AddOperator(operatorType);
            }
        }
        else
            viewer->SetOperatorOptions(operatorType);
    }
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisOperatorWindow::apply
//
// Purpose: 
//   This is a Qt slot function to apply the operator attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:40:04 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisOperatorWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function to make the current operator attributes
//   the defaults.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:38:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::makeDefault()
{
    // Tell the viewer to set the default pc attributes.
    GetCurrentValues(-1);
    subject->Notify();
    viewer->SetDefaultOperatorOptions(operatorType);
}

// ****************************************************************************
// Method: QvisOperatorWindow::reset
//
// Purpose: 
//   This is a Qt slot function to reset the AsliceAttributes to the
//   last applied values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:39:03 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    viewer->ResetOperatorOptions(operatorType);
}





