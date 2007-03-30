#include <QvisCheckListManagerWidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgrid.h>
#include <qlayout.h>
#include <qscrollview.h>

static const int MAX_CHECKBOXES = 150;

// ****************************************************************************
// Method: QvisCheckListManagerWidget::QvisCheckListManagerWidget
//
// Purpose: 
//   This is the constructor for the QvisCheckListManagerWidget class.
//
// Arguments:
//   parent : A pointer to the widget's parent widget.
//   name   : The name of this widget instance.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:13:12 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCheckListManagerWidget::QvisCheckListManagerWidget(QWidget *parent,
    const char *name) : QWidget(parent, name), checkBoxes()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // How many checkboxes are visible
    nCheckBoxes = 0;

    // Create the scrollview.
    scrollView = new QScrollView(this, "scrollView");
    scrollView->setVScrollBarMode(QScrollView::Auto);
    scrollView->setHScrollBarMode(QScrollView::Auto);
    scrollView->viewport()->setBackgroundColor(colorGroup().background());
    topLayout->addWidget(scrollView);

    // Create the QGrid widget that will manage the layout of the buttons.
    grid = new QGrid(6, QGrid::Horizontal, scrollView->viewport(), "grid");
    grid->setSpacing(10);
    scrollView->addChild(grid);

    // Create the QButtonGroup used for all of the check boxes. It will not
    // be visible in the window, it will only be used to link the checkBoxes
    // together.
    checkBoxGroup = new QButtonGroup(0, "checkBoxParent");
    connect(checkBoxGroup, SIGNAL(clicked(int)),
            this, SLOT(emitChecked(int)));

    // Create a bunch of checkboxes
    checkBoxes = new QCheckBox * [MAX_CHECKBOXES];
    for(int i = 0; i < MAX_CHECKBOXES; ++i)
    {
        QString temp, cbName;
        temp.sprintf("newCheckBox_%d", i);
        cbName.sprintf("%d", i);
        checkBoxes[i] = new QCheckBox(cbName, grid, temp);
        checkBoxes[i]->setChecked(true);
        checkBoxes[i]->hide();
        checkBoxGroup->insert(checkBoxes[i], i);
    }
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::~QvisCheckListManagerWidget
//
// Purpose: 
//   This is the destructor for the QvisCheckListManagerWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:14:09 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCheckListManagerWidget::~QvisCheckListManagerWidget()
{
    for(int i = 0; i < MAX_CHECKBOXES; ++i)
    {
        checkBoxGroup->remove(checkBoxes[i]);
        delete checkBoxes[i];
    }
    delete [] checkBoxes;
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::removeLastEntry()
//
// Purpose: 
//   This method removes the last checkbox in the list from the widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:15:58 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::removeLastEntry()
{
    // If there are checkboxes, then remove the last one.
    if(nCheckBoxes > 0)
    {
        --nCheckBoxes;
        checkBoxes[nCheckBoxes]->hide();
    }
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::setCheckBox
//
// Purpose: 
//   This method sets the name and checked state of the specified checkbox.
//
// Arguments:
//   index   : The index of the checkbox we want to change.
//   name    : The new text to display with the specified checkbox.
//   checked : The new checked state of the specified checkbox.
//
// Note:       If the specified index is greater than the number of checkboxes
//   int the list, a new checkbox is added to the list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:16:34 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::setCheckBox(int index, const QString &name,
    bool checked)
{
    if(index >= MAX_CHECKBOXES)
        return;

    if(index >= nCheckBoxes)
    {
        ++nCheckBoxes;
        checkBoxes[index]->show();
    }

    checkBoxes[index]->setText(name);
    checkBoxGroup->blockSignals(true);
    checkBoxes[index]->setChecked(checked);
    checkBoxGroup->blockSignals(false);
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::numCheckBoxes
//
// Purpose: 
//   Returns the number of checkboxes in the list.
//
// Returns:    The number of checkboxes in the list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:18:53 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisCheckListManagerWidget::numCheckBoxes() const
{
    return nCheckBoxes;
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::emitChecked
//
// Purpose: 
//   This is a private Qt slot function that is hooked up to the QButtonGroup
//   of which the checkboxes are members. It is emitted when a checkbox changes
//   state.
//
// Arguments:
//   index : The index of the checkbox that whose state changed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:19:20 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::emitChecked(int index)
{
    if(index >= 0 && index < nCheckBoxes)
    {
        emit checked(index, checkBoxes[index]->isChecked());
    }
}
