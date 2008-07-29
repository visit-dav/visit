// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#define MAXLENGTH 256

#define H5NIMROD_SUCCESS 0
#define H5NIMROD_ERR -1

struct _iter_op_data
{
  int stop_idx;
  int count;
  int type;
  char *name;
  size_t len;
  char *pattern;
};
int _transpose_3D (float *data, hsize_t * dims);

hid_t _normalize_h5_type (hid_t type);

herr_t _iteration_operator (hid_t group_id,	/*!< [in]  group id */
			    const char *member_name,	/*!< [in]  group name */
			    void *operator_data);	/*!< [in,out] data passed to the iterator */

int _get_num_groups_matching_pattern (hid_t group_id,
				      const char *group_name,
				      const hid_t type, char *const pattern);


int H5NIMROD_read_attrib (hid_t id, const char *attrib_name,
			  void *attrib_value);
int H5NIMROD_read_string_attrib (hid_t id, const char *attrib_name,
				 char **attrib_value);
int H5NIMROD_read_dims (hid_t id, const char *dataset_name, int *ndims,
			hsize_t * grid_dims);
int H5NIMROD_get_object_name (hid_t group_id, const char *group_name,
			      const hid_t type, const int idx, char *obj_name,
			      const int len_obj_name);
int H5NIMROD_get_num_objects_matching_pattern (hid_t group_id,
					       const char *group_name,
					       const hid_t type,
					       char *const pattern);
int H5NIMROD_get_object_name (hid_t group_id, const char *group_name,
			      const hid_t type, const int idx, char *obj_name,
			      const int len_obj_name);

int H5NIMROD_read_float32_array (hid_t parent_id,
				 const char *dataset_name,
				 hsize_t * offset,
				 int ndims, hsize_t * dims, float *array);
