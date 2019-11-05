/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ***************************************************************************
//
// file PMDField.cpp
//
// Methods of the PMDField class
//
// Programmer: Mathieu Lobet
// Creation: Fri Oct 14 2016
//
// ***************************************************************************

#include "PMDField.h"

// ***************************************************************************
// Method: PMDField::PMDField
//
// Purpose:
//       Constructor
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDField::PMDField()
{
    this->unitSI=1;
    this->gridUnitSI=1;
    // Data size: 4 - simple precision, 8 - double precision
    this->dataSize = 0;
    this->thetaImSign = 1;
    strcpy(this->name,"");
    strcpy(this->datasetPath,"");
    strcpy(this->groupPath,"");

    // Discretization for the theta mode
    this->thetaNbNodes = 100;

}

// ***************************************************************************
// Method: PMDField::~PMDField
//
// Purpose:
//      Destructor
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDField::~PMDField()
{

}

// ***************************************************************************
// Method: PMDField::ScanAttributes
//
// Purpose:
//      This method scans from a group or a dataset the attributes
//      that the current PMDField object needs.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDField::ScanAttributes(hid_t objectId)
{
#ifdef VERBOSE_MODE
    cerr << " PMDField::ScanAttributes: " << endl;
#endif

    int     numAttr;
    int     i;
    hid_t   attrId;
    hid_t   attrType;
    hid_t   attrSpace;
    int     ndims;
    hsize_t sdim[64];
    herr_t  err;
    char    name[64];

    // Number of attributes
    numAttr = H5Aget_num_attrs(objectId);

    // This solution with H5Aiterate2 does not work because GetAttributeInfo
    // needs to be static and therefore field attribute can not be modified
    // err = H5Aiterate2(objectId, H5_INDEX_NAME, H5_ITER_INC, NULL,
    // GetAttributeInfo, NULL);
    // But I am not an expert in C++, Mathieu

    // iteration over the attributes
    for (i = 0; i < numAttr; i++)
    {
        // Open the attribute using its loop id.
        attrId = H5Aopen_idx(objectId, (unsigned int)i );

        // Get the name
        H5Aget_name(attrId, 64, name );

        // The type of the attribute
        attrType  = H5Aget_type(attrId);
        // Space
        attrSpace = H5Aget_space(attrId);
        // Number of dimensions
        ndims = H5Sget_simple_extent_ndims(attrSpace);
        //
        err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

        if (strcmp(name,"gridSpacing")==0)
        {
            SetGridSpacing(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"gridGlobalOffset")==0)
        {
            SetGridGlobalOffset(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"position")==0)
        {
            SetGridPosition(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"axisLabels")==0)
        {
            SetAxisLabels(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"geometry")==0)
        {
            SetGeometry(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"geometryParameters")==0)
        {
           SetGeometryParameters(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"unitSI")==0)
        {
            SetUnitSI(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"gridUnitSI")==0)
        {
            SetGridUnitSI(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"timeOffset")==0)
        {
            // To be implemented
        }
        else if (strcmp(name,"unitDimension")==0)
        {
            SetUnitDimension(name, attrId, attrType, attrSpace);
        }
        /*else if (strcmp(name,"fieldBoundary")==0)
        {
            SetFieldBoundary(name, attrId, attrType, attrSpace);
        }*/
        /*else if (strcmp(name,"fieldBoundaryParameters")==0)
        {
            SetFieldBoundaryParameters(name, attrId, attrType, attrSpace);
        }*/
        else if (strcmp(name,"dataOrder")==0)
        {
            SetDataOrder(name, attrId, attrType, attrSpace);
        }
        else if (strcmp(name,"fieldSolver")==0)
        {
            // To be implemented
        }
        else if (strcmp(name,"chargeCorrection")==0)
        {
            // To be implemented
        }
        else if (strcmp(name,"currentSmoothing")==0)
        {
            // To be implemented
        }
        H5Aclose(attrId);
    }

    // Determine number of modes for the theta modes
    if (this->geometry=="thetaMode")
    {
      this->nbModes = (this->nbNodes[0]-1)/2;
    }

}

// ***************************************************************************
// Method: PMDField::SetGridDimensions
//
// Purpose:
//      This method opens the given dataset to determine the number of
//      points/nodes each direction for the field grid.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDField::SetGridDimensions(hid_t datasetId)
{
    // Data space
    hid_t dataset_space = H5Dget_space(datasetId);
    // Data type
    hid_t dataset_type  = H5Dget_type(datasetId);
    // Storage size
    hsize_t dataset_storage_size = H5Dget_storage_size(datasetId);
    // Dimension from the data space
    hsize_t dims[3];
    hsize_t maxdims[3];
    this->ndims = H5Sget_simple_extent_dims(dataset_space, dims, maxdims );

    if (ndims==3)
    {
        this->nbNodes[0] = dims[0];
        this->nbNodes[1] = dims[1];
        this->nbNodes[2] = dims[2];
    }
    else if (ndims==2)
    {
        this->nbNodes[0] = dims[0];
        this->nbNodes[1] = dims[1];
        this->nbNodes[2] = 1;
    }
    else
    {
        cerr << "Error in PMDField::SetGridDimensions, ndims = " << ndims
             << endl;
    }

}

// ***************************************************************************
// Method: PMDField::SetGridSpacing
//
// Purpose:
//      This method reads from the dataset attributes GridSpacing the grid
//      spacing (dx, dy, dz) of the field grid.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
// - Nov 11 2016 - Mathieu - add 2d case
//
// ***************************************************************************
void
PMDField::SetGridSpacing(char * name,
                         hid_t attrId,
                         hid_t attrType,
                         hid_t attrSpace)
{
    herr_t     err;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        double *tmpArray = new double[npoints];

        err = H5Aread(attrId, attrType, tmpArray);

        if (npoints == 3)
        {
            this->gridSpacing[0] = tmpArray[0];
            this->gridSpacing[1] = tmpArray[1];
            this->gridSpacing[2] = tmpArray[2];
        }
        else if (npoints == 2)
        {
            this->gridSpacing[0] = tmpArray[0];
            this->gridSpacing[1] = tmpArray[1];
            this->gridSpacing[2] = 0;
        }
        
        delete tmpArray;
    }
}

// ***************************************************************************
// Method: PMDField::SetGridGlobalOffset
//
// Purpose:
//      This method reads from the dataset attributes GridGlobalOffset
//      the relative offset of the field grid (usually relative to the
//      rho grid).
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
// - Nov 11 2016 - Mathieu - add 2d case
//
// ***************************************************************************
void
PMDField::SetGridGlobalOffset(char * name,
                              hid_t attrId,
                              hid_t attrType,
                              hid_t attrSpace)
{
    herr_t     err;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        double *tmpArray = new double[npoints];

        err = H5Aread(attrId, attrType, tmpArray);

        if (npoints==3)
        {
            this->gridGlobalOffset[0] = tmpArray[0];
            this->gridGlobalOffset[1] = tmpArray[1];
            this->gridGlobalOffset[2] = tmpArray[2];
        }
        else if (npoints==2)
        {
            this->gridGlobalOffset[0] = tmpArray[0];
            this->gridGlobalOffset[1] = tmpArray[1];
            this->gridGlobalOffset[2] = 0;
        }

        delete tmpArray;
    }
}

// ***************************************************************************
// Method: PMDField::SetGridPosition
//
// Purpore:
//      This method reads from the dataset attributes GridPosition
//      the origin of the grid (xmin, ymin, zmin).
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
// - Nov 11 2016 - Mathieu - add 2d case
// ***************************************************************************
void
PMDField::SetGridPosition(char * name,
                          hid_t attrId,
                          hid_t attrType,
                          hid_t attrSpace)
{
    herr_t     err;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        double *tmpArray = new double[npoints];

        err = H5Aread(attrId, attrType, tmpArray);

        if (npoints==3)
        {
            gridPosition[0] = tmpArray[0];
            gridPosition[1] = tmpArray[1];
            gridPosition[2] = tmpArray[2];
        }
        else if (npoints==2)
        {
            gridPosition[0] = tmpArray[0];
            gridPosition[1] = tmpArray[1];
            gridPosition[2] = 0;
        }

        delete tmpArray;
    }
}

// ***************************************************************************
// Method: PMDField::SetUnitSI
//
// Purpose:
//      this method captures the arrtibute unitSI from a group or a dataset.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetUnitSI(char * name,
                    hid_t attrId,
                    hid_t attrType,
                    hid_t attrSpace)
{
    herr_t     err;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        err = H5Aread(attrId, attrType, &unitSI);

    }
}

// ***************************************************************************
// Method: PMDField::SetGridUnitSI
//
// Purpose:
//      This method reads from attribute GridUnitSI the factor to get the
//      grid units in SI.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetGridUnitSI(char * name,
                        hid_t attrId, hid_t attrType, hid_t attrSpace)
{
    herr_t     err;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        err = H5Aread(attrId, attrType, &gridUnitSI);

    }
}

// ***************************************************************************
// Method: PMDField::SetGeometry
//
// Purpose:
//      This method reads from attribute geometry the type of grid.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetGeometry(char * name,
                      hid_t attrId,
                      hid_t attrType,
                      hid_t attrSpace)
{
    herr_t     err;
    if (H5T_STRING == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);
        size_t size = H5Tget_size (attrType);

        // buffer
        char *buffer = new char[size+1];

        err = H5Aread(attrId, attrType, buffer);
        buffer[size] = '\0';

        if (strstr(buffer,"cartesian"))
        {
            geometry = "cartesian";
        }
        else if (strstr(buffer,"thetaMode"))
        {
            geometry = "thetaMode";
        }
        else
        {
            cerr << " Error in PMDField::SetGeometry:" << endl;
            cerr << " tmpchar is not a valid geometry" << endl;
        }

        delete buffer;
    }
}

// ***************************************************************************
// Method: PMDField::SetAxisLabels
//
// Purpose:
//      This method reads from attribute axisLabel the labels for each axis.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetAxisLabels(char * name,
                        hid_t attrId,
                        hid_t attrType,
                        hid_t attrSpace)
{
    int        iLabel;
    int        i,j;
    herr_t     err;

    if (H5T_STRING == H5Tget_class(attrType)) {

        // Number of elements in the string array
        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        //int rank = H5Sget_simple_extent_ndims(attrSpace);
        //err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

        // Size of one of the string of the array
        // All string should have the same size
        size_t size = H5Tget_size (attrType);

        /*cerr << " npoints: " << npoints
             //<< " rank: " << rank
             //<< " sdim: " << sdim[0] << " " << sdim[1]
             << " size: " << size
             << endl;*/

        char *buffer = new char[size*npoints];

        // We put all the labels in buffer
        err = H5Aread(attrId, attrType, buffer);

        for (iLabel = 0; iLabel < npoints ; iLabel++)
        {
            this->axisLabels[iLabel] = "";
            for (i = 0; i < size ; i++)
            {
                this->axisLabels[iLabel] += buffer[i + iLabel*size];
            }
        }

        delete buffer;
    }
}

// ***************************************************************************
// Method: PMDField::GetUnitDimension
//
// Purpose:
//      This method reads the UnitDimension attributes and generates
//      the unitsLabel paramerer.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDField::SetUnitDimension(char * name,
                                hid_t attrId,
                                hid_t attrType,
                                hid_t attrSpace)
{
    herr_t     err;
    int     i;
    string  units;
    int     firstunits = 0;

    if (H5T_FLOAT == H5Tget_class(attrType)) {

        // Number of units here
        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        // Unit powers
        double *powers = new double[npoints];

        err = H5Aread(attrId, attrType, powers);

        this->unitsLabel = "";

        // Go through all units
        for(i=0;i<7;i++)
        {

            if (int(powers[i])!=0)
            {

                if (firstunits==0)
                {
                    units = "";
                    firstunits=1;
                }
                else
                {
                    units = ".";
                }

                // List of SI units
                switch(i)
                {
                // Distance, meter
                case 0:
                    units += "m";
                    break;
                // Mass, kg
                case 1:
                    units += "kg";
                    break;
                // Time, second
                case 2:
                    units += "s";
                    break;
                // Electric Current, Ampere
                case 3:
                    units += "A";
                    break;
                // Temperature, Kelvin
                case 4:
                    units += "K";
                    break;
                //amount of substance, mole
                case 5:
                    units += "mol";
                    break;
                //luminous intensity, candela
                case 6:
                    units += "candela";
                    break;
                }
                //amount of substance, mole
                //luminous intensity, candela

                if (int(powers[i]) != 1)
                {
                    char power[8];
                    sprintf(power,"%d",int(powers[i]));
                    units += "^";
                    units += power;
                }
                // Creation of the unitsLabel
                this->unitsLabel += units;
            }
        }
        //cerr << this->unitsLabel << endl;

        delete powers;
    }
}

// ***************************************************************************
// Method: PMDField::SetFieldBoundary
//
// Purpose:
//      This method reads the field boundaries
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetFieldBoundary(char * name,
                           hid_t attrId,
                           hid_t attrType,
                           hid_t attrSpace)
{
    herr_t     err;
    int        iLabel;
    int        i,j;

    if (H5T_STRING == H5Tget_class(attrType)) {

        // Number of elements
        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        //int rank = H5Sget_simple_extent_ndims(attrSpace);
        //err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

        size_t size = H5Tget_size (attrType);

        /*cerr << " npoints: " << npoints
             << " rank: " << rank
             << " sdim: " << sdim[0] << " " << sdim[1]
             << " size: " << size
             << endl;*/

        // Creation of the buffer
        char *buffer = new char[size*npoints];

        err = H5Aread(attrId, attrType, buffer);

        for (iLabel = 0; iLabel < npoints ; iLabel++)
        {
            this->fieldBoundary[iLabel] = "";
            for (i = 0; i < size ; i++)
            {
                this->fieldBoundary[iLabel] += buffer[i + iLabel*size];
            }
            //cerr << this->fieldBoundary[iLabel] << endl;
        }

        delete buffer;
    }
}

// ***************************************************************************
// Method: PMDField::SetFieldBoundaryParameters
//
// Purpose:
//      This method reads the field boundaries parameters attribute
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetFieldBoundaryParameters(char * name,
                           hid_t attrId,
                           hid_t attrType,
                           hid_t attrSpace)
{
    herr_t     err;
    int        iLabel;
    int        i,j;

    if (H5T_STRING == H5Tget_class(attrType)) {

        // Number of elements
        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        //int rank = H5Sget_simple_extent_ndims(attrSpace);
        //err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

        size_t size = H5Tget_size (attrType);

        /*cerr << " npoints: " << npoints
             << " rank: " << rank
             << " sdim: " << sdim[0] << " " << sdim[1]
             << " size: " << size
             << endl;*/

        char *buffer = new char[size*npoints];

        err = H5Aread(attrId, attrType, buffer);

        for (iLabel = 0; iLabel < npoints ; iLabel++)
        {
            this->fieldBoundaryParameters[iLabel] = "";
            for (i = 0; i < size ; i++)
            {
                this->fieldBoundaryParameters[iLabel] += buffer[i + iLabel*size];
            }
        }

        delete buffer;
    }
}


// ***************************************************************************
// Method: PMDField::SetDataOrder
//
// Purpose:
//      This method reads the data order
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDField::SetDataOrder(char * name,
                       hid_t attrId,
                       hid_t attrType,
                       hid_t attrSpace)
{
    herr_t     err;
    if (H5T_STRING == H5Tget_class(attrType)) {

        // Number of elements
        int npoints = H5Sget_simple_extent_npoints(attrSpace);
        // Size of an elements
        size_t size = H5Tget_size (attrType);

        char *buffer = new char[size+1];

        err = H5Aread(attrId, attrType, buffer);
        buffer[size] = '\0';

        this->dataOrder = buffer;

        delete buffer;
    }
}

// ***************************************************************************
// Method: PMDField::SetGeometryParameters
//
// Purpose:
//      This method reads the geometry parameters.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Thu Fre 08 2017
//
// Modifications:
//
// ***************************************************************************
int
PMDField::SetGeometryParameters(char * name,
                       hid_t attrId,
                       hid_t attrType,
                       hid_t attrSpace)
{
  herr_t     err;
  if (H5T_STRING == H5Tget_class(attrType)) {

      // Number of elements
      int npoints = H5Sget_simple_extent_npoints(attrSpace);
      // Size of an element
      size_t size = H5Tget_size (attrType);

      // Buffer to get the attribute
      char *buffer = new char[size+1];

      // Reading of the attribute
      err = H5Aread(attrId, attrType, buffer);
      buffer[size] = '\0';

      // Get the sign of imag
      if (strcspn(buffer,"-")!=strlen(buffer))
      {
          this->thetaImSign = -1;
      }
      else if (strcspn(buffer,"+")!=strlen(buffer))
      {
          this->thetaImSign = 1;
      }
      else
      {
        cerr << " Error in PMDField::SetGeometryParamters" << endl;
        cerr << " Sign of imag is not recongnized" << endl;
        return -1;
      }

      delete buffer;
  }
  else
  {
      cerr << " Error in PMDField::SetGeometryParamters" << endl;
      cerr << " Attribute is not a string" << endl;
      return -1;
  }
  return 0;
}


// ***************************************************************************
// Method: PMDField::GetNumValues
//
// Purpose:
//      This method returns the total number of nodes/points that
//      compose a grid.
//
// Arguments:
//      name : name of the attribute
//      attrId : attribute Id for hdf5
//      attribute type : hdf5 attribute type
//      attribute space : hdf5 attribute space
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
// - Nov 11 2016 - Mathieu - add 2d case
//
// ***************************************************************************
int PMDField::GetNumValues() const
{
    if (this->ndims==3)
    {
        return this->nbNodes[0]*this->nbNodes[1]*this->nbNodes[2];
    }
    else if (this->ndims==2)
    {
        return this->nbNodes[0]*this->nbNodes[1];
    }
    return 0;
}

// ***************************************************************************
// Method: PMDField::GetBlockProperties
//
// Purpose:
//      This method returns the properties of the required block when the
//      fields are readed by block (parallel)
//
// Programmer: Mathieu Lobet
// Creation:   Mon Nov 14 2016
//
// Arguments:
//      blockDim : number of domains to divide the field
//      blockId : index of the block
//      fieldBlock : structure containing the properties of the block
//
// Modifications:
//
// ***************************************************************************
int
PMDField::GetBlockProperties(int blockDim,
                             int blockId,
                     fieldBlockStruct * fieldBlock) const
{

#ifdef VERBOSE_MODE
    cerr << "PMDField::GetBlockProperties"
         << "(blockDim=" << blockDim
         << ", blockId=" << blockId
         << ")" << endl;
#endif

    // Parameter declaration
    int r;                          // division rest

    // Copy the name of the dataset
    strcpy(fieldBlock->dataSetPath,this->datasetPath);

    // 3D datasets
    if (this->ndims==3)
    {
        // Set the dimension of the block
        fieldBlock->ndims=this->ndims;

        // Computation of the number of Nodes
        // We divide the field dataset into blockDim domains
        // in the last direction
        fieldBlock->nbNodes[0] = this->nbNodes[0] / blockDim;
        r = this->nbNodes[0]%blockDim;
        if (blockId < r )
        {
            fieldBlock->nbNodes[0] += 1;
        }

        fieldBlock->nbNodes[1] = this->nbNodes[1];
        fieldBlock->nbNodes[2] = this->nbNodes[2];

        // Computation of minimum idexes
        if (blockId < r )
        {
            fieldBlock->minNode[0] = blockId*fieldBlock->nbNodes[0];
        }
        else
        {
            fieldBlock->minNode[0] = r*(fieldBlock->nbNodes[0]+1)
                                   + (blockId - r)*fieldBlock->nbNodes[0];
        }
        fieldBlock->minNode[1] = 0;
        fieldBlock->minNode[2] = 0;

        // Adjust so that the domains have a common node
        if (blockId > 0)
        {
            fieldBlock->nbNodes[0] += 1;
        }
        if (blockId > 0)
        {
            fieldBlock->minNode[0] -= 1;
        }

        // Total number of nodes
        fieldBlock->nbTotalNodes = fieldBlock->nbNodes[0]
                                   *fieldBlock->nbNodes[1]
                                   *fieldBlock->nbNodes[2];

        // Computation of maximum indexes
        fieldBlock->maxNode[0] = fieldBlock->minNode[0]
                               + fieldBlock->nbNodes[0] -1;
        fieldBlock->maxNode[1] = fieldBlock->minNode[1]
                               + fieldBlock->nbNodes[1] -1;
        fieldBlock->maxNode[2] = fieldBlock->minNode[2]
                               + fieldBlock->nbNodes[2] -1;
    }
    // 2D datasets
    else if (this->ndims==2)
    {

        // Set the dimension of the block
        fieldBlock->ndims=this->ndims;

        // Computation of the number of Nodes
        // We divide the field dataset into blockDim domains
        // in the last direction
        fieldBlock->nbNodes[0] = this->nbNodes[0] / blockDim;
        r = this->nbNodes[0]%blockDim;
        if (blockId < r )
        {
            fieldBlock->nbNodes[0] += 1;
        }

        fieldBlock->nbNodes[1] = this->nbNodes[1];
        fieldBlock->nbNodes[2] = 1;

        // Computation of minimum idexes
        if (blockId < r )
        {
            fieldBlock->minNode[0] = blockId*fieldBlock->nbNodes[0];
        }
        else
        {
            fieldBlock->minNode[0] = r*(fieldBlock->nbNodes[0]+1)
                                   + (blockId - r)*fieldBlock->nbNodes[0];
        }
        fieldBlock->minNode[1] = 0;
        fieldBlock->minNode[2] = 0;

        // Adjust so that the domains have a common node
        if (blockId > 0)
        {
            fieldBlock->nbNodes[0] += 1;
        }
        if (blockId > 0)
        {
            fieldBlock->minNode[0] -= 1;
        }

        // Total number of nodes
        fieldBlock->nbTotalNodes = fieldBlock->nbNodes[0]
                                 * fieldBlock->nbNodes[1];

        // Computation of maximum indexes
        fieldBlock->maxNode[0] = fieldBlock->minNode[0]
                               + fieldBlock->nbNodes[0] -1;
        fieldBlock->maxNode[1] = fieldBlock->minNode[1]
                               + fieldBlock->nbNodes[1] -1;
        fieldBlock->maxNode[2] = 0;

    }
    else
    {
        cerr << " This dimension, "
             << this->ndims
             << ", can not be read in parallel"
             << endl;
        return -1;
    }

    // Return 0 if no error
    return 0;
}


// ***************************************************************************
// Method: PMDField::ComputeArrayThetaMode
//
// Purpose:
//      This method computes using the given dataSet (dataSetArray)
//      the final array with all the modes (finalDataArray).
//
// Programmer: Mathieu Lobet
// Creation:   Thu Feb 8 2017
//
// Arguments:
//      dataSetArray: array given by the dataSet
//      finalDataArray: final array for VisIt
//
// Modifications:
//
// ***************************************************************************
int
PMDField::ComputeArrayThetaMode(void * dataSetArray,
                                void * finalDataArray)
{

#ifdef VERBOSE_MODE
    cerr << "PMDField::ComputeArrayThetaMode"
         << endl;
#endif

    // Parameters
    int k,j,i,l,m;
    int mode;
    int offset0;
    int offset1;

    // Offset for index computation
    offset0 = this->nbNodes[2]*this->nbNodes[1];

    cerr << this->dataSize << endl;

    // Create a pointer depending of the data size
    // Simple precision
    if (this->dataSize==4)
    {
        float * dataSetArrayTmp = (float*) (dataSetArray);
        float * finalDataArrayTmp = (float*) (finalDataArray);

        // Treatment of the data
        // We first build the mode 0
        for(k = 0; k < this->thetaNbNodes; ++k) // Loop theta
        for(j = 0; j < this->nbNodes[1]; ++j) // Loop r
        for(i = 0; i < this->nbNodes[2]; ++i) // Loop z
        {
            // Absolute indexes
            l = i + j*this->nbNodes[2];
            m = l + k*offset0;

            // Update of data
            finalDataArrayTmp[m] = dataSetArrayTmp[l];
        }

        // Upper modes
        if (this->nbModes > 0)
        {
            float theta = 0;
            float dtheta = 2.*3.14159265359/(this->thetaNbNodes-1);
            for (mode = 1; mode <= this->nbModes; ++mode)
            {
                // Offset to llok at the right part of the array
                // dataSetArray depending on the mode
                offset1 = 2*mode*offset0;

                for(k = 0; k < this->thetaNbNodes; ++k) // Loop theta
                {
                    theta = k*dtheta;

                    for(j = 0; j < this->nbNodes[1]; ++j) // Loop r
                    for(i = 0; i < this->nbNodes[2]; ++i) // Loop z
                    {
                        // Absolute indexes
                        l = i + j*this->nbNodes[2];
                        m = l + k*offset0;

                        // Update of data with the real part
                        finalDataArrayTmp[m] += 
			           dataSetArrayTmp[l + offset1 - offset0]
                                   *cos(mode*theta);

                        // Update of the data with the imaginary
                        // part
                        finalDataArrayTmp[m] +=
                                   dataSetArrayTmp[l + offset1]
                                  *sin(mode*theta);

                    }
                }
            }
        }

    }
    // Double precision
    else if (this->dataSize==8)
    {
        double * dataSetArrayTmp = (double*) (dataSetArray);
        double * finalDataArrayTmp = (double*) (finalDataArray);

        // Treatment of the data
        // We first build the mode 0
        for(k = 0; k < this->thetaNbNodes; ++k) // Loop theta
        for(j = 0; j < this->nbNodes[1]; ++j) // Loop r
        for(i = 0; i < this->nbNodes[2]; ++i) // Loop z
        {
            // Absolute indexes
            l = i + j*this->nbNodes[2];
            m = l + k*offset0;

            // Update of data
            finalDataArrayTmp[m] = dataSetArrayTmp[l];
        }

        // Upper modes
        if (this->nbModes > 0)
        {
            double theta = 0;
            double dtheta = 2.*3.14159265359/(this->thetaNbNodes-1);
            for (mode = 1; mode <= this->nbModes; ++mode)
            {
                // Offset to llok at the right part of the array
                // dataSetArray depending on the mode
                offset1 = 2*mode*offset0;

                for(k = 0; k < this->thetaNbNodes; ++k) // Loop theta
                {
                    // Computation of the current theta value
                    theta = k*dtheta;

                    for(j = 0; j < this->nbNodes[1]; ++j) // Loop r
                    for(i = 0; i < this->nbNodes[2]; ++i) // Loop z
                    {
                        // Absolute indexes
                        l = i + j*this->nbNodes[2];
                        m = l + k*offset0;

                        // Update of data with the real part
                        finalDataArrayTmp[m] += 
			          dataSetArrayTmp[l + offset1 - offset0]
                                   *cos(mode*theta);

                        // Update of the data with the imaginary
                        // part
                        finalDataArrayTmp[m] +=
                                  dataSetArrayTmp[l + offset1]
                                  *thetaImSign*sin(mode*theta);

                    }
                }
            }
        }
    }
    else
    {
        cerr << " Error in PMDField::ComputeArrayThetaMode" << endl;
        cerr << " DataSize is not recognized: "
             << this->dataSize
             << endl;
    }
    return 0;
}
