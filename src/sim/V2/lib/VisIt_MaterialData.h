#ifndef VISIT_MATERIALDATA_H
#define VISIT_MATERIALDATA_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_MaterialData_alloc(visit_handle*);
int VisIt_MaterialData_free(visit_handle);

int VisIt_MaterialData_addMaterial(visit_handle obj, 
                                   const char *matName, int *matno);

/* Add materials 1 cell at a time... */
int VisIt_MaterialData_appendCells(visit_handle h, int ncells);

int VisIt_MaterialData_addCleanCell(visit_handle h, int cell, int matno);

int VisIt_MaterialData_addMixedCell(visit_handle h, int cell, 
        const int *matnos, const float *mixvf, int nmats);

/* Or add materials all at once */
int VisIt_MaterialData_setMaterials(visit_handle obj, 
                                    visit_handle matlist);

int VisIt_MaterialData_setMixedMaterials(visit_handle obj,
        visit_handle mix_mat, visit_handle mix_zone, 
        visit_handle mix_next, visit_handle mix_vf);

#ifdef __cplusplus
}
#endif

#endif
