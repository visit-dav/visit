// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_VOLUMEPLOT_WIDGET_H
#define ANARI_VOLUMEPLOT_WIDGET_H

#include <gui_exports.h>
#include <QWidget>

#include <anari/anari_cpp.hpp>

#include <vector>
#include <memory>

class VolumeAttributes;
class QvisVolumePlotWindow;
class QGroupBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QStackedLayout;

namespace anari_visit
{
    enum class BackendType
    {
        NONE,
        EXAMPLE,
        USD,
        VISRTX,
        VISGL,
        OSPRAY,
        RADEONPRORENDER
    };

    enum class USDParameter
    {
        COMMIT,
        BINARY,
        MATERIAL,
        PREVIEW,
        MDL,
        MDLCOLORS,
        DISPLAY
    };
}

using BackendType = anari_visit::BackendType;
using USDParameter = anari_visit::USDParameter;

class GUI_API AnariVolumePlotWidget : public QWidget
{
    Q_OBJECT
public:
    AnariVolumePlotWidget(QvisVolumePlotWindow *,
                          VolumeAttributes *,
                          QWidget *parent = nullptr);
    ~AnariVolumePlotWidget() = default;

    int GetRowCount() const { return totalRows; }

    // General
    void SetChecked(const bool);
    void UpdateLibraryName(const std::string);
    void UpdateLibrarySubtypes(const std::string);
    void UpdateRendererSubtypes(const std::string);

    // Back-end
    void UpdateSamplesPerPixel(const int);
    void UpdateAOSamples(const int);
    void UpdateLightFalloff(const float);
    void UpdateAmbientIntensity(const float);
    void UpdateMaxDepth(const int);
    void UpdateRValue(const float);

    // USD Back-end
    void UpdateUSDOutputLocation(const std::string);
    void UpdateUSDParameter(const USDParameter, const bool);

signals:
    void currentBackendChanged(int);

private slots:
    void renderingToggled(bool);
    void libraryChanged();
    void librarySubtypeChanged(const QString &);
    void rendererSubtypeChanged(const QString &);

    // General
    void samplesPerPixelChanged(int);
    void aoSamplesChanged(int);
    void lightFalloffChanged();
    void ambientIntensityChanged();
    void maxDepthChanged(int);
    void rValueChanged();

    // USD
    void outputLocationChanged();
    void selectButtonPressed();
    void commitToggled(bool);
    void binaryToggled(bool);
    void materialToggled(bool);
    void previewSurfaceToggled(bool);
    void mdlToggled(bool);
    void mdlColorsToggled(bool);
    void displayColorsToggled(bool);

private:
    QWidget *CreateGeneralWidget(int &);
    QWidget *CreateBackendWidget(int &);
    QWidget *CreateUSDWidget(int &);

    BackendType GetBackendType(const std::string &) const;

    void UpdateUI();
    void UpdateRendererParams(const std::string &, anari::Device anariDevice = nullptr);

    QvisVolumePlotWindow *renderingWindow;
    VolumeAttributes *volumeAttributes;
    QStackedLayout *backendStackedLayout;

    std::unique_ptr<std::vector<std::string>> rendererParams;
    int totalRows;

    // General Widget Components
    QGroupBox   *renderingGroup;
    QLineEdit   *libraryName;
    QComboBox   *librarySubtypes;
    QComboBox   *rendererSubtypes;

    // Backend widget UI components
    QSpinBox    *samplesPerPixel;
    QSpinBox    *aoSamples;
    QLineEdit   *lightFalloff;
    QLineEdit   *ambientIntensity;
    QSpinBox    *maxDepth;
    QLineEdit   *rValue;

    // USD widget UI components
    std::unique_ptr<QString>    outputDir;
    QLineEdit   *dirLineEdit{ nullptr};
    QCheckBox   *commitCheckBox{ nullptr };
    QCheckBox   *binaryCheckBox{ nullptr };
    QCheckBox   *materialCheckBox{ nullptr };
    QCheckBox   *previewCheckBox{ nullptr };
    QCheckBox   *mdlCheckBox{ nullptr };
    QCheckBox   *mdlColorCheckBox{ nullptr };
    QCheckBox   *displayColorCheckBox{ nullptr };
};

#endif