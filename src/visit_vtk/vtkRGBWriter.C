#include "vtkRGBWriter.h"
#include "vtkObjectFactory.h"
#include <vtkImageData.h>
#include <string.h>

typedef struct {
  unsigned char imagic[2];
  unsigned char type[2];
  unsigned char dim[2];
  unsigned char xsize[2];
  unsigned char ysize[2];
  unsigned char zsize[2];
  unsigned char min[4];
  unsigned char max[4];
  unsigned char wastebytes[4];
  char name[80];
} image_hdr;

typedef struct {
  image_hdr      header;        // header read from/written to disk
  ofstream       *file;         // stuff used in core only
  unsigned char  *rowstart;
  unsigned char  *rowsize;
  unsigned char  *rle_buf;
  unsigned char  **red_list;
  unsigned char  **grn_list;
  unsigned char  **blu_list;
  unsigned long  current_offset;
} rgbfile_t;

#define WriteShort(charptr, svalue)  {\
   (charptr)[0] = (unsigned char)(((svalue)&0x0000FF00) >> 8); \
   (charptr)[1] = (unsigned char)((svalue)&0x000000FF); \
}

#define WriteInt(charptr, ivalue)  {\
   (charptr)[0] = (unsigned char)(((ivalue)&0xFF000000) >> 24); \
   (charptr)[1] = (unsigned char)(((ivalue)&0x00FF0000) >> 16); \
   (charptr)[2] = (unsigned char)(((ivalue)&0x0000FF00) >> 8); \
   (charptr)[3] = (unsigned char)((ivalue)&0x000000FF); \
}

#define ReadShort(charptr) \
   (short)(((short)((charptr)[0] << 8) & 0xFF00) | \
   ((short)((charptr)[1]) & 0x00FF))

#define ReadInt(charptr) \
   (int)(((int)((charptr)[0] << 24) & 0xFF000000) | \
   ((int)((charptr)[1] << 16) & 0x00FF0000) | \
   ((int)((charptr)[2] << 8 ) & 0x0000FF00) | \
   ((int)((charptr)[3]      ) & 0x000000FF))

// Static prototypes
static void rgbfile_putrow(rgbfile_t *, unsigned char*, int, int);
static int  rgbfile_rle_encode(rgbfile_t *,unsigned char*,unsigned char*);
static void rgbfile_writeheader(rgbfile_t *);
static void rgbfile_writefile(rgbfile_t *);

//------------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//   Replace 'New' method with Macro to match VTK 4.0 API.
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkRGBWriter);

vtkRGBWriter::vtkRGBWriter()
{
  this->FileLowerLeft = 1;
}

void vtkRGBWriter::WriteFileHeader(ofstream *, vtkImageData *)
{
    // Do nothing since we need to actually compress the data BEFORE
    // writing the file. It does not look possible to rewind the file
    // since it is a stream.
}


void vtkRGBWriter::WriteFile(ofstream *file, vtkImageData *data,
                 int extent[6])
{
    int i, bpp = data->GetNumberOfScalarComponents();
  
    // Make sure we actually have data.
    if(!data->GetPointData()->GetScalars())
    {
        vtkErrorMacro(<< "Could not get data from input.");
        return;
    }

    // take into consideration the scalar type
    if(data->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
        vtkErrorMacro("RGBWriter only accepts unsigned char scalars!");
        return; 
    }

    // Create a rgb "object"
    rgbfile_t rgb;
    memset(&rgb, 0, sizeof(rgbfile_t));
 
    int xsize = extent[1] - extent[0] + 1;
    int ysize = extent[3] - extent[2] + 1;
    int zsize = 3;

    // Fill the header with some basic information.
    rgb.file = file;
    WriteShort(rgb.header.imagic, 474);
    WriteShort(rgb.header.type, 257);
    WriteShort(rgb.header.dim, 3);
    WriteShort(rgb.header.xsize, xsize);
    WriteShort(rgb.header.ysize, ysize);
    WriteShort(rgb.header.zsize, zsize);
    WriteInt  (rgb.header.min, 0);
    WriteInt  (rgb.header.max, 255);
    WriteInt  (rgb.header.wastebytes, 0);
    strcpy    (rgb.header.name, "SGI rgb file");
    
    int table_size = ysize * zsize * 4;
    rgb.rowstart = new unsigned char[table_size];
    rgb.rowsize = new unsigned char[table_size];
    rgb.rle_buf = new unsigned char[xsize];
    rgb.red_list = new unsigned char *[ysize];
    rgb.grn_list = new unsigned char *[ysize];
    rgb.blu_list = new unsigned char *[ysize];
    rgb.current_offset = 512L + (2 * table_size);

    // Create some buffers to use for seperating the scanline data into
    // its respective r,g,b channels.
    unsigned char *red_row = new unsigned char[xsize];
    unsigned char *grn_row = new unsigned char[xsize];
    unsigned char *blu_row = new unsigned char[xsize];

    // If we couldn't get memory, get out of here.
    if(rgb.rowstart == NULL || rgb.rowsize == NULL  ||
       rgb.rle_buf == NULL || red_row == NULL ||
       grn_row == NULL || blu_row == NULL ||
       rgb.red_list == NULL || rgb.grn_list == NULL ||
       rgb.blu_list == NULL)
    {
        delete [] rgb.rowstart;
        delete [] rgb.rowsize;
        delete [] rgb.rle_buf;
        delete [] rgb.red_list;
        delete [] rgb.grn_list;
        delete [] rgb.blu_list;

        delete [] red_row;
        delete [] grn_row;
        delete [] blu_row;

        vtkErrorMacro("RGBWriter could not allocate enough memory!");
        return;
    }

    // Stuff to calculate progress.
    unsigned long count = 0;
    unsigned long target;
    float progress = this->Progress;
    float area;
    int *wExtent;
    wExtent = this->GetInput()->GetWholeExtent();
    area = ((extent[5] - extent[4] + 1)*(extent[3] - extent[2] + 1)*
            (extent[1] - extent[0] + 1)) / 
           ((wExtent[5] -wExtent[4] + 1)*(wExtent[3] -wExtent[2] + 1)*
            (wExtent[1] -wExtent[0] + 1));
    
    target = (unsigned long)((extent[5]-extent[4]+1)*
              (extent[3]-extent[2]+1)/(50.0*area));
    ++target;

    // Run-length encode each scanline.
    for(int y = extent[2]; y <= extent[3]; ++y)
    {
        if(!(count % target))
        {
            this->UpdateProgress(progress + count/(50.0*target));
        }
        ++count;

        // Get a pointer to the RGB pixel data.
        unsigned char *ptr, *cptr;
        ptr = (unsigned char *)data->GetScalarPointer(extent[0], y, 0);
        cptr = ptr;

        // Split the rgb data into seperate channels.
        if(bpp == 1)
        {
            for(i = 0; i < xsize; ++i)
            {
                red_row[i] = ptr[i];
                grn_row[i] = ptr[i];
                blu_row[i] = ptr[i];
            }
        }
        else if(bpp == 2)
        {
            for(i = 0; i < xsize; ++i, cptr += 2)
            {
                red_row[i] = *cptr;
                grn_row[i] = *cptr;
                blu_row[i] = *cptr;
            }
        }
        else if(bpp == 3)
        {
            for(i = 0; i < xsize; ++i)
            {
                red_row[i] = *cptr++;
                grn_row[i] = *cptr++;
                blu_row[i] = *cptr++;
            }
        } 
        else if(bpp == 4)
        {
            for(i = 0; i < xsize; ++i, ++cptr)
            {
                red_row[i] = *cptr++;
                grn_row[i] = *cptr++;
                blu_row[i] = *cptr++;
            }
        }

        // Run length encode each channel and store the resulting array.
        rgbfile_putrow(&rgb, red_row, y, 0);
        rgbfile_putrow(&rgb, grn_row, y, 1);
        rgbfile_putrow(&rgb, blu_row, y, 2);
    }

    // Write the RGB file.
    rgbfile_writefile(&rgb);

    // Free the memory used to encode the RGB image.
    delete [] rgb.rowstart;
    delete [] rgb.rowsize;
    delete [] rgb.rle_buf;
    for(i = 0; i < ysize; ++i)
        delete [] rgb.red_list[i];
    delete [] rgb.red_list;
    for(i = 0; i < ysize; ++i)
        delete [] rgb.grn_list[i];
    delete [] rgb.grn_list;
    for(i = 0; i < ysize; ++i)
        delete [] rgb.blu_list[i];
    delete [] rgb.blu_list;
    delete [] red_row;
    delete [] grn_row;
    delete [] blu_row;
}

//
// Some static functions that help encode the RGB image.
//

static void
rgbfile_writefile(rgbfile_t *rgb)
{
    if(rgb->file != NULL)
    {
        // Write the rgb header.
        rgbfile_writeheader(rgb);

        int height = ReadShort(rgb->header.ysize);

        for(int y = 0; y < height; ++y)
        {
            int i, rowWidth;

            // Write the red channel
            int index = 0 * height + y;
            rowWidth = ReadInt(rgb->rowsize + (index * 4));
            for(i = 0; i < rowWidth; ++i)
                rgb->file->put(rgb->red_list[y][i]);

            // Write the green channel
            index = 1 * height + y;
            rowWidth = ReadInt(rgb->rowsize + (index * 4));
            for(i = 0; i < rowWidth; ++i)
                rgb->file->put(rgb->grn_list[y][i]);

            // Write the blue channel
            index = 2 * height + y;
            rowWidth = ReadInt(rgb->rowsize + (index * 4));
            for(i = 0; i < rowWidth; ++i)
                rgb->file->put(rgb->blu_list[y][i]);
        }
    }    
}

static void
rgbfile_writeheader(rgbfile_t *rgb)
{
    int i;

    // Write the top of the header
    char *cptr = (char *)&(rgb->header);
    for(i = 0; i < 104; ++i, ++cptr)
        rgb->file->put(*cptr);

    // Write some padding so the top of the header totals 512 bytes.
    for(i = 0; i < 408; ++i)
        rgb->file->put((unsigned char)0);

    // Compute the size of the rowstart and rowsize tables.
    int table_size = ReadShort(rgb->header.ysize) * 
                     ReadShort(rgb->header.zsize) * 4;

    // Write the rowstart table and rowsize table.
    cptr = (char *)rgb->rowstart;
    for(i = 0; i < table_size; ++i, ++cptr)
        rgb->file->put(*cptr);

    cptr = (char *)rgb->rowsize;
    for(i = 0; i < table_size; ++i, ++cptr)
        rgb->file->put(*cptr);
}

static void 
rgbfile_putrow(rgbfile_t *rgb, unsigned char *buffer, int y, int channel) 
{
    unsigned char *temp_ptr = NULL;

    // Extract height information.
    int height = (int)ReadShort(rgb->header.ysize);

    if(y >= 0 && y < height && 
       rgb->file != NULL && 
       channel >= 0 && channel < 3) 
    {
        int index = channel * height + y;

        // Run length encode the data stored in buffer.
        int size = rgbfile_rle_encode(rgb, buffer, rgb->rle_buf);

        // Allocate enough memory to hold the compressed buffer.
        unsigned char *compress = new unsigned char[size];
        memcpy((void *)compress, (void *)rgb->rle_buf, size);
       
        // Assign the compressed buffer to the right channel.
        if(channel == 0)
            rgb->red_list[y] = compress;
        else if(channel == 1)
            rgb->grn_list[y] = compress;
        else
            rgb->blu_list[y] = compress;

        // Save the size,offset into the rowstart and rowsize tables.
        temp_ptr = rgb->rowstart + (4 * index);
        WriteInt(temp_ptr, rgb->current_offset);
        temp_ptr = rgb->rowsize + (4 * index);
        WriteInt(temp_ptr, size);
        rgb->current_offset += size;
    }  
}

static int
rgbfile_rle_encode(rgbfile_t *rgb,
                   unsigned char *iptr,
                   unsigned char *outbuf)
{
    short         todo, cc;
    int           count, width;
    unsigned char *ibufend = NULL;
    unsigned char *sptr = NULL;
    unsigned char *optr = NULL;

    width = (int)ReadShort(rgb->header.xsize);
    ibufend = iptr + width;
    optr    = (unsigned char *)outbuf;
     
    while(iptr < ibufend)
    {
        sptr = iptr;
        iptr += 2;
        while((iptr<ibufend)&&((iptr[-2]!=iptr[-1])||(iptr[-1]!=iptr[0])))
            iptr++;
        iptr -= 2;
        count = iptr - sptr;
        while(count)
        {
            todo = (count > 126) ? 126 : count;
            count -= todo;
            *optr++ = 0x80 | todo;
            while(todo--)
               *optr++ = *sptr++;
        }
        sptr = iptr;
        cc = *iptr++;
        while((iptr < ibufend) && (*iptr == cc)) 
            iptr++;
        count = iptr-sptr;
        while(count)
        {
            todo = (count > 126) ? 126 : count;
            count -= todo;
            *optr++ = todo;
            *optr++ = cc;
        }
    }
    *optr++ = 0;

    return (optr - (unsigned char *)outbuf);
}
