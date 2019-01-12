#ifndef VTK_ARRAY_COMPONENT_H
#define VTK_ARRAY_COMPONENT_H

// ****************************************************************************
// Class: vtkArrayComponentStride
//
// Purpose:
//   This array component object lets us iterate over blocks of memory using
//   a stride and then we apply an offset to access parts within the block.
//   This lets us access AOS data as though it was contiguous.
//
// Notes:    For zero-copy access of SOA data. It can be used for AOS data too.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 19 11:52:42 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class vtkArrayComponentStride
{
public:
    vtkArrayComponentStride()
    {
        this->start = NULL;
        this->offset = 0;
        this->stride = 0;
        this->type = 0;
        this->owns = false;
    }

    vtkArrayComponentStride(void *start, size_t offset, size_t stride, int type, bool owns)
    {
        this->start = (unsigned char *)start;
        this->offset = offset;
        this->stride = stride;
        this->type = type;
        this->owns = owns;
    }

    ~vtkArrayComponentStride()
    {
    }

    void Delete()
    {
        if(this->owns && this->start != NULL)
        {
            free(start);
            start = NULL;
        }
    }

    template <typename DestinationType>
    void GetValue(vtkIdType id, DestinationType &out) const
    {
        if(this->start != NULL)
        {
            unsigned char *addr = this->start + this->offset + id * this->stride;
            switch(this->type)
            {
            vtkTemplateMacro(
                VTK_TT *ptr = reinterpret_cast<VTK_TT *>(addr);
                out = static_cast<DestinationType>(*ptr);
            );
            }
        }
        else
        {
            out = DestinationType();
        }
    }

protected:
    // Memory traversal attributes.
    unsigned char *start;
    size_t         offset;
    size_t         stride;
    int            type;
    bool           owns;
};

////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: vtkArrayComponentIJKSlice
//
// Purpose:
//   This array component object lets us select an IJK volume of interest in a
//   larger IJK block of memory. We can access the subvolume using normal
//   vtkDataArray indexing and we handle the id translation here so we index
//   into the original volume appropriately.
//
// Notes:    For zero-copy access of subvolumes of structured data.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 19 11:52:42 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class vtkArrayComponentIJKSlice
{
public:
    vtkArrayComponentIJKSlice()
    {
        this->start = NULL;
        this->offset = 0;
        this->stride = 0;
        this->type = 0;
        for(int i = 0; i < 3; ++i)
        {
            this->whole_start_index[i] = 0;
            this->whole_end_index[i]   = 0;
            this->slice_start_index[i] = 0;
            this->slice_end_index[i]   = 0;

            this->slice_size[i] = 0;
            this->whole_size[i] = 0;
        }
        this->sliceNXNY = 0;
        this->wholeNXNY = 0;
    }

    vtkArrayComponentIJKSlice(void *start, size_t offset, size_t stride, int type,
        const int whole_start_index[3], const int whole_end_index[3],
        const int slice_start_index[3], const int slice_end_index[3])
    {
        this->start = (unsigned char *)start;
        this->offset = offset;
        this->stride = stride;
        this->type = type;
        for(int i = 0; i < 3; ++i)
        {
            this->whole_start_index[i] = whole_start_index[i];
            this->whole_end_index[i]   = whole_end_index[i];
            this->slice_start_index[i] = slice_start_index[i];
            this->slice_end_index[i]   = slice_end_index[i];

            this->slice_size[i] = slice_end_index[i] - slice_start_index[i] + 1;
            this->whole_size[i] = whole_end_index[i] - whole_start_index[i] + 1;
        }
        this->sliceNXNY = slice_size[0] * slice_size[1];
        this->wholeNXNY = whole_size[0] * whole_size[1];
    }

    void Delete()
    {
    }

    template <typename DestinationType>
    void GetValue(vtkIdType id, DestinationType &out) const
    {
        // id is the index in the smaller sliced volume. Figure out where that
        // exists in the larger volume.

        vtkIdType slice_K = id / this->sliceNXNY;
        vtkIdType idInIJPlane = id % this->sliceNXNY;
        vtkIdType slice_J = idInIJPlane / this->slice_size[0];
        vtkIdType slice_I = idInIJPlane % this->slice_size[0];

        vtkIdType whole_I = slice_I + this->slice_start_index[0] - this->whole_start_index[0];
        vtkIdType whole_J = slice_J + this->slice_start_index[1] - this->whole_start_index[1];
        vtkIdType whole_K = slice_K + this->slice_start_index[2] - this->whole_start_index[2];

        vtkIdType wholeId = whole_K * this->wholeNXNY + 
                              whole_J * whole_size[0] + 
                              whole_I;
#if 0
cout << "GetValue: id=" << id 
     << ", sliceIJK={" << slice_I << ", " << slice_J << ", " << slice_K
     << "}, wholeIJK={" << whole_I << ", " << whole_J << ", " << whole_K
     << "}, wholeId=" << wholeId << endl;
#endif
        unsigned char *addr = this->start + this->offset + wholeId * this->stride;
        switch(this->type)
        {
        vtkTemplateMacro(
            VTK_TT *ptr = reinterpret_cast<VTK_TT *>(addr);
            out = static_cast<DestinationType>(*ptr);
        );
        }
    }

    bool GetPrint() const { return false; }
private:
    // Memory traversal attributes.
    unsigned char *start;
    size_t         offset;
    size_t         stride;
    int            type;

    // The IJK extents of the whole data array.
    int            whole_start_index[3];
    int            whole_end_index[3];
    // The IJK extents of the volume of interest.
    int            slice_start_index[3];
    int            slice_end_index[3];
    // Derived data
    vtkIdType      slice_size[3];
    vtkIdType      whole_size[3];
    vtkIdType      sliceNXNY;
    vtkIdType      wholeNXNY;
};

#endif
