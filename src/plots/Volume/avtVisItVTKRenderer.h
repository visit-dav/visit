// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVisItVTKRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VISIT_VTK_RENDERER_H
#define AVT_VISIT_VTK_RENDERER_H

#include <filters_exports.h>

#include <avtVisItVTKDevice.h>

#include <memory>
#include <string>


namespace avt
{
    namespace visit_vtk
    {
        enum class DeviceType
        {
            VTK, // Including Intel OSPRay
            NONE,
            UNKNOWN
        };
    }
}

using DeviceType = avt::visit_vtk::DeviceType;

// ****************************************************************************
//  Class: avtVisItVTKRenderer
//
//  Purpose: Factory class for creating instances derived from
//      avtVisItVTKDevice that implement the appropriate VTK backend.
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
//  Modifications:
//
//
// ****************************************************************************

class AVTFILTERS_API avtVisItVTKRenderer
{
public:
    static avtVisItVTKDevice  *GetDevice();
    static void                DeleteDevice();
    static void                SetDeviceType(const DeviceType);
    static const char         *GetDeviceTypeStr();

    static DeviceType          GetDeviceType() { return m_deviceType; }

    static void                WriteArrayToPPM(std::string, const float *, int, int);

private:
    static DeviceType m_deviceType;
    static std::unique_ptr<avtVisItVTKDevice> m_devicePtr;
};

#endif
