#include "QvisMetricThresholdWindow.h"

#include <MetricThresholdAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

using std::string;

// ***
// Table: PresetValueTable.
//
// This table holds the preset values for the attr fields based on the type of metric test.
//
// Programmer: Akira Haddox
// Created: June 17 2002
// ***

struct PresetValueTableType
{
    bool hexOn;
    double hexLo;
    double hexHi;
    bool tetOn;
    double tetLo;
    double tetHi;
    bool wedgeOn;
    double wedgeLo;
    double wedgeHi;
    bool pyrOn;
    double pyrLo;
    double pyrHi;
    bool triOn;
    double triLo;
    double triHi;
    bool quadOn;
    double quadLo;
    double quadHi;
};

const int _LAST_PRESET = (int)(MetricThresholdAttributes::Largest_Angle);

const struct PresetValueTableType PresetValueTable[_LAST_PRESET+1] =
{
    //None
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Aspect_Ratio
    {    true,    1,    4,
        true,    1,    3,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    1,    4    },

    //Aspect_Gamma
    {    false,    0,    0,
        true,    1,    3,
        false,    0,    0,
        false,    0,    0,
        true,    1,    1.3,
        false,    0,    0    },

    //Skew
    {    true,    0,    0.5,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    0,    0.5    },

    //Taper
    {    true,    0,    0.4,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    0,    0.7    },

    //Element Volume
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Stretch
    {    true,    0.25,    1,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    0.25,    1    },

    //Diagonal
    {    true,    0.65,    1,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Dimension
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Oddy
    {    true,    0,    20,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    0,    16    },

    //Condition
    {    true,    1,    8,
        true,    1,    3,
        false,    0,    0,
        false,    0,    0,
        true,    1,    1.3,
        true,    1,    4    },

    //Jacobian
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Scaled Jacobian
    {    true,    0.5,    1,
        true,    0.5,    1.1414,
        false,    0,    0,
        false,    0,    0,
        true,    0.5,    1.155,
        true,    0.5,    1    },

    //Shear
    {    true,    0.3,    1,
        true,    0.3,    1,
        false,    0,    0,
        false,    0,    0,
        true,    0.75,    1,
        true,    0.3,    1    },

    //Shape
    {    true,    0.3,    1,
        true,    0.3,    1,
        false,    0,    0,
        false,    0,    0,
        true,    0.75,    1,
        true,    0.3,    1,    },

    //Relative Size
    {    true,    0.5,    1,
        true,    0.5,    1,
        false,    0,    0,
        false,    0,    0,
        true,    0.75,    1,
        true,    0.5,    1    },

    //Shape and Size
    {    true,    0.2,    1,
        true,    0.2,    1,
        false,    0,    0,
        false,    0,    0,
        true,    0.75,    1,
        true,    0.2,    1    },

    //Area
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0    },

    //Warpage
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    0,    0.1    },

    //Smallest Angle
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    30,    60,
        true,    45,    90    },

    //Largest Angle
    {    false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        false,    0,    0,
        true,    60,    90,
        true,    90,    135    }
};


// ****************************************************************************
// Method: QvisMetricThresholdWindow::QvisMetricThresholdWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisMetricThresholdWindow::QvisMetricThresholdWindow(const int type,
                         MetricThresholdAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisMetricThresholdWindow::~QvisMetricThresholdWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisMetricThresholdWindow::~QvisMetricThresholdWindow()
{
}


// ****************************************************************************
// Method: QvisMetricThresholdWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMetricThresholdWindow::CreateWindowContents()
{

    QGridLayout *mainLayout = new QGridLayout(topLayout, 13,2,  10, "mainLayout");


    // Preset
    QHBoxLayout *presetLayout = new QHBoxLayout(mainLayout);
    presetLayout->addWidget(new QLabel("Preset Data for: ", central, "presetLayout"));
    preset = new QComboBox(false, central, "preset");
    preset->insertItem("None");
    preset->insertItem("Aspect Ratio");
    preset->insertItem("Aspect Gamma");
    preset->insertItem("Skew");
    preset->insertItem("Taper");
    preset->insertItem("Volume");
    preset->insertItem("Stretch");
    preset->insertItem("Diagonal");
    preset->insertItem("Dimension");
    preset->insertItem("Oddy");
    preset->insertItem("Condition");
    preset->insertItem("Jacobian");
    preset->insertItem("Scaled Jacobian");
    preset->insertItem("Shear");
    preset->insertItem("Shape");
    preset->insertItem("Relative Size");
    preset->insertItem("Shape and Size");
    preset->insertItem("Area");
    preset->insertItem("Warpage");
    preset->insertItem("Smallest Angle");
    preset->insertItem("Largest Angle");
    presetLayout->addWidget(preset);

    connect(preset, SIGNAL(activated(int)),
            this, SLOT(presetChanged(int)));
    mainLayout->addWidget(preset, 0,1);

    Hexahedron = new QCheckBox("Hexahedron", central, "Hexahedron");
    connect(Hexahedron, SIGNAL(toggled(bool)),
            this, SLOT(HexahedronChanged(bool)));
    mainLayout->addWidget(Hexahedron, 2,0);

    mainLayout->addWidget(new QLabel("Enable Type", central, "et_label"),1,0);
    mainLayout->addWidget(new QLabel("Exclusion Range", central, "er_label"),1,1);

    mainLayout->addWidget(new QLabel("hex_lower", central, "hex_lowerLabel"),3,0);
    hex_lower = new QLineEdit(central, "hex_lower");
    connect(hex_lower, SIGNAL(returnPressed()),
            this, SLOT(hex_lowerProcessText()));
    mainLayout->addWidget(hex_lower, 3,1);

    mainLayout->addWidget(new QLabel("hex_upper", central, "hex_upperLabel"),4,0);
    hex_upper = new QLineEdit(central, "hex_upper");
    connect(hex_upper, SIGNAL(returnPressed()),
            this, SLOT(hex_upperProcessText()));
    mainLayout->addWidget(hex_upper, 4,1);

    Tetrahedron = new QCheckBox("Tetrahedron", central, "Tetrahedron");
    connect(Tetrahedron, SIGNAL(toggled(bool)),
            this, SLOT(TetrahedronChanged(bool)));
    mainLayout->addWidget(Tetrahedron, 5,0);

    mainLayout->addWidget(new QLabel("tet_lower", central, "tet_lowerLabel"),6,0);
    tet_lower = new QLineEdit(central, "tet_lower");
    connect(tet_lower, SIGNAL(returnPressed()),
            this, SLOT(tet_lowerProcessText()));
    mainLayout->addWidget(tet_lower, 6,1);

    mainLayout->addWidget(new QLabel("tet_upper", central, "tet_upperLabel"),7,0);
    tet_upper = new QLineEdit(central, "tet_upper");
    connect(tet_upper, SIGNAL(returnPressed()),
            this, SLOT(tet_upperProcessText()));
    mainLayout->addWidget(tet_upper, 7,1);

    Wedge = new QCheckBox("Wedge", central, "Wedge");
    connect(Wedge, SIGNAL(toggled(bool)),
            this, SLOT(WedgeChanged(bool)));
    mainLayout->addWidget(Wedge, 8,0);

    mainLayout->addWidget(new QLabel("wed_lower", central, "wed_lowerLabel"),9,0);
    wed_lower = new QLineEdit(central, "wed_lower");
    connect(wed_lower, SIGNAL(returnPressed()),
            this, SLOT(wed_lowerProcessText()));
    mainLayout->addWidget(wed_lower, 9,1);

    mainLayout->addWidget(new QLabel("wed_upper", central, "wed_upperLabel"),10,0);
    wed_upper = new QLineEdit(central, "wed_upper");
    connect(wed_upper, SIGNAL(returnPressed()),
            this, SLOT(wed_upperProcessText()));
    mainLayout->addWidget(wed_upper, 10,1);

    Pyramid = new QCheckBox("Pyramid", central, "Pyramid");
    connect(Pyramid, SIGNAL(toggled(bool)),
            this, SLOT(PyramidChanged(bool)));
    mainLayout->addWidget(Pyramid, 11,0);

    mainLayout->addWidget(new QLabel("pyr_lower", central, "pyr_lowerLabel"),12,0);
    pyr_lower = new QLineEdit(central, "pyr_lower");
    connect(pyr_lower, SIGNAL(returnPressed()),
            this, SLOT(pyr_lowerProcessText()));
    mainLayout->addWidget(pyr_lower, 12,1);

    mainLayout->addWidget(new QLabel("pyr_upper", central, "pyr_upperLabel"),13,0);
    pyr_upper = new QLineEdit(central, "pyr_upper");
    connect(pyr_upper, SIGNAL(returnPressed()),
            this, SLOT(pyr_upperProcessText()));
    mainLayout->addWidget(pyr_upper, 13,1);

    Triangle = new QCheckBox("Triangle", central, "Triangle");
    connect(Triangle, SIGNAL(toggled(bool)),
            this, SLOT(TriangleChanged(bool)));
    mainLayout->addWidget(Triangle, 14,0);

    mainLayout->addWidget(new QLabel("tri_lower", central, "tri_lowerLabel"),15,0);
    tri_lower = new QLineEdit(central, "tri_lower");
    connect(tri_lower, SIGNAL(returnPressed()),
            this, SLOT(tri_lowerProcessText()));
    mainLayout->addWidget(tri_lower, 15,1);

    mainLayout->addWidget(new QLabel("tri_upper", central, "tri_upperLabel"),16,0);
    tri_upper = new QLineEdit(central, "tri_upper");
    connect(tri_upper, SIGNAL(returnPressed()),
            this, SLOT(tri_upperProcessText()));
    mainLayout->addWidget(tri_upper, 16,1);

    Quad = new QCheckBox("Quad", central, "quad");
    connect(Quad, SIGNAL(toggled(bool)),
            this, SLOT(QuadChanged(bool)));
    mainLayout->addWidget(Quad, 17,0);

    mainLayout->addWidget(new QLabel("quad_lower", central, "quad_lowerLabel"),18,0);
    quad_lower = new QLineEdit(central, "quad_lower");
    connect(quad_lower, SIGNAL(returnPressed()),
            this, SLOT(quad_lowerProcessText()));
    mainLayout->addWidget(quad_lower, 18,1);

    mainLayout->addWidget(new QLabel("quad_upper", central, "quad_upperLabel"),19,0);
    quad_upper = new QLineEdit(central, "quad_upper");
    connect(quad_upper, SIGNAL(returnPressed()),
            this, SLOT(quad_upperProcessText()));
    mainLayout->addWidget(quad_upper, 19,1);
}



// ****************************************************************************
// Method: QvisMetricThresholdWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
// ****************************************************************************

void
QvisMetricThresholdWindow::UpdateWindow(bool doAll)
{
    QString temp;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case 0: //preset
            preset->setCurrentItem(atts->GetPreset());
            break;
          case 1: //Hexahedron
            Hexahedron->setChecked(atts->GetHexahedron());
            break;
          case 2: //hex_lower
            temp.setNum(atts->GetHex_lower());
            hex_lower->setText(temp);
            break;
          case 3: //hex_upper
            temp.setNum(atts->GetHex_upper());
            hex_upper->setText(temp);
            break;
          case 4: //Tetrahedron
            Tetrahedron->setChecked(atts->GetTetrahedron());
            break;
          case 5: //tet_lower
            temp.setNum(atts->GetTet_lower());
            tet_lower->setText(temp);
            break;
          case 6: //tet_upper
            temp.setNum(atts->GetTet_upper());
            tet_upper->setText(temp);
            break;
          case 7: //Wedge
            Wedge->setChecked(atts->GetWedge());
            break;
          case 8: //wed_lower
            temp.setNum(atts->GetWed_lower());
            wed_lower->setText(temp);
            break;
          case 9: //wed_upper
            temp.setNum(atts->GetWed_upper());
            wed_upper->setText(temp);
            break;
          case 10: //Pyramid
            Pyramid->setChecked(atts->GetPyramid());
            break;
          case 11: //pyr_lower
            temp.setNum(atts->GetPyr_lower());
            pyr_lower->setText(temp);
            break;
          case 12: //pyr_upper
            temp.setNum(atts->GetPyr_upper());
            pyr_upper->setText(temp);
            break;
          case 13: //Triangle
            Triangle->setChecked(atts->GetTriangle());
            break;
          case 14: //tri_lower
            temp.setNum(atts->GetTri_lower());
            tri_lower->setText(temp);
            break;
          case 15: //tri_upper
            temp.setNum(atts->GetTri_upper());
            tri_upper->setText(temp);
            break;
          case 16: //Quad
            Quad->setChecked(atts->GetQuad());
            break;
          case 17: //quad_lower
            temp.setNum(atts->GetQuad_lower());
            quad_lower->setText(temp);
            break;
          case 18: //quad_upper
            temp.setNum(atts->GetQuad_upper());
            quad_upper->setText(temp);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisMetricThresholdWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMetricThresholdWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do preset
    if(which_widget == 0 || doAll)
    {
        // Nothing for preset
    }

    // Do Hexahedron
    if(which_widget == 1 || doAll)
    {
        // Nothing for Hexahedron
    }

    // Do hex_lower
    if(which_widget == 2 || doAll)
    {
        temp = hex_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetHex_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of hex_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetHex_lower());
            Message(msg);
            atts->SetHex_lower(atts->GetHex_lower());
        }
    }

    // Do hex_upper
    if(which_widget == 3 || doAll)
    {
        temp = hex_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetHex_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of hex_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetHex_upper());
            Message(msg);
            atts->SetHex_upper(atts->GetHex_upper());
        }
    }

    // Do Tetrahedron
    if(which_widget == 4 || doAll)
    {
        // Nothing for Tetrahedron
    }

    // Do tet_lower
    if(which_widget == 5 || doAll)
    {
        temp = tet_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTet_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tet_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTet_lower());
            Message(msg);
            atts->SetTet_lower(atts->GetTet_lower());
        }
    }

    // Do tet_upper
    if(which_widget == 6 || doAll)
    {
        temp = tet_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTet_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tet_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTet_upper());
            Message(msg);
            atts->SetTet_upper(atts->GetTet_upper());
        }
    }

    // Do Wedge
    if(which_widget == 7 || doAll)
    {
        // Nothing for Wedge
    }

    // Do wed_lower
    if(which_widget == 8 || doAll)
    {
        temp = wed_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetWed_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of wed_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetWed_lower());
            Message(msg);
            atts->SetWed_lower(atts->GetWed_lower());
        }
    }

    // Do wed_upper
    if(which_widget == 9 || doAll)
    {
        temp = wed_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetWed_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of wed_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetWed_upper());
            Message(msg);
            atts->SetWed_upper(atts->GetWed_upper());
        }
    }

    // Do Pyramid
    if(which_widget == 10 || doAll)
    {
        // Nothing for Pyramid
    }

    // Do pyr_lower
    if(which_widget == 11 || doAll)
    {
        temp = pyr_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetPyr_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of pyr_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetPyr_lower());
            Message(msg);
            atts->SetPyr_lower(atts->GetPyr_lower());
        }
    }

    // Do pyr_upper
    if(which_widget == 12 || doAll)
    {
        temp = pyr_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetPyr_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of pyr_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetPyr_upper());
            Message(msg);
            atts->SetPyr_upper(atts->GetPyr_upper());
        }
    }

    // Do Triangle
    if(which_widget == 13 || doAll)
    {
        // Nothing for Triangle
    }

    // Do tri_lower
    if(which_widget == 14 || doAll)
    {
        temp = tri_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTri_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tri_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTri_lower());
            Message(msg);
            atts->SetTri_lower(atts->GetTri_lower());
        }
    }

    // Do tri_upper
    if(which_widget == 15 || doAll)
    {
        temp = tri_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetTri_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of tri_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetTri_upper());
            Message(msg);
            atts->SetTri_upper(atts->GetTri_upper());
        }
    }

    // Do Quad
    if(which_widget == 16 || doAll)
    {
        // Nothing for Quad
    }

    // Do quad_lower
    if(which_widget == 17 || doAll)
    {
        temp = quad_lower->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetQuad_lower(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of quad_lower was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetQuad_lower());
            Message(msg);
            atts->SetQuad_lower(atts->GetQuad_lower());
        }
    }

    // Do quad_upper
    if(which_widget == 18 || doAll)
    {
        temp = quad_upper->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            atts->SetQuad_upper(val);
        }

        if(!okay)
        {
            msg.sprintf("The value of quad_upper was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetQuad_upper());
            Message(msg);
            atts->SetQuad_upper(atts->GetQuad_upper());
        }
    }

}


//
// Qt Slot functions
//


void
QvisMetricThresholdWindow::presetChanged(int val)
{
    if(val != atts->GetPreset())
    {
        atts->SetPreset(MetricThresholdAttributes::Preset(val));
        Apply();
    }

    MetricThresholdAttributes::Preset p=(MetricThresholdAttributes::Preset)(val);
    if(p>_LAST_PRESET)
    return; // Something's not supported/not right

    int index=(int)p;

    atts->SetHexahedron(PresetValueTable[index].hexOn);
    atts->SetHex_lower(PresetValueTable[index].hexLo);
    atts->SetHex_upper(PresetValueTable[index].hexHi);
    atts->SetTetrahedron(PresetValueTable[index].tetOn);
    atts->SetTet_lower(PresetValueTable[index].tetLo);
    atts->SetTet_upper(PresetValueTable[index].tetHi);
    atts->SetWedge(PresetValueTable[index].wedgeOn);
    atts->SetWed_lower(PresetValueTable[index].wedgeLo);
    atts->SetWed_upper(PresetValueTable[index].wedgeHi);
    atts->SetPyramid(PresetValueTable[index].pyrOn);
    atts->SetPyr_lower(PresetValueTable[index].pyrLo);
    atts->SetPyr_upper(PresetValueTable[index].pyrHi);
    atts->SetTriangle(PresetValueTable[index].triOn);
    atts->SetTri_lower(PresetValueTable[index].triLo);
    atts->SetTri_upper(PresetValueTable[index].triHi);
    atts->SetQuad(PresetValueTable[index].quadOn);
    atts->SetQuad_lower(PresetValueTable[index].quadLo);
    atts->SetQuad_upper(PresetValueTable[index].quadHi);
    Apply();

}


void
QvisMetricThresholdWindow::HexahedronChanged(bool val)
{
    atts->SetHexahedron(val);
    Apply();
}


void
QvisMetricThresholdWindow::hex_lowerProcessText()
{
    GetCurrentValues(2);
    Apply();
}


void
QvisMetricThresholdWindow::hex_upperProcessText()
{
    GetCurrentValues(3);
    Apply();
}


void
QvisMetricThresholdWindow::TetrahedronChanged(bool val)
{
    atts->SetTetrahedron(val);
    Apply();
}


void
QvisMetricThresholdWindow::tet_lowerProcessText()
{
    GetCurrentValues(5);
    Apply();
}


void
QvisMetricThresholdWindow::tet_upperProcessText()
{
    GetCurrentValues(6);
    Apply();
}


void
QvisMetricThresholdWindow::WedgeChanged(bool val)
{
    atts->SetWedge(val);
    Apply();
}


void
QvisMetricThresholdWindow::wed_lowerProcessText()
{
    GetCurrentValues(8);
    Apply();
}


void
QvisMetricThresholdWindow::wed_upperProcessText()
{
    GetCurrentValues(9);
    Apply();
}


void
QvisMetricThresholdWindow::PyramidChanged(bool val)
{
    atts->SetPyramid(val);
    Apply();
}


void
QvisMetricThresholdWindow::pyr_lowerProcessText()
{
    GetCurrentValues(11);
    Apply();
}


void
QvisMetricThresholdWindow::pyr_upperProcessText()
{
    GetCurrentValues(12);
    Apply();
}


void
QvisMetricThresholdWindow::TriangleChanged(bool val)
{
    atts->SetTriangle(val);
    Apply();
}


void
QvisMetricThresholdWindow::tri_lowerProcessText()
{
    GetCurrentValues(14);
    Apply();
}


void
QvisMetricThresholdWindow::tri_upperProcessText()
{
    GetCurrentValues(15);
    Apply();
}


void
QvisMetricThresholdWindow::QuadChanged(bool val)
{
    atts->SetQuad(val);
    Apply();
}


void
QvisMetricThresholdWindow::quad_lowerProcessText()
{
    GetCurrentValues(17);
    Apply();
}


void
QvisMetricThresholdWindow::quad_upperProcessText()
{
    GetCurrentValues(18);
    Apply();
}


