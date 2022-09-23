// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_XRAYIMAGE_QUERY_WIDGET_H
#define QVIS_XRAYIMAGE_QUERY_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <vectortypes.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QLineEdit;
class MapNode;

// ****************************************************************************
// Class: QvisXRayImageQueryWidget
//
// Purpose:
//   This widget provides options for performing a XRayImage query.
//
// Notes:      
//
// Programmer: Kathleen Biagas
// Creation:   June 17, 2011 
//
// Modifications:
//   Kathleen Biagas, Wed Oct 17 12:12:10 PDT 2012
//   Added upVector.
//
//   Eric Brugger, Fri May 22 15:50:50 PDT 2015
//   I updated the window to use the new view description and support the
//   recently added background intensity parameter.
//
//   Eric Brugger, Wed May 27 17:27:31 PDT 2015
//   I added an option to family output files.
//
//   Eric Brugger, Thu Jun  4 17:23:58 PDT 2015
//   I added an option to enable outputting the ray bounds to a vtk file.
// 
//   Justin Privitera, Tue Jun 14 10:21:03 PDT 2022
//   Added outdir line edit.
//
// ****************************************************************************

class GUI_API QvisXRayImageQueryWidget : public QWidget
{
    Q_OBJECT
public:
    QvisXRayImageQueryWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~QvisXRayImageQueryWidget();

    bool GetQueryParameters(MapNode &params);


private:
    bool             GetDoubleValues(int whichWidget, doubleVector &pt);
    bool             GetDoubleValues(int whichWidget, int n, double *pt);
    bool             GetIntValues(int whichWidget, int *pt);

    QComboBox       *imageFormat;
    QLineEdit       *outDir;
    QCheckBox       *divideFlag;
    QLineEdit       *backgroundIntensities;
    QLineEdit       *normal;
    QLineEdit       *focus;
    QLineEdit       *viewUp;
    QLineEdit       *viewAngle;
    QLineEdit       *parallelScale;
    QLineEdit       *nearPlane;
    QLineEdit       *farPlane;
    QLineEdit       *imagePan;
    QLineEdit       *imageZoom;
    QCheckBox       *perspective;
    QComboBox       *filenameType;
    QCheckBox       *outputRayBounds;
    QLineEdit       *imageSize;
};

#endif
