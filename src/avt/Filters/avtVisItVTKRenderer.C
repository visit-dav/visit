// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVisItVTKRenderer.C                            //
// ************************************************************************* //

#include <avtVisItVTKRenderer.h>
#include <avtVisItVTKOSPRayDevice.h>

DeviceType avtVisItVTKRenderer::m_deviceType = DeviceType::NONE;
std::unique_ptr<avtVisItVTKDevice> avtVisItVTKRenderer::m_devicePtr(nullptr);

// ****************************************************************************
//  Method: avtVisItVTKRenderer::SetDeviceType
//
//  Purpose:
//      Set the device type that this factory class will return when the 
//      GetDevice method is called. Setting device type to NONE or UNKNOWN 
//      will delete any memory previously allocated for the device.
//
//  Arguments:
//      deviceType  An enum representing the VisItVTK rendering backend
//      
//  Throws a VisIt exception if the device type specified by deviceType is not 
//  supported. 
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

void
avtVisItVTKRenderer::SetDeviceType(const DeviceType deviceType) 
{
    if(m_deviceType != deviceType)
    {
        m_deviceType = deviceType;

        switch(deviceType) 
        {
            case DeviceType::OSPRAY:
                #ifdef VISIT_OSPRAY
                    m_devicePtr.reset(new avtVisItVTKOSPRayDevice());
                #else
                    // throw std::invalid_argument("OSPRay Device is not supported");
                    EXCEPTION1(VisItException, "[VisItVTK] OSPRay backend is not supported");
                #endif
                break;
            case DeviceType::NONE: case DeviceType::UNKNOWN:
                if(m_devicePtr) 
                {
                    m_devicePtr.reset(nullptr);            
                }
                break;
            default:
                char msg[50];
                int retVal = snprintf(msg, 50, "[VisItVTK] Unsupported device type: %d", static_cast<int>(deviceType));

                if(retVal > 0) 
                {
                    EXCEPTION1(VisItException, msg);
                }
                else
                {
                    EXCEPTION1(VisItException, "[VisItVTK] Unsupported device type");
                }
        }
    }
}

// ****************************************************************************
//  Method: avtVisItVTKRenderer::GetDeviceTypeStr
//
//  Purpose:
//      Return the VisItVTK backend device type as a string     
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

const char *
avtVisItVTKRenderer::GetDeviceTypeStr()
{
    if(m_devicePtr)
        return m_devicePtr->GetDeviceType();

    return nullptr;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderer::GetDevice
//
//  Purpose: Return a generic device pointer to the underlying
//      VisItVTK backend device implementation. The pointer will need
//      to be cast to the specific VisItVTK backend device.
//
//  Returns:  
//      avtVisItVTKDevice pointer    
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

avtVisItVTKDevice  *
avtVisItVTKRenderer::GetDevice() 
{ 
    if(m_devicePtr) 
        return m_devicePtr.get(); 

    return nullptr;
}

// ****************************************************************************
//  Method: avtVisItVTKRenderer::DeleteDevice
//
//  Purpose: Release any memory allocated for the underlying VisItVTK
//      backend device.
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

void
avtVisItVTKRenderer::DeleteDevice()
{
    SetDeviceType(DeviceType::NONE);
}

#ifndef CLAMP
# define CLAMP(x, l, h) (x > l ? x < h ? x : h : l)
#endif

// ****************************************************************************
//  Method: avtVisItVTKRenderer::WriteArrayToPPM
//
//  Purpose:
//
//  Returns:      
//
//  Programmer: Kevin Griffin
//  Creation:   February 22, 2021
//
// ****************************************************************************

void
avtVisItVTKRenderer::WriteArrayToPPM(std::string filename, 
				     const float * image, 
				     int dimX, int dimY)
{
    std::ofstream outputFile((filename + ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);

    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 

    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x)*4;
            char color[3];
            float alpha = image[index + 3];
            color[0] = CLAMP(image[index + 0]*alpha, 0.0f, 1.0f) * 255;
            color[1] = CLAMP(image[index + 1]*alpha, 0.0f, 1.0f) * 255;
            color[2] = CLAMP(image[index + 2]*alpha, 0.0f, 1.0f) * 255;
            outputFile.write(color,3);
        }
    } 

    outputFile.close();
}
