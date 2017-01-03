/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                               visit-hdf5.h                                //
// ************************************************************************* //

#ifndef VISIT_HDF5_H 
#define VISIT_HDF5_H 
#include <string.h>
#include <stdlib.h>

//
// Useful macro for comparing HDF5 versions
//
#define HDF5_VERSION_GE(Maj,Min,Rel)  \
        (((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR==Min) && (H5_VERS_RELEASE>=Rel)) || \
         ((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR>Min)) || \
         (H5_VERS_MAJOR>Maj))

//
// Macros to re-map H5Fopen and H5Fclose so that HDF5 files are always handled
// using the H5F_CLOSE_SEMI property. This permits us to detect when attempts
// to close files fail due to a buggy plugin (e.g. one that fails to close all
// the objects in a file). This is a problematic issue when multiple HDF5
// plugins are attempted on the same HDF5 file and a buggy plugin winds up
// accidentally leaving the file open.
//
#ifndef VISIT_DONT_REDEFINE_H5FOPENCLOSE
#ifdef H5_VERS_MAJOR
#include <avtCallback.h>
static void VisIt_IssueH5Warning(int phase)
{
    static bool haveIssuedOpenWarning = false;
    static bool haveIssuedCloseWarning = false;
    char msg[512];
    if (phase == 0 && !haveIssuedOpenWarning)
    {
#ifndef NDEBUG
        SNPRINTF(msg, sizeof(msg), "Detected attempt to open an HDF5 file without H5F_CLOSE_SEMI.\n"
            "Please contact VisIt developers to have this issue fixed.");
        haveIssuedOpenWarning = true;
        if (!avtCallback::IssueWarning(msg))
           cerr << msg << endl;
#endif
    }
    else if (phase == 1 && !haveIssuedCloseWarning)
    {
        SNPRINTF(msg, sizeof(msg), "An attempt to close an HDF5 file failed, incidating a bug in the plugin.\n"
            "Please contact VisIt developers to have this issue fixed.");
        haveIssuedCloseWarning = true;
        if (!avtCallback::IssueWarning(msg))
           cerr << msg << endl;
    }

}
static hid_t VisIt_H5Fopen(const char *name, int flags, hid_t fapl)
{
    bool created_fapl = false;

    if (fapl == H5P_DEFAULT)
    {
        // Issue error message indicating plugin is using default open
        VisIt_IssueH5Warning(0);

        fapl = H5Pcreate(H5P_FILE_ACCESS);
        created_fapl = true;

        // Build a fapl with semi close behavior
        H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);
    }
    else
    {
        H5F_close_degree_t cd;
        H5Pget_fclose_degree(fapl, &cd);
        if (cd != H5F_CLOSE_SEMI)
        {
            // Issue message if not
            VisIt_IssueH5Warning(0);

            // copy the fapl
            fapl = H5Pcopy(fapl);
            created_fapl = true;

            // Add semi close property if not
            H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);
        }
    }

    hid_t retval = H5Fopen(name, flags, fapl);

    if (created_fapl) H5Pclose(fapl);

    return retval;
}

static herr_t VisIt_H5Fclose(hid_t fid)
{
#ifndef VISIT_DONT_CHECK_H5OPENOBJECTS
    static bool haveIssuedOpenObjectsWarning = false;
    const unsigned int obj_flags = H5F_OBJ_LOCAL | H5F_OBJ_DATASET |
        H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR;

    /* Check for any open objects in this file */
    int noo = H5Fget_obj_count(fid, obj_flags);
    if (noo > 0 && !haveIssuedOpenObjectsWarning)
    {
        int n;
        char msg[4096];
        char msg2[8192];
        hid_t *ooids = (hid_t *) malloc(noo * sizeof(hid_t));
        SNPRINTF(msg, sizeof(msg), "Internal plugin error: %d objects left open in file: ", noo);
#if HDF5_VERSION_GE(1,6,5)
        H5Fget_obj_ids(fid, obj_flags, noo, ooids);
#else
        H5Fget_obj_ids(fid, obj_flags, noo, ooids);
#endif
        n = strlen(msg);
        for (int i = 0; i < noo && (size_t)n < sizeof(msg); i++)
        {
            char name[256], tmp[256];
            H5Iget_name(ooids[i], name, sizeof(name));
            SNPRINTF(tmp, sizeof(tmp), "\n    \"%.230s\" (id=%d)", name, ooids[i]);
            if ((strlen(msg) + strlen(tmp) + 1) >= sizeof(msg))
                break;
            strcat(msg, tmp);
            n += strlen(tmp);
        }
        free(ooids);
        SNPRINTF(msg2, sizeof(msg2), "The HDF5 library indicates the plugin has left the following "
            "objects in the file open...%s", msg);
        if (!avtCallback::IssueWarning(msg2))
           cerr << msg2 << endl;
        haveIssuedOpenObjectsWarning = true;
    }
#endif
    herr_t err = H5Fclose(fid);
    if (err < 0)
        VisIt_IssueH5Warning(1);
    return err;
}

#define H5Fopen(NAME, FLAGS, FAPL) VisIt_H5Fopen(NAME, FLAGS, FAPL)
#define H5Fclose(FID) VisIt_H5Fclose(FID)
#endif
#endif

#endif
