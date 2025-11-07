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
#include <QMessageBox>

#include <algorithm>

const std::string AnariRenderingWidget::USD_WIDGET_KEY = "usd";
const std::string AnariRenderingWidget::DEFAULT_WIDGET_KEY = "default";

namespace anari_visit
{
    void StatusCallback(const void* userData, anari::Device device,
                        anari::Object source, anari::DataType sourceType, anari::StatusSeverity severity,
                        anari::StatusCode code, const char* message)
    {
        if (severity == ANARI_SEVERITY_FATAL_ERROR)
        {
            debug5 << "[ANARI::FATAL] " << message << std::endl;
        }
        else if (severity == ANARI_SEVERITY_ERROR)
        {
            debug5 << "[ANARI::ERROR] " << message << ", DataType: " << (int)sourceType << std::endl;
        }
        else if (severity == ANARI_SEVERITY_WARNING)
        {
            debug5 << "[ANARI::WARN] " << message << ", DataType: " << (int)sourceType << std::endl;
        }
        else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING)
        {
            debug5 << "[ANARI::PERF] " << message << std::endl;
        }
        else if (severity == ANARI_SEVERITY_INFO)
        {
            debug5 << "[ANARI::INFO] " << message << std::endl;
        }
        else if (severity == ANARI_SEVERITY_DEBUG)
        {
            debug5 << "[ANARI::DEBUG] " << message << std::endl;
        }
        else
        {
            debug5 << "[ANARI::STATUS] " << message << std::endl;
        }

        (void)userData;
        (void)device;
        (void)source;
        (void)code;
    }
}

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

    libraryName = new QLineEdit("", generalOptionsWidget);
    connect(libraryName, &QLineEdit::editingFinished,
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
    if(paramInfo.GetName() == "name" || paramInfo.GetName() == "background")
    {
        // Skip these parameters
        // name is used internally for USD and background is handled by VisIt
        return nullptr;
    }

    auto anariDataType = paramInfo.GetType();

    switch(anariDataType)
    {
        case ANARI_INT32:
        {
            QSpinBox *spinBox = new QSpinBox();
            spinBox->setObjectName(paramInfo.GetName().c_str());

            if(paramInfo.m_defaultValue)
            {
                auto intPtr = static_cast<const int *>(paramInfo.m_defaultValue);
                spinBox->setValue(*intPtr);
            }

            int min = std::numeric_limits<int>::min();
            int max = std::numeric_limits<int>::max();

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const int *>(paramInfo.m_minimum);
                min = *minPtr;
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const int *>(paramInfo.m_maximum);
                max = *maxPtr;
            }

            spinBox->setMinimum(min);
            spinBox->setMaximum(max);

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

            if(paramInfo.m_defaultValue)
            {
                auto floatPtr = static_cast<const float *>(paramInfo.m_defaultValue);
                lineEdit->setText(QString::number(*floatPtr));
            }

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const float *>(paramInfo.m_minimum);
                min = *minPtr;
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const float *>(paramInfo.m_maximum);
                max = *maxPtr;
            }

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

            if(paramInfo.m_defaultValue)
            {
                auto doublePtr = static_cast<const double *>(paramInfo.m_defaultValue);
                lineEdit->setText(QString::number(*doublePtr));
            }

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const double *>(paramInfo.m_minimum);
                min = *minPtr;
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const double *>(paramInfo.m_maximum);
                max = *maxPtr;
            }

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

            if(paramInfo.m_defaultValue)
            {
                lineEdit->setText(static_cast<const char *>(paramInfo.m_defaultValue));
            }

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

            if(paramInfo.m_defaultValue)
            {
                auto boolPtr = static_cast<const int32_t *>(paramInfo.m_defaultValue);
                checkBox->setChecked(boolPtr && *boolPtr);
            }

            connect(checkBox, &QCheckBox::toggled,
                    this, &AnariRenderingWidget::checkBoxToggled);

            return checkBox;
        }
        case ANARI_INT32_VEC3: case ANARI_FLOAT32_VEC3: case ANARI_FLOAT64_VEC3:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            if(paramInfo.m_defaultValue)
            {
                auto vecPtr = paramInfo.m_defaultValue;

                if(anariDataType == ANARI_INT32_VEC3)
                {
                    auto intVecPtr = static_cast<const int *>(vecPtr);
                    lineEdit->setText(QString::number(intVecPtr[0]) + " " +
                                      QString::number(intVecPtr[1]) + " " +
                                      QString::number(intVecPtr[2]));
                }
                else if(anariDataType == ANARI_FLOAT32_VEC3)
                {
                    auto floatVecPtr = static_cast<const float *>(vecPtr);
                    lineEdit->setText(QString::number(floatVecPtr[0]) + " " +
                                      QString::number(floatVecPtr[1]) + " " +
                                      QString::number(floatVecPtr[2]));
                }
                else if(anariDataType == ANARI_FLOAT64_VEC3)
                {
                    auto doubleVecPtr = static_cast<const double *>(vecPtr);
                    lineEdit->setText(QString::number(doubleVecPtr[0]) + " " +
                                      QString::number(doubleVecPtr[1]) + " " +
                                      QString::number(doubleVecPtr[2]));
                }
            }

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_INT32_VEC4: case ANARI_FLOAT32_VEC4: case ANARI_FLOAT64_VEC4:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            if(paramInfo.m_defaultValue)
            {
                auto vecPtr = paramInfo.m_defaultValue;

                if(anariDataType == ANARI_INT32_VEC4)
                {
                    auto intVecPtr = static_cast<const int *>(vecPtr);
                    lineEdit->setText(QString::number(intVecPtr[0]) + " " +
                                      QString::number(intVecPtr[1]) + " " +
                                      QString::number(intVecPtr[2]) + " " +
                                      QString::number(intVecPtr[3]));
                }
                else if(anariDataType == ANARI_FLOAT32_VEC4)
                {
                    auto floatVecPtr = static_cast<const float *>(vecPtr);
                    lineEdit->setText(QString::number(floatVecPtr[0]) + " " +
                                      QString::number(floatVecPtr[1]) + " " +
                                      QString::number(floatVecPtr[2]) + " " +
                                      QString::number(floatVecPtr[3]));
                }
                else if(anariDataType == ANARI_FLOAT64_VEC4)
                {
                    auto doubleVecPtr = static_cast<const double *>(vecPtr);
                    lineEdit->setText(QString::number(doubleVecPtr[0]) + " " +
                                      QString::number(doubleVecPtr[1]) + " " +
                                      QString::number(doubleVecPtr[2]) + " " +
                                      QString::number(doubleVecPtr[3]));
                }
            }

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        default:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

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
//   anariDevice  The ANARI device
//   subtype      The renderer subtype
//   key          The key to the dynamic layout map
//   isUSD        If the back-end is USD
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::CreateDynamicWidget(anari::Device anariDevice, const char *subtype, const std::string &key, bool isUSD)
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

            const ANARIParameter *parameterList =
                static_cast<const ANARIParameter*>(anariGetObjectInfo(anariDevice,
                                                                      ANARI_RENDERER,
                                                                      subtype,
                                                                      "parameter",
                                                                      ANARI_PARAMETER_LIST));

            for(const ANARIParameter *param = parameterList; param && param->name != nullptr; ++param)
            {
                AnariParameterInfo paramInfo = std::move(GetParameterInfo(anariDevice, ANARI_RENDERER, subtype, param));

                // Create the UI
                QWidget *uiWidget = MakeWidgetFromParameterInfo(paramInfo);

                if(uiWidget == nullptr)
                {
                    continue;
                }

                if(paramInfo.GetType() != ANARI_BOOL)
                {
                    QLabel *label = new QLabel(param->name);
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

    return BackendType::NONE;
}

// ****************************************************************************
// Method: AnariRenderingWidget::GetParameterInfo
//
// Purpose:
//   Gets the parameter information for the given object.
//
// Arguments:
//   device         The ANARI device
//   objectType     The type of object (Renderer, Camera, etc.)
//   objectSubtype  The subtype of the object
//   param          The parameter to get information for
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

AnariParameterInfo
AnariRenderingWidget::GetParameterInfo(anari::Device device,
                                       ANARIDataType objectType,
                                       const char *objectSubtype,
                                       const ANARIParameter *param)
{
    AnariParameterInfo paramInfo;

    paramInfo.SetName(param->name);
    paramInfo.SetType(param->type);

    // Explanation of the parameter, e.g., for a tooltip
    paramInfo.SetDescription(anariGetParameterInfo(device,
                                                   objectType,
                                                   objectSubtype,
                                                   param->name,
                                                   param->type,
                                                   "description",
                                                   ANARI_STRING));
    // set values will be clamped to this minimum
    paramInfo.m_minimum = anariGetParameterInfo(device,
                                                objectType,
                                                objectSubtype,
                                                param->name,
                                                param->type,
                                                "minimum",
                                                param->type);
    // set values will be clamped to this maximum
    paramInfo.m_maximum = anariGetParameterInfo(device,
                                                objectType,
                                                objectSubtype,
                                                param->name,
                                                param->type,
                                                "maximum",
                                                param->type);
    // default value, must be in minumum and maximum if present
    paramInfo.m_defaultValue = anariGetParameterInfo(device,
                                                     objectType,
                                                     objectSubtype,
                                                     param->name,
                                                     param->type,
                                                     "default",
                                                     param->type);
    // list of accepted values
    paramInfo.SetAcceptedValues((const char **)anariGetParameterInfo(device,
                                                                     objectType,
                                                                     objectSubtype,
                                                                     param->name,
                                                                     param->type,
                                                                     "value",
                                                                     ANARI_STRING_LIST));

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
AnariRenderingWidget::UpdateLibrarySubtypes(const std::string subtype)
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
AnariRenderingWidget::UpdateLibraryName(const std::string libname)
{
    libraryName->blockSignals(true);
    libraryName->setText(QString::fromStdString(libname));
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
AnariRenderingWidget::UpdateRendererSubtypes(const std::string subtype)
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
            std::string key = param.substr(0, param.find(";"));
            std::string value = param.substr(param.find(";") + 1);

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
        std::string key = param.substr(0, param.find(";"));
        std::string value = param.substr(param.find(";") + 1);

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
        this->libraryChanged();
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::libraryChanged
//
// Purpose:
//      Triggered when ANARI Back-end rendering library has changed. If no
//      library is specified, it will first look to see if the ANARI_LIBRARY 
//      environment variable is set. If not, the default library (helide) will 
//      be used.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::libraryChanged()
{
    anariAttributes->SetUsingUsdDevice(false);
    std::string libname = libraryName->text().trimmed().toStdString();
    
    if(libname.empty())
    {
        libname = "environment";
    }
    
    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);

    if(anariLibrary)
    {
        // Get ANARI_LIBRARY environment variable
        if(libname == "environment")
        {
            libname = std::string(getenv("ANARI_LIBRARY"));
        }
        
        libraryName->blockSignals(true);
        libraryName->setText(libname.c_str());
        libraryName->blockSignals(false);
        
        UpdateLibraryUI(anariLibrary, libname);
        anariUnloadLibrary(anariLibrary);
        UpdateRenderingAttributes(false);
    }
    else
    {
        QString message;

        if(libname == "environment")
        {
            message.append(tr("ANARI_LIBRARY environment variable not set or set incorrectly. ") +
                           tr("Using default back-end (helide)"));
        }
        else
        {
            message.append(tr("%1 is not a valid back-end name or not on your library path. ").arg(libname.c_str()) + 
                           tr("Using default back-end (helide)"));
        }

        QMessageBox::critical(this, tr("ANARI"), message);
        debug1 << "Could not load the ANARI library (" << libname << ") using default back-end (helide)." << std::endl;
        
        libname = "helide";        
        anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);
        
        if(anariLibrary)
        {
            libraryName->blockSignals(true);
            libraryName->setText(libname.c_str());
            libraryName->blockSignals(false);
        
            UpdateLibraryUI(anariLibrary, libname);
            anariUnloadLibrary(anariLibrary);
            UpdateRenderingAttributes(false);
        }
        else
        {
            QString message1 = tr("Could not load the default ANARI library (helide). ") +
                               tr("Disabling ANARI rendering.");
            QMessageBox::critical(this, tr("ANARI"), message1);
            debug1 << "Could not load the default ANARI library (helide)." << std::endl;        
        
            // Reset Back-end Subtype and Renderer to "default"
            librarySubtypes->blockSignals(true);
            librarySubtypes->clear();
            librarySubtypes->addItem("default");
            librarySubtypes->blockSignals(false);
            auto libSubtype =  librarySubtypes->currentText().toStdString();
            anariAttributes->SetAnariLibrarySubtype(libSubtype);

            rendererSubtypes->blockSignals(true);
            rendererSubtypes->clear();
            rendererSubtypes->addItem("default");
            rendererSubtypes->blockSignals(false);
            auto rendererSubtype = rendererSubtypes->currentText().toStdString();            
            anariAttributes->SetAnariRendererSubtype(rendererSubtype);

            // Reset to blank widget
            emit currentBackendChanged(0);
            ClearAnariParameterAttributes();
            renderingGroup->setChecked(false);
        }
    }
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
//
// ****************************************************************************

void
AnariRenderingWidget::librarySubtypeChanged(const QString &subtype)
{
    auto libSubtype = subtype.toStdString();
    anariAttributes->SetAnariLibrarySubtype(libSubtype);

    auto libname = libraryName->text().trimmed().toStdString();
    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);
    auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

    if(anariDevice)
    {
        // Update renderers
        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();
        const char **renderers = anariGetObjectSubtypes(anariDevice, ANARI_RENDERER);

        if(renderers)
        {
            for(const char **d = renderers; *d != NULL; d++)
            {
                rendererSubtypes->addItem(*d);
            }
        }
        else
        {
            rendererSubtypes->addItem("default");
        }

        auto rendererSubtype =  rendererSubtypes->currentText().toStdString();
        anariAttributes->SetAnariRendererSubtype(rendererSubtype);
        rendererSubtypes->blockSignals(false);

        // Create Dynamic Widget
        std::string key = libname + ":" + libSubtype + ":" + rendererSubtype;
        CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, GetBackendType(libname) == BackendType::USD);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        UpdateRenderingAttributes(false);
    }
    else
    {
        debug1 << "Could not create the ANARI back-end device (" << libname << ") to update the Rendering UI." << std::endl;
        emit currentBackendChanged(0);
        ClearAnariParameterAttributes();
        renderingWindow->SetUpdateApply(false);
    }
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
//
// ****************************************************************************

void
AnariRenderingWidget::rendererSubtypeChanged(const QString &subtype)
{
    auto rendererSubtype = subtype.toStdString();
    anariAttributes->SetAnariRendererSubtype(rendererSubtype);

    auto libname = libraryName->text().trimmed().toStdString();
    auto libSubtype = librarySubtypes->currentText().toStdString();

    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);
    auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

    if(anariDevice)
    {
        // Create Dynamic Widget
        auto key = libname + ":" + libSubtype + ":" + rendererSubtype;
        CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, GetBackendType(libname) == BackendType::USD);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        UpdateRenderingAttributes(false);
    }
    else
    {
        debug1 << "Could not create the ANARI back-end device (" << libname << ") to update the Rendering UI." << std::endl;
        emit currentBackendChanged(0);

        // Clear Parameters
        ClearAnariParameterAttributes();
        renderingWindow->SetUpdateApply(false);
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
    auto children = widget->findChildren<QWidget *>(Qt::FindDirectChildrenOnly);
    stringVector params;

    for(auto child : children)
    {
        std::string name = child->objectName().toStdString();

        if(name.empty())
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
        else
        {
            std::cerr << "Unknown widget type: " << child->metaObject()->className() << std::endl;
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

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibraryUI
//
// Purpose:
//      Updates the UI elements related to the ANARI rendering library.
// 
// Arguments:
//      anariLibrary the ANARI library
//      libname     the name of the ANARI library
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibraryUI(anari::Library anariLibrary, const std::string &libname)
{
    anariAttributes->SetAnariLibrary(libname);
    auto backendType = GetBackendType(libname);

    if(backendType == BackendType::USD)
    {
        anariAttributes->SetUsingUsdDevice(true);
    }
    else
    {
        anariAttributes->SetUsingUsdDevice(false);
    }

    // Update back-end subtypes
    librarySubtypes->blockSignals(true);
    librarySubtypes->clear();
    const char **devices = anariGetDeviceSubtypes(anariLibrary);

    if(devices)
    {
        for(const char **d = devices; *d != NULL; d++)
        {
            librarySubtypes->addItem(*d);
        }
    }
    else
    {
        librarySubtypes->addItem("default");
    }

    librarySubtypes->blockSignals(false);
    auto libSubtype =  librarySubtypes->currentText().toStdString();
    anariAttributes->SetAnariLibrarySubtype(libSubtype);

    auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

    // Update renderers
    rendererSubtypes->blockSignals(true);
    rendererSubtypes->clear();

    const char **renderers = anariGetObjectSubtypes(anariDevice, ANARI_RENDERER);

    if(renderers)
    {
        for(const char **d = renderers; *d != NULL; d++)
        {
            rendererSubtypes->addItem(*d);
        }
    }
    else
    {
        rendererSubtypes->addItem("default");
    }

    auto rendererSubtype = rendererSubtypes->currentText().toStdString();
    anariAttributes->SetAnariRendererSubtype(rendererSubtype);
    rendererSubtypes->blockSignals(false);

    // Create Dynamic Widget
    std::string key = libname + ":" + libSubtype + ":" + rendererSubtype;
    CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, backendType == BackendType::USD);

    // Clean-up
    anari::release(anariDevice, anariDevice);
}