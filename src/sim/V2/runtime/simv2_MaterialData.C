#include <vector>
#include <string>
#include <cstring>
#include <snprintf.h>

#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"
#include "simv2_MaterialData.h"

struct VisIt_MaterialData : public VisIt_ObjectBase
{
    VisIt_MaterialData();
    virtual ~VisIt_MaterialData();
    void FreeMaterials();
    void FreeMixedMaterials();

    // Material info
    std::vector<int>         matnos;
    std::vector<std::string> matNames;

    bool appendMode;
    int  ncells;

    // Material list
    visit_handle matlist;

    // Mixed material list
    visit_handle mix_mat;
    visit_handle mix_zone;
    visit_handle mix_next;
    visit_handle mix_vf;

    // Material arrays for append mode
    int               *matlistA;
    std::vector<int>   mix_matA;
    std::vector<int>   mix_zoneA;
    std::vector<int>   mix_nextA;
    std::vector<float> mix_vfA;
};

VisIt_MaterialData::VisIt_MaterialData() : VisIt_ObjectBase(VISIT_MATERIAL_DATA),
    matnos(), matNames()
{
    appendMode = false;
    ncells = 0;

    matlist = VISIT_INVALID_HANDLE;
    mix_mat = VISIT_INVALID_HANDLE;
    mix_zone = VISIT_INVALID_HANDLE;
    mix_next = VISIT_INVALID_HANDLE;
    mix_vf = VISIT_INVALID_HANDLE;

    matlistA = NULL; // this pointer points to data inside matlist.
}

VisIt_MaterialData::~VisIt_MaterialData()
{
    FreeMaterials();
    FreeMixedMaterials();
}

void
VisIt_MaterialData::FreeMaterials()
{
    if(matlist != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(matlist);
        matlist = VISIT_INVALID_HANDLE;
    }
}

void
VisIt_MaterialData::FreeMixedMaterials()
{
    if(mix_mat != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(mix_mat);
        mix_mat = VISIT_INVALID_HANDLE;
    }
    if(mix_zone != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(mix_zone);
        mix_zone = VISIT_INVALID_HANDLE;
    }
    if(mix_next != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(mix_next);
        mix_next = VISIT_INVALID_HANDLE;
    }
    if(mix_vf != VISIT_INVALID_HANDLE)
    {
        simv2_VariableData_free(mix_vf);
        mix_vf = VISIT_INVALID_HANDLE;
    }
}

static VisIt_MaterialData *
GetObject(visit_handle h)
{
    VisIt_MaterialData *obj = (VisIt_MaterialData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_MATERIAL_DATA)
        {
            VisItError("The provided handle does not point to a MaterialData object.");
            obj = NULL;
        }
    }
    else
    {
        VisItError("An invalid handle was provided.");
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_MaterialData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_MaterialData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_MaterialData_free(visit_handle h)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_addMaterial(visit_handle h, const char *matName, int *matno)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(matName == NULL)
    {
        VisItError("A material name must be provided");
        return VISIT_ERROR;
    }
    if(matno == NULL)
    {
        VisItError("An invalid address was passed for the output material "
                   "number");
        return VISIT_ERROR;
    }
    if(obj != NULL)
    {
        if(obj->appendMode)
        {
            int nextmatno = obj->matnos.size();
            *matno = nextmatno;            
        }
        else
        {
            if(*matno < 0)
            {
                VisItError("Material number must be >= 0.");
                return VISIT_ERROR;
            }
        }
        obj->matnos.push_back(*matno);
        obj->matNames.push_back(matName);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_appendCells(visit_handle h, int ncells)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        obj->appendMode = true;

        obj->FreeMaterials();
        obj->FreeMixedMaterials();
        obj->mix_matA.clear();
        obj->mix_zoneA.clear();
        obj->mix_nextA.clear();
        obj->mix_vfA.clear();

        obj->ncells = ncells;
        simv2_VariableData_alloc(&obj->matlist);
        obj->matlistA = (int *)malloc(ncells * sizeof(int));
        simv2_VariableData_setData(obj->matlist, VISIT_OWNER_VISIT,
            VISIT_DATATYPE_INT, 1, ncells, obj->matlistA);

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_addCleanCell(visit_handle h, int cell, int matno)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(!obj->appendMode)
        {
            VisItError("The MaterialData object is not in append mode.");
            return VISIT_ERROR;
        }
        if(cell < 0)
        {
            VisItError("Cell numbers must be >= 0");
            return VISIT_ERROR;
        }
        if(cell >= obj->ncells)
        {
            char tmp[60];
            SNPRINTF(tmp, 60, "Cell numbers must be < %d", obj->ncells);
            VisItError(tmp);
            return VISIT_ERROR;
        }

        obj->matlistA[cell] = matno;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_addMixedCell(visit_handle h, int cell, 
   const int *matnos, const float *mixvf, int nmats)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(!obj->appendMode)
        {
            VisItError("The MaterialData object is not in append mode.");
            return VISIT_ERROR;
        }
        if(cell < 0)
        {
            VisItError("Cell numbers must be >= 0");
            return VISIT_ERROR;
        }
        if(cell >= obj->ncells)
        {
            char tmp[60];
            SNPRINTF(tmp, 60, "Cell numbers must be < %d", obj->ncells);
            VisItError(tmp);
            return VISIT_ERROR;
        }
        if(matnos == NULL)
        {
            VisItError("An invalid material number array was provided");
            return VISIT_ERROR;
        }
        if(mixvf == NULL)
        {
            VisItError("An invalid volume fraction array was provided");
            return VISIT_ERROR;
        }
        if(nmats < 1 || nmats > static_cast<int>(obj->matnos.size()))
        {
            VisItError("An invalid number of materials was provided");
            return VISIT_ERROR;
        }

        int mixlen = obj->mix_matA.size();

        obj->matlistA[cell] = -(mixlen + 1);

        for(int i = 0; i < nmats; ++i)
        {
            obj->mix_matA.push_back(matnos[i]);
            obj->mix_zoneA.push_back(cell);
            obj->mix_vfA.push_back(mixvf[i]);

            if(i < nmats - 1)
                obj->mix_nextA.push_back(mixlen + 2);
            else
                obj->mix_nextA.push_back(0);

            ++mixlen;
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_setMaterials(visit_handle h, visit_handle matlist)
{
    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(matlist, owner, dataType, nComps, nTuples,
        data) == VISIT_ERROR)
    {
        VisItError("An invalid matlist was provided.");
        return VISIT_ERROR;
    }
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->appendMode)
        {
            VisItError("The setMaterials function cannot be called when "
                "the MaterialData object is in append mode.");
            return VISIT_ERROR;
        }
        obj->FreeMaterials();
        obj->matlist = matlist;
        retval = VISIT_OKAY;
    }
    return retval; 
}

int
simv2_MaterialData_setMixedMaterials(visit_handle h,
    visit_handle mix_mat, visit_handle mix_zone, 
    visit_handle mix_next, visit_handle mix_vf)
{
    const char *names[4] = {"mix_mat","mix_zone","mix_next","mix_vf"};
    int cHandles[4];
    cHandles[0] = mix_mat;
    cHandles[1] = mix_zone;
    cHandles[2] = mix_next;
    cHandles[3] = mix_vf;
    int owner[4], dataType[4], nComps[4], nTuples[4];
    void *data[4] = {NULL,NULL,NULL,NULL};
    char tmp[100];
    for(int i = 0; i < 4; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i], dataType[i],
            nComps[i], nTuples[i], data[i]) == VISIT_ERROR)
        {
            SNPRINTF(tmp, 100, "An invalid %s was provided", names[i]);
            VisItError(tmp);
            return VISIT_ERROR; 
        }
        if(nComps[i] != 1)
        {
            SNPRINTF(tmp, 100, "A %s array must have 1 component", names[i]);
            VisItError(tmp);
            return VISIT_ERROR; 
        }
        if(i < 3 && dataType[i] != VISIT_DATATYPE_INT)
        {
            SNPRINTF(tmp, 100, "A %s array must contain integers", names[i]);
            VisItError(tmp);
            return VISIT_ERROR; 
        }
        if(i == 3 && 
           dataType[i] != VISIT_DATATYPE_DOUBLE && 
           dataType[i] != VISIT_DATATYPE_FLOAT)
        {
            SNPRINTF(tmp, 100, "A %s array must contain doubles or floats", names[i]);
            VisItError(tmp);
            return VISIT_ERROR; 
        }
    }
    for(int i = 1; i < 4; ++i)
    {
        if(nTuples[0] != nTuples[i])
        {
            VisItError("All mix arrays must have the same number of tuples");
            return VISIT_ERROR;
        }
    }

    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->appendMode)
        {
            VisItError("The setMixedMaterials function cannot be called when "
                "the MaterialData object is in append mode.");
            return VISIT_ERROR;
        }

        obj->FreeMixedMaterials();
        obj->mix_mat = mix_mat;
        obj->mix_zone = mix_zone;
        obj->mix_next = mix_next;
        obj->mix_vf = mix_vf;
        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/
int
simv2_MaterialData_getNumMaterials(visit_handle h, int &nmat)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        nmat = obj->matnos.size();
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_getMaterial(visit_handle h, int i,
    int &matno, char *matname, int maxlen)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL && i >= 0 && i < static_cast<int>(obj->matnos.size()))
    {
        matno = obj->matnos[i];
        strncpy(matname, obj->matNames[i].c_str(), maxlen);
        retval = VISIT_OKAY;
    }
    return retval;  
}

int
simv2_MaterialData_getMaterials(visit_handle h, visit_handle &matlist)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        matlist = obj->matlist;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_getMixedMaterials(visit_handle h,
    visit_handle &mix_mat, visit_handle &mix_zone, 
    visit_handle &mix_next, visit_handle &mix_vf)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->appendMode)
        {
           obj->FreeMixedMaterials(); // Free old, empty handles
           if(obj->mix_matA.size() > 0)
           {
               // Create new handles to the vector data that the MaterialData
               // object currently stores. We use VISIT_OWNER_SIM since the
               // vectors' data will be destroyed with a destructor.
               simv2_VariableData_alloc(&obj->mix_mat);
               simv2_VariableData_setData(obj->mix_mat, VISIT_OWNER_SIM,
                  VISIT_DATATYPE_INT, 1, (int)obj->mix_matA.size(), 
                  (void *)&obj->mix_matA[0]);

               simv2_VariableData_alloc(&obj->mix_zone);
               simv2_VariableData_setData(obj->mix_zone, VISIT_OWNER_SIM,
                  VISIT_DATATYPE_INT, 1, (int)obj->mix_zoneA.size(), 
                  (void *)&obj->mix_zoneA[0]);

               simv2_VariableData_alloc(&obj->mix_next);
               simv2_VariableData_setData(obj->mix_next, VISIT_OWNER_SIM,
                  VISIT_DATATYPE_INT, 1, (int)obj->mix_nextA.size(), 
                  (void *)&obj->mix_nextA[0]);

               simv2_VariableData_alloc(&obj->mix_vf);
               simv2_VariableData_setData(obj->mix_vf, VISIT_OWNER_SIM,
                  VISIT_DATATYPE_FLOAT, 1, (int)obj->mix_vfA.size(), 
                  (void *)&obj->mix_vfA[0]);
           }
        }

        mix_mat = obj->mix_mat;
        mix_zone = obj->mix_zone;
        mix_next = obj->mix_next;
        mix_vf = obj->mix_vf;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialData_check(visit_handle h)
{
    VisIt_MaterialData *obj = GetObject(h);
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->matnos.size() == 0)
        {
            VisItError("The MaterialData object does not have materials.");
            return VISIT_ERROR;
        }
        if(obj->matlist == VISIT_INVALID_HANDLE)
        {
            VisItError("The MaterialData object does not have a material list.");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }

    return retval;
}

