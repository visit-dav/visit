/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <esriShapefile.h>

int
main(int argc, char *argv[])
{
    FILE *outputLog = 0;
    esriShapefile_t *f = 0;
    esriFileError_t code;
    int keepReading = 1;

    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return -1;
    }

    /* Open the output log file. */
    outputLog = fopen("output.log", "wt");
    if(outputLog == 0)
        return -1;

    esriInitialize(1, 0);

    f = esriShapefileOpen(argv[1], &code);
    if(code == esriFileErrorSuccess)
        fprintf(outputLog, "Opened the file!\n");
    else if(code == esriFileErrorInvalidFile)
    {
        fprintf(outputLog, "The file was not an ESRI shape file!\n");
        return -1;
    }
    else
    {
        fprintf(outputLog, "Could not open the file!\n");
        return -1;
    }

    // Process the file.
    do
    {
        void *s = 0;
        // Read the record header.
        esriShapefileRecordHeader_t header;
        esriReadError_t rcErr;
        esriShapefileReadRecordHeader(f, &header, &rcErr);

        if(rcErr == esriReadErrorSuccess)
        {
            /*fprintf(stderr, "Record %d: ", header.recordNumber);*/
            switch(header.shapeType)
            {
            case esriNullShape:
                break;
            case esriPoint:
                s = esriMalloc(sizeof(esriPoint_t));
                esriShapefileReadPoint(&header, (esriPoint_t *)s, &rcErr);
                esriPrintPoint(outputLog, (esriPoint_t *)s);
                break;
            case esriPolyLine:
                s = esriMalloc(sizeof(esriPolyLine_t));
                esriShapefileReadPolyLine(&header, (esriPolyLine_t *)s, &rcErr);
                esriPrintPolyLine(outputLog, (esriPolyLine_t *)s);
                break;
            case esriPolygon:
                s = esriMalloc(sizeof(esriPolygon_t));
                esriShapefileReadPolygon(&header, (esriPolygon_t *)s, &rcErr);
                esriPrintPolygon(outputLog, (esriPolygon_t *)s);
                break;
            case esriMultiPoint:
                s = esriMalloc(sizeof(esriMultiPoint_t));
                esriShapefileReadMultiPoint(&header, (esriMultiPoint_t *)s, &rcErr);
                esriPrintMultiPoint(outputLog, (esriMultiPoint_t *)s);
                break;
            case esriPointZ:
                s = esriMalloc(sizeof(esriPointZ_t));
                esriShapefileReadPointZ(&header, (esriPointZ_t *)s, &rcErr);
                esriPrintPointZ(outputLog, (esriPointZ_t *)s);
                break;
            case esriPolyLineZ:
                s = esriMalloc(sizeof(esriPolyLineZ_t));
                esriShapefileReadPolyLineZ(&header, (esriPolyLineZ_t *)s, &rcErr);
                esriPrintPolyLineZ(outputLog, (esriPolyLineZ_t *)s);
                break;
            case esriPolygonZ:
                s = esriMalloc(sizeof(esriPolygonZ_t));
                esriShapefileReadPolygonZ(&header, (esriPolygonZ_t *)s, &rcErr);
                esriPrintPolygonZ(outputLog, (esriPolygonZ_t *)s);
                break;
            case esriMultiPointZ:
                s = esriMalloc(sizeof(esriMultiPointZ_t));
                esriShapefileReadMultiPointZ(&header, (esriMultiPointZ_t *)s, &rcErr);
                esriPrintMultiPointZ(outputLog, (esriMultiPointZ_t *)s);
                break;
            case esriPointM:
                s = esriMalloc(sizeof(esriPointM_t));
                esriShapefileReadPointM(&header, (esriPointM_t *)s, &rcErr);
                esriPrintPointM(outputLog, (esriPointM_t *)s);
                break;
            case esriPolyLineM:
                s = esriMalloc(sizeof(esriPolyLineM_t));
                esriShapefileReadPolyLineM(&header, (esriPolyLineM_t *)s, &rcErr);
                esriPrintPolyLineM(outputLog, (esriPolyLineM_t *)s);
                break;
            case esriPolygonM:
                s = esriMalloc(sizeof(esriPolygonM_t));
                esriShapefileReadPolygonM(&header, (esriPolygonM_t *)s, &rcErr);
                esriPrintPolygonM(outputLog, (esriPolygonM_t *)s);
                break;
            case esriMultiPointM:
                s = esriMalloc(sizeof(esriMultiPointM_t));
                esriShapefileReadMultiPointM(&header, (esriMultiPointM_t *)s, &rcErr);
                esriPrintMultiPointM(outputLog, (esriMultiPointM_t *)s);
                break;
            case esriMultiPatch:
                s = esriMalloc(sizeof(esriMultiPatch_t));
                esriShapefileReadMultiPatch(&header, (esriMultiPatch_t *)s, &rcErr);
                esriPrintMultiPatch(outputLog, (esriMultiPatch_t *)s);
                break;
            default:
                fprintf(outputLog, "Skipping shapeType %d. Size=%d bytes.\n",
                       (int)header.shapeType, header.contentLength);
                /* Skip the record. */
                fseek(f->fp, (long)header.contentLength, SEEK_CUR);
            }

            keepReading = rcErr == esriReadErrorSuccess;
            if(s != 0)
                esriFreeShape(header.shapeType, s);
        }
        else
            keepReading = 0;
    } while(keepReading);


    esriShapefileClose(f);

    esriFinalize();

    fclose(outputLog);

    return 0;
}
