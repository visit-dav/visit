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
}

using BackendType = anari_visit::BackendType;

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

private:
    QWidget *CreateGeneralWidget(int &);
    QWidget *CreateBackendWidget(int &);

    BackendType GetBackendType(const std::string &) const;

    void UpdateUI();
    void UpdateRendererParams(const std::string &, anari::Device anariDevice = nullptr);

    QvisVolumePlotWindow *renderingWindow;
    VolumeAttributes *volumeAttributes;

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
};

#endif