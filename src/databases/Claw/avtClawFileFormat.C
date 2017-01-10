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
//                            avtClawFileFormat.C                           //
// ************************************************************************* //

#include <avtClawFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>

#include <snprintf.h>
#include <DebugStream.h>
#include <Expression.h>
#include <StringHelpers.h>
#include <DataNode.h>
#include <FileFunctions.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <ImproperUseException.h>

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include <io.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <sstream>

using std::map;
using std::string;
using std::vector;
using std::ostringstream;

// ****************************************************************************
//  Function: InitTimeHeader 
//
//  Purpose: Initializes a time header 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 18, 2007 
//
// ****************************************************************************
static void
InitTimeHeader(TimeHeader_t *hdr)
{
    hdr->time = -1.0;
    hdr->meqn = -1;
    hdr->ngrids = -1;
    hdr->naux = -1;
    hdr->ndims = -1;
}

// ****************************************************************************
//  Function: GetFilenames
//
//  Purpose: Given a directory to search and either a scanf style pattern or
//  a regular expression style pattern, find all filenames in the directory
//  that match the given pattern
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications
//    Mark C. Miller, Mon Jul 12 16:03:24 PDT 2010
//    Replaced use of StringHelpers::FindRE with ExtractRESubstring. Using
//    the former was simply a bug.
//
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use FileFunctions::ReadAndProcessDirectory in conjunction with
//    ClawAppendMatchingFiles callback function for cross-platform portability.
//
// ****************************************************************************

struct claw_file_match_struct
{
    const int *nExpectedMatches;
    const string *scanfStr;
    const string *regexStr;
    vector<string> *fnames;
};


void
ClawAppendMatchingFiles(void *cbData, const std::string &fullFileName,
                        bool isDir, bool canAccess, long fileSize)
{
    if (isDir)
        return;

    claw_file_match_struct *ptr = (claw_file_match_struct *)cbData;
    int nexpectedMatches(*ptr->nExpectedMatches);
    string scanfStr(*ptr->scanfStr);
    string regexStr(*ptr->regexStr);

    // fullFileName may contain path, so remove it.
    string fileName(FileFunctions::Basename(fullFileName));

    // use either scanf pattern or regex pattern to match the entry
    if (scanfStr != "")
    {
        // we use this funky scanf call because we don't really
        // care about the data, here. Just whether we can get
        // a match. But, we'll need a place for scanf to store
        // anything it assigns and so we just stick it into dummy
        char dummyStr[2048];
        int nmatch = sscanf(fileName.c_str(), scanfStr.c_str(),
                         (void*) dummyStr, (void*) dummyStr, (void*) dummyStr,
                         (void*) dummyStr, (void*) dummyStr, (void*) dummyStr,
                         (void*) dummyStr, (void*) dummyStr, (void*) dummyStr,
                         (void*) dummyStr, (void*) dummyStr, (void*) dummyStr,
                         (void*) dummyStr, (void*) dummyStr, (void*) dummyStr,
                         (void*) dummyStr);
        if (nmatch == nexpectedMatches)
        {
            ptr->fnames->push_back(fileName);
            debug5 << "   Added \"" << fileName << "\"" << endl;
        }
    }
    else if (regexStr != "")
    {
        if (StringHelpers::ExtractRESubstr(fileName.c_str(), regexStr.c_str()) != "")
        {
            ptr->fnames->push_back(fileName);
            debug5 << "   Added \"" << fileName << "\"" << endl;
        }
    }
}


static int
GetFilenames(string scanfStr, string regexStr, string rootDir,
    vector<string> &fnames)
{
    // if we're using scanf's, compute number of matches we should get
    int nexpectedMatches = 0;
    if (scanfStr != "")
    {
        for (size_t i = 0; i < scanfStr.size()-1; i++)
        {
            // all '%' except '%%' indicate an argument conversion specifier
            if (scanfStr[i] == '%' && scanfStr[i+1] != '%')
                nexpectedMatches++;
        }

        // in the scanf below, we assume a max of 16 conversion specifiers
        if (nexpectedMatches > 16)
        {
            char msg[256];
            SNPRINTF(msg, sizeof(msg), "scanf pattern contains %d conversion "
                "specifiers. Max is 16", nexpectedMatches);
            EXCEPTION1(ImproperUseException, msg);
        }
    }
    claw_file_match_struct cbData;
    cbData.nExpectedMatches = &nexpectedMatches;
    cbData.scanfStr = &scanfStr;
    cbData.regexStr = &regexStr;
    cbData.fnames = &fnames;
    FileFunctions::ReadAndProcessDirectory(rootDir, ClawAppendMatchingFiles,
                                          (void*)&cbData, false);

    return (int)fnames.size();
}

// ****************************************************************************
//  Struct: FileNameAndRank_t
//
//  Purpose: Container to pair filename with its sort rank 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Mark C. Miller, Mon Jul 12 16:04:23 PDT 2010
//    Replaced C++ string object fname with int origIndex as use of C++ object
//    in the context of qsort where bit-for-bit copy is used is shady at best.
//
// ****************************************************************************
typedef struct
{
    int origIndex;
    double rank;
} FileNameAndRank_t;

// ****************************************************************************
//  Function: CompareFNR 
//
//  Purpose: Comparison function to support qsort() in SortFilenames 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Mark C. Miller, Mon Jul 12 16:05:44 PDT 2010
//    Replaced clear and re-build of fnames with build of copy and assign.
//
// ****************************************************************************
static int
CompareFNR(const void *a1, const void *a2)
{
    FileNameAndRank_t *p1 = (FileNameAndRank_t *) a1;
    FileNameAndRank_t *p2 = (FileNameAndRank_t *) a2;
    if (p1->rank < p2->rank)
        return -1;
    else if (p1->rank > p2->rank)
        return 1;
    else
        return 0;
}

// ****************************************************************************
//  Function: SortFilenames
//
//  Purpose: Given a list of files and an optional regular expression to 
//  extract cycle numbers from filenames, this function will sort files
//  according to the extracted cycle numbers or, if the cycleRegex is not
//  specified, according to the modification time returned by fstat
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//
//    Mark C. Miller, Wed Aug  6 09:50:51 PDT 2008
//    Made it check for possible stat error before assigning result
//     
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use VISIT_SLASH_STRING when appending fname to rootDir.
//
// ****************************************************************************

static void
SortFilenames(vector<string> &fnames, string cycleRegex, string rootDir)
{
    size_t n = fnames.size();
    size_t i;

    FileNameAndRank_t *fnrs = new FileNameAndRank_t[n];

    // populate a list of filename/rank pairs where rank
    // is based on modification time from stat info or
    // from guessing cycle number from filename
    for (i = 0; i < n; i++)
    {
        double rank = -1.0;
        if (cycleRegex == "")
        {
            string fullFileName = rootDir + VISIT_SLASH_STRING + fnames[i];
            FileFunctions::VisItStat_t stbuf;
            if (FileFunctions::VisItStat(fullFileName.c_str(), &stbuf) == 0)
                rank = (double) stbuf.st_mtime;
        }
        else
        {
            rank = (double) avtFileFormat::GuessCycle(fnames[i].c_str(),
                                                      cycleRegex.c_str());
        }

        fnrs[i].origIndex = (int)i;
        fnrs[i].rank = rank; 
    }

    // sort this list
    qsort(fnrs, n, sizeof(FileNameAndRank_t), CompareFNR);

    // now, clear fnames and re-populate in new order
    vector<string> newfnames;
    debug5 << "Sorted list..." << endl;
    for (i = 0; i < n; i++)
    {
        newfnames.push_back(fnames[fnrs[i].origIndex]);
        debug5 << "   \"" << newfnames[i] << "\"" << endl;
    }
    fnames = newfnames;

    delete [] fnrs;

}

// ****************************************************************************
//  Function: ReadTimeStepHeader 
//
//  Purpose: Given a Claw timestep header file, this function reads it
//  using scanfs to some metadata.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Added support for 'ndims' header info
//
//    Jeremy Meredith, Thu Aug  7 15:54:10 EDT 2008
//    Use %ld format for longs.
//
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use VISIT_SLASH_STRING when appending fname to rootDir.
//    Added support for Windows.
//
// ****************************************************************************

static void 
ReadTimeStepHeader(string rootDir, string fileName, TimeHeader_t *hdr)
{
    char buf[2048];
    string fullFileName = rootDir + VISIT_SLASH_STRING + fileName;
    int fd =
#ifndef WIN32
        open(fullFileName.c_str(), O_RDONLY);
#else
        ::_open(fullFileName.c_str(), _O_RDONLY|_O_BINARY);
#endif
    int nread = 
#ifndef WIN32
        read(fd, buf, sizeof(buf)-1);
#else
        ::_read(fd, buf, sizeof(buf)-1);
#endif
    if (nread >= (int)sizeof(buf)-1)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Buffer size of %ld insufficient "
            "to read time header", sizeof(buf));
        EXCEPTION1(ImproperUseException, msg);
    }
    close(fd);
    buf[nread+1] = '\0';

    InitTimeHeader(hdr);

    int nscan = sscanf(buf, " %lf time\n %d meqn\n %d ngrids\n %d naux\n %d ndims",
                         &(hdr->time), &(hdr->meqn), &(hdr->ngrids), &(hdr->naux),
                         &(hdr->ndims));
    if (nscan != 5)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "scanf() matched only %d of 5 "
            "items in time header", nscan);
        EXCEPTION1(ImproperUseException, msg);
    }

    debug1 << "From time step header file \"" << fileName << "\"..." << endl;
    debug1 << "   time = " << hdr->time << endl;
    debug1 << "   meqn = " << hdr->meqn << endl;
    debug1 << "   ngrids = " << hdr->ngrids << endl;
    debug1 << "   naux = " << hdr->naux << endl;
    debug1 << "   ndims = " << hdr->ndims << endl;
}

// ****************************************************************************
//  Function: DataSegmentLengthInChars
//
//  Purpose: Compute the total length, in characters, of the portion of an
//  ascii Claw data file containing the data for a given grid. This assumes a
//  fixed columnar format of the ascii data.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
// ****************************************************************************
static int
DataSegmentLengthInChars(const GridHeader_t *ghdr, int ndims)
{
    // Compute the offset of next grid header.
    //
    // Here is the offset math:
    //     a) the value of 'offset' points to the first character
    //        of the first line of a grid header
    //     b) the value of i is 4 charcters *before* the
    //        first data line following grid header
    //     c) There are ghdr->charsPerLine characters in a single data line
    //        There will be mx * my (2d) or mx * my * mz (3d) of these.
    //        However, between each 'row' of mx data lines 
    //        there is a 'blank' line consisting of 3 characters,
    //        two spaces and a '\n', hence the 3*my term. For 3d, there
    //        is similar 'blank' line between each slice henc the 3*mz term.
    //     
    if (ndims == 2)
        return ghdr->my * (ghdr->mx * ghdr->charsPerLine + 3);
    else
        return ghdr->mz * (ghdr->my * (ghdr->mx * ghdr->charsPerLine + 3) + 3);
}

// ****************************************************************************
//  Function: ReadGridHeader
//
//  Purpose: Given a file descriptor and offset, seek to and attempt to read
//  the ascii grid header information. Also, on the basis of what is read for
//  the grid header, compute the offset to the next grid header.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
//
//    Mark C. Miller, Wed Aug  6 09:52:21 PDT 2008
//    Checked for number of values assigned by sscanf and issue error
//
//    Hank Childs, Sat Sep  4 21:49:51 PDT 2010
//    Fix off-by-one error that caused out-of-bounds array write.
//
// ****************************************************************************
static void 
ReadGridHeader(int fd, int offset, const TimeHeader_t* thdr, GridHeader_t *ghdr, int *nextoff)
{
    char buf[2048];

    // read in a buffer full of characters at the specified offset
    lseek(fd, offset, SEEK_SET);
    int nread = read(fd, buf, sizeof(buf)-1);
    buf[nread] = '\0';

    // scan the buffer using sscanf for grid header information
    if (thdr->ndims == 2)
    {
        int nscan = sscanf(buf, " %d grid_number\n %d AMR_level\n"
                    " %d mx\n %d my\n"
                    " %lf xlow\n %lf ylow\n"
                    " %lf dx\n %lf dy\n",
                    &(ghdr->grid_number), &(ghdr->AMR_level),
                    &(ghdr->mx), &(ghdr->my),
                    &(ghdr->xlow), &(ghdr->ylow),
                    &(ghdr->dx), &(ghdr->dy));
        if (nscan != 8)
        {
            EXCEPTION1(InvalidFilesException, "Unable to read grid header");
        }
    }
    else if (thdr->ndims == 3)
    {
        int nscan = sscanf(buf, " %d grid_number\n %d AMR_level\n"
                    " %d mx\n %d my\n %d mz\n"
                    " %lf xlow\n %lf ylow\n %lf zlow\n"
                    " %lf dx\n %lf dy\n %lf dz\n",
                    &(ghdr->grid_number), &(ghdr->AMR_level),
                    &(ghdr->mx), &(ghdr->my), &(ghdr->mz),
                    &(ghdr->xlow), &(ghdr->ylow), &(ghdr->zlow),
                    &(ghdr->dx), &(ghdr->dy), &(ghdr->dz));
        if (nscan != 11)
        {
            EXCEPTION1(InvalidFilesException, "Unable to read grid header");
        }
    }
    else
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Unsupported value of %d for 'ndims' "
            "in time header", thdr->ndims);
        EXCEPTION1(InvalidFilesException, msg);
    }

    // scan forward throug buf to just after end
    // of header to start of data. Last 2 lines are
    // either "dy\n\n" (2d) or "dz\n\n" (3d)
    char c = thdr->ndims == 2 ? 'y' : 'z';
    int i = 0;
    while (buf[i+0] != 'd' ||
           buf[i+1] != c ||
           buf[i+2] != '\n' ||
           buf[i+3] != '\n')
        i++;
    int j = i + 4; // the above while loop stops 4 before first data line
    ghdr->dataOffset = offset + j;

    // size of a data line 
    while (buf[j] != '\n')
        j++;
    int charsPerLine = j - i - 4 + 1;
    ghdr->charsPerLine = charsPerLine;

    // compute offset to next grid header
    *nextoff = offset + i + 4 + DataSegmentLengthInChars(ghdr, thdr->ndims);

    // some useful debugging output
    debug5 << "Grid header..." << endl;
    debug5 << "   grid_number = " << ghdr->grid_number << endl;
    debug5 << "   AMR_level = " << ghdr->AMR_level << endl;
    debug5 << "   mx = " << ghdr->mx << endl;
    debug5 << "   my = " << ghdr->my << endl;
    if (thdr->ndims == 3) {
        debug5 << "   mz = " << ghdr->mz << endl;
    }
    debug5 << "   xlow = " << ghdr->xlow << endl;
    debug5 << "   ylow = " << ghdr->ylow << endl;
    if (thdr->ndims == 3) {
        debug5 << "   zlow = " << ghdr->zlow << endl;
    }
    debug5 << "   dx = " << ghdr->dx << endl;
    debug5 << "   dy = " << ghdr->dy << endl;
    if (thdr->ndims == 3) {
        debug5 << "   dz = " << ghdr->dz << endl;
    }
    debug5 << "   charsPerLine = " << ghdr->charsPerLine << endl;
    debug5 << "   dataOffset = " << ghdr->dataOffset << endl;
}

// ****************************************************************************
//  Function: ReadGridHeaders
//
//  Purpose: Loop to leap-frog through a grid file and read all the grid
//  headers storing the information away.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 13, 2007 
//
//  Modifications:
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use VISIT_SLASH_STRING when appending fname to rootDir.
//    Added support for Windows.
//
// ****************************************************************************

static void 
ReadGridHeaders(string rootDir, string fileName, const TimeHeader_t *thdr,
    vector<GridHeader_t> &gridHeaders, map<int, GridHeader_t> &gridHeaderMap)
{
    // open a grid file
    //char buf[2048];
    string fullFileName = rootDir + VISIT_SLASH_STRING + fileName;
    int fd =
#ifndef WIN32
        open(fullFileName.c_str(), O_RDONLY);
#else
        ::_open(fullFileName.c_str(), _O_RDONLY|_O_BINARY);
#endif
    int offset = 0;
    int ng = 0;
    while (ng < thdr->ngrids)
    {
        int nextoff = 0;
        GridHeader_t ghdr;
        ReadGridHeader(fd, offset, thdr, &ghdr, &nextoff);
        gridHeaders.push_back(ghdr);
        offset = nextoff;
        ng++;
    }
    close(fd);

    // Build gridHeaderMap, too 
    for (size_t i = 0; i < gridHeaders.size(); i++)
    {
        const GridHeader_t &ghdr = gridHeaders[i];
        if (gridHeaderMap.find(ghdr.AMR_level) == gridHeaderMap.end())
            gridHeaderMap[ghdr.AMR_level-1] = ghdr; // claw indexes levels from 1
    }
}

#define STRMATCH(s)     (!strncmp(s,tmpStr,sizeof(s)-1))
#define STRSTRIP(s)     (&tmpStr[sizeof(s)-1])

// ****************************************************************************
//  Method: avtClawFileFormat constructor
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
//  Modifications:
//
//    Mark C. Miller, Wed Aug  6 09:53:08 PDT 2008
//    Construct roorDir so that it does NOT contain a trailing "/."
//
//    Mark C. Miller, Mon Jul 12 16:06:22 PDT 2010
//    Replaced use of scanf with fgets as former would fail on inputs
//    containing spaces such as in a regex specification. Also, replaced
//    explicit ordering of tests for file keywords with loop over fgets.
//
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use VISIT_SLASH_STRING when creating rootDir.
//
// ****************************************************************************

avtClawFileFormat::avtClawFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    // open and read the claw bootstrap file
    char tmpStr[1024];
    FILE *bootFile = fopen(filename, "r");

    string bootFileDir = FileFunctions::Dirname(filename);
    rootDir = ".";
    timeScanf = "";
    timeRegex = "";
    gridScanf = "";
    gridRegex = "";
    optMode = "i/o";

    while (fgets(tmpStr, sizeof(tmpStr), bootFile))
    {
        size_t n = strlen(tmpStr);
        tmpStr[n-1] = '\0'; // get rid of newline char at end
        if      (STRMATCH("DIR="))
            rootDir = bootFileDir + VISIT_SLASH_STRING + string(STRSTRIP("DIR="));
        else if (STRMATCH("TIME_FILES_SCANF="))
            timeScanf = string(STRSTRIP("TIME_FILES_SCANF="));
        else if (STRMATCH("TIME_FILES_REGEX="))
            timeRegex = string(STRSTRIP("TIME_FILES_REGEX="));
        else if (STRMATCH("GRID_FILES_SCANF="))
            gridScanf = string(STRSTRIP("GRID_FILES_SCANF="));
        else if (STRMATCH("GRID_FILES_REGEX="))
            gridRegex = string(STRSTRIP("GRID_FILES_REGEX="));
        else if (STRMATCH("CYCLE_REGEX="))
            cycleRegex = string(STRSTRIP("CYCLE_REGEX="));
        else if (STRMATCH("OPTIMIZE_MODE="))
            optMode = string(STRSTRIP("OPTIMIZE_MODE="));
    }
    fclose(bootFile);

    debug1 << "DIR=" << rootDir << endl;
    if (timeScanf != "") {
        debug1 << "TIME_FILES_SCANF=" << timeScanf << endl;
    }
    if (timeRegex != "") {
        debug1 << "TIME_FILES_REGEX=" << timeRegex << endl;
    }
    if (gridScanf != "") {
        debug1 << "GRID_FILES_SCANF=" << gridScanf << endl;
    }
    if (gridRegex != "") {
        debug1 << "GRID_FILES_REGEX=" << gridRegex << endl;
    }
    if (cycleRegex != "") {
        debug1 << "CYCLE_REGEX=" << cycleRegex << endl;
    }
    if (optMode != "") {
        debug1 << "OPTIMIZE_MODE=" << optMode << endl;
    }
}

// ****************************************************************************
//  Method: avtClawFileFormat::GetFilenames
//
//  Purpose: Gets list of files to be processed
//
//  Programmer: Mark C. Miller 
//  Creation:   Wed Sep 12 09:20:22 PDT 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:54:10 EDT 2008
//    Use %ld format for longs.
//
//    Mark C. Miller, Mon Jul 12 16:07:51 PDT 2010
//    Pass timeRegex to SortFilenames for timeFilenames and gridRegex to
//    SortFilenames for gridFilenames.
// ****************************************************************************
void
avtClawFileFormat::GetFilenames()
{
    bool sortTime = false, sortGrid = false;

    // if we don't already have filename lists, create them
    if (timeFilenames.size() == 0)
    {
        debug5 << "Getting list of time files..." << endl;
        ::GetFilenames(timeScanf, timeRegex, rootDir, timeFilenames);
        sortTime = true;
    }
    if (gridFilenames.size() == 0)
    {
        debug5 << "Getting list of grid files..." << endl;
        ::GetFilenames(gridScanf, gridRegex, rootDir, gridFilenames);
        sortGrid = true;
    }

    // if lists sizes don't agree, thats bad
    if (gridFilenames.size() != timeFilenames.size())
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Number of time filenames, %ld, doesn't agree "
            " with number of grid filenames, %ld",
            timeFilenames.size(), gridFilenames.size());
        EXCEPTION1(InvalidFilesException, msg);
    }

    // sort the lists if we just created them, above
    if (sortTime)
    {
        TimeHeader_t thdr;
        InitTimeHeader(&thdr);
        SortFilenames(timeFilenames, timeRegex, rootDir);
        timeHeaders.resize(timeFilenames.size(), thdr);
    }
    if (sortGrid)
    {
        SortFilenames(gridFilenames, gridRegex, rootDir);
        gridHeaders.resize(gridFilenames.size());
        gridHeaderMaps.resize(gridFilenames.size());
    }
}

// ****************************************************************************
//  Method: avtClawFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
// ****************************************************************************

int
avtClawFileFormat::GetNTimesteps(void)
{
    GetFilenames();
    return (int)timeFilenames.size();
}

double
avtClawFileFormat::GetTime(int ts)
{
    GetFilenames();
    return timeHeaders[ts].time;
}

// ****************************************************************************
//  Method: avtClawFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
// ****************************************************************************

void
avtClawFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method: avtClawFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
//
//    Jeremy Meredith, Thu Aug  7 15:54:10 EDT 2008
//    Use %ld format for longs.
//
// ****************************************************************************

void
avtClawFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    GetFilenames();

    // get the time header for this timestep if we don't already have it
    if (timeHeaders[timeState].ndims == -1)
    {
        TimeHeader_t thdr;
        ReadTimeStepHeader(rootDir, timeFilenames[timeState], &thdr);
        timeHeaders[timeState] = thdr; 
    }
    const TimeHeader_t &timeHdr = timeHeaders[timeState];

    // get the grid headers for this timestep if we don't already have 'em
    if (gridHeaders[timeState].size() == 0)
        ReadGridHeaders(rootDir, gridFilenames[timeState], &timeHdr, gridHeaders[timeState],
            gridHeaderMaps[timeState]);
    const vector<GridHeader_t> &gridHdrs = gridHeaders[timeState];
    const map<int, GridHeader_t> &levelsMap = gridHeaderMaps[timeState];

    // sanity check
    if (timeHdr.ngrids != (int)gridHeaders[timeState].size())
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Time header's ngrid value, %d, doesn't agree "
            "with number of headers actuall read, %ld", timeHdr.ngrids, gridHdrs.size());
        EXCEPTION1(InvalidFilesException, msg);
    }


    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "claw_mesh";
    mesh->meshType = AVT_AMR_MESH;
    mesh->topologicalDimension = timeHdr.ndims; 
    mesh->spatialDimension = timeHdr.ndims; 
    mesh->hasSpatialExtents = false;

    // spoof a group/domain mesh
    mesh->numBlocks = timeHdr.ngrids;
    mesh->blockTitle = "grids";
    mesh->blockPieceName = "grid";
    mesh->blockOrigin = 1;
    mesh->numGroups = (int)levelsMap.size();
    mesh->groupTitle = "levels";
    mesh->groupPieceName = "level";
    mesh->groupOrigin = 1;
    vector<int> groupIds(timeHdr.ngrids);
    vector<string> blockPieceNames(timeHdr.ngrids);
    for (int i = 0; i < timeHdr.ngrids; i++)
    {
           char tmpName[64];
           SNPRINTF(tmpName, sizeof(tmpName), "level%d,grid_num%d",
               gridHdrs[i].AMR_level, gridHdrs[i].grid_number);

           // subtract off group origin here because internally
           // visit is expecting zero-indexed arrays
           groupIds[i] = gridHdrs[i].AMR_level - 1; 
           blockPieceNames[i] = tmpName;
    }
    mesh->blockNames = blockPieceNames;
    md->Add(mesh);
    md->AddGroupInformation((int)levelsMap.size(), timeHdr.ngrids, groupIds);

    //
    // add default plot (but only if we actually have 'levels'
    //
    if (levelsMap.size() > 1)
    {
        avtDefaultPlotMetaData *plot = new avtDefaultPlotMetaData("Subset_1.0", "levels");
        char attribute[250];
        sprintf(attribute,"%d NULL ViewerPlot", INTERNAL_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d ViewerPlot SubsetAttributes", INTERNAL_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d SubsetAttributes lineWidth 1", INT_NODE);
        plot->AddAttribute(attribute);
        sprintf(attribute,"%d SubsetAttributes wireframe true", BOOL_NODE);
        plot->AddAttribute(attribute);
        md->Add(plot);
    }
#ifdef MDSERVER
    else
    {
        char msg[512];
        static bool haveIssuedWarning = false;
        SNPRINTF(msg, sizeof(msg), "Ordinarily, VisIt displays a wireframe, subset "
            "plot of 'levels' automatically upon opening a SAMRAI file. However, such "
            "a plot is not applicable in the case that there is only one level. So, "
            "the normal subset plot is not being displayed.");
        if (!haveIssuedWarning)
        {
            haveIssuedWarning = true;
            if (!avtCallback::IssueWarning(msg))
                cerr << msg << endl;
        }
    }
#endif

    // ok, now add information on the variables that are defined
    for (int i = 0; i < timeHdr.meqn; i++)
    {
        char tmpName[64];
        SNPRINTF(tmpName, sizeof(tmpName), "col_%02d", i);
        AddScalarVarToMetaData(md, tmpName, "claw_mesh", AVT_ZONECENT);
    }
}

// ****************************************************************************
//  Method: avtClawFileFormat::BuildDomainAuxiliaryInfo
//
//  Purpose: Build the two data structures needed to support nesting and
//  abutting of AMR subgrids.
//
//  Note: These are *never* explicitly served up to VisIt like a GetMesh or
//  GetVar call would do. Instead, we essentially 'publish' them to VisIt
//  by sticking the structures we create here into the database cache. VisIt
//  will try to look for them there when it needs them.
//
//  Programmer: Mark C. Miller 
//  Creation:   Wed Sep 12 09:20:22 PDT 2007
//
//  Modifications:
//
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
//
//    Hank Childs, Mon Oct 29 14:19:21 PDT 2007
//    Remove +1 in for maxI, maxJ, maxK.  This makes the DBI object be
//    correctly sized.
//
//    Mark C. Miller, Wed Aug  6 09:53:57 PDT 2008
//    Fixed UMR in 2D where logic to test for 'continue' case was always
//    assuming 3D.
//     
//    Hank Childs, Sat Sep  4 18:30:11 PDT 2010
//    Fix bug with roundoff error for refinement ratios.  Also find minimum
//    X, Y, and Z for calculating logical coordinates (we were getting
//    negative logical coords before).
//
//    Burlen Loring, Fri Jul 11 17:41:14 PDT 2014
//    fix out-of-bounds index of vector reported by gcc STL debug mode.
//
// ****************************************************************************

void
avtClawFileFormat::BuildDomainAuxiliaryInfo(int timeState)
{
#ifdef MDSERVER
    return;
#endif

    const TimeHeader_t &timeHdr = timeHeaders[timeState];
    const vector<GridHeader_t> &gridHdrs = gridHeaders[timeState];
    map<int, GridHeader_t> levelsMap = gridHeaderMaps[timeState];

    int num_dims = timeHdr.ndims;
    int num_levels = (int)levelsMap.size();
    int num_patches = (int)gridHdrs.size();

    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                  timeState, -1);
    if ((*vrTmp == NULL) && (num_patches > 0))
    {
        //
        // build the avtDomainNesting object
        //
        avtStructuredDomainNesting *dn =
            new avtStructuredDomainNesting(num_patches, num_levels);

        dn->SetNumDimensions(num_dims);

        //
        // Set refinement level ratio information
        //
        vector<int> ratios(3,1);
        dn->SetLevelRefinementRatios(0, ratios);
        for (int i = 1; i < num_levels; ++i)
        {
           ratios[0] = (int) (levelsMap[i-1].dx / levelsMap[i].dx+0.5);
           ratios[1] = (int) (levelsMap[i-1].dy / levelsMap[i].dy+0.5);
           ratios[2] = num_dims == 3 ? (int) (levelsMap[i-1].dz / levelsMap[i].dz+0.5) : 0;
           dn->SetLevelRefinementRatios(i, ratios);
        }

        double lowestX = std::numeric_limits<double>::max();
        double lowestY = std::numeric_limits<double>::max();
        double lowestZ = std::numeric_limits<double>::max();
        for (int i = 0; i < num_patches; ++i)
        {
            lowestX = std::min(lowestX, gridHdrs[i].xlow);
            lowestY = std::min(lowestY, gridHdrs[i].ylow);
            lowestZ = std::min(lowestZ, gridHdrs[i].zlow);
        }

        //
        // set each domain's level, children and logical extents
        //
        for (int i = 0; i < num_patches; ++i)
        {
            vector<int> childPatches;
            float x0 = gridHdrs[i].xlow;
            float x1 = x0 + gridHdrs[i].mx * gridHdrs[i].dx;
            float y0 = gridHdrs[i].ylow;
            float y1 = y0 + gridHdrs[i].my * gridHdrs[i].dy;
            float z0 = gridHdrs[i].zlow;
            float z1 = z0 + gridHdrs[i].mz * gridHdrs[i].dz;
            for (int j = 0; j < num_patches; j++)
            {
                if (gridHdrs[j].AMR_level != gridHdrs[i].AMR_level+1)
                    continue;

                float a0 = gridHdrs[j].xlow;
                float a1 = a0 + gridHdrs[j].mx * gridHdrs[j].dx;
                float b0 = gridHdrs[j].ylow;
                float b1 = b0 + gridHdrs[j].my * gridHdrs[j].dy;
                float c0 = gridHdrs[j].zlow;
                float c1 = c0 + gridHdrs[j].mz * gridHdrs[j].dz;

                if (a0 >= x1 || x0 >= a1 ||
                    b0 >= y1 || y0 >= b1 ||
                    ((num_dims == 3) && (c0 >= z1 || z0 >= c1)))
                    continue;

                childPatches.push_back(j);
            }

            // the '+0.5' is because casting to (int) is a floor operation
            // and our floating pt. arithmatic might fall just below the
            // integral value it is intended to represent
            vector<int> logExts(6);
            logExts[0] = (int) ((gridHdrs[i].xlow-lowestX) / gridHdrs[i].dx + 0.5); 
            logExts[1] = (int) ((gridHdrs[i].ylow-lowestY) / gridHdrs[i].dy + 0.5); 
            logExts[2] = num_dims == 3 ? (int) ((gridHdrs[i].zlow-lowestZ) / gridHdrs[i].dz + 0.5) : 0;
            logExts[3] = logExts[0] + gridHdrs[i].mx - 1;
            logExts[4] = logExts[1] + gridHdrs[i].my - 1;
            logExts[5] = num_dims == 3 ? logExts[2] + gridHdrs[i].mz - 1 : 0;

            dn->SetNestingForDomain(i, gridHdrs[i].AMR_level-1, 
                childPatches, logExts);
        }

        void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);

        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
            timeState, -1, vr);
    }

    void_ref_ptr dbTmp = cache->GetVoidRef("any_mesh",
                                           AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                           timeState, -1);
    if (*dbTmp == NULL)
    {
        bool canComputeNeighborsFromExtents = true;
        avtStructuredDomainBoundaries *sdb = 0;

        sdb = new avtRectilinearDomainBoundaries(canComputeNeighborsFromExtents);

        sdb->SetNumDomains(num_patches);
        for (int i = 0 ; i < num_patches ; ++i)
        {
            int e[6];
            e[0] = (int) (gridHdrs[i].xlow / gridHdrs[i].dx + 0.5);
            e[1] = e[0] + gridHdrs[i].mx;
            e[2] = (int) (gridHdrs[i].ylow / gridHdrs[i].dy + 0.5);
            e[3] = e[2] + gridHdrs[i].my;
            e[4] = num_dims == 3 ? (int) (gridHdrs[i].zlow / gridHdrs[i].dz + 0.5) : 0;
            e[5] = num_dims == 3 ? e[4] + gridHdrs[i].mz  : 1;
            sdb->SetIndicesForAMRPatch(i, gridHdrs[i].AMR_level-1, e);
        }
        sdb->CalculateBoundaries();
        void_ref_ptr vsdb = void_ref_ptr(sdb,avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                            AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                            timeState, -1, vsdb);

    }
}

// ****************************************************************************
//  Method: avtClawFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
// ****************************************************************************

vtkDataSet *
avtClawFileFormat::GetMesh(int timeState, int domain, const char *meshname)
{
    if (timeHeaders[timeState].ndims == -1)
    {
        TimeHeader_t thdr;
        ReadTimeStepHeader(rootDir, timeFilenames[timeState], &thdr);
        timeHeaders[timeState] = thdr; 
    }
    const TimeHeader_t &timeHdr = timeHeaders[timeState];

    // get the grid headers for this timestep if we don't already have 'em
    if (gridHeaders[timeState].size() == 0)
        ReadGridHeaders(rootDir, gridFilenames[timeState], &timeHdr, gridHeaders[timeState],
            gridHeaderMaps[timeState]);
    const GridHeader_t &gridHdr = gridHeaders[timeState][domain];

    BuildDomainAuxiliaryInfo(timeState);

    int dims[3];
    dims[0] = gridHdr.mx+1;
    dims[1] = gridHdr.my+1;
    dims[2] = timeHdr.ndims == 2 ? 1 : gridHdr.mz+1;

    vtkFloatArray *xcoords = vtkFloatArray::New();
    xcoords->SetNumberOfTuples(dims[0]);
    for (int i = 0; i < dims[0]; i++)
        xcoords->SetComponent(i, 0, gridHdr.xlow + i * gridHdr.dx);

    vtkFloatArray *ycoords = vtkFloatArray::New();
    ycoords->SetNumberOfTuples(dims[1]);
    for (int i = 0; i < dims[1]; i++)
        ycoords->SetComponent(i, 0, gridHdr.ylow + i * gridHdr.dy);

    vtkFloatArray *zcoords = vtkFloatArray::New();
    if (timeHdr.ndims == 3)
    {
        zcoords->SetNumberOfTuples(dims[2]);
        for (int i = 0; i < dims[2]; i++)
            zcoords->SetComponent(i, 0, gridHdr.zlow + i * gridHdr.dz);
    }
    else
    {
        zcoords->SetNumberOfTuples(1);
        zcoords->SetComponent(0, 0, 0.);
    }
        
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(xcoords);
    xcoords->Delete();
    rgrid->SetYCoordinates(ycoords);
    ycoords->Delete();
    rgrid->SetZCoordinates(zcoords);
    zcoords->Delete();

    return rgrid;
}


// ****************************************************************************
//  Method: avtClawFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
//  Modifications:
//    Mark C. Miller, Tue Sep 18 11:08:52 PDT 2007
//    Changed naux to ndims 
//
//    Burlen Loring, Fri Jul 11 18:32:43 PDT 2014
//    fix invalid write when null terminating string, and handle
//    failed file system operations.
//
//    Kathleen Biagas, Fri Jan 6, 11:46:55 MST 2017
//    Use VISIT_SLASH_STRING when appending fname to rootDir.
//    Added support for Windows.
//
// ****************************************************************************

vtkDataArray *
avtClawFileFormat::GetVar(int timeState, int domain, const char *varname)
{
    if (timeHeaders[timeState].ndims == -1)
    {
        TimeHeader_t thdr;
        ReadTimeStepHeader(rootDir, timeFilenames[timeState], &thdr);
        timeHeaders[timeState] = thdr; 
    }
    const TimeHeader_t &timeHdr = timeHeaders[timeState];

    // get the grid headers for this timestep if we don't already have 'em
    if (gridHeaders[timeState].size() == 0)
        ReadGridHeaders(rootDir, gridFilenames[timeState], &timeHdr, gridHeaders[timeState],
            gridHeaderMaps[timeState]);
    const GridHeader_t &gridHdr = gridHeaders[timeState][domain];

    // extract the 'meqn' column number from the variable name
    int colNeeded;
    if (sscanf(varname, "col_%d", &colNeeded) != 1)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Unable to obtain meqn column number "
            "from \"%s\"", varname);
        EXCEPTION1(InvalidVariableException, msg);
    }

    // compute total length of data segment in characters
    int dsOffset = gridHdr.dataOffset;
    int dsLength = DataSegmentLengthInChars(&gridHdr, timeHdr.ndims);

    // open grid file, seek to correct offset and read the data there
    char *buf = new char[dsLength+1];
    string fullFileName = rootDir + VISIT_SLASH_STRING + gridFilenames[timeState];
    int fd =
#ifndef WIN32
        open(fullFileName.c_str(), O_RDONLY);
#else
        ::_open(fullFileName.c_str(), _O_RDONLY|_O_BINARY);
#endif
    if (fd < 0)
    {
        ostringstream oss;
        oss << "open \"" << fullFileName
          << "\" failed with error \"" << strerror(errno) << "\"";
        EXCEPTION1(InvalidFilesException, oss.str().c_str());
    }
    lseek(fd, dsOffset, SEEK_SET);
    size_t nread =
#ifndef WIN32
        read(fd, buf, dsLength);
#else
        ::_read(fd, buf, dsLength);
#endif
    if (nread < 0)
    {
        ostringstream oss;
        oss << "read " << dsLength << " from \"" << fullFileName
          << "\" failed with error \"" << strerror(errno) << "\"";
        EXCEPTION1(InvalidFilesException, oss.str().c_str());
    }
    buf[nread] = '\0';
    close(fd);

    // find the character offset within first line of desired column
    int lineOffset = 0;
    int colCount = -1;
    while (true)
    {
        char c0 = buf[lineOffset];
        char c1 = buf[lineOffset+1];

        // every transition from space to numeric characters is
        // beginning of a column
        /// TODO: check fix for the paranthesis warning
        if (c0 == ' ' && ((c1 >= '0' && c1 <= '9') || c1 == '.' || c1 =='-'))
        {
            colCount++;
            if (colCount == colNeeded)
                break;
        }
        lineOffset++;
    }
    lineOffset++;

    // scan forward to first space after this column's numerics
    int lineOffset2 = lineOffset;
    while (true)
    {
        char c0 = buf[lineOffset2];
        /// TODO: fix for paranthesis warning
        if ((c0 >= '0' && c0 <= '9') || 
            c0 == '.' || c0 =='-' || c0 == '+' || 
            c0 == 'e' || c0 == 'E')
            lineOffset2++;
        else
            break;
    }

    int nz = timeHdr.ndims == 3 ? gridHdr.mz : 1;
    int nvals = gridHdr.mx * gridHdr.my * nz; 

    vtkFloatArray *data = vtkFloatArray::New();
    data->SetNumberOfTuples(nvals);
    float *datap = (float *) data->GetVoidPointer(0);

    char *bufp = &buf[lineOffset];
    for (int zi = 0; zi < nz; zi++)
    {
        for (int yi = 0; yi < gridHdr.my; yi++)
        {
            for (int xi = 0; xi < gridHdr.mx; xi++)
            {
                // pinch the string at end of column
                *(bufp + lineOffset2) = '\0';

                // convert this column's value to float
                char *bufptmp = 0;
                errno = 0;
                float val = (float) strtod(bufp, &bufptmp);
                if (((val == 0.0) && (bufp == bufptmp)) || (errno != 0))
                {
                    char msg[256];
                    SNPRINTF(msg, sizeof(msg), "Error converting ascii \"%s\" to float",
                        bufp);
                    EXCEPTION1(InvalidVariableException, msg);

                }
                *datap++ = val;
                bufp += gridHdr.charsPerLine;
            }
            bufp += 3;
        }
        bufp += 3;
    }

    delete [] buf;

    return data;
}


// ****************************************************************************
//  Method: avtClawFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: miller -- generated by xml2avt
//  Creation:   Mon Sep 10 23:24:53 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtClawFileFormat::GetVectorVar(int timestate, int domain,const char *varname)
{
    return 0;
}
