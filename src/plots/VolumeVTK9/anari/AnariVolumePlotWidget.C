// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariVolumePlotWidget.h>
#include <QvisVolumePlotWindow.h>
#include <VolumeAttributes.h>
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

namespace anari_visit
{
    void StatusCallback(const void* userData, anari::Device device,
                        anari::Object source, anari::DataType sourceType, anari::StatusSeverity severity,
                        anari::StatusCode code, const char* message)
    {
        std::cerr << message << std::endl;
        // if (severity == ANARI_SEVERITY_FATAL_ERROR)
        // {
        //     std::cout << "[ANARI::FATAL] " << message;
        // }
        // else if (severity == ANARI_SEVERITY_ERROR)
        // {
        //     std::cout << "[ANARI::ERROR] " << %s, DataType: %d\n", message, (int)sourceType);
        // }
        // else if (severity == ANARI_SEVERITY_WARNING)
        // {
        //     std::cout(WARNING, "[ANARI::WARN] %s, DataType: %d\n", message, (int)sourceType);
        // }
        // else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING)
        // {
        //     std::cout(WARNING, "[ANARI::PERF] %s\n", message);
        // }
        // else if (severity == ANARI_SEVERITY_INFO)
        // {
        //     std::cout(INFO, "[ANARI::INFO] %s\n", message);
        // }
        // else if (severity == ANARI_SEVERITY_DEBUG)
        // {
        //     std::cout(TRACE, "[ANARI::DEBUG] %s\n", message);
        // }
        // else
        // {
        //     std::cout(INFO, "[ANARI::STATUS] %s\n", message);
        // }
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::AnariVolumePlotWidget
//
// Purpose:
//   Constructor for the AnariVolumePlotWidget class.
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

AnariVolumePlotWidget::AnariVolumePlotWidget(QvisVolumePlotWindow *qrw,
                                             VolumeAttributes *ra,
                                             QWidget *parent) :
    QWidget(parent),
    renderingWindow(qrw),
    volumeAttributes(ra),
    rendererParams(new std::vector<std::string>()),
    totalRows(0),
    renderingGroup(nullptr),
    libraryName(nullptr),
    librarySubtypes(nullptr),
    rendererSubtypes(nullptr),
    samplesPerPixel(nullptr),
    aoSamples(nullptr),
    lightFalloff(nullptr),
    ambientIntensity(nullptr),
    maxDepth(nullptr),
    rValue(nullptr),
    outputDir()
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
            this, &AnariVolumePlotWidget::renderingToggled);

    QVBoxLayout *renderingGroupVBoxLayout = new QVBoxLayout(renderingGroup);
    int rowCnt =  totalRows;

    renderingGroupVBoxLayout->addWidget(CreateGeneralWidget(rowCnt));
    totalRows += rowCnt;

    backendStackedLayout = new QStackedLayout();

    // Create and add the back-end specific widgets
    int rowCnt1 = 0;
    backendStackedLayout->addWidget(CreateBackendWidget(rowCnt1));
    // renderingGroupVBoxLayout->addWidget(CreateBackendWidget(rowCnt));

    // Create USD back-end specific widgets
    int rowCnt2 = 0;
    backendStackedLayout->addWidget(CreateUSDWidget(rowCnt2));

    totalRows += std::max(rowCnt1, rowCnt2);
    renderingGroupVBoxLayout->addLayout(backendStackedLayout);
    mainLayout->addWidget(renderingGroup);

    connect(this, &AnariVolumePlotWidget::currentBackendChanged,
            backendStackedLayout, &QStackedLayout::setCurrentIndex);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::CreateGeneralWidget
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
AnariVolumePlotWidget::CreateGeneralWidget(int &rows)
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
            this, &AnariVolumePlotWidget::libraryChanged);

    // Back-end and subtype
    QLabel *backendLabel = new QLabel(tr("Back-end"));
    backendLabel->setToolTip(tr("ANARI back-end device"));

    gridLayout->addWidget(backendLabel, rows, 0, 1, 1);
    gridLayout->addWidget(libraryName, rows, 1, 1, 2);

    // Back-end subtype
    librarySubtypes = new QComboBox();
    librarySubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(librarySubtypes, &QComboBox::currentTextChanged,
            this, &AnariVolumePlotWidget::librarySubtypeChanged);

    QLabel *subtypeLabel = new QLabel(tr("Back-end Subtype"));

    gridLayout->addWidget(subtypeLabel, rows, 3, 1, 1);
    gridLayout->addWidget(librarySubtypes, rows, 4, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 4, 1, 1);

    // Renderer
    rendererSubtypes = new QComboBox();
    rendererSubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(rendererSubtypes, &QComboBox::currentTextChanged,
            this, &AnariVolumePlotWidget::rendererSubtypeChanged);

    QLabel *rendererLabel = new QLabel(tr("Renderer"));
    rendererLabel->setToolTip(tr("Renderer subtype"));

    gridLayout->addWidget(rendererLabel, rows, 0, 1, 1);
    gridLayout->addWidget(rendererSubtypes, rows, 1, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 3, 1, 3);

    return generalOptionsWidget;
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::CreateBackendWidget
//
// Purpose:
//   Creates the UI components for selecting rendering options used by ANARI
//   back-ends (not including the USD back-end).
//
// Arguments:
//   rows keeps track of the total rows used to create this widget
//
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariVolumePlotWidget::CreateBackendWidget(int &rows)
{
    QWidget *widget = new QWidget(this);

    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    // Row 1
    // pixelSamples (ANARI_INT32) - all
    samplesPerPixel = new QSpinBox();
    samplesPerPixel->setMinimum(1);
    samplesPerPixel->setValue(volumeAttributes->GetAnariSPP());

    connect(samplesPerPixel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AnariVolumePlotWidget::samplesPerPixelChanged);

    QLabel *anariSPPLabel = new QLabel("SPP");
    anariSPPLabel->setToolTip(tr("Samples Per Pixel"));

    gridLayout->addWidget(anariSPPLabel, rows, 0, 1, 1);
    gridLayout->addWidget(samplesPerPixel, rows, 1, 1, 1);

    // ambientSamples ANARI_INT32 - scivis, ao
    aoSamples = new QSpinBox();
    aoSamples->setMinimum(0);
    aoSamples->setValue(volumeAttributes->GetAnariAO());

    connect(aoSamples, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AnariVolumePlotWidget::aoSamplesChanged);

    QLabel *aoLabel = new QLabel(tr("AO Samples"));
    aoLabel->setToolTip(tr("Ambient Occlusion Samples"));

    gridLayout->addWidget(aoLabel, rows, 2, 1, 1);
    gridLayout->addWidget(aoSamples, rows++, 3, 1, 1);

    // Row 2
    lightFalloff = new QLineEdit(QString::number(volumeAttributes->GetAnariLightFalloff()),
                                 widget);
    QDoubleValidator *dv0 = new QDoubleValidator();
    dv0->setDecimals(4);
    lightFalloff->setValidator(dv0);

    connect(lightFalloff, &QLineEdit::editingFinished,
            this, &AnariVolumePlotWidget::lightFalloffChanged);

    QLabel *lfoLabel = new QLabel(tr("Falloff"));
    lfoLabel->setToolTip(tr("Light Falloff"));

    gridLayout->addWidget(lfoLabel, rows, 0, 1, 1);
    gridLayout->addWidget(lightFalloff, rows, 1, 1, 1);

    // ambientIntensity ANARI_FLOAT32 - scivis
    ambientIntensity = new QLineEdit(QString::number( volumeAttributes->GetAnariAmbientIntensity()),
                                       widget);
    QDoubleValidator *dv1 = new QDoubleValidator(0.0, 1.0, 4);
    ambientIntensity->setValidator(dv1);

    connect(ambientIntensity, &QLineEdit::editingFinished,
            this, &AnariVolumePlotWidget::ambientIntensityChanged);

    QLabel *intensityLabel = new QLabel(tr("Ambient Intensity"));
    intensityLabel->setToolTip(tr("0.0 <= Ambient Light Intensity <= 1.0"));

    gridLayout->addWidget(intensityLabel, rows, 2, 1, 1);
    gridLayout->addWidget(ambientIntensity, rows++, 3, 1, 1);

    // Row 3
    // maxDepth ANRI_INT32 - dpt
    maxDepth = new QSpinBox();
    maxDepth->setMinimum(0);
    maxDepth->setValue(volumeAttributes->GetAnariMaxDepth());

    connect(maxDepth, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AnariVolumePlotWidget::maxDepthChanged);

    QLabel *maxDepthLabel = new QLabel(tr("Max Depth"));
    maxDepthLabel->setToolTip(tr("Max depth for tracing rays"));

    gridLayout->addWidget(maxDepthLabel, rows, 0, 1, 1);
    gridLayout->addWidget(maxDepth, rows, 1, 1, 1);

    // R  ANARI_FLOAT32 - dpt
    rValue = new QLineEdit(QString::number(volumeAttributes->GetAnariRValue()),
                            widget);
    QDoubleValidator *dv2 = new QDoubleValidator(0.0, 1.0, 4);
    rValue->setValidator(dv2);

    connect(rValue, &QLineEdit::editingFinished,
            this, &AnariVolumePlotWidget::rValueChanged);

    QLabel *rLabel = new QLabel(tr("R"));
    rLabel->setToolTip(tr("0.0 <= R <= 1.0"));

    gridLayout->addWidget(rLabel, rows, 2, 1, 1);
    gridLayout->setAlignment(rLabel, Qt::AlignRight);

    gridLayout->addWidget(rValue, rows++, 3, 1, 1);

    UpdateUI();
    return widget;
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::CreateUSDWidget
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
AnariVolumePlotWidget::CreateUSDWidget(int &rows)
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    gridLayout->setColumnStretch(1, 3);

    // row, col, rowspan, colspan
    // Row 1
    QLabel *locationLabel = new QLabel("Directory");
    locationLabel->setToolTip(tr("Output location for saving the USD files"));

    // Output location for the USD files
    // outputDir.reset(new QString(QDir::homePath()));
    outputDir.reset(new QString(volumeAttributes->GetUsdDir().c_str()));
    dirLineEdit = new QLineEdit(*outputDir);
    connect(dirLineEdit, &QLineEdit::editingFinished, this, &AnariVolumePlotWidget::outputLocationChanged);
    // outputLocationChanged();

    QPushButton *dirSelectButton = new QPushButton("Select");
    connect(dirSelectButton, &QPushButton::pressed, this, &AnariVolumePlotWidget::selectButtonPressed);

    commitCheckBox = new QCheckBox(tr("commit"));
    commitCheckBox->setToolTip(tr("Write USD at ANARI commit call"));
    commitCheckBox->setChecked(volumeAttributes->GetUsdAtCommit());

    connect(commitCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::commitToggled);

    gridLayout->addWidget(locationLabel, 0, 0, 1, 1);
    gridLayout->addWidget(dirLineEdit, 0, 1, 1, 2);
    gridLayout->addWidget(dirSelectButton, 0, 3, 1, 1);
    gridLayout->addWidget(commitCheckBox, 0, 4, 1, 1);

    mainLayout->addLayout(gridLayout);

    // Row 2
    rows++;
    QGroupBox *outputGroup = new QGroupBox(tr("Output"));

    QGridLayout *gridLayout2 = new QGridLayout(outputGroup);
    gridLayout2->setSpacing(10);
    gridLayout2->setContentsMargins(10,10,10,10);

    binaryCheckBox = new QCheckBox(tr("Binary"));
    binaryCheckBox->setToolTip(tr("Binary or text output"));
    binaryCheckBox->setChecked(volumeAttributes->GetUsdOutputBinary());

    connect(binaryCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::binaryToggled);
    gridLayout2->addWidget(binaryCheckBox, 0, 0, 1, 1);

    materialCheckBox = new QCheckBox(tr("Material"));
    materialCheckBox->setToolTip(tr("Include material objects in the output"));
    materialCheckBox->setChecked(volumeAttributes->GetUsdOutputMaterial());

    connect(materialCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::materialToggled);
    gridLayout2->addWidget(materialCheckBox, 0, 1, 1, 1);

    previewCheckBox = new QCheckBox(tr("Preview Surface"));
    previewCheckBox->setToolTip(tr("Include preview surface shader prims in the output for material objects"));
    previewCheckBox->setChecked(volumeAttributes->GetUsdOutputPreviewSurface());

    connect(previewCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::previewSurfaceToggled);
    gridLayout2->addWidget(previewCheckBox, 0, 2, 1, 1);

    // Row 3
    rows++;

    mdlCheckBox = new QCheckBox(tr("MDL"));
    mdlCheckBox->setToolTip(tr("Include MDL shader prims in the output for material objects"));
    mdlCheckBox->setChecked(volumeAttributes->GetUsdOutputMDL());

    connect(mdlCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::mdlToggled);
    gridLayout2->addWidget(mdlCheckBox, 1, 0, 1, 1);

    mdlColorCheckBox = new QCheckBox(tr("MDL Colors"));
    mdlColorCheckBox->setToolTip(tr("Include MDL colors in the output for material objects"));
    mdlColorCheckBox->setChecked(volumeAttributes->GetUsdOutputMDLColors());

    connect(mdlColorCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::mdlColorsToggled);
    gridLayout2->addWidget(mdlColorCheckBox, 1, 1, 1, 1);

    displayColorCheckBox = new QCheckBox(tr("Display Colors"));
    displayColorCheckBox->setToolTip(tr("Include display colors in the output"));
    displayColorCheckBox->setChecked(volumeAttributes->GetUsdOutputDisplayColors());

    connect(displayColorCheckBox, &QCheckBox::toggled, this, &AnariVolumePlotWidget::displayColorsToggled);
    gridLayout2->addWidget(displayColorCheckBox, 1, 2, 1, 1);

    rows++;
    mainLayout->addWidget(outputGroup);

    return widget;
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::GetBackendType
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
AnariVolumePlotWidget::GetBackendType(const std::string &libname) const
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
// Method: AnariVolumePlotWidget::UpdateUI
//
// Purpose:
//   Update the state of the UI components
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateUI()
{
    auto start = rendererParams->begin();
    auto stop = rendererParams->end();
    auto result = std::find(start, stop, "pixelSamples");

    if(samplesPerPixel != nullptr)
    {
        samplesPerPixel->setEnabled(result != stop);
    }

    if( aoSamples != nullptr)
    {
        result = std::find(start, stop, "aoSamples");
        auto result2 = std::find(start, stop, "ambientSamples");
        aoSamples->setEnabled((result != stop) || (result2 != stop));
    }

    if( lightFalloff != nullptr)
    {
        result = std::find(start, stop, "lightFalloff");
        lightFalloff->setEnabled(result != stop);
    }

    if( ambientIntensity != nullptr)
    {
        result = std::find(start, stop, "ambientRadiance");
        ambientIntensity->setEnabled(result != stop);
    }

    if( maxDepth != nullptr)
    {
        result = std::find(start, stop, "maxDepth");
        maxDepth->setEnabled(result != stop);
    }

    if( rValue != nullptr)
    {
        result = std::find(start, stop, "R");
        rValue->setEnabled(result != stop);
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateRendererParams
//
// Purpose:
//   Update the list of supported parameters for a specific renderer subtype.
//
// Arguments:
//   rendererSubtype the renderer subtype (e.g., scivis)
//   library the ANARI library (e.g., visrtx)
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateRendererParams(const std::string &rendererSubtype, anari::Device anariDevice)
{
    bool unloadLibrary = false;
    anari::Library anariLibrary = nullptr;

    if(anariDevice == nullptr)
    {
        auto libname =  libraryName->text().trimmed().toStdString().c_str();
        anariLibrary = anari::loadLibrary(libname, anari_visit::StatusCallback);
        unloadLibrary = true;

        auto libSubtype =  librarySubtypes->currentText().toStdString();
        anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());
    }

    if(anariDevice)
    {
        const anari::Parameter *params =
            static_cast<const anari::Parameter *>(anariGetObjectInfo(anariDevice,
                                                                     ANARI_RENDERER,
                                                                     rendererSubtype.c_str(),
                                                                     "parameter",
                                                                     ANARI_PARAMETER_LIST));
        // Clear old renderer parameters
        rendererParams.reset(new std::vector<std::string>());

        // Add new renderer parameters
        if(params)
        {
            for(auto p = params; p->name != NULL; p++)
            {
                std::string param(p->name);
                rendererParams->emplace_back(param);
            }
        }

        if(unloadLibrary)
        {
            anari::release(anariDevice, anariDevice);
            anariUnloadLibrary(anariLibrary);
        }
    }
    else
    {
        debug1 << "Could not create an ANARI device to update the renderer parameters list." << std::endl;
    }
}

// External Updates
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateLibrarySubtypes
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
AnariVolumePlotWidget::UpdateLibrarySubtypes(const std::string subtype)
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
// Method: AnariVolumePlotWidget::UpdateLibraryName
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
AnariVolumePlotWidget::UpdateLibraryName(const std::string libname)
{
    libraryName->blockSignals(true);
    libraryName->setText(QString::fromStdString(libname).trimmed());
    libraryName->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateRendererSubtypes
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
AnariVolumePlotWidget::UpdateRendererSubtypes(const std::string subtype)
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
// Method: AnariVolumePlotWidget::SetChecked
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
AnariVolumePlotWidget::SetChecked(const bool val)
{
    renderingGroup->setChecked(val);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateSamplesPerPixel
//
// Purpose:
//   Updates the samples per pixel that will be used by the currently selected
//   renderer.
//
// Arguments:
//   val the new samples per pixel value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateSamplesPerPixel(const int val)
{
    samplesPerPixel->blockSignals(true);
    samplesPerPixel->setValue(val);
    samplesPerPixel->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateAOSamples
//
// Purpose:
//   Updates the ambient occulsion value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new ambient occlusion value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateAOSamples(const int val)
{
    aoSamples->blockSignals(true);
    aoSamples->setValue(val);
    aoSamples->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateLightFalloff
//
// Purpose:
//   Update the light falloff value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new light falloff value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateLightFalloff(const float val)
{
    lightFalloff->blockSignals(true);
    lightFalloff->setText(QString::number(val));
    lightFalloff->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateAmbientIntensity
//
// Purpose:
//   Update the ambient intensity value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new ambient intensity value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateAmbientIntensity(const float val)
{
    ambientIntensity->blockSignals(true);
    ambientIntensity->setText(QString::number(val));
    ambientIntensity->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateMaxDepth
//
// Purpose:
//   Updates the max depth value that will be used by the currently
//   selected renderer.
//
// Arguments:
//   val the new max depth value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateMaxDepth(const int val)
{
    maxDepth->blockSignals(true);
    maxDepth->setValue(val);
    maxDepth->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateRValue
//
// Purpose:
//   Update the R value that will be used by the currently selected renderer.
//
// Arguments:
//   val the new R value
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateRValue(const float val)
{
    rValue->blockSignals(true);
    rValue->setText(QString::number(val));
    rValue->blockSignals(false);
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateUSDOutputLocation
//
// Purpose:
//   Updates the USD output location path.
//
// Arguments:
//   path output location for saving the USD files
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateUSDOutputLocation(const std::string path)
{
    QString directoryQStr = QString::fromStdString(path);

    if(!directoryQStr.isEmpty())
    {
        QDir directory(directoryQStr);

        if(directory.exists())
        {
            dirLineEdit->blockSignals(true);
            dirLineEdit->setText(directoryQStr);
            dirLineEdit->blockSignals(false);
        }
        else
        {
            debug5 << "AnariVolumePlotWidget::UpdateUSDOutputLocation: " << path << " does not exist" << std::endl;
        }
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::UpdateUSDParameter
//
// Purpose:
//   Sets the checked state of the USD Ooutput parameter check boxes.
//
// Arguments:
//   param  the USD output parameter to update
//   bool   if true then param is selected
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::UpdateUSDParameter(const USDParameter param, const bool val)
{
    switch(param)
    {
        case USDParameter::COMMIT:
            commitCheckBox->blockSignals(true);
            commitCheckBox->setChecked(val);
            commitCheckBox->blockSignals(false);
            break;
        case USDParameter::BINARY:
            binaryCheckBox->blockSignals(true);
            binaryCheckBox->setChecked(val);
            binaryCheckBox->blockSignals(false);
            break;
        case USDParameter::MATERIAL:
            materialCheckBox->blockSignals(true);
            materialCheckBox->setChecked(val);
            materialCheckBox->blockSignals(false);
            break;
        case USDParameter::PREVIEW:
            previewCheckBox->blockSignals(true);
            previewCheckBox->setChecked(val);
            previewCheckBox->blockSignals(false);
            break;
        case USDParameter::MDL:
            mdlCheckBox->blockSignals(true);
            mdlCheckBox->setChecked(val);
            mdlCheckBox->blockSignals(false);
            break;
        case USDParameter::MDLCOLORS:
            mdlColorCheckBox->blockSignals(true);
            mdlColorCheckBox->setChecked(val);
            mdlColorCheckBox->blockSignals(false);
            break;
        case USDParameter::DISPLAY:
            displayColorCheckBox->blockSignals(true);
            displayColorCheckBox->setChecked(val);
            displayColorCheckBox->blockSignals(false);
            break;
    }
}


// SLOTS
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariVolumePlotWidget::renderingToggled
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
AnariVolumePlotWidget::renderingToggled(bool val)
{
    volumeAttributes->SetAnariRendering(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::libraryChanged
//
// Purpose:
//      Triggered when ANARI Back-end rendering library has changed.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::libraryChanged()
{
    auto libname = libraryName->text().trimmed().toStdString().c_str();
    auto anariLibrary = anari::loadLibrary(libname, anari_visit::StatusCallback);

    if(anariLibrary)
    {
        volumeAttributes->SetAnariLibrary(libname);

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
        volumeAttributes->SetAnariLibrarySubtype(libSubtype);

        // Update renderers
        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();

        auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());
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
        UpdateRendererParams(rendererSubtype, anariDevice);

        volumeAttributes->SetAnariRendererSubtype(rendererSubtype);
        rendererSubtypes->blockSignals(false);

        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        auto backendType = GetBackendType(libraryName->text().trimmed().toStdString());

        if(backendType == BackendType::USD)
        {
            emit currentBackendChanged(1);
        }
        else
        {
            emit currentBackendChanged(0);
            UpdateUI();
        }

        renderingWindow->SetApply();
    }
    else
    {
        QString message;

        if(libraryName->text().trimmed() == "environment")
        {
            message.append(tr("ANARI_LIBRARY not set."));
        }
        else
        {
            message.append(tr("%1 is not a valid back-end name or not on your library path.").arg(libname));
        }

        QMessageBox::critical(this, tr("ANARI"), message);
        debug1 << "Could not load the ANARI library (" << libname << ") to update the Rendering UI." << std::endl;

        // Reset Back-end Subtype and Renderer to "default"
        librarySubtypes->blockSignals(true);
        librarySubtypes->clear();
        librarySubtypes->addItem("default");
        librarySubtypes->blockSignals(false);
        auto libSubtype =  librarySubtypes->currentText().toStdString();
        volumeAttributes->SetAnariLibrarySubtype(libSubtype);

        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();
        rendererSubtypes->addItem("default");
        rendererSubtypes->blockSignals(false);
        auto rendererSubtype = rendererSubtypes->currentText().toStdString();
        volumeAttributes->SetAnariRendererSubtype(rendererSubtype);

        // Clear/Disable all options
        rendererParams.reset(new std::vector<std::string>());
        UpdateUI();
        emit currentBackendChanged(0);
        renderingWindow->SetApply();
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::librarySubtypeChanged
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
AnariVolumePlotWidget::librarySubtypeChanged(const QString &subtype)
{
    auto libSubtype = subtype.toStdString();

    auto libname = libraryName->text().trimmed().toStdString().c_str();
    auto anariLibrary = anari::loadLibrary(libname, anari_visit::StatusCallback);

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
        UpdateRendererParams(rendererSubtype, anariDevice);

        volumeAttributes->SetAnariRendererSubtype(rendererSubtype);
        rendererSubtypes->blockSignals(false);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        UpdateUI();

        volumeAttributes->SetAnariLibrarySubtype(libSubtype);
        renderingWindow->SetApply();
    }
    else
    {
        debug1 << "Could not create the ANARI back-end device (" << libname << ") to update the Rendering UI." << std::endl;
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::rendererSubtypeChanged
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
AnariVolumePlotWidget::rendererSubtypeChanged(const QString &subtype)
{
    auto rendererSubtype = subtype.toStdString();

    UpdateRendererParams(rendererSubtype);
    UpdateUI();

    volumeAttributes->SetAnariRendererSubtype(rendererSubtype);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::anariSPPChanged
//
// Purpose:
//      Triggered when ANARI samples per pixel has changed.
//
// Arguments:
//      val     new samples per pixel value
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::samplesPerPixelChanged(int val)
{
    volumeAttributes->SetAnariSPP(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::aoSamplesChanged
//
// Purpose:
//      Triggered when ANARI ambient occlusion samples has changed.
//
// Arguments:
//      val     new ANARI ambient occlusion sample count
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::aoSamplesChanged(int val)
{
    volumeAttributes->SetAnariAO(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::lightFalloffChanged
//
// Purpose:
//      Triggered when the light falloff value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::lightFalloffChanged()
{
    bool ok;
    QString text =  lightFalloff->text();
    float val = text.toFloat(&ok);

    if(ok)
    {
        volumeAttributes->SetAnariLightFalloff(val);
        renderingWindow->SetApply();
    }
    else
    {
        debug5 << "Failed to convert Light Falloff input text (" << text.toStdString() << ") to a float" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::ambientIntensityChanged
//
// Purpose:
//      Triggered when the ambient intensity value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::ambientIntensityChanged()
{
    bool ok;
    QString text =  ambientIntensity->text();
    float val = text.toFloat(&ok);

    if(ok)
    {
        volumeAttributes->SetAnariAmbientIntensity(val);
        renderingWindow->SetApply();
    }
    else
    {
        debug5 << "Failed to convert Ambient Intensity input text (" << text.toStdString() << ") to a float" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::maxDepthChanged
//
// Purpose:
//      Triggered when max depth has changed.
//
// Arguments:
//      val     new max depth value
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::maxDepthChanged(int val)
{
    volumeAttributes->SetAnariMaxDepth(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::rValueChanged
//
// Purpose:
//      Triggered when the R value changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::rValueChanged()
{
    bool ok;
    QString text =  rValue->text();
    float val = text.toFloat(&ok);

    if(ok)
    {
        volumeAttributes->SetAnariRValue(val);
        renderingWindow->SetApply();
    }
    else
    {
        debug5 << "Failed to convert R value input text (" << text.toStdString() << ") to a float" << std::endl;
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::outputLocationChanged
//
// Purpose:
//      Triggered when the USD output directory changes
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::outputLocationChanged()
{
    QDir directory(dirLineEdit->text());
    *outputDir = directory.absolutePath();

    if(directory.exists())
    {
        volumeAttributes->SetUsdDir(outputDir->toStdString());
        renderingWindow->SetApply();
    }
    else
    {
        QString message = tr("%1 doesn't exist").arg(*outputDir);
        QMessageBox::critical(this, tr("USD Output Directory"), message);
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::selectButtonPressed
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
AnariVolumePlotWidget::selectButtonPressed()
{
    auto dir = QFileDialog::getExistingDirectory(this,
                                                 tr("Open Directory"),
                                                 *outputDir,
                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty())
    {
        dirLineEdit->setText(dir);
        outputLocationChanged();
    }
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::commitToggled
//
// Purpose:
//      Triggered when USD commit is toggled.
//
// Arguments:
//      val when true writing to USD will happen immediately at the anariCommit
//          call, otherwise it will happen at anariRenderFrame.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::commitToggled(bool val)
{
    volumeAttributes->SetUsdAtCommit(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::binaryToggled
//
// Purpose:
//      Triggered when output type is toggled (binary or text).
//
// Arguments:
//      val if true USD output is binary, otherwise text
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::binaryToggled(bool val)
{
    volumeAttributes->SetUsdOutputBinary(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::materialToggled
//
// Purpose:
//      Triggered when material checkbox is toggled to determine if material
//      objects are included in the USD output.
//
// Arguments:
//      val if true material objects are included in the USD output
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::materialToggled(bool val)
{
    volumeAttributes->SetUsdOutputMaterial(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::previewSurfaceToggled
//
// Purpose:
//      Triggered when preview surface checkbox is toggled to determine if
//      preview surface shader prims are included in the output for material
//      objects.
//
// Arguments:
//      val if true preview surface shader prims are included in output for
//          material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::previewSurfaceToggled(bool val)
{
    volumeAttributes->SetUsdOutputPreviewSurface(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::mdlToggled
//
// Purpose:
//      Triggered when the mdl checkbox is toggled to determine if mdl shader
//      prims are included in the output for material objects.
//
// Arguments:
//      val if true mdl shader prims are included in output for material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::mdlToggled(bool val)
{
    volumeAttributes->SetUsdOutputMDL(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::mdlColorsToggled
//
// Purpose:
//      Triggered when the mdl colors checkbox is toggled to determine if mdl
//      colors are included in the output for material objects.
//
// Arguments:
//      val if true mdl colors are included in output for material objects
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::mdlColorsToggled(bool val)
{
    volumeAttributes->SetUsdOutputMDLColors(val);
    renderingWindow->SetApply();
}

// ****************************************************************************
// Method: AnariVolumePlotWidget::displayColorsToggled
//
// Purpose:
//      Triggered when the display colors checkbox is toggled to determine if
//      display colors are included in the output.
//
// Arguments:
//      val if true include display colors in the output
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariVolumePlotWidget::displayColorsToggled(bool val)
{
    volumeAttributes->SetUsdOutputDisplayColors(val);
    renderingWindow->SetApply();
}
