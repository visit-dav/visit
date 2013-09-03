#include "VisItWriteData.h"
#include "visit-config.h"

#ifdef HAVE_LIBNETCDF
#include <netcdf>
#endif

#include <vtkAbstractArray.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <string>
#include <vector>

void
VisItWriteData::write_data(const std::string& filename, const std::string& varname, vtkAbstractArray* vtkarray)
{
#ifdef HAVE_LIBNETCDF

    vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(vtkarray);

    std::cout << "netcdf: " << filename << " " << varname << " "
              << vtkarray->GetDataSize() << " " << vtkarray->GetClassName() << " "
              << array->GetNumberOfTuples() << " " << std::endl;

    int ncidp, varidp, dimidsp;

    nc_create(filename.c_str(), NC_CLOBBER ,&ncidp);
    nc_def_dim(ncidp,"x",vtkarray->GetNumberOfTuples(), &dimidsp);
    int dims[1] = { dimidsp };

    nc_def_var(ncidp,varname.c_str(), NC_DOUBLE, 1, dims, &varidp);
    nc_enddef(ncidp);

    nc_put_var_double(ncidp, varidp, array->GetPointer(0) );
    nc_close(ncidp);
#else
    std::cerr << "only netcdf format writing is supported and netcdf was not available" << std::endl;
#endif
}

void
VisItWriteData::write_data(const std::string &filename,
               const stringVector &dimNm,
               const std::vector<std::vector<double> > &dimensions,
               const stringVector &varnames,
               const intVector &indices,
               const intVector &arrayShape,
               vtkAbstractArray *vtkarray)
{

#ifdef HAVE_LIBNETCDF

    int type = vtkarray->GetDataType();

    nc_type nc_type = NC_INT;

    if(type == VTK_INT) nc_type = NC_INT;
    else if(type == VTK_FLOAT) nc_type = NC_FLOAT;
    else if(type == VTK_DOUBLE) nc_type = NC_DOUBLE;
    else
    {
        std::cout << "unhandled format: " << vtkarray->GetClassName() << std::endl;
        return;
    }

    //todo: handle more types..

    int ncID;
    int nVals = arrayShape[0];
    int nArrs = arrayShape[1];
    int nVars = varnames.size();
    int nDims = dimensions.size();

    if(nVars == 0 || nDims == 0)
    {
        std::cout << "no variables or dimensions requested" << std::endl;
        return;
    }

    nc_create(filename.c_str(), NC_CLOBBER, &ncID);

    //Define dimensions.
    int *dimIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
    {
        nc_def_dim(ncID, dimNm[i].c_str() , dimensions[i].size(), &dimIds[i]);
        //std::cout << res << " " << nc_strerror(res) << std::endl;
    }
    //Define vars.
    int *dimVarIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
    {
        nc_def_var(ncID, dimNm[i].c_str(), NC_DOUBLE, 1, &dimIds[i], &dimVarIds[i]);
        //std::cout << res << " " << nc_strerror(res) << std::endl;
    }
    //Reverse the dim ordering...
    int *swapIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
    swapIds[i] = dimIds[nDims-1 -i];
    for (int i = 0; i < nDims; i++)
    dimIds[i] = swapIds[i];
    delete [] swapIds;

    int *varIds = new int[nVars];
    for (int i = 0; i < nVars; i++){
        nc_def_var(ncID, varnames[i].c_str(), nc_type, nDims, dimIds, &varIds[i]);
    }
    nc_enddef(ncID);

    //dump out dim values.
    for (int i = 0; i < nDims; i++)
    {
    double *d = new double[dimensions[i].size()];
    for (int j = 0; j < dimensions[i].size(); j++)
        d[j] = dimensions[i][j];
    nc_put_var_double(ncID, dimVarIds[i], d);
    delete [] d;
    }

    std::vector<size_t> start; //(nDims, 0);
    std::vector<size_t> count; //(nDims);
    std::vector<ptrdiff_t> stride;//(nDims,1);
    std::vector<ptrdiff_t> imapp;//(nDims, 1);

    start.resize(nDims,0);
    count.resize(nDims,0);
    stride.resize(nDims,1);
    imapp.resize(nDims,1);

    /// why is this in reverse dimensions?
    for(int i = 0; i < count.size(); ++i)
        count[i] = dimensions[count.size()-i-1].size();

    imapp[0] = dimensions[0].size()*nArrs;
    imapp[1] = nArrs;

    for (int i = 0; i < nVars; i++)
    {
//        std::cout << start[0] << " " << count[0] << " " << stride[0] << " " << imapp[0] << " " << vtkarray << std::endl;
//        std::cout <<  vtkarray->GetVoidPointer(indices[i]) << std::endl;
//        int varId = varIds[i];
//        int res = nc_put_varm(ncID,varId, start, count, stride, imapp, vtkarray->GetVoidPointer(indices[i]));
//        std::cout << res << " " << nc_strerror(res) << std::endl;

        if(nc_type == NC_INT)
        {
            vtkIntArray* array = vtkIntArray::SafeDownCast(vtkarray);
            //std::cout << array << " " << array->GetNumberOfTuples() << " " << array->GetNumberOfComponents() << " " << array->GetDataSize() << std::endl;
//            for(size_t j = 0; j < array->GetNumberOfTuples(); ++j)
//                std::cout << array->GetValue(j) << std::endl;

            int res = nc_put_varm_int(ncID, varIds[i], &start.front(), &count.front(), &stride.front(), &imapp.front(), array->GetPointer(indices[i]));
            std::cout << res << " " << nc_strerror(res) << std::endl;
        }
        else if(nc_type == NC_FLOAT)
        {
            vtkFloatArray* array = vtkFloatArray::SafeDownCast(vtkarray);
            //nc_put_varm_float(ncID,varIds[i],start, count, stride, imapp, array->GetPointer(indices[i]));
            int res = nc_put_varm_float(ncID, varIds[i], &start.front(), &count.front(), &stride.front(), &imapp.front(), array->GetPointer(indices[i]));
        }
        else if(nc_type == NC_DOUBLE)
        {
            vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(vtkarray);
            //nc_put_varm_double(ncID,varIds[i],start, count, stride, imapp, array->GetPointer(indices[i]));
            int res = nc_put_varm_double(ncID, varIds[i], &start.front(), &count.front(), &stride.front(), &imapp.front(), array->GetPointer(indices[i]));
        }
    }
    nc_close(ncID);
    
    //delete [] tmp;
    delete [] varIds;
    delete [] dimVarIds;
    delete [] dimIds;
#else
    std::cerr << "only netcdf write is supported and netcdf support was unavailable" << std::endl;
#endif

}
