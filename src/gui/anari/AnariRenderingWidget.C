// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariRenderingWidget.h>
#include <AnariParameterInfo.h>
#include <QvisRenderingWindow.h>
#include <RenderingAttributes.h>
#include <DebugStream.h>

#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QString>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QCheckBox>
#include <QSpacerItem>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QColorDialog>

#include <limits>

const std::string AnariRenderingWidget::USD_WIDGET_KEY = "usd";
const std::string AnariRenderingWidget::DEFAULT_WIDGET_KEY = "default";


// ****************************************************************************
// Method: AnariRenderingWidget::AnariRenderingWidget
//
// Purpose:
//   Constructor for the AnariRenderingWidget class.
//
// Arguments:
//   qrw        Window that displays rendering settings
//   ra         Contains ANARI rendering attributes
//   parent     If parent is another widget, this widget becomes a
//              child window inside parent. The new widget is deleted
//                          when its parent is deleted.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

AnariRenderingWidget::AnariRenderingWidget(QvisRenderingWindow *qrw,
                                           RenderingAttributes *ra,
                                           QWidget *parent)
    : QWidget(parent)
    , renderingWindow(qrw)
    , anariAttributes(&(ra->GetAnariAttributes()))
    , dynamicLayouts(nullptr)
    , dynamicLayoutMap()
    , topRows(0)
    , bottomRows(0)
    , renderingGroup(nullptr)
    , libraryName(nullptr)
    , librarySubtypes(nullptr)
    , rendererSubtypes(nullptr)
    , currentDirectory()
    , dirLineEdit(nullptr)
{
    // row, col, rowspan, colspan
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // Rendering Group
    renderingGroup = new QGroupBox(tr("ANARI Rendering"));
    renderingGroup->setCheckable(true);
    renderingGroup->setChecked(false);
    connect(renderingGroup, &QGroupBox::toggled,
            this, &AnariRenderingWidget::renderingToggled);

    QVBoxLayout *renderingGroupVBoxLayout = new QVBoxLayout(renderingGroup);

    // General
    renderingGroupVBoxLayout->addWidget(CreateGeneralWidget(topRows));

    // Widget Cache
    dynamicLayouts = new QStackedLayout();

    // Placeholder Widget
    int widgetIndex = dynamicLayouts->addWidget(new QWidget(this));
    dynamicLayoutMap[AnariRenderingWidget::DEFAULT_WIDGET_KEY] = widgetIndex;

    // USD Widget
    widgetIndex = dynamicLayouts->addWidget(CreateUSDWidget(bottomRows));
    dynamicLayoutMap[AnariRenderingWidget::USD_WIDGET_KEY] = widgetIndex;

    renderingGroupVBoxLayout->addLayout(dynamicLayouts);
    mainLayout->addWidget(renderingGroup);

    connect(this, &AnariRenderingWidget::currentBackendChanged,
            dynamicLayouts, &QStackedLayout::setCurrentIndex);
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateGeneralWidget
//
// Purpose:
//   Creates the UI components for selecting back-end options used by all
//   ANARI libraries.
//
// Arguments:
//   rows keeps track of the total rows used to create this widget
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateGeneralWidget(int &rows)
{
    QWidget *generalOptionsWidget = new QWidget(this);

    QGridLayout *gridLayout = new QGridLayout(generalOptionsWidget);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    gridLayout->setColumnStretch(1, 2);
    gridLayout->setColumnStretch(3, 2);
    gridLayout->setColumnStretch(4, 5);

    libraryName = new QComboBox(generalOptionsWidget);
    connect(libraryName, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnariRenderingWidget::libraryChanged);

    // Back-end and subtype
    QLabel *backendLabel = new QLabel(tr("Back-end"));
    backendLabel->setToolTip(tr("ANARI back-end device"));

    gridLayout->addWidget(backendLabel, rows, 0, 1, 1);
    gridLayout->addWidget(libraryName, rows, 1, 1, 2);

    // Back-end subtype
    librarySubtypes = new QComboBox();
    librarySubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(librarySubtypes, &QComboBox::currentTextChanged,
            this, &AnariRenderingWidget::librarySubtypeChanged);

    QLabel *subtypeLabel = new QLabel(tr("Back-end Subtype"));

    gridLayout->addWidget(subtypeLabel, rows, 3, 1, 1);
    gridLayout->addWidget(librarySubtypes, rows, 4, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 4, 1, 1);

    // Renderer
    rendererSubtypes = new QComboBox();
    rendererSubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(rendererSubtypes, &QComboBox::currentTextChanged,
            this, &AnariRenderingWidget::rendererSubtypeChanged);

    QLabel *rendererLabel = new QLabel(tr("Renderer"));
    rendererLabel->setToolTip(tr("Renderer subtype"));

    gridLayout->addWidget(rendererLabel, rows, 0, 1, 1);
    gridLayout->addWidget(rendererSubtypes, rows, 1, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 3, 1, 3);

    return generalOptionsWidget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateUSDWidget
//
// Purpose:
//   Creates the UI components used by ANARI back-ends.
//
// Arguments:
//   rows keeps track of the total rows of UI components
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateUSDWidget(int &rows)
{
    auto widget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(widget);

    auto gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    gridLayout->setColumnStretch(1, 3);

    // row, col, rowspan, colspan
    // Output location for the USD files
    // Row 1
    auto locationLabel = new QLabel("Directory");
    locationLabel->setToolTip(tr("Output location for saving the USD files"));

    currentDirectory = QString(QDir::homePath());
    dirLineEdit = new QLineEdit(currentDirectory);
    dirLineEdit->setObjectName("usd::serialize.location");

    connect(dirLineEdit, &QLineEdit::editingFinished, this, &AnariRenderingWidget::lineEditingFinished);

    auto dirSelectButton = new QPushButton("Select");
    connect(dirSelectButton, &QPushButton::pressed, this, &AnariRenderingWidget::selectButtonPressed);

    auto commitCheckBox = new QCheckBox(tr("commit"));
    commitCheckBox->setObjectName("usd::writeatcommit");
    commitCheckBox->setToolTip(tr("Write USD at ANARI commit call"));
    commitCheckBox->setChecked(true);

    connect(commitCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);

    gridLayout->addWidget(locationLabel, 0, 0, 1, 1);
    gridLayout->addWidget(dirLineEdit, 0, 1, 1, 2);
    gridLayout->addWidget(dirSelectButton, 0, 3, 1, 1);
    gridLayout->addWidget(commitCheckBox, 0, 4, 1, 1);

    mainLayout->addLayout(gridLayout);

    // Row 2
    rows++;
    auto outputGroup = new QGroupBox(tr("Output"));

    auto gridLayout2 = new QGridLayout(outputGroup);
    gridLayout2->setSpacing(10);
    gridLayout2->setContentsMargins(10,10,10,10);

    auto binaryCheckBox = new QCheckBox(tr("Binary"));
    binaryCheckBox->setObjectName("usd::serialize.outputbinary");
    binaryCheckBox->setToolTip(tr("Binary or text output"));
    binaryCheckBox->setChecked(true);

    connect(binaryCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(binaryCheckBox, 0, 0, 1, 1);

    auto materialCheckBox = new QCheckBox(tr("Material"));
    materialCheckBox->setObjectName("usd::output.material");
    materialCheckBox->setToolTip(tr("Include material objects in the output"));
    materialCheckBox->setChecked(true);

    connect(materialCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(materialCheckBox, 0, 1, 1, 1);

    auto previewCheckBox = new QCheckBox(tr("Preview Surface"));
    previewCheckBox->setObjectName("usd::output.previewsurfaceshader");
    previewCheckBox->setToolTip(tr("Include preview surface shader prims in the output for material objects"));
    previewCheckBox->setChecked(false);

    connect(previewCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(previewCheckBox, 0, 2, 1, 1);

    // Row 3
    rows++;

    auto mdlCheckBox = new QCheckBox(tr("MDL"));
    mdlCheckBox->setObjectName("usd::output.mdlshader");
    mdlCheckBox->setToolTip(tr("Include MDL shader prims in the output for material objects"));
    mdlCheckBox->setChecked(true);

    connect(mdlCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(mdlCheckBox, 1, 0, 1, 1);

    auto mdlColorCheckBox = new QCheckBox(tr("MDL Colors"));
    mdlColorCheckBox->setObjectName("usd::output.mdlcolors");
    mdlColorCheckBox->setToolTip(tr("Include MDL colors in the output for material objects"));
    mdlColorCheckBox->setChecked(true);

    connect(mdlColorCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(mdlColorCheckBox, 1, 1, 1, 1);

    auto displayColorCheckBox = new QCheckBox(tr("Display Colors"));
    displayColorCheckBox->setObjectName("usd::output.displaycolors");
    displayColorCheckBox->setToolTip(tr("Include display colors in the output"));
    displayColorCheckBox->setChecked(false);

    connect(displayColorCheckBox, &QCheckBox::toggled, this, &AnariRenderingWidget::checkBoxToggled);
    gridLayout2->addWidget(displayColorCheckBox, 1, 2, 1, 1);

    rows++;
    mainLayout->addWidget(outputGroup);

    return widget;
}

// ****************************************************************************
// Method: AnariRenderingWidget::MakeWidgetFromParameterInfo
//
// Purpose:
//   Creates the individual UI element from the parameter information obtained
//   from ANARI's object introspection.
//
// Arguments:
//   paramInfo  Contains information (description, min, max, default value, etc.)
//              about the parameter.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::MakeWidgetFromParameterInfo(const AnariParameterInfo &paramInfo)
{
    const auto paramName = QString::fromStdString(paramInfo.GetName());
    const auto paramType = paramInfo.GetType();

     // Ignore ANARI metadata and handle-like types that cannot be edited here.
    if (paramName == "name" || paramName == "background" ||
        (paramType < ANARI_INT8 && 
         paramType != ANARI_STRING && 
         paramType != ANARI_STRING_LIST &&
         paramType != ANARI_BOOL) )
    {
        return nullptr;
    }
    
    // Color-based parameters
    if ( paramName.contains("ambientColor", Qt::CaseInsensitive) ||
         paramName.contains("color", Qt::CaseInsensitive))
    {
        auto* container = new QWidget(this);
        auto* hbox = new QHBoxLayout(container);
        hbox->setContentsMargins(0, 0, 0, 0);
        auto* button = CreateColorButton(container, paramType, paramInfo.GetDefaultValueText());
        button->setObjectName(paramInfo.GetName().c_str());
        hbox->addWidget(button);

        return container;
    }

    switch(paramType)
    {
        case ANARI_INT32:
        {
            QSpinBox *spinBox = new QSpinBox();
            spinBox->setObjectName(paramInfo.GetName().c_str());

            if(!paramInfo.GetDefaultValueText().empty())
            {
                spinBox->setValue(QString::fromStdString(paramInfo.GetDefaultValueText()).toInt());
            }

            int min = std::numeric_limits<int>::min();
            int max = std::numeric_limits<int>::max();

            if(paramInfo.HasMinimumText())
                min = QString::fromStdString(paramInfo.GetMinimumText()).toInt();

            if(paramInfo.HasMaximumText())
                max = QString::fromStdString(paramInfo.GetMaximumText()).toInt();

            spinBox->setRange(min, max);

            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, &AnariRenderingWidget::spinBoxValueChanged);

            return spinBox;
        }
        case ANARI_FLOAT32:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            float min = std::numeric_limits<float>::min();
            float max = std::numeric_limits<float>::max();

            if(!paramInfo.GetDefaultValueText().empty())
                lineEdit->setText(QString::fromStdString(paramInfo.GetDefaultValueText()));

            if(paramInfo.HasMinimumText())
                min = QString::fromStdString(paramInfo.GetMinimumText()).toFloat();

            if(paramInfo.HasMaximumText())
                max = QString::fromStdString(paramInfo.GetMaximumText()).toFloat();

            QDoubleValidator *dv = new QDoubleValidator(min, max, 4);
            lineEdit->setValidator(dv);

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_FLOAT64:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            double min = std::numeric_limits<double>::min();
            double max = std::numeric_limits<double>::max();

            if(!paramInfo.GetDefaultValueText().empty())
                lineEdit->setText(QString::fromStdString(paramInfo.GetDefaultValueText()));

            if(paramInfo.HasMinimumText())
                min = QString::fromStdString(paramInfo.GetMinimumText()).toDouble();

            if(paramInfo.HasMaximumText())
                max = QString::fromStdString(paramInfo.GetMaximumText()).toDouble();

            QDoubleValidator *dv = new QDoubleValidator(min, max, 4);
            lineEdit->setValidator(dv);

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_STRING:
        {
            auto acceptedVals = paramInfo.GetAcceptedValues();

            if(!acceptedVals.empty())
            {
                QComboBox *comboBox = new QComboBox();
                comboBox->setObjectName(paramInfo.GetName().c_str());

                for(const std::string &value : acceptedVals)
                {
                    comboBox->addItem(value.c_str());
                }

                connect(comboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
                        this, &AnariRenderingWidget::comboBoxTextChanged);

                return comboBox;
            }

            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            if(!paramInfo.GetDefaultValueText().empty())
                lineEdit->setText(QString::fromStdString(paramInfo.GetDefaultValueText()));

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_BOOL:
        {
            QCheckBox *checkBox = new QCheckBox(paramInfo.GetName().c_str());
            checkBox->setObjectName(paramInfo.GetName().c_str());

            auto toolTip = paramInfo.GetDescription();

            if(!toolTip.empty())
            {
                checkBox->setToolTip(tr(toolTip.c_str()));
            }

            if(!paramInfo.GetDefaultValueText().empty())
                checkBox->setChecked(paramInfo.GetDefaultValueText() == "true");

            connect(checkBox, &QCheckBox::toggled,
                    this, &AnariRenderingWidget::checkBoxToggled);

            return checkBox;
        }
        default:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            if(!paramInfo.GetDefaultValueText().empty())
                lineEdit->setText(QString::fromStdString(paramInfo.GetDefaultValueText()));

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateDynamicWidget
//
// Purpose:
//   Creates the panel for the dynamic parameters of the selected back-end,
//   back-end subtype, and renderer subtype.
//
// Arguments:
//   parameters   MapNode of parameter name -> parameter info, as reported by
//                the engine's AnariDeviceInfoRPC (see BuildParameterInfoFromMapNode)
//   key          The key to the dynamic layout map
//   isUSD        If the back-end is USD
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Thu 27 Aug 2026
//   Build from a MapNode reported by the engine instead of a live
//   anari::Device, since the client may not have ANARI backends installed.
//
// ****************************************************************************

void
AnariRenderingWidget::CreateDynamicWidget(const MapNode &parameters, const std::string &key, bool isUSD)
{
    int stackLayoutIndex = this->dynamicLayoutMap[AnariRenderingWidget::DEFAULT_WIDGET_KEY];

    if(isUSD)
    {
        stackLayoutIndex = this->dynamicLayoutMap[AnariRenderingWidget::USD_WIDGET_KEY];
    }
    else
    {
        auto resultIter = dynamicLayoutMap.find(key);

        if(resultIter == dynamicLayoutMap.end())
        {
            QWidget *dynamicWidget = new QWidget(this);

            QGridLayout *gridLayout = new QGridLayout(dynamicWidget);
            gridLayout->setSpacing(10);
            gridLayout->setContentsMargins(10,10,10,10);

            int rows = 0;
            int cols = 0;

            stringVector paramNames;
            parameters.GetEntryNames(paramNames);

            for(const std::string &paramName : paramNames)
            {
                const MapNode *paramNode = parameters.GetEntry(paramName);
                if(paramNode == nullptr)
                    continue;

                AnariParameterInfo paramInfo = BuildParameterInfoFromMapNode(paramName, *paramNode);

                // Create the UI
                QWidget *uiWidget = MakeWidgetFromParameterInfo(paramInfo);

                if(uiWidget == nullptr)
                {
                    continue;
                }

                if(paramInfo.GetType() != ANARI_BOOL)
                {
                    QLabel *label = new QLabel(paramName.c_str());
                    std::string toolTip = paramInfo.GetDescription();

                    if(!toolTip.empty())
                    {
                        label->setToolTip(tr(toolTip.c_str()));
                    }

                    gridLayout->addWidget(label, rows, cols++, 1, 1);
                    gridLayout->addWidget(uiWidget, rows, cols++, 1, 1);
                }
                else
                {
                    gridLayout->addWidget(uiWidget, rows, cols, 1, 2);
                    cols += 2;
                }

                // Update rows and columns
                cols %= 4;

                if (cols == 0)
                {
                    ++rows;
                }
            }

            stackLayoutIndex = dynamicLayouts->addWidget(dynamicWidget);
            dynamicLayoutMap[key] = stackLayoutIndex;
        }
        else
        {
            stackLayoutIndex = resultIter->second;
        }
    }

    emit currentBackendChanged(stackLayoutIndex);
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateColorButton
//
// Purpose:
//   Creates a push button used to edit a color-valued ANARI parameter. The
//   button's swatch/label reflect the current color; clicking it opens a
//   QColorDialog and, if a new color is picked, updates the swatch and the
//   ANARI rendering attributes.
//
// Arguments:
//   parent parent widget
//   type ANARI data type
//   defaultValueText the default value as a string
//
// Programmer:  Kevin Griffin
// Creation:    Fri Aug 28 03:26:30 PM CDT 2026
//
// ****************************************************************************
QPushButton* 
AnariRenderingWidget::CreateColorButton(QWidget* parent, ANARIDataType type, const std::string &defaultValueText)
{
    auto* button = new QPushButton(parent);
    QColor initial = TextToColor(type, defaultValueText);

    SetColorButtonSwatch(button, initial);
    button->setProperty("color", initial);
    button->setProperty("anariType", static_cast<int>(type));

    connect(button, &QPushButton::clicked, this, [this, button, parent]() {
        QColor c = button->property("color").value<QColor>();
        QColor picked = QColorDialog::getColor(c, parent,
                                               tr("Pick parameter color"),
                                               QColorDialog::ShowAlphaChannel);
        if (picked.isValid()) 
        {
            button->setProperty("color", picked);
            this->SetColorButtonSwatch(button, picked);
            this->UpdateRenderingAttributes(false);
        }
    });

    return button;
}

// ****************************************************************************
// Method: AnariRenderingWidget::SetColorButtonSwatch
//
// Purpose:
//   Updates a color button's label and style sheet to display the given
//   color as a swatch, with the button's text color (black/white) chosen
//   for contrast against the swatch.
//
// Arguments:
//   button the color button to update
//   color  the color to display
//
// Programmer:  Kevin Griffin
// Creation:    Fri Aug 28 03:26:30 PM CDT 2026
//
// ****************************************************************************
void 
AnariRenderingWidget::SetColorButtonSwatch(QPushButton* button, const QColor& color)
{
    button->setText(color.name(QColor::HexArgb));
    QString style = QStringLiteral("QPushButton { background-color: %1; color: %2; }").arg(
        color.name(),color.lightness() > 128 ? QStringLiteral("black") : QStringLiteral("white"));
    button->setStyleSheet(style);
}

// ****************************************************************************
// Method: AnariRenderingWidget::TextToColor
//
// Purpose:
//   Parses the space-separated component string the engine reports for a
//   color-valued ANARI parameter's default value (see
//   NetworkManager::AnariValueToString) into a QColor, normalizing each
//   component according to type. Returns black if defaultValueText is
//   empty, type isn't a recognized color type, or the component count
//   doesn't match what type expects.
//
// Arguments:
//   type ANARI data type
//   defaultValueText the default value as a string
//
// Programmer:  Kevin Griffin
// Creation:    Fri Aug 28 03:26:30 PM CDT 2026
//
// ****************************************************************************
QColor
AnariRenderingWidget::TextToColor(ANARIDataType type, const std::string &defaultValueText)
{
    if (defaultValueText.empty())
    {
        return QColor(Qt::black);
    }

    // Component count and normalization divisor for this ANARI color type.
    // The engine formats these as space-separated components (see
    // NetworkManager::AnariValueToString): FLOAT32 components are already
    // in [0,1]; UFIXED8/16/32 (including the *_SRGB variants) are their raw
    // unsigned integer storage value, normalized here by the type's max.
    int numComponents = 0;
    double maxValue = 1.0;

    switch(type)
    {
        case ANARI_FLOAT32:
            numComponents = 1;
            break;
        case ANARI_FLOAT32_VEC2:
            numComponents = 2;
            break;
        case ANARI_FLOAT32_VEC3:
            numComponents = 3;
            break;
        case ANARI_FLOAT32_VEC4:
            numComponents = 4;
            break;
        case ANARI_UFIXED8: case ANARI_UFIXED8_R_SRGB:
            numComponents = 1; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC2: case ANARI_UFIXED8_RA_SRGB:
            numComponents = 2; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC3: case ANARI_UFIXED8_RGB_SRGB:
            numComponents = 3; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC4: case ANARI_UFIXED8_RGBA_SRGB:
            numComponents = 4; maxValue = 255.0;
            break;
        case ANARI_UFIXED16:
            numComponents = 1; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC2:
            numComponents = 2; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC3:
            numComponents = 3; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC4:
            numComponents = 4; maxValue = 65535.0;
            break;
        case ANARI_UFIXED32:
            numComponents = 1; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC2:
            numComponents = 2; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC3:
            numComponents = 3; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC4:
            numComponents = 4; maxValue = 4294967295.0;
            break;
        default:
            return QColor(Qt::black);
    }

    QStringList tokens = QString::fromStdString(defaultValueText).split(' ', Qt::SkipEmptyParts);

    if(tokens.size() != numComponents)
    {
        debug1 << "[ANARI] TextToColor - expected " << numComponents
               << " components but got " << tokens.size()
               << " for value '" << defaultValueText << "'" << std::endl;
        return QColor(Qt::black);
    }

    // components[0..2] default to black, components[3] (alpha) defaults to
    // opaque, for the 1- and 2-component cases filled in below.
    double components[4] = {0.0, 0.0, 0.0, 1.0};
    bool ok = true;

    for(int i = 0; i < numComponents && ok; ++i)
    {
        components[i] = qBound(0.0, tokens[i].toDouble(&ok) / maxValue, 1.0);
    }

    if(!ok)
    {
        return QColor(Qt::black);
    }

    QColor color;

    switch(numComponents)
    {
        // Single channel (e.g. UFIXED8_R_SRGB): grayscale, opaque.
        case 1:
            color.setRgbF(components[0], components[0], components[0], 1.0);
            break;
        // Two channels (e.g. UFIXED8_RA_SRGB): grayscale + alpha.
        case 2:
            color.setRgbF(components[0], components[0], components[0], components[1]);
            break;
        case 3:
            color.setRgbF(components[0], components[1], components[2], 1.0);
            break;
        default:
            color.setRgbF(components[0], components[1], components[2], components[3]);
            break;
    }

    return color;
}

// ****************************************************************************
// Method: AnariRenderingWidget::ColorToText
//
// Purpose:
//   Inverse of TextToColor: formats a QColor as the space-separated
//   component string expected by the engine for the given ANARI color
//   type (see NetworkManager::AnariValueToString).
//
// Arguments:
//   type  ANARI data type
//   color the color to format
//
// Programmer:  Kevin Griffin
// Creation:    Mon 31 Aug 2026
//
// ****************************************************************************

std::string
AnariRenderingWidget::ColorToText(ANARIDataType type, const QColor &color)
{
    int numComponents = 0;
    double maxValue = 1.0;

    switch(type)
    {
        case ANARI_FLOAT32:
            numComponents = 1;
            break;
        case ANARI_FLOAT32_VEC2:
            numComponents = 2;
            break;
        case ANARI_FLOAT32_VEC3:
            numComponents = 3;
            break;
        case ANARI_FLOAT32_VEC4:
            numComponents = 4;
            break;
        case ANARI_UFIXED8: case ANARI_UFIXED8_R_SRGB:
            numComponents = 1; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC2: case ANARI_UFIXED8_RA_SRGB:
            numComponents = 2; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC3: case ANARI_UFIXED8_RGB_SRGB:
            numComponents = 3; maxValue = 255.0;
            break;
        case ANARI_UFIXED8_VEC4: case ANARI_UFIXED8_RGBA_SRGB:
            numComponents = 4; maxValue = 255.0;
            break;
        case ANARI_UFIXED16:
            numComponents = 1; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC2:
            numComponents = 2; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC3:
            numComponents = 3; maxValue = 65535.0;
            break;
        case ANARI_UFIXED16_VEC4:
            numComponents = 4; maxValue = 65535.0;
            break;
        case ANARI_UFIXED32:
            numComponents = 1; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC2:
            numComponents = 2; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC3:
            numComponents = 3; maxValue = 4294967295.0;
            break;
        case ANARI_UFIXED32_VEC4:
            numComponents = 4; maxValue = 4294967295.0;
            break;
        default:
            return std::string();
    }

    // components[0] is gray for the 1- and 2-component cases (mirrors
    // TextToColor, which reads r==g==b into a single gray channel).
    double components[4] = {0.0, 0.0, 0.0, 0.0};

    switch(numComponents)
    {
        case 1:
            components[0] = color.redF();
            break;
        case 2:
            components[0] = color.redF();
            components[1] = color.alphaF();
            break;
        case 3:
            components[0] = color.redF();
            components[1] = color.greenF();
            components[2] = color.blueF();
            break;
        default:
            components[0] = color.redF();
            components[1] = color.greenF();
            components[2] = color.blueF();
            components[3] = color.alphaF();
            break;
    }

    QStringList tokens;

    for(int i = 0; i < numComponents; ++i)
    {
        double val = components[i] * maxValue;

        if(maxValue == 1.0)
            tokens << QString::number(val);
        else
            tokens << QString::number(qRound(val));
    }

    return tokens.join(' ').toStdString();
}

// ****************************************************************************
// Method: AnariRenderingWidget::GetBackendType
//
// Purpose:
//   Gets the back-end type represented by libname.
//
// Arguments:
//   libname the name of the back-end to load
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

BackendType
AnariRenderingWidget::GetBackendType(const std::string &libname) const
{
    if(libname == "helide")
    {
        return BackendType::EXAMPLE;
    }
    else if(libname == "usd")
    {
        return BackendType::USD;
    }
    else if(libname == "visrtx")
    {
        return BackendType::VISRTX;
    }
    else if(libname == "visgl")
    {
        return BackendType::VISGL;
    }
    else if(libname == "ospray")
    {
        return BackendType::OSPRAY;
    }
    else if(libname == "rpr")
    {
        return BackendType::RADEONPRORENDER;
    }
    else if(libname == "phenocryst")
    {
        return BackendType::PHENOCRYST;
    }

    return BackendType::NONE;
}

// ****************************************************************************
// Method: AnariRenderingWidget::BuildParameterInfoFromMapNode
//
// Purpose:
//   Builds an AnariParameterInfo from the MapNode the engine reported for
//   one ANARI renderer parameter (see NetworkManager::GetAnariDeviceInfo).
//   This replaces the old device-based GetParameterInfo(), which called
//   anariGetParameterInfo() on a locally-created anari::Device -- the client
//   may not have any ANARI backend libraries installed at all, so all of
//   that introspection now happens on the engine instead.
//
// Arguments:
//   name       The parameter name.
//   paramNode  The MapNode describing this parameter (type/description/
//              default/minimum/maximum/acceptedValues).
//
// Programmer: Kevin Griffin
// Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

AnariParameterInfo
AnariRenderingWidget::BuildParameterInfoFromMapNode(const std::string &name,
                                                    const MapNode &paramNode)
{
    AnariParameterInfo paramInfo;
    paramInfo.SetName(name.c_str());

    if(paramNode.HasEntry("type"))
        paramInfo.SetType((ANARIDataType)paramNode.GetEntry("type")->AsInt());

    if(paramNode.HasEntry("description"))
    {
        std::string desc = paramNode.GetEntry("description")->AsString();
        paramInfo.SetDescription(desc.c_str());
    }

    if(paramNode.HasEntry("default"))
        paramInfo.SetDefaultValueText(paramNode.GetEntry("default")->AsString());

    if(paramNode.HasEntry("minimum"))
        paramInfo.SetMinimumText(paramNode.GetEntry("minimum")->AsString());

    if(paramNode.HasEntry("maximum"))
        paramInfo.SetMaximumText(paramNode.GetEntry("maximum")->AsString());

    if(paramNode.HasEntry("acceptedValues"))
        paramInfo.SetAcceptedValues(paramNode.GetEntry("acceptedValues")->AsStringVector());

    return paramInfo;
}

// External Updates
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateAnariAttributes
//
// Purpose:
//   Updates the ANARI rendering widget with the given attributes.
//
// Arguments:
//   attrs the AnariAttributes object containing the new settings
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateAnariAttributes(const AnariAttributes &attrs)
{
    SetChecked(attrs.GetAnariRendering());
    UpdateLibraryName(attrs.GetAnariLibrary());
    UpdateLibrarySubtypes(attrs.GetAnariLibrarySubtype());
    UpdateRendererSubtypes(attrs.GetAnariRendererSubtype());
    UpdateRendererParameters(attrs.GetAnariRendererParameters());
    UpdateUSDParameters(attrs.GetAnariUSDParameters());
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibrarySubtypes
//
// Purpose:
//   Adds subtype to the library subtypes combo box. If subtype is already in
//   the list, it will be ignored.
//
// Arguments:
//   subtype the library subtype to add to the combo box
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibrarySubtypes(const std::string &subtype)
{
    librarySubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index =  librarySubtypes->findText(textItem);

    if(index == -1)
    {
        librarySubtypes->addItem(textItem);
    }

    librarySubtypes->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibraryName
//
// Purpose:
//   Updates the available ANARI back-end.
//
// Arguments:
//   libname the name of the ANARI back-end
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibraryName(const std::string &libname)
{
    libraryName->blockSignals(true);

    QString textItem = QString::fromStdString(libname);
    if(libraryName->findText(textItem) == -1)
    {
        libraryName->addItem(textItem);
    }
    libraryName->setCurrentText(textItem);

    libraryName->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRendererSubtypes
//
// Purpose:
//   Updates the list of available renderers. If subtype is already in the list
//   it will not be added again.
//
// Arguments:
//   subtype the renderer subtype to add
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRendererSubtypes(const std::string &subtype)
{
    rendererSubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index =  rendererSubtypes->findText(textItem);

    if(index == -1)
    {
        rendererSubtypes->addItem(textItem);
    }

    rendererSubtypes->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRendererParameters
//
// Purpose:
//   Updates the renderer UI elements.
//
// Arguments:
//   params the list of parameters to update
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRendererParameters(const stringVector &params)
{
    // Loop through all widgets in the current layout
    for(int i = 0; i < dynamicLayouts->count(); ++i)
    {
        if(i == this->dynamicLayoutMap[AnariRenderingWidget::DEFAULT_WIDGET_KEY] ||
           i == this->dynamicLayoutMap[AnariRenderingWidget::USD_WIDGET_KEY])
        {
            // No renderer parameters to update
            continue;
        }

        auto widget = dynamicLayouts->widget(i);
        auto children = widget->findChildren<QWidget *>();

        for (const auto& param : params)
        {
            size_t sep = param.find(";");
            if(sep == std::string::npos)
                continue;
            std::string key = param.substr(0, sep);
            std::string value = param.substr(sep + 1);

            for(auto child : children)
            {
                std::string name = child->objectName().toStdString();

                if(name.empty())
                {
                    continue;
                }

                if(name == key)
                {
                    if(qobject_cast<QSpinBox *>(child) != nullptr)
                    {
                        auto spinBox = qobject_cast<QSpinBox *>(child);
                        spinBox->blockSignals(true);

                        try
                        {
                            auto val = std::stoi(value);
                            spinBox->setValue(val);
                        }
                        catch(...)
                        {
                            debug5 << "[ANARI] UpdateRendererParameters - Could not convert value to int: " << value;
                        }

                        spinBox->blockSignals(false);
                    }
                    else if(qobject_cast<QLineEdit *>(child) != nullptr)
                    {
                        auto lineEdit = qobject_cast<QLineEdit *>(child);
                        lineEdit->blockSignals(true);
                        lineEdit->setText(QString::fromStdString(value));
                        lineEdit->blockSignals(false);
                    }
                    else if(qobject_cast<QCheckBox *>(child) != nullptr)
                    {
                        auto checkBox = qobject_cast<QCheckBox *>(child);
                        checkBox->blockSignals(true);
                        checkBox->setChecked(value == "1");
                        checkBox->blockSignals(false);
                    }
                    else if(qobject_cast<QComboBox *>(child) != nullptr)
                    {
                        auto comboBox = qobject_cast<QComboBox *>(child);
                        comboBox->blockSignals(true);
                        comboBox->setCurrentText(QString::fromStdString(value));
                        comboBox->blockSignals(false);
                    }

                    break;
                }
            }
        }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateUSDParameters
//
// Purpose:
//   Updates the USD UI elements.
//
// Arguments:
//   params the list of parameters to update
//
// Programmer: Kevin Griffin
// Creation: Mon Oct 6 10:20:01 CST 2025
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateUSDParameters(const stringVector &params)
{
    const int usdWidgetIndex = this->dynamicLayoutMap[AnariRenderingWidget::USD_WIDGET_KEY];
    auto widget = dynamicLayouts->widget(usdWidgetIndex);
    auto children = widget->findChildren<QWidget *>();

    for (const auto& param : params)
    {
        size_t sep = param.find(";");
        if(sep == std::string::npos)
            continue;
        std::string key = param.substr(0, sep);
        std::string value = param.substr(sep + 1);

        for(auto child : children)
        {
            std::string name = child->objectName().toStdString();

            if(name.empty())
            {
                continue;
            }

            if(name == key)
            {
                if(qobject_cast<QLineEdit *>(child) != nullptr)
                {
                    auto lineEdit = qobject_cast<QLineEdit *>(child);
                    lineEdit->blockSignals(true);
                    lineEdit->setText(QString::fromStdString(value));
                    lineEdit->blockSignals(false);
                }
                else if(qobject_cast<QCheckBox *>(child) != nullptr)
                {
                    auto checkBox = qobject_cast<QCheckBox *>(child);
                    checkBox->blockSignals(true);
                    checkBox->setChecked(value == "1");
                    checkBox->blockSignals(false);
                }

                break;
            }
        }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::SetChecked
//
// Purpose:
//   Sets the check state of the ANARI rendering group box.
//
// Arguments:
//   val    If true, surface rendering will be done by an ANARI back-end
//          renderer, otherwise, the default rendering is used.
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::SetChecked(const bool val)
{
    renderingGroup->setChecked(val);
}

// SLOTS
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::renderingToggled
//
// Purpose:
//      Triggered when ANARI rendering is toggled.
//
// Arguments:
//      val when true use ANARI for rendering
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Mon 31 Aug 2026
//   The first time ANARI rendering is enabled, ask the engine which
//   libraries are available (populating the libraryName combo box) instead
//   of assuming a library is already selected.
//
// ****************************************************************************

void
AnariRenderingWidget::renderingToggled(bool val)
{
    anariAttributes->SetAnariRendering(val);
    renderingWindow->SetUpdateApply(false);

    if(val)
    {
        // Allows VisIt to check if a valid back-end is available before trying
        // to render with ANARI.
        if(libraryName->count() == 0)
        {
            RequestDeviceInfo("", "", "");
        }
        else
        {
            this->libraryChanged();
        }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::libraryChanged
//
// Purpose:
//      Triggered when the selected ANARI Back-end rendering library has
//      changed.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Thu 27 Aug 2026
//   Ask the engine which ANARI libraries/subtypes/renderers/parameters are
//   available instead of loading a library locally since the client may not
//   have any ANARI backend libraries installed at all. The result arrives
//   asynchronously via UpdateDeviceInfo().
//
//   Kevin Griffin, Mon 31 Aug 2026
//   libraryName is now a (non-editable) combo box populated from the
//   engine's reported library list, with the initial selection resolved
//   from the ANARI_LIBRARY environment variable in UpdateDeviceInfo(). If
//   it's not yet populated, re-request the library list instead of
//   defaulting to the "environment" sentinel.
//
// ****************************************************************************

void
AnariRenderingWidget::libraryChanged()
{
    std::string libname = libraryName->currentText().trimmed().toStdString();

    if(libname.empty())
    {
        RequestDeviceInfo("", "", "");
        return;
    }

    anariAttributes->SetUsingUsdDevice(GetBackendType(libname) == BackendType::USD);
    anariAttributes->SetAnariLibrary(libname);

    RequestDeviceInfo(libname, "", "");
}

// ****************************************************************************
// Method: AnariRenderingWidget::librarySubtypeChanged
//
// Purpose:
//      Triggered when ANARI Library subtype has changed.
//
// Arguments:
//      subtype the new library subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Thu 27 Aug 2026
//   Ask the engine for the available renderer subtypes instead of creating
//   a local ANARI device.
//
// ****************************************************************************

void
AnariRenderingWidget::librarySubtypeChanged(const QString &subtype)
{
    auto libSubtype = subtype.toStdString();
    anariAttributes->SetAnariLibrarySubtype(libSubtype);

    auto libname = libraryName->currentText().trimmed().toStdString();

    RequestDeviceInfo(libname, libSubtype, "");
}

// ****************************************************************************
// Method: AnariRenderingWidget::rendererSubtypeChanged
//
// Purpose:
//      Triggered when ANARI renderer subtype has changed.
//
// Arguments:
//      subtype the new renderer subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Thu 27 Aug 2026
//   Ask the engine for the renderer's parameters instead of creating a
//   local ANARI device.
//
// ****************************************************************************

void
AnariRenderingWidget::rendererSubtypeChanged(const QString &subtype)
{
    auto rendererSubtype = subtype.toStdString();
    anariAttributes->SetAnariRendererSubtype(rendererSubtype);

    auto libname = libraryName->currentText().trimmed().toStdString();
    auto libSubtype = librarySubtypes->currentText().toStdString();

    RequestDeviceInfo(libname, libSubtype, rendererSubtype);
}

// ****************************************************************************
// Method: AnariRenderingWidget::RequestDeviceInfo
//
// Purpose:
//   Asks the engine (via the viewer) for whichever ANARI info can be
//   determined from the given library/subtype/renderer combination. Any of
//   the three may be empty; see NetworkManager::GetAnariDeviceInfo() for
//   what each combination returns. The result arrives asynchronously via
//   UpdateDeviceInfo().
//
// Programmer: Kevin Griffin
// Creation:   Thu 27 Aug 2026
//
// ****************************************************************************

void
AnariRenderingWidget::RequestDeviceInfo(const std::string &libraryName_,
                                        const std::string &librarySubtype,
                                        const std::string &rendererSubtype)
{
    renderingWindow->RequestAnariDeviceInfo(libraryName_, librarySubtype, rendererSubtype);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateDeviceInfo
//
// Purpose:
//   Consumes the ANARI library/subtype/renderer/parameter info the engine
//   reported in response to RequestDeviceInfo(), and continues the
//   library -> subtype -> renderer -> parameters cascade by issuing the
//   next request as each level of the UI is populated.
//
// Arguments:
//   info : The MapNode reported by the engine (see
//          NetworkManager::GetAnariDeviceInfo()).
//
// Programmer: Kevin Griffin
// Creation:   Thu 27 Aug 2026
//
// Modifications:
//   Kevin Griffin, Mon 31 Aug 2026
//   Handle the "libraries" entry the engine returns for an empty
//   libraryName request: populate the (non-editable) libraryName combo box
//   and select the first library the engine reported, then continue the cascade.
//   If ANARI_LIBRARY environment variable is specified, the engine will place it
//   first in the list (assuming it is valid).
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateDeviceInfo(const MapNode &info)
{
    if(info.HasEntry("available") && info.GetEntry("available")->AsInt() == 0)
    {
        auto libname = libraryName->currentText().trimmed().toStdString();
        debug1 << "[ANARI] Engine could not load ANARI library/device for '"
               << libname << "'." << std::endl;
        emit currentBackendChanged(0);
        ClearAnariParameterAttributes();
        renderingWindow->SetUpdateApply(false);
        return;
    }

    if(info.HasEntry("libraries"))
    {
        stringVector libraries = info.GetEntry("libraries")->AsStringVector();

        libraryName->blockSignals(true);
        libraryName->clear();

        for(const std::string &l : libraries)
        {
            libraryName->addItem(QString::fromStdString(l));
        }

        std::string selected;

        if(libraryName->count() > 0)
        {
            selected = libraryName->itemText(0).toStdString();
        }

        if(!selected.empty())
        {
            libraryName->setCurrentText(QString::fromStdString(selected));
        }

        libraryName->blockSignals(false);

        if(selected.empty())
        {
            debug1 << "[ANARI] Engine reported no available ANARI libraries." << std::endl;
            return;
        }

        anariAttributes->SetUsingUsdDevice(GetBackendType(selected) == BackendType::USD);
        anariAttributes->SetAnariLibrary(selected);

        RequestDeviceInfo(selected, "", "");
        return;
    }

    auto libname = libraryName->currentText().trimmed().toStdString();
    auto backendType = GetBackendType(libname);

    if(info.HasEntry("parameters"))
    {
        auto libSubtype = librarySubtypes->currentText().toStdString();
        auto rendererSubtype = rendererSubtypes->currentText().toStdString();
        std::string key = libname + ":" + libSubtype + ":" + rendererSubtype;

        CreateDynamicWidget(*info.GetEntry("parameters"), key, backendType == BackendType::USD);
        UpdateRenderingAttributes(false);
    }
    else if(info.HasEntry("renderers"))
    {
        stringVector renderers = info.GetEntry("renderers")->AsStringVector();

        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();

        for(const std::string &r : renderers)
        {
            rendererSubtypes->addItem(QString::fromStdString(r));
        }

        if(rendererSubtypes->count() == 0)
        {
            rendererSubtypes->addItem("default");
        }

        rendererSubtypes->blockSignals(false);

        auto libSubtype = librarySubtypes->currentText().toStdString();
        auto rendererSubtype = rendererSubtypes->currentText().toStdString();
        anariAttributes->SetAnariRendererSubtype(rendererSubtype);

        RequestDeviceInfo(libname, libSubtype, rendererSubtype);
    }
    else if(info.HasEntry("subtypes"))
    {
        stringVector subtypes = info.GetEntry("subtypes")->AsStringVector();

        librarySubtypes->blockSignals(true);
        librarySubtypes->clear();

        for(const std::string &s : subtypes)
        {
            librarySubtypes->addItem(QString::fromStdString(s));
        }

        if(librarySubtypes->count() == 0)
        {
            librarySubtypes->addItem("default");
        }

        librarySubtypes->blockSignals(false);

        auto libSubtype = librarySubtypes->currentText().toStdString();
        anariAttributes->SetAnariLibrarySubtype(libSubtype);

        RequestDeviceInfo(libname, libSubtype, "");
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::selectButtonPressed
//
// Purpose:
//      Triggered when the USD output directory select button is pressed.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::selectButtonPressed()
{
    auto dir = QFileDialog::getExistingDirectory(this,
                                                 tr("Open Directory"),
                                                 this->currentDirectory,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        QDir directory(dir);
        this->currentDirectory = directory.absolutePath();
        this->dirLineEdit->setText(dir);
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::spinBoxValueChanged
//
// Purpose:
//      Triggered when a spin box value has changed. Mainly user for the
//      dynamic UI components.
//
// Arguments:
//      value   The new value of the spin box
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void AnariRenderingWidget::spinBoxValueChanged(int value)
{
    UpdateRenderingAttributes(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::lineEditingFinished
//
// Purpose:
//      Triggered when a line edit value has changed. Mainly used for the
//      dynamic UI components.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::lineEditingFinished()
{
    UpdateRenderingAttributes(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::comboBoxTextChanged
//
// Purpose:
//      Triggered when a combo box value has changed.
//
// Arguments:
//      text    The new value of the combo box
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::comboBoxTextChanged(const QString &text)
{
    UpdateRenderingAttributes(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::checkBoxToggled
//
// Purpose:
//      Triggered when a check box is selected or deselected.
//
// Arguments:
//      checked     If true, the check box is selected
//
// Programmer:      Kevin Griffin
// Creation:        Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::checkBoxToggled(bool checked)
{
    UpdateRenderingAttributes(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRenderingAttributes
//
// Purpose:
//      Updates the rendering attributes with the current values from the UI.
//
// Arguments:
//      updateApply If true, the rendering window will be updated
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRenderingAttributes(const bool updateApply)
{
    auto widget = dynamicLayouts->currentWidget();
    if(!widget)
    {
        debug1 << "No current widget found in dynamicLayouts" << std::endl;
        return;
    }
    auto children = widget->findChildren<QWidget *>();
    stringVector params;

    for(auto child : children)
    {
        std::string name = child->objectName().toStdString();

        // Check if name begins with "qt_", which indicates it's a child widget created by 
        // Qt (e.g., the line edits in a combo box) rather than a parameter widget. If so, skip it.
        if(name.empty() || name.rfind("qt_", 0) == 0)
        {
            continue;
        }

        if(qobject_cast<QSpinBox *>(child) != nullptr)
        {
            auto spinBox = qobject_cast<QSpinBox *>(child);
            auto val = spinBox->value();
            std::string valStr = name + ";" + std::to_string(val);
            params.push_back(valStr);
        }
        else if(qobject_cast<QLineEdit *>(child) != nullptr)
        {
            auto lineEdit = qobject_cast<QLineEdit *>(child);
            auto val = lineEdit->text().toStdString();
            std::string valStr = name + ";" + val;
            params.push_back(valStr);
        }
        else if(qobject_cast<QCheckBox *>(child) != nullptr)
        {
            auto checkBox = qobject_cast<QCheckBox *>(child);
            auto val = checkBox->isChecked() ? "1" : "0";
            std::string valStr = name + ";" + val;
            params.push_back(valStr);
        }
        else if(qobject_cast<QComboBox *>(child) != nullptr)
        {
            auto comboBox = qobject_cast<QComboBox *>(child);
            auto val = comboBox->currentText().toStdString();
            std::string valStr = name + ";" + val;
            params.push_back(valStr);
        }
        else if(qobject_cast<QPushButton *>(child) != nullptr)
        {
            auto pushButton = qobject_cast<QPushButton *>(child);
            const QVariant color = pushButton->property("color");
            const QVariant anariType = pushButton->property("anariType");

            if(color.isValid() && anariType.isValid())
            {
                auto type = static_cast<ANARIDataType>(anariType.toInt());
                std::string colorText = ColorToText(type, color.value<QColor>());

                if(!colorText.empty())
                {
                    std::string valStr = name + ";" + colorText;
                    params.push_back(valStr);
                }
            }
        }
    }

    if(!anariAttributes->GetUsingUsdDevice())
    {
        anariAttributes->SetAnariRendererParameters(params);
    }
    else
    {
        anariAttributes->SetAnariUSDParameters(params);
    }

    renderingWindow->SetUpdateApply(updateApply);
}

// ****************************************************************************
// Method: AnariRenderingWidget::ClearAnariParameterAttributes
//
// Purpose:
//      Clears the ANARI Renderer and USD parameters.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void AnariRenderingWidget::ClearAnariParameterAttributes()
{
    stringVector params;
    anariAttributes->SetAnariRendererParameters(params);
    anariAttributes->SetAnariUSDParameters(params);
}
