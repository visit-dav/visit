/**
 * W-SLDA Toolkit
 * This file is taken from repository
 * https://gitlab.fizyka.pw.edu.pl/wtools/wdata
 * which provides C implementation of WData format under GNU GPLv3 license. 
 * */

/*****************************************************************************
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <libgen.h>
#else
#include <direct.h> /* for _getcwd, _chdir */
#define getcwd _getcwd
#define chdir _chdir
#endif
// #include <complex.h>
#include <ctype.h>

#include "wdata.h"

#define MAX_REC_LEN 1024

#define WRKDIR_SET 123987

/**
 * Basic functions for extracting lattice
 * */
int wdata_getnx(wdata_metadata *md) { return md->nx; }
int wdata_getny(wdata_metadata *md) { return md->ny; }
int wdata_getnz(wdata_metadata *md) { return md->nz; }
double wdata_getdx(wdata_metadata *md) { return md->dx; }
double wdata_getdy(wdata_metadata *md) { return md->dy; }
double wdata_getdz(wdata_metadata *md) { return md->dz; }

void wdata_setnx(wdata_metadata *md, int nx) { md->nx = nx; }
void wdata_setny(wdata_metadata *md, int ny) { md->ny = ny; }
void wdata_setnz(wdata_metadata *md, int nz) { md->nz = nz; }
void wdata_setdx(wdata_metadata *md, double dx) { md->dx = dx; }
void wdata_setdy(wdata_metadata *md, double dy) { md->dy = dy; }
void wdata_setdz(wdata_metadata *md, double dz) { md->dz = dz; }
void wdata_setprefix(wdata_metadata *md, const char *prefix) { strcpy(md->prefix, prefix); }

int wdata_set_lattice(wdata_metadata *md, int nx, int ny, int nz, double dx, double dy, double dz)
{
    md->nx = nx;
    md->ny = ny;
    md->nz = nz;
    md->dx = dx;
    md->dy = dy;
    md->dz = dz;
    return 0;
}

char __wdata__basedir[MAX_REC_LEN];
void wdata_goto_wrkdir(wdata_metadata *md)
{
    if (md->issetwrkdir == WRKDIR_SET)
    {
        getcwd(__wdata__basedir, MAX_REC_LEN);
        chdir(md->wrkdir);
        //         printf("changing to workdir=%s\n", md->wrkdir);
    }
}

void wdata_goback_wrkdir(wdata_metadata *md)
{
    if (md->issetwrkdir == WRKDIR_SET)
    {
        chdir(__wdata__basedir);
        //         printf("changing to basedir=%s\n", __wdata__basedir);
    }
}

static char *str_tolower(char *str)
{
    int str_len = int(strlen(str)) + 1;
    int j;

    for (j = 0; j < str_len; j++)
        str[j] = tolower(str[j]);

    return str;
}

// Taken from:
// https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c
static void replace_str(char *str, char *org, char *rep)
{
    char *ToRep = strstr(str, org);
    char *Rest = (char *)malloc(strlen(ToRep));

    strcpy(Rest, ((ToRep) + strlen(org)));
    strcpy(ToRep, rep);

    strcat(ToRep, Rest);

    free(Rest);
}

static int check_metadata(wdata_metadata *md)
{
    // Issue if User does not provide (nx, ny, nz)
    if (md->nx == 0 && md->ny == 0 && md->nz == 0)
    {
        fprintf(stderr, "You did not provide size of the lattice.\nTry to add nx, ny, nz, depending on data dimension\n");
        return WDATA_ERROR_WTXTFILE_INCOMPLETE_EMPTY_N;
    }
    // Issue if User does not provide (dx, dy, dz)
    else if (md->dx == 0.0 && md->dy == 0.0 && md->dz == 0.0)
    {
        fprintf(stderr, "You did not provide lattice spacing.\nTry to add dx, dy, dz depending on the data dimension\n");
        return WDATA_ERROR_WTXTFILE_INCOMPLETE_EMPTY_D;
    }
    // See if datadim matches number of provided dimensions. Issue if User provides (nx, ny, nz) and (datadim != 3)
    else if (md->nx != 0 && md->ny != 0 && md->nz != 0)
    {
        if (md->datadim != 3 && md->datadim != 2 && md->datadim != 1)
        {
            fprintf(stderr, "Provided number of dimensions (nx, ny, nz) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM3_N;
        }
    }
    // See if datadim matches number of provided lattice spacings. Issue if User provides (dx, dy, dz) and (datadim != 3)
    else if (md->dx != 0.0 && md->dy != 0.0 && md->dz != 0.0)
    {
        if (md->datadim != 3 && md->datadim != 2 && md->datadim != 1)
        {
            fprintf(stderr, "Provided number of lattice spacing (dx, dy, dz) does not match to datadim: %d.\n\
            Try to match number of dimensions (dx, dy, dz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM3_D;
        }
    }
    // Issue if User provides (nx, ny) and (datadim != 2)
    else if (md->nx != 0 && md->ny != 0 && md->nz == 0)
    {
        if (md->datadim != 2 && md->datadim != 1)
        {
            fprintf(stderr, "Provided number of dimensions (nx, ny) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM2_N;
        }
    }
    // Issue if User provides (dx, dy) and (datadim != 2)
    else if (md->dx != 0.0 && md->dy != 0.0 && md->dz == 0.0)
    {
        if (md->datadim != 2 && md->datadim != 1)
        {
            fprintf(stderr, "Provided number of lattice spacing (dx, dy) does not match to datadim: %d.\n\
            Try to match number of dimensions (dx, dy, dz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM2_D;
        }
    }
    // Issue if User provides (nx) and (datadim != 1)
    else if (md->nx != 0 && md->ny == 0 && md->nz == 0)
    {
        if (md->datadim != 1)
        {
            fprintf(stderr, "Provided number of dimensions (nx) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM1_N;
        }
    } // Issue if User provides (dx) and (datadim != 1)
    else if (md->dx != 0.0 && md->dy == 0.0 && md->dz == 0.0)
    {
        if (md->datadim != 1)
        {
            fprintf(stderr, "Provided number of lattice spacing (dx) does not match to datadim: %d.\n\
            Try to match number of lattice spacing (dx, dy, dz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM1_D;
        }
    }
    // See if provided dimensions matches datadim.
    else if (md->datadim == 3)
    {
        if (md->nx == 0 || md->ny == 0 || md->nz == 0)
        {
            fprintf(stderr, "Provided number of dimensions (nx, ny, nz) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM3_N;
        }
        if (md->dx == 0.0 || md->dy == 0.0 || md->dz == 0.0)
        {
            fprintf(stderr, "Provided number lattice spacing (dx, dy, dz) does not match to datadim: %d.\n\
            Try to match number of dimensions (dx, dy, dz) to (nx, ny, nz) and datadim.\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM3_D;
        }
    }
    else if (md->datadim == 2)
    {
        if (md->nx == 0 || md->ny == 0)
        {
            fprintf(stderr, "Provided number of dimensions (nx, ny) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM2_N;
        }
        if (md->dx == 0.0 || md->dy == 0.0)
        {
            fprintf(stderr, "Provided number lattice spacing (dx, dy) does not match to datadim: %d.\n\
            Try to match number of dimensions (dx, dy, dz) to (nx, ny, nz) and datadim.\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM2_D;
        }
        if (md->nz == 0)
        {
            md->nz = 1;
            md->dz = 1.0;
        }
    }
    else if (md->datadim == 1)
    {
        if (md->nx == 0)
        {
            fprintf(stderr, "Provided number of dimensions (nx) does not match to datadim: %d.\n\
            Try to match number of dimensions (nx, ny, nz) and datadim to binary file you want to use\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM1_N;
        }
        if (md->dx == 0.0)
        {
            fprintf(stderr, "Provided number lattice spacing (dx) does not match to datadim: %d.\n\
            Try to match number of dimensions (dx, dy, dz) to (nx, ny, nz) and datadim.\n",
                    md->datadim);
            return WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM1_D;
        }
        if (md->ny == 0)
        {
            md->ny = 1;
            md->dy = 1.0;
        }
        if (md->nz == 0)
        {
            md->nz = 1;
            md->dz = 1.0;
        }
    }
    // See if prefix is provided
    else if (strlen(md->prefix) == 0)
    {
        fprintf(stderr, "There is no prefix included. Prefix for files belonging to binary files\
        must have names of a format: prefix_variable-name.format\n");
        return WDATA_ERROR_WTXTFILE_INCOMPLETE_PREFIX;
    }

    return EXIT_SUCCESS;
}

int wdata_parse_metadata_file(const char *file_name, wdata_metadata *md)
{
    int ivars, ierr, itag = 0;

    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL)
        return 1; // Cannot open file

    // reset vars
    md->nvar = 0;
    md->nlink = 0;
    md->nconsts = 0;

    // buffers
    char s[MAX_REC_LEN];
    char tag[MAX_REC_LEN];
    char ptag[MAX_REC_LEN];

    // default values
    md->nx = 0;
    md->ny = 0;
    md->nz = 0;
    md->dx = 0.0;
    md->dy = 0.0;
    md->dz = 0.0;
    md->datadim = 0;
    md->prefix[0] = '\0';

    while (fgets(s, MAX_REC_LEN, fp) != NULL)
    {
        // Read first element of line
        tag[0] = '#';
        tag[1] = '\0';
        sscanf(s, "%s %*s", tag);
        str_tolower(tag);

        // Loop over known tags;
        if (strcmp(tag, "#") == 0)
            continue;
        else if (strcmp(tag, "nx") == 0)
            sscanf(s, "%s %d %*s", tag, &md->nx);
        else if (strcmp(tag, "ny") == 0)
            sscanf(s, "%s %d %*s", tag, &md->ny);
        else if (strcmp(tag, "nz") == 0)
            sscanf(s, "%s %d %*s", tag, &md->nz);
        else if (strcmp(tag, "dx") == 0)
            sscanf(s, "%s %lf %*s", tag, &md->dx);
        else if (strcmp(tag, "dy") == 0)
            sscanf(s, "%s %lf %*s", tag, &md->dy);
        else if (strcmp(tag, "dz") == 0)
            sscanf(s, "%s %lf %*s", tag, &md->dz);
        else if (strcmp(tag, "datadim") == 0)
            sscanf(s, "%s %d %*s", tag, &md->datadim);
        else if (strcmp(tag, "prefix") == 0)
            sscanf(s, "%s %s %*s", tag, md->prefix);
        else if (strcmp(tag, "cycles") == 0)
            sscanf(s, "%s %d %*s", tag, &md->cycles);
        else if (strcmp(tag, "dt") == 0)
            sscanf(s, "%s %lf %*s", tag, &md->dt);
        else if (strcmp(tag, "t0") == 0)
            sscanf(s, "%s %lf %*s", tag, &md->t0);

        // variables
        else if (strcmp(tag, "var") == 0)
        {
            itag = 0;
            for (ivars = 0; ivars < 4; ivars++)
            {
                ierr = sscanf(s, "%s %s %*s", tag, ptag);
                if (ierr < 2)
                    break;
                if (ptag[0] == '#')
                    break;
                if (itag == 0)
                    sprintf(md->var[md->nvar].name, "%s", ptag);
                else if (itag == 1)
                    sprintf(md->var[md->nvar].type, "%s", ptag);
                else if (itag == 2)
                {
                    if (strcmp(ptag, "wdat") == 0 || strcmp(ptag, "npy") == 0 || strcmp(ptag, "dpca") == 0)
                    {
                        sprintf(md->var[md->nvar].unit, "none");
                        sprintf(md->var[md->nvar].format, "%s", ptag);
                        itag++;
                        break;
                    }
                    else
                        sprintf(md->var[md->nvar].unit, "%s", ptag);
                }
                else if (itag == 3)
                {
                    if (strcmp(ptag, "wdat") == 0 || strcmp(ptag, "npy") == 0 || strcmp(ptag, "dpca") == 0)
                        sprintf(md->var[md->nvar].format, "%s", ptag);
                    else if (strcmp(ptag, "wdat") != 0)
                    {
                        fprintf(stderr, "WData format does not support %s. Consider using `wdat`, `npy` or `dpca`.\n", ptag);
                        return WDATA_ERROR_WTXTFILE_NOTSUPPORTED_VAR_FORMAT;
                    }
                    else if (strcmp(ptag, "npy") != 0)
                    {
                        fprintf(stderr, "WData format does not support %s. Consider using `wdat`, `npy` or `dpca`.\n", ptag);
                        return WDATA_ERROR_WTXTFILE_NOTSUPPORTED_VAR_FORMAT;
                    }
                    else if (strcmp(ptag, "dpca") != 0)
                    {
                        fprintf(stderr, "WData format does not support %s. Consider using `wdat`, `npy` or `dpca`.\n", ptag);
                        return WDATA_ERROR_WTXTFILE_NOTSUPPORTED_VAR_FORMAT;
                    }
                }
                replace_str(s, ptag, " ");
                //                 printf("itag: %d", itag);
                itag++;
            }
            if (itag < 2)
            {
                fprintf(stderr, "Not enough number of tags in `var` (less than 2). We emphasize that the non recquired one is the `unit` and `format` tag.\n");
                return WDATA_ERROR_WTXTFILE_INCOMPLETE_VAR_ITAG2;
            }
            if (itag == 2)
            {
                sprintf(md->var[md->nvar].unit, "none");
                sprintf(md->var[md->nvar].format, "wdat");
            }
            md->nvar++;
        }

        // links
        else if (strcmp(tag, "link") == 0)
        {
            sscanf(s, "%s %s %s %*s", tag, md->link[md->nlink].name, md->link[md->nlink].linkto);
            md->nlink++;
        }

        // consts
        else if (strcmp(str_tolower(tag), "const") == 0)
        {
            itag = 0;
            for (ivars = 0; ivars < 3; ivars++)
            {
                ierr = sscanf(s, "%s %s %*s", tag, ptag);
                if (ierr < 2)
                    break;
                if (ptag[0] == '#')
                    break;
                if (itag == 0)
                    sprintf(md->consts[md->nconsts].name, "%s", ptag);
                else if (itag == 1)
                    md->consts[md->nconsts].value = atof(ptag);
                else if (itag == 2)
                    sprintf(md->consts[md->nconsts].unit, "%s", ptag);
                replace_str(s, ptag, " ");
                itag++;
            }
            if (itag < 2)
            {
                fprintf(stderr, "Not enough number of tags in `consts` (less than 2). We emphasize that the only non recquired one is the `unit` tag.\n");
                return WDATA_ERROR_WTXTFILE_INCOMPLETE_CONST_ITAG2;
            }
            if (itag == 2)
                sprintf(md->consts[md->nconsts].unit, "none");
            md->nconsts++;
        }
    }

    fclose(fp);
    // set working dir to be consistent with wtxt file location
    char ctmp[MD_CHAR_LGTH];
    strcpy(ctmp, file_name);
#ifdef _WIN32
    /* KSB 9-22-2021, Windows doesn't have 'dirname', so use _splitpath_s.*/
    char dtmp[MD_CHAR_LGTH];
    if (_splitpath_s(ctmp, NULL, 0, dtmp, MD_CHAR_LGTH, NULL, 0, NULL, 0) == 0)
        strcpy(md->wrkdir, dtmp);
#else
    strcpy(md->wrkdir, dirname(ctmp));
#endif
    md->issetwrkdir = WRKDIR_SET;

    // check corretness
    ierr = check_metadata(md);
    if (ierr != EXIT_SUCCESS)
        return ierr;

    return EXIT_SUCCESS;
}

void wdata_print_metadata(wdata_metadata *md, FILE *out)
{
    int i;
    fprintf(out, "# W-DATA %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    fprintf(out, "# More information about the WData format see: https://gitlab.fizyka.pw.edu.pl/wtools/wdata/-/wikis/home\n");

    fprintf(out, "nx %24d   # lattice\n", md->nx);
    fprintf(out, "ny %24d   # lattice\n", md->ny);
    fprintf(out, "nz %24d   # lattice\n", md->nz);
    fprintf(out, "dx %24g   # spacing\n", md->dx);
    fprintf(out, "dy %24g   # spacing\n", md->dy);
    fprintf(out, "dz %24g   # spacing\n", md->dz);
    fprintf(out, "datadim %19d   # dimension of block size: 1=nx, 2=nx*ny, 3=nx*ny*nz\n", md->datadim);
    fprintf(out, "prefix %20s   # prefix for files belonging to this data set, binary files have names `prefix_variable-name.format`\n", md->prefix);
    fprintf(out, "cycles %20d   # number of cycles (measurements)\n", md->cycles);
    fprintf(out, "t0 %24g   # time value for the first cycle\n", md->t0);
    fprintf(out, "dt %24g   # time interval between cycles. If `dt` negative then time step is varying and is read from binary file `prefix__t.wdata`\n", md->dt);

    // variables
    fprintf(out, "\n");
    fprintf(out, "# variables\n");
    fprintf(out, "# tag                  name                    type                    unit                  format\n");
    for (i = 0; i < md->nvar; i++)
        wdata_print_variable(&md->var[i], out);

    // links
    fprintf(out, "\n");
    fprintf(out, "# links\n");
    fprintf(out, "# tag                  name                 link-to\n");
    for (i = 0; i < md->nlink; i++)
        wdata_print_link(&md->link[i], out);

    // consts
    fprintf(out, "\n");
    fprintf(out, "# consts\n");
    fprintf(out, "# tag                  name                   value                    unit\n");
    for (i = 0; i < md->nconsts; i++)
        wdata_print_const(&md->consts[i], out);

    fprintf(out, "\n");
}

void wdata_print_variable(wdata_variable *md, FILE *out)
{
    fprintf(out, "var%24s%24s%24s%24s\n", md->name, md->type, md->unit, md->format);
}

void wdata_print_link(wdata_link *md, FILE *out)
{
    fprintf(out, "link%23s%24s\n", md->name, md->linkto);
}

void wdata_print_const(wdata_const *md, FILE *out)
{
    fprintf(out, "const%22s%24g%24s\n", md->name, md->value, md->unit);
}

void wdata_add_variable(wdata_metadata *md, wdata_variable *var)
{
    md->var[md->nvar] = *var;
    md->nvar++;
}

void wdata_add_variable_many(wdata_metadata *md, wdata_variable_many *var_many)
{
    int i = 0;
    wdata_variable wdata_vars;
    while (var_many->names[i] != NULL)
    {
        strcpy(wdata_vars.name, var_many->names[i]);
        strcpy(wdata_vars.type, var_many->types[i]);
        strcpy(wdata_vars.unit, var_many->units[i]);
        strcpy(wdata_vars.format, var_many->formats[i]);

        wdata_add_variable(md, &wdata_vars);
        i++;
    }
}

void wdata_add_link(wdata_metadata *md, wdata_link *link)
{
    md->link[md->nlink] = *link;
    md->nlink++;
}

void wdata_add_const(wdata_metadata *md, wdata_const *_const)
{
    md->consts[md->nconsts] = *_const;
    md->nconsts++;
}

int wdata_get_blocklength(wdata_metadata *md)
{
    if (md->datadim == 3)
        return md->nx * md->ny * md->nz;
    if (md->datadim == 2)
        return md->nx * md->ny;
    if (md->datadim == 1)
        return md->nx;
    return -1; // error!!!
}

int wdata_set_time(const wdata_metadata *md, const int icycle, double *current_time)
{
    if (md->dt >= 0)
    {
        *current_time = md->t0 + md->dt * icycle;
        return 0;
    }
    else
    {
        char file_name[MD_CHAR_LGTH];
        sprintf(file_name, "%s_%s.wdat", md->prefix, "_t");
        const size_t count = 1;

        FILE *pFile;

        pFile = fopen(file_name, "ab");
        if (pFile == NULL)
            return 1; // cannot open

        size_t test_ele = fwrite(current_time, sizeof(double), count, pFile);
        if (test_ele != count)
            return 2; // data not written

        fclose(pFile);

        return 0;
    }
}

int wdata_get_time(const wdata_metadata *md, const int icycle, double *current_time)
{
    if (md->dt >= 0)
    {
        *current_time = md->t0 + md->dt * icycle;
        return 0;
    }
    else
    {
        char file_name[MD_CHAR_LGTH];
        sprintf(file_name, "%s_%s.wdat", md->prefix, "_t");
        const size_t count = 1;

        FILE *pFile;

        pFile = fopen(file_name, "rb");
        if (pFile == NULL)
            return 1; // cannot open

        fseek(pFile, sizeof(double) * icycle, SEEK_SET);

        size_t test_ele = fread(current_time, sizeof(double), count, pFile);
        if (test_ele != count)
            return 2; // data not written

        fclose(pFile);

        return 0;
    }
}

size_t wdata_get_blocksize(wdata_metadata *md, wdata_variable *var)
{
    if (strcmp(var->type, "real") == 0 || strcmp(var->type, "real8") == 0)
        return sizeof(double) * wdata_get_blocklength(md);
    if (strcmp(var->type, "real4") == 0)
        return sizeof(float) * wdata_get_blocklength(md);
    if (strcmp(var->type, "complex") == 0 || strcmp(var->type, "complex16") == 0)
        return sizeof(double) * wdata_get_blocklength(md) * 2;
    if (strcmp(var->type, "complex8") == 0)
        return sizeof(float) * wdata_get_blocklength(md) * 2;
    if (strcmp(var->type, "vector") == 0 || strcmp(var->type, "vector8") == 0)
        return sizeof(double) * wdata_get_blocklength(md) * 3;
    if (strcmp(var->type, "vector4") == 0)
        return sizeof(float) * wdata_get_blocklength(md) * 3;

    return 0; // error
}

int wdata_add_datablock(wdata_metadata *md, wdata_variable *var, void *data)
{
    wdata_goto_wrkdir(md);

    int ierr = -1;
    if (strcmp(var->format, "wdat") == 0)
        ierr = wdata_add_datablock_wdat(md, var, data);
    else if (strcmp(var->format, "dpca") == 0)
        ierr = wdata_add_datablock_dpca(md, var, data);
    else if (strcmp(var->format, "npy") == 0)
        ierr = wdata_add_datablock_npy(md, var, data);

    wdata_goback_wrkdir(md);

    return ierr; // error code
}

int wdata_add_datablock_wdat(wdata_metadata *md, wdata_variable *var, void *data)
{
    char file_name[MD_CHAR_LGTH];
    wdata_get_filename(md, var, file_name);
    //     printf("wdata_add_datablock: file_name=%s\n", file_name);

    FILE *pFile;

    pFile = fopen(file_name, "ab");
    if (pFile == NULL)
        return 1; // cannot open

    size_t test_ele = fwrite(data, wdata_get_blocksize(md, var), 1, pFile);
    if (test_ele != 1)
        return 2; // data not written

    fclose(pFile);

    return 0;
}

int wdata_add_datablock_dpca(wdata_metadata *md, wdata_variable *var, void *data)
{
    return -99; // pdca format is deprecated, thus writing files in this format is no longer supported!
}

int wdata_add_datablock_npy(wdata_metadata *md, wdata_variable *var, void *data)
{
    return -99; // TODO
}

int wdata_load_datablock(wdata_metadata *md, wdata_variable *var, int cycle, void *data)
{
    wdata_goto_wrkdir(md);

    int ierr = -1;
    if (strcmp(var->format, "wdat") == 0)
        ierr = wdata_load_datablock_wdat(md, var, cycle, data);
    else if (strcmp(var->format, "dpca") == 0)
        ierr = wdata_load_datablock_dpca(md, var, cycle, data);
    else if (strcmp(var->format, "npy") == 0)
        ierr = wdata_load_datablock_npy(md, var, cycle, data);

    wdata_goback_wrkdir(md);

    return ierr; // error code
}

int wdata_load_datablock_wdat(wdata_metadata *md, wdata_variable *var, int cycle, void *data)
{
    char file_name[MD_CHAR_LGTH];
    wdata_get_filename(md, var, file_name);
    //     printf("wdata_read_cycle: Reading from file %s\n", file_name);

    FILE *pFile;

    pFile = fopen(file_name, "rb");
    if (pFile == NULL)
        return 1; // cannot open

    // set pointer to correct location
    if (fseek(pFile, wdata_get_blocksize(md, var) * cycle, SEEK_SET) != 0)
        return 3; // cannot seek pointer

    size_t test_ele = fread(data, wdata_get_blocksize(md, var), 1, pFile);
    if (test_ele != 1)
        return 2; // data not read

    fclose(pFile);

    return 0;
}

int wdata_load_datablock_dpca(wdata_metadata *md, wdata_variable *var, int cycle, void *data)
{
    char file_name[MD_CHAR_LGTH];
    wdata_get_filename(md, var, file_name);
    //     printf("wdata_read_cycle: Reading from file %s\n", file_name);

    FILE *pFile;

    pFile = fopen(file_name, "rb");
    if (pFile == NULL)
        return 1; // cannot open

// set pointer to correct location
#define MIO_CNT_INTS 4
#define MIO_CNT_double 6
    size_t header_size = (MIO_CNT_INTS + 1) * sizeof(int) + MIO_CNT_double * sizeof(double);
    if (fseek(pFile, header_size + wdata_get_blocksize(md, var) * cycle, SEEK_SET) != 0)
        return 3; // cannot seek pointer

    size_t test_ele = fread(data, wdata_get_blocksize(md, var), 1, pFile);
    if (test_ele != 1)
        return 2; // data not read

    fclose(pFile);

    return 0;
}

int wdata_load_datablock_npy(wdata_metadata *md, wdata_variable *var, int cycle, void *data)
{
    return -99; // TODO
}

int wdata_write_cycle(wdata_metadata *md, const char *varname, void *data)
{
    int ierr;
    wdata_variable var;
    ierr = wdata_get_variable(md, varname, &var);
    if (ierr > 0)
        return 10 + ierr;

    return wdata_add_datablock(md, &var, data);
}

int wdata_write_cycle_many(wdata_metadata *md, const wdata_variable_many *var_many, void *data[])
{
    int i, ierr;
    wdata_variable var;

    for (i = 0; i < md->nvar; i++)
    {
        ierr = wdata_get_variable(md, var_many->names[i], &var);
        if (ierr > 0)
        {
            fprintf(stderr, "Problem with `wdata_get_variable()` in `wdata_write_cycle_many()` function with ERROR: %d. Varname: %s\n", ierr, var_many->names[i]);
            return WDATA_ERROR_WDATA_GET_VARIABLE_WCM;
        }
        if (wdata_add_datablock(md, &var, data[i]))
        {
            fprintf(stderr, "Problem with `wdata_add_datablock()` in `wdata_write_cycle_many()` function with ERROR: %d. Varname: %s\n", ierr, var_many->names[i]);
            return WDATA_ERROR_WDATA_ADD_DATABLOCK_WCM;
        }
    }
    return EXIT_SUCCESS;
}

int wdata_read_cycle(wdata_metadata *md, const char *varname, int cycle, void *data)
{
    int ierr;
    wdata_variable var;
    ierr = wdata_get_variable(md, varname, &var);
    if (ierr > 0)
        return 10 + ierr;

    return wdata_load_datablock(md, &var, cycle, data);
}

int wdata_add_cycle(wdata_metadata *md)
{
    md->cycles++;
    return EXIT_SUCCESS;
}

void wdata_get_filename(wdata_metadata *md, wdata_variable *var, char *file_name)
{
    if (strcmp(var->format, "wdat") == 0)
        sprintf(file_name, "%s_%s.wdat", md->prefix, var->name);
    else if (strcmp(var->format, "dpca") == 0)
        sprintf(file_name, "%s_%s.dpca", md->prefix, var->name);
    else if (strcmp(var->format, "npy") == 0)
        sprintf(file_name, "%s_%s.npy", md->prefix, var->name);
}

int wdata_get_variable(wdata_metadata *md, const char *varname, wdata_variable *var)
{
    int i;
    char tvarname[MD_CHAR_LGTH];      // target variable name
    sprintf(tvarname, "%s", varname); // copy to tvarname

    // check is links redirects
    for (i = 0; i < md->nlink; i++)
        if (strcmp(md->link[i].name, varname) == 0)
            sprintf(tvarname, "%s", md->link[i].linkto);

    // find variable
    for (i = 0; i < md->nvar; i++)
        if (strcmp(md->var[i].name, tvarname) == 0)
        {
            *var = md->var[i];
            return EXIT_SUCCESS;
        }

    // cannot find variable
    return 1;
}

int wdata_get_const(wdata_metadata *md, const char *constname, wdata_const *_const)
{
    int i;
    for (i = 0; i < md->nconsts; i++)
        if (strcmp(md->consts[i].name, constname) == 0)
        {
            *_const = md->consts[i];
            return EXIT_SUCCESS;
        }

    // cannot find const;
    return EXIT_FAILURE;
}

double wdata_getconst_value(wdata_metadata *md, const char *constname)
{
    int ierr = 0;
    wdata_const _const;
    ierr = wdata_get_const(md, constname, &_const);
    if (ierr == 0)
        return _const.value;
    else
        return 0.0;
}

double wdata_getconst(wdata_metadata *md, const char *constname)
{
    int i;
    for (i = 0; i < md->nconsts; i++)
        if (strcmp(md->consts[i].name, constname) == 0)
            return md->consts[i].value;

    return 0.0;
}

void wdata_setconst(wdata_metadata *md, const char *constname, double constvalue)
{
    int i;
    for (i = 0; i < md->nconsts; i++)
        if (strcmp(md->consts[i].name, constname) == 0)
        {
            md->consts[i].value = constvalue;
            return;
        }

    wdata_const _const;
    strcpy(_const.name, constname);
    strcpy(_const.unit, "none");
    _const.value = constvalue;
    md->consts[md->nconsts] = _const;
    md->nconsts++;

    return;
}

void wdata_setconst_unit(wdata_metadata *md, const char *constname, double constvalue, const char *unit)
{
    int i;
    for (i = 0; i < md->nconsts; i++)
        if (strcmp(md->consts[i].name, constname) == 0)
        {
            md->consts[i].value = constvalue;
            return;
        }

    wdata_const _const;
    strcpy(_const.name, constname);
    strcpy(_const.unit, unit);
    _const.value = constvalue;
    md->consts[md->nconsts] = _const;
    md->nconsts++;

    return;
}

int wdata_file_exists(wdata_metadata *md, const char *varname)
{
    wdata_goto_wrkdir(md);

    int ierr;
    wdata_variable var;
    ierr = wdata_get_variable(md, varname, &var);
    if (ierr != 0)
        return 0;

    char file_name[MD_CHAR_LGTH];
    wdata_get_filename(md, &var, file_name);

    FILE *file;
    if ((file = fopen(file_name, "r")))
    {
        fclose(file);
        return 1;
    }

    wdata_goback_wrkdir(md);

    return 0;
}

void wdata_clear_file(wdata_metadata *md, const char *varname)
{
    wdata_goto_wrkdir(md);

    char file_name[MD_CHAR_LGTH];
    sprintf(file_name, "%s_%s.wdat", md->prefix, varname);

    remove(file_name);

    wdata_goback_wrkdir(md);
}

void wdata_clear_database(wdata_metadata *md)
{
    wdata_goto_wrkdir(md);

    char file_name[MD_CHAR_LGTH];
    int i;
    for (i = 0; i < md->nvar; i++)
    {
        wdata_get_filename(md, &md->var[i], file_name);
        remove(file_name);
    }
    md->cycles = 0;

    wdata_goback_wrkdir(md);
}

int wdata_write_metadata_to_file(wdata_metadata *md, const char *filename)
{
    char file_name[MD_CHAR_LGTH];
    if (strcmp(filename, "") == 0)
        sprintf(file_name, "%s.wtxt", md->prefix);
    else
        sprintf(file_name, "%s", filename);

    //     printf("creating %s\n", file_name);
    FILE *fout = fopen(file_name, "w");
    if (fout == NULL)
        return EXIT_FAILURE;
    else
    {
        wdata_print_metadata(md, fout);
        fclose(fout);
        return EXIT_SUCCESS;
    }
}

void wdata_set_working_dir(wdata_metadata *md, const char *wrkdir)
{
    strcpy(md->wrkdir, wrkdir);
    md->issetwrkdir = WRKDIR_SET;
}

int wdata_add_var_to_metadata_file(const char *file_name, wdata_variable *var)
{
    FILE *fout = fopen(file_name, "a");
    if (fout == NULL)
        return 1;
    wdata_print_variable(var, fout);
    fclose(fout);
    return 0;
}

int wdata_add_link_to_metadata_file(const char *file_name, wdata_link *link)
{
    FILE *fout = fopen(file_name, "a");
    if (fout == NULL)
        return 1;
    wdata_print_link(link, fout);
    fclose(fout);
    return 0;
}

int wdata_add_const_to_metadata_file(const char *file_name, wdata_const *_const)
{
    FILE *fout = fopen(file_name, "a");
    if (fout == NULL)
        return 1;
    wdata_print_const(_const, fout);
    fclose(fout);
    return 0;
}

int wdata_add_comment_to_metadata_file(const char *file_name, const char *comment)
{
    FILE *fout = fopen(file_name, "a");
    if (fout == NULL)
        return 1;
    fprintf(fout, "# %s\n", comment);
    fclose(fout);
    return 0;
}

int wdata_has_variable(wdata_metadata *md, const char *varname)
{
    int i;

    // find variable
    for (i = 0; i < md->nvar; i++)
        if (strcmp(md->var[i].name, varname) == 0)
            return 1;

    // cannot find variable
    return 0;
}

int wdata_has_link(wdata_metadata *md, const char *linkname)
{
    int i;

    // find variable
    for (i = 0; i < md->nlink; i++)
        if (strcmp(md->link[i].name, linkname) == 0)
            return 1;

    // cannot find variable
    return 0;
}

int wdata_has_const(wdata_metadata *md, const char *constname)
{
    int i;

    // find variable
    for (i = 0; i < md->nconsts; i++)
        if (strcmp(md->consts[i].name, constname) == 0)
            return 1;

    // cannot find variable
    return 0;
}
