#include <QvisAppearanceWindow.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qfontdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QvisColorButton.h>
#include <AppearanceAttributes.h>
#include <ViewerProxy.h>

//
// Tables of available styles.
//
static const char *styleNamesInMenu[] = {"Motif", "CDE", "Windows", "Platinum"
#if QT_VERSION >= 230
, "SGI"
#endif
#if QT_VERSION >= 300
#ifdef Q_WS_MACX
, "Aqua", "Macintosh"
#endif
#endif
};

static const char *styleNames[] = {"motif", "cde", "windows", "platinum"
#if QT_VERSION >= 230
, "sgi"
#endif
#if QT_VERSION >= 300
#ifdef Q_WS_MACX
, "aqua", "macintosh"
#endif
#endif
};

// Determine the number of styles.
#if QT_VERSION >= 300
#ifdef Q_WS_MACX
static const int numStyleNames = 7;
#else
static const int numStyleNames = 5;
#endif 
#elif QT_VERSION >= 230
static const int numStyleNames = 5; // account for sgi
#else
static const int numStyleNames = 4;
#endif

// ****************************************************************************
// Method: QvisAppearanceWindow::QvisAppearanceWindow
//
// Purpose: 
//   This is the constructor for the QvisAppearanceWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:27:16 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisAppearanceWindow::QvisAppearanceWindow(AppearanceAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisAppearanceWindow::~QvisAppearanceWindow
//
// Purpose: 
//   This is the destructor for the QvisAppearanceWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:28:27 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisAppearanceWindow::~QvisAppearanceWindow()
{
    // Nothing here
}

// ****************************************************************************
// Method: QvisAppearanceWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:31:03 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:16:20 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Fri Aug 15 13:08:37 PST 2003
//   I changed how the style menu is populated.
//
// ****************************************************************************

void
QvisAppearanceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 5, 2, 10);

    // Create the background color button.
    backgroundColorButton = new QvisColorButton(central, "backgroundColorButton");
    connect(backgroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(backgroundChanged(const QColor &)));
    mainLayout->addWidget(backgroundColorButton, 0, 1, AlignLeft);
    mainLayout->addWidget(new QLabel(backgroundColorButton, "GUI background",
                                     central, "backgroundLabel"),0,0);

    // Create the background color button.
    foregroundColorButton = new QvisColorButton(central, "foregroundColorButton");
    connect(foregroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(foregroundChanged(const QColor &)));
    mainLayout->addWidget(foregroundColorButton, 1, 1, AlignLeft);
    mainLayout->addWidget(new QLabel(foregroundColorButton, "GUI foreground",
                                     central, "foregroundLabel"),1,0);

    // Create the style combo box.
    styleComboBox = new QComboBox(central, "styleComboBox");
    for(int i = 0; i < numStyleNames; ++i)
        styleComboBox->insertItem(styleNamesInMenu[i], i);
    connect(styleComboBox, SIGNAL(activated(int)),
            this, SLOT(styleChanged(int)));
    mainLayout->addWidget(styleComboBox, 2, 1, AlignLeft);
    mainLayout->addWidget(new QLabel(styleComboBox, "GUI style",
                                     central, "foregroundLabel"),2,0);

    // Create the orientation combo box.
    orientationComboBox = new QComboBox(central, "orientationComboBox");
    orientationComboBox->insertItem("Vertical", 0);
    orientationComboBox->insertItem("Horizontal", 1);
    connect(orientationComboBox, SIGNAL(activated(int)),
            this, SLOT(orientationChanged(int)));
    mainLayout->addWidget(orientationComboBox, 3, 1, AlignLeft);
    mainLayout->addWidget(new QLabel(orientationComboBox, "GUI orientation",
                                     central, "orientationLabel"),3,0);

#if 0
    // Create the font button.
    QPushButton *fontButton = new QPushButton("Select font...", central, "fontButton");
    connect(fontButton, SIGNAL(clicked()),
            this, SLOT(handleFontClicked()));
    mainLayout->addWidget(fontButton, 4, 0);
#endif
}

// ****************************************************************************
// Method: QvisAppearanceWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the appearance attributes are updated.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:57:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:14:42 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Tue Apr 1 09:15:14 PDT 2003
//   I made it use QColor's parsing instead of sscanf.
//
//   Brad Whitlock, Fri Aug 15 13:10:28 PST 2003
//   I changed how the styles are set.
//
// ****************************************************************************

void
QvisAppearanceWindow::UpdateWindow(bool doAll)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    int  j;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case 0: // background
        { // new scope
            QColor bg(atts->GetBackground().c_str());
            backgroundColorButton->blockSignals(true);
            backgroundColorButton->setButtonColor(bg);
            backgroundColorButton->blockSignals(false);
        }
            break;
        case 1: //foreground
        { // new scope
            QColor fg(atts->GetForeground().c_str());
            foregroundColorButton->blockSignals(true);
            foregroundColorButton->setButtonColor(fg);
            foregroundColorButton->blockSignals(false);
        }
            break;
        case 2: // fontDescription
            break;
        case 3: // style
            for(j = 0; j < numStyleNames; ++j)
            {
                if(atts->GetStyle() == styleNames[j])
                {
                    styleComboBox->blockSignals(true);
                    styleComboBox->setCurrentItem(j);
                    styleComboBox->blockSignals(false);
                    break;
                }
            }
            break;
        case 4: // orientation
            orientationComboBox->blockSignals(true);
            if(atts->GetOrientation() == 0)
                orientationComboBox->setCurrentItem(0);
            else
                orientationComboBox->setCurrentItem(1);
            orientationComboBox->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::Apply
//
// Purpose: 
//   This method applies the changes to the appearance.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:15:11 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 18 13:58:39 PST 2001
//   Removed an unused variable.
//
// ****************************************************************************

void
QvisAppearanceWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        emit changeAppearance(true);
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisAppearanceWindow::ColorsNotTooClose
//
// Purpose: 
//   Prevents bad colors from being used together.
//
// Arguments:
//   c0    : The first color to check.
//   c1str : The string representation of the second color to check.
//
// Returns:    True if the colors can be used together; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 3 10:02:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisAppearanceWindow::ColorsNotTooClose(const QColor &c0, const char *c1str)
{
    QColor c1(c1str);

    int dR = int(c0.red()) - int(c1.red());
    int dG = int(c0.green()) - int(c1.green());
    int dB = int(c0.blue()) - int(c1.blue());

    const int threshold = 10;
    bool rClose = dR > -threshold && dR < threshold;
    bool gClose = dG > -threshold && dG < threshold;
    bool bClose = dB > -threshold && dB < threshold;

    bool retval = true;
    if(rClose && gClose && bClose)
    {
        // Update the window so the color buttons get set back properly.
        UpdateWindow(true);

        // Tell the user that it was a bad idea.
        Warning("The background color and foreground color will not be "
                "changed because the selected colors are too similar and "
                "using them would make it too difficult to use VisIt.");
        retval = false;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisAppearanceWindow::backgroundChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the GUI
//   background color via the color button.
//
// Arguments:
//   bg : The new background color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:36 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 10:03:19 PDT 2003
//   Prevented bad colors from being used together.
//
// ****************************************************************************

void
QvisAppearanceWindow::backgroundChanged(const QColor &bg)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;

    if(ColorsNotTooClose(bg, atts->GetForeground().c_str()))
    {
        QString tmp;
        tmp.sprintf("#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
        atts->SetBackground(tmp.latin1());
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::foregroundChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the GUI
//   foreground color via the color button.
//
// Arguments:
//   fg : The new foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:36 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 10:03:19 PDT 2003
//   Prevented bad colors from being used together.
//   
// ****************************************************************************

void
QvisAppearanceWindow::foregroundChanged(const QColor &fg)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;

    if(ColorsNotTooClose(fg, atts->GetBackground().c_str()))
    {
        QString tmp;
        tmp.sprintf("#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
        atts->SetForeground(tmp.latin1());
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::styleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the style.
//
// Arguments:
//   index : The index of the new style.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:17:42 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Aug 15 13:11:13 PST 2003
//   I made it use a table lookup.
//
// ****************************************************************************

void
QvisAppearanceWindow::styleChanged(int index)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetStyle(styleNames[index]);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAppearanceWindow::handleFontClicked
//
// Purpose: 
//   This is a Qt slot function that is called when the Font... button is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 19:38:04 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::handleFontClicked()
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    bool  okay;

    // Get a font using the current font.
    QFont currentFont(atts->GetFontDescription().c_str());

    // Open the font dialog box and select a new font.
    QFont newFont = QFontDialog::getFont(&okay, currentFont, this);
    if(okay)
    {
        QApplication::setFont(newFont, true);

        // The user selected a valid font. Put the name of that font into the
        // appearance attributes.
        atts->SetFontDescription(newFont.rawName().latin1());
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::orientationChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the orientation is changed.
//
// Arguments:
//   index : The new orientation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 13:21:26 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::orientationChanged(int index)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetOrientation((index == 0) ? 0 : 2);
    SetUpdate(false);
    Apply();
}
