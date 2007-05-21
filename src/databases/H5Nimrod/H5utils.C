#include "H5utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

hid_t
_normalize_h5_type (hid_t type)
{
    H5T_class_t tclass = H5Tget_class (type);
    int size = H5Tget_size (type);

    switch (tclass)
    {
	case H5T_INTEGER:
	    if (size == 8)
	    {
		return H5T_NATIVE_INT64;
	    }
	    else if (size == 4)
	    {
		return H5T_NATIVE_INT32;
	    }
	    else if (size == 1)
	    {
		return H5T_NATIVE_CHAR;
	    }
	    break;
	case H5T_FLOAT:
	    if (size == 8)
	    {
		return H5T_NATIVE_DOUBLE;
	    }
	    else if (size == 4)
	    {
		return H5T_NATIVE_FLOAT;
	    }
	default:
	    ;				/* NOP */
    }
    printf ("Unknown type %d", (int) type);

    return -1;
}

int
_transpose_3D (float *data, hsize_t * dims)
{

    hsize_t i, j, k, indx;
    float *newdata;
    hsize_t ndata;

    ndata = dims[0] * dims[1] * dims[2];
    newdata = (float *) malloc (sizeof (float) * ndata);
    indx = 0;
    for (k = 0; k < dims[2]; k++)
    {
	for (i = 0; i < dims[1]; i++)
	{
	    for (j = 0; j < dims[0]; j++)
	    {
		newdata[indx] = data[k + dims[2] * (i + dims[1] * j)];	/* check this */
		indx += 1;
	    }
	}
    }

    memcpy (data, newdata, sizeof (float) * ndata);
    free (newdata);

    return H5NIMROD_SUCCESS;
}

int
H5NIMROD_read_attrib (hid_t parent_id,
	const char *attrib_name, void *attrib_value)
{

    herr_t herr;
    hid_t attrib_id;
    hid_t space_id;
    hid_t type_id;
    hid_t mytype;
    hsize_t nelem;

    attrib_id = H5Aopen_name (parent_id, attrib_name);
    if (attrib_id <= 0)
	return H5NIMROD_ERR;

    mytype = H5Aget_type (attrib_id);
    if (mytype < 0)
	return H5NIMROD_ERR;

    space_id = H5Aget_space (attrib_id);
    if (space_id < 0)
	return H5NIMROD_ERR;

    nelem = H5Sget_simple_extent_npoints (space_id);
    if (nelem < 0)
	return H5NIMROD_ERR;

    type_id = _normalize_h5_type (mytype);

    herr = H5Aread (attrib_id, type_id, attrib_value);
    if (herr < 0)
	return H5NIMROD_ERR;

    herr = H5Sclose (space_id);
    if (herr < 0)
	return H5NIMROD_ERR;

    herr = H5Tclose (mytype);
    if (herr < 0)
	return H5NIMROD_ERR;

    herr = H5Aclose (attrib_id);
    if (herr < 0)
	return H5NIMROD_ERR;

    return H5NIMROD_SUCCESS;
}

int
H5NIMROD_read_string_attrib (hid_t parent_id,
	const char *attrib_name, char **attrib_value)
{
    herr_t herr;
    hid_t attrib_id;
    hid_t type_id;
    hid_t mytype, type;
    hsize_t nelem;
    char *string_value;
    *attrib_value = NULL;

    attrib_id = H5Aopen_name (parent_id, attrib_name);
    if (attrib_id <= 0)
	return H5NIMROD_ERR;

    mytype = H5Aget_type (attrib_id);
    if (mytype < 0)
	return H5NIMROD_ERR;
    nelem = H5Tget_size (mytype);
    if (nelem > 0)
    {
	string_value = (char *) malloc (nelem);
	H5Aread (attrib_id, mytype, string_value);
	*attrib_value = string_value;
    }
    else
    {
	return H5NIMROD_ERR;
    }
    herr = H5Tclose (mytype);
    if (herr < 0)
	return H5NIMROD_ERR;

    herr = H5Aclose (attrib_id);
    if (herr < 0)
	return H5NIMROD_ERR;


    return H5NIMROD_SUCCESS;
}

int
H5NIMROD_read_dims (hid_t parent_id,
	const char *dataset_name, int *ndims, hsize_t * grid_dims)
{
    hsize_t dims[16];
    int i, j;

    hid_t dataset_id = H5Dopen (parent_id, dataset_name);
    if (dataset_id < 0)
	return H5NIMROD_ERR;
    hid_t dataspace_id = H5Dget_space (dataset_id);
    if (dataspace_id < 0)
	return H5NIMROD_ERR;

    *ndims = H5Sget_simple_extent_dims (dataspace_id, dims, NULL);

    for (i = 0, j = *ndims - 1; i < *ndims; i++, j--)
	grid_dims[i] = dims[j];

    H5Sclose (dataspace_id);
    H5Dclose (dataset_id);
    return H5NIMROD_SUCCESS;
}

int
H5NIMROD_read_float32_array (hid_t parent_id,
	const char *dataset_name,
	hsize_t * offset,
	int ndims, hsize_t * dims, float *array)
{
    hid_t dataspace, dataset, memspace, datatype;
    dataset = H5Dopen (parent_id, dataset_name);
    if (dataset < 0)
    {
	printf ("could not open dataset %s\n", dataset_name);
	return H5NIMROD_ERR;
    }
    if (offset == NULL)
	dataspace = H5S_ALL;
    else
	dataspace = H5Dget_space (dataset);	/* dataspace identifier */

    if (dims == NULL)
    {
	memspace = H5S_ALL;
    }
    else
    {
	memspace = H5Screate_simple (ndims, dims, NULL);
	H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset, NULL,
		dims, NULL);
    }

    /*  datatype=H5Dget_type(dataset); */
    H5Dread (dataset,		/* handle for the dataset */
	    H5T_NATIVE_FLOAT,	/* the datatype we use in memory
				   you can change it to FLOAT if you want */
	    memspace,		/* shape/size of data 
				   in memory (the complement to disk hyperslab) */
	    dataspace,		/* shape/size of data on disk  i
				   (get hyperslab if needed) */
	    H5P_DEFAULT,		/* ignore... its for parallel reads */
	    array);		/* the data array we are reading into */

    if (memspace != H5S_ALL)
	H5Sclose (memspace);
    if (dataspace != H5S_ALL)
	H5Sclose (dataspace);

    H5Dclose (dataset);		/* release the dataset handle */
    return H5NIMROD_SUCCESS;
}

herr_t
_iteration_operator (hid_t group_id,	/*!< [in]  group id */
	const char *member_name,	/*!< [in]  group name */
	void *operator_data	/*!< [in,out] data passed to the iterator */
	)
{

    struct _iter_op_data *data = (struct _iter_op_data *) operator_data;
    herr_t herr;
    H5G_stat_t objinfo;

    if (data->type != H5G_UNKNOWN)
    {
	herr = H5Gget_objinfo (group_id, member_name, 1, &objinfo);
	if (herr < 0)
	    return herr;

	if (objinfo.type != data->type)
	    return 0;		/* don't count, continue iteration */
    }

    if (data->name && (data->stop_idx == data->count))
    {
	memset (data->name, 0, data->len);
	strncpy (data->name, member_name, data->len - 1);

	return 1;			/* stop iteration */
    }
    /*
       count only if pattern is NULL or member name matches
     */
    if (!data->pattern ||
	    (strncmp (member_name, data->pattern, strlen (data->pattern)) == 0))
    {
	data->count++;
    }
    return 0;			/* continue iteration */
}

int
H5NIMROD_get_num_objects_matching_pattern (hid_t group_id,
	const char *group_name,
	const hid_t type,
	char *const pattern)
{

    int herr;
    int idx = 0;
    struct _iter_op_data data;

    memset (&data, 0, sizeof (data));
    data.type = type;
    data.pattern = pattern;

    herr = H5Giterate (group_id, group_name, &idx, _iteration_operator, &data);
    if (herr < 0)
	return herr;

    return data.count;
}

int
H5NIMROD_get_object_name (hid_t group_id,
	const char *group_name,
	const hid_t type,
	const int idx,
	char *obj_name, const int len_obj_name)
{

    herr_t herr;
    struct _iter_op_data data;
    int iterator_idx = 0;

    memset (&data, 0, sizeof (data));
    data.stop_idx = idx;
    data.type = type;
    data.name = obj_name;
    data.len = len_obj_name;

    herr = H5Giterate (group_id, group_name, &iterator_idx,
	    _iteration_operator, &data);
    if (herr < 0)
	return herr;

    return H5NIMROD_SUCCESS;
}
