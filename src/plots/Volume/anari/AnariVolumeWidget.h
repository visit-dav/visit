// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_VOLUME_WIDGET
#define ANARI_VOLUME_WIDGET

#include <QWidget>

#include <anari/anari_cpp.hpp>
#include <vectortypes.h>

#include <map>
#include <memory>
#include <string>

class VolumeAttributes;
class AnariAttributes;
class QvisVolumePlotWindow;
class QGroupBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QStackedLayout;
class AnariParameterInfo;
class MapNode;

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
        RADEONPRORENDER,
        PHENOCRYST
    };
}

using BackendType = anari_visit::BackendType;

// ****************************************************************************
// Class: AnariVolumeWidget
//
// Purpose:
//   Widget for configuring ANARI volume rendering settings. All ANARI
//   library/device/renderer introspection is done on the engine and
//   reported back as a MapNode (see NetworkManager::GetAnariDeviceInfo);
//   this widget never creates a local ANARI device, since the client may
//   not have any ANARI backend libraries installed at all. See
//   AnariRenderingWidget, which follows the same pattern for surface
//   rendering.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//   Kevin Griffin, Tue 22 Sep 2026
//   Reworked to ask the engine for ANARI device info (via
//   QvisVolumePlotWindow::RequestAnariDeviceInfo/UpdateDeviceInfo) instead
//   of calling anari::loadLibrary/anari::newDevice locally, giving volume
//   rendering the same client-server ANARI treatment as surface rendering.
//
//   Kevin Griffin, Tue 22 Sep 2026
//   Added CreateColorButton/SetColorButtonSwatch/TextToColor/ColorToText,
//   matching AnariRenderingWidget, so color-valued ANARI renderer
//   parameters get a color-picker button instead of a raw text field.
//
// ****************************************************************************

class AnariVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    AnariVolumeWidget(QvisVolumePlotWindow *,
                      VolumeAttributes *,
                      QWidget *parent = nullptr);
    ~AnariVolumeWidget() = default;

    int GetRowCount() const { return (topRows + bottomRows); }
    void UpdateAnariAttributes(const AnariAttributes &);

    // Consumes the ANARI library/subtype/renderer/parameter info the engine
    // reported in response to RequestDeviceInfo(), and continues the
    // library -> subtype -> renderer -> parameters cascade by issuing the
    // next request as each level of the UI is populated.
    void UpdateDeviceInfo(const MapNode &info);

    void SetChecked(const bool);

    static QColor TextToColor(ANARIDataType, const std::string &);
    static std::string ColorToText(ANARIDataType, const QColor &);

signals:
    void currentBackendChanged(int);

private slots:
    void renderingToggled(bool);
    void libraryChanged();
    void librarySubtypeChanged(const QString &);
    void rendererSubtypeChanged(const QString &);

    void selectButtonPressed();

    // Dynamic
    void spinBoxValueChanged(int);
    void lineEditingFinished();
    void comboBoxTextChanged(const QString &);
    void checkBoxToggled(bool);

private:
    void UpdateLibraryName(const std::string &);
    void UpdateLibrarySubtypes(const std::string &);
    void UpdateRendererSubtypes(const std::string &);

    void UpdateRendererParameters(const stringVector &);
    void UpdateUSDParameters(const stringVector &);
    QWidget *CreateGeneralWidget(int &);
    QWidget *CreateUSDWidget(int &);
    void CreateDynamicWidget(const MapNode &parameters, const std::string &key, bool isUSD = false);
    QPushButton *CreateColorButton(QWidget *, ANARIDataType, const std::string &);
    void SetColorButtonSwatch(QPushButton *, const QColor &);

    BackendType GetBackendType(const std::string &) const;
    AnariParameterInfo BuildParameterInfoFromMapNode(const std::string &name, const MapNode &paramNode);
    QWidget *MakeWidgetFromParameterInfo(const AnariParameterInfo &);
    void UpdateRenderingAttributes(const bool);
    void ClearAnariParameterAttributes();

    void RequestDeviceInfo(const std::string &libraryName,
                           const std::string &librarySubtype,
                           const std::string &rendererSubtype);

    QvisVolumePlotWindow *renderingWindow;
    VolumeAttributes *volumeAttributes;
    AnariAttributes *anariAttributes;
    QStackedLayout *dynamicLayouts; // Caches the dynamic widgets

    // Mapping of dynamic widget key (backend:subtype:renderer) to index in
    // dynamicLayouts
    std::map<std::string, int> dynamicLayoutMap;
    int topRows;
    int bottomRows;

    // General Widget Components
    QGroupBox   *renderingGroup;
    QComboBox   *libraryName;
    QComboBox   *librarySubtypes;
    QComboBox   *rendererSubtypes;

    // File Chooser
    QString     currentDirectory;
    QLineEdit   *dirLineEdit;

    static const std::string USD_WIDGET_KEY;
    static const std::string DEFAULT_WIDGET_KEY;
};

#endif
