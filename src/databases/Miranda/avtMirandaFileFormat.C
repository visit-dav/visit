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
//                            avtMirandaFileFormat.C                           //
// ************************************************************************* //

#include <avtMirandaFileFormat.h>


#include <string>

#ifdef _WIN32
#include <direct.h> /* for _getcwd */
#else
#include <unistd.h>
#endif

#include <DebugStream.h>
#include <TimingsManager.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>
#include <avtMaterial.h>

#include <Expression.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <snprintf.h>

using std::string;
using std::vector; 

#ifndef STREQUAL
#if defined(_WIN32) 
#  define STREQUAL(a,b)              stricmp(a,b)
#else
#  define STREQUAL(a,b)              strcasecmp(a,b)
#endif
#endif

#define DOTIMING 0
#if DOTIMING
#define TIMERSTART int total = visitTimer->StartTimer
#define TIMERSTOP(S) visitTimer->StopTimer(total, S)
#else
void nothing(void) {return; }
#define TIMERSTART if (0) nothing
#define TIMERSTOP(S)  if (0) nothing()
#endif


// ****************************************************************************
//  Method: PrintVec
//
//  Purpose:
//      Convenience function.  Print a vector. 
//
//  Programmer: Dave Bremer
//  Creation:   Fri Jun 13 15:54:11 PDT 2008
//
// ****************************************************************************
template <class T>
string Vec2String(string name, T *vec, int numelems) {
  string s = name + " = [" ;
  int elem = 0;
  char buf[32];
  
  while (elem < numelems ) {
    float value = vec[elem];
    SNPRINTF(buf,31,"%f",value); 
    s += buf ;
    if (elem == numelems - 1) {
      s+= "]"; 
    } else {
      s+= ", "; 
    }
    elem ++; 
  }
  return s;
} 


// ****************************************************************************
//  Method: Fix2DFileOrder
//
//  Purpose:
//      Modifies iFileOrder for 2D datasets.  fileOrder will come in as a 3 
//      element array holding a permutation of 0,1,2.  a and b each hold 0, 1, 
//      or 2, where a != b.        
/*      a and b are the names of the elements of iFileOrder which "matter."  
        On OUTPUT, iFileOrder[0] and iFileOrder[1] will be ordered 
        with respect to the relative INPUT order of a and b in iFileOrder. 
        iFileOrder[2] remains unchanged.  
        Fix2DFileOrder(0,2) means "fix iFileOrder to respect the order of X and Z"
        Example:  input iFileOrder = 2,0,1
        Fix2DFileOrder(0,1) -->  0 comes before 1 in iFileOrder --> iFileOrder[0:1] = 0,1
        Fix2DFileOrder(0,2) -->  0 comes after 2 in iFileOrder -->  iFileOrder[0:1] = 1,0

*/
//
//  Programmer: Dave Bremer
//  Creation:   Fri Jun 13 15:54:11 PDT 2008
//
// ****************************************************************************

static void Fix2DFileOrder(int a, int b, int *fileOrder)
{
  debug5 << "Fix2DFileOrder(" << a << ", "<< b << ", " << Vec2String("fileOrder",fileOrder,3) << ") " << endl; 
  int posA = 0, posB = 0; ///TODO: check on fix for uninitialized values
    int ii;
    /* set posA to the position in iFileOrder in which a appears.  
       set posB to the position in iFileOrder in which b appears.  
    */
    for (ii = 0; ii < 3; ii++)
      {
        if (fileOrder[ii] == a)
          posA = ii;
        if (fileOrder[ii] == b)
          posB = ii;
      }

    if (posA < posB) /* a came before b in iFileOrder */
      {
        fileOrder[0] = 0;
        fileOrder[1] = 1;
      }
    else /* b came before a in iFileOrder */
      {
        fileOrder[0] = 1;
        fileOrder[1] = 0;
      }
    debug5 << "After Fix2DFileOrder: "<<Vec2String("fileOrder",fileOrder,3) << endl;
}

// ****************************************************************************
// Class: format
//
// Purpose:
//   Small stand-in for boost::format.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 14 16:45:52 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class format
{
public:
  format() : count(1), fmt() { }
  format(const format &f) : count(f.count), fmt(f.fmt) { }
  format(const std::string &f) : count(1), fmt(f) { }
  format operator % (std::string value)
  {
    format retval(*this);
    std::string f(currentFormat());
    std::string::size_type n = fmt.find(f);
    if(n != std::string::npos)
      retval.fmt.replace(n, f.size(), value.c_str());
    retval.count = count + 1;
    return retval;
  }
  format operator % (int value)
  {
    char tmp[20];
    sprintf(tmp, "%d", value);
    return this->operator % (std::string(tmp));
  }
  operator std::string () {return fmt; }
private:    
  std::string currentFormat()
  {
    char tmp[20];
    sprintf(tmp, "%%%d%%", count);
    return std::string(tmp);
  }

  int         count;
  std::string fmt;
};

// ****************************************************************************
//  Method: avtMiranda constructor
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
//  Modifications:
//    Dave Bremer and Kathleen Bonnell, Mon Mar 19 19:04:11 PDT 2007
//    Fix fopen on windows.  Add error checking.
//
//    Dave Bremer, Wed Feb 20 15:25:12 PST 2008
//    Added support for version 1.2 of the .raw files, which specifies block
//    ordering using a tag of the form "fileorder: ZYX", rather than using a
//    separate grid file per process.  Also allow comments that use a full line.
//
//    Dave Bremer, Fri Jun 13 15:54:11 PDT 2008
//    Fix a bug in the use of the fileorder tag that comes up with some 2D data.
//
//    Dave Bremer, June 25, 2009
//    Updated to read curvilinear or rectilinear data.
// ****************************************************************************

avtMirandaFileFormat::avtMirandaFileFormat(const char *filename, DBOptionsAttributes *readOpts)
  : avtMTMDFileFormat(filename)
{
  string tag, buf1;
  char buf[512];
  ifstream  f(filename);
  int ii, jj;

  dim = 3;
  flatDim = -1;
  iFileOrder[0] = -1;
  iFileOrder[1] = -1;
  iFileOrder[2] = -1;
  bCurvilinear = false;
  bZonal = false;  // normally interpret rectilinear miranda data as zonal
  string  isZonal = "default";
  iInteriorSize[0] = iInteriorSize[1] = iInteriorSize[2] = -1; 
  iBoundarySize[0] = iBoundarySize[1] = iBoundarySize[2] = -1; 
  // Verify that the 'magic' and version number are right
  f >> buf1 >> sFileVersion;
    
  if (buf1 != "VERSION")
    {
      EXCEPTION1(InvalidDBTypeException, "Not a raw miranda file." );
    }
  if (sFileVersion != "1.0" && sFileVersion != "1.1" && sFileVersion != "1.2" && sFileVersion != "2.0")
    {
      EXCEPTION1(InvalidDBTypeException, 
                 "Only raw miranda version 1.0, 1.1, 1.2 and 2.0 are supported." );
    }

  // Process a tag at a time until all lines have been read
  while (f.good())
    {
      f >> tag;
      if (f.eof()) {
        f.clear();
        break;
      }

      if (tag[0] == '#')
        {
          SkipToEndOfLine( f, false );
        }
      else if (STREQUAL("gridfiles:", tag.c_str())==0)
        {
          f >> gridTemplate;
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("datafiles:", tag.c_str())==0)
        {
          f >> fileTemplate;
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("domainsize:", tag.c_str())==0)
        {
          f >> iGlobalDim[0] >> iGlobalDim[1] >> iGlobalDim[2];
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("blocksize:", tag.c_str())==0)
        {
          f >> iBlockSize[0] >> iBlockSize[1] >> iBlockSize[2];
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("interiorsize:", tag.c_str())==0)
        {
          f >> iInteriorSize[0] >> iInteriorSize[1] >> iInteriorSize[2];
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("bndrysize:", tag.c_str())==0)
        {
          f >> iBoundarySize[0] >> iBoundarySize[1] >> iBoundarySize[2];
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("origin:", tag.c_str())==0)
        {
          fOrigin[0] = GetFortranDouble(f);
          fOrigin[1] = GetFortranDouble(f);
          fOrigin[2] = GetFortranDouble(f);
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("spacing:", tag.c_str())==0)
        {
          fStride[0] = GetFortranDouble(f);
          fStride[1] = GetFortranDouble(f);
          fStride[2] = GetFortranDouble(f);
          SkipToEndOfLine( f );
        }
      else if (STREQUAL("variables:", tag.c_str())==0)
        {
          int nVars = 0;
          f >> nVars;
          SkipToEndOfLine( f );
        
          aVarNames.resize(nVars);
          aVarNumComps.resize(nVars);
          aVarMinMax.resize(nVars);
        
          for (ii = 0 ; ii < nVars ; ii++)
            {
              f >> aVarNames[ii] >> aVarNumComps[ii];
              aVarMinMax[ii].resize( 2*aVarNumComps[ii] );
                
              for (jj = 0 ; jj < aVarNumComps[ii] ; jj++)
                {
                  aVarMinMax[ii][jj*2] = GetFortranDouble(f);
                  aVarMinMax[ii][jj*2+1] = GetFortranDouble(f);
                }
              SkipToEndOfLine( f );
            }
        }
      else if (STREQUAL("materials:", tag.c_str())==0)
        {
          int nMats = 0;
          f >> nMats;
          SkipToEndOfLine( f );
            
          aMatNames.resize(nMats);
          for (ii = 0 ; ii < nMats ; ii++)
            {
              f >> aMatNames[ii];
              SkipToEndOfLine( f, false );
            }
        }
      else if (STREQUAL("timesteps:", tag.c_str())==0)
        {
          int nDumps = 0;
          f >> nDumps;
          SkipToEndOfLine( f );
            
          aCycles.resize(nDumps);
          aSimTimes.resize(nDumps);
            
          for (ii = 0 ; ii < nDumps ; ii++)
            {
              f >> aCycles[ii];
              aSimTimes[ii] = GetFortranDouble(f);
              SkipToEndOfLine( f );
            }
        }
      else if (STREQUAL("fileorder:", tag.c_str())==0)
        {
          //order will be some permutation of xyz or XYZ, or YZ, xy, etc.
          string  order;
          f >> order;
          if (order.size() != 2 && order.size() != 3)
            EXCEPTION1(InvalidDBTypeException, "Error parsing file.  "
                       "fileorder: should be followed by a permutation of XYZ");

          for (ii = 0 ; ii < (int)order.size() ; ii++)
            {
              if ('x' <= order[ii] && order[ii] <= 'z')
                iFileOrder[ii] = order[ii] - 'x';
              else if ('X' <= order[ii] && order[ii] <= 'Z')
                iFileOrder[ii] = order[ii] - 'X';
              else
                EXCEPTION1(InvalidDBTypeException, "Error parsing file.  "
                           "fileorder: should be followed by a permutation of XYZ");
            }
          if (order.size()==2)
            iFileOrder[2] = 3 - (iFileOrder[0]+iFileOrder[1]);
        }
      else if (STREQUAL("curvilinear:", tag.c_str())==0)
        {
          string  isCurved;
          f >> isCurved;

          if (isCurved == "yes")
            bCurvilinear = true;
        }
      else if (tag == "zonal:")
        {
          f >> isZonal;

          if (isZonal == "yes")               
            bZonal = true;
        }
      else
        {
          sprintf(buf, "Error parsing file.  Unknown tag %s", tag.c_str());
          EXCEPTION1(InvalidDBTypeException, buf);
        }
    }

  if (f.fail())
    {
      sprintf(buf, "Error parsing file at tag '%s'", tag.c_str());
      EXCEPTION1(InvalidDBTypeException, buf);
    }
  // revert to old behavior: 
  if (isZonal == "default") {     
    if (sFileVersion != "2.0" && !bCurvilinear) {
      bZonal = true;
    } else {
      bZonal = false;
    }
  }
  debug5 << "bZonal is " << bZonal << endl; 
    
  // make the file template into an absolute path
  for (ii = (int)strlen(filename)-1 ; ii >= 0 ; ii--)
    {
      if (filename[ii] == '/' || filename[ii] == '\\')
        {
          fileTemplate.insert(0, filename, ii+1);
          gridTemplate.insert(0, filename, ii+1);
          break;
        }
    }
  if (ii == -1)
    {
#ifdef _WIN32
      _getcwd(buf, 512);
#else
      char* res = getcwd(buf, 512); (void) res;
#endif
      strcat(buf, "/");
      fileTemplate.insert(0, buf, strlen(buf));
      gridTemplate.insert(0, buf, strlen(buf));
    }

#ifdef _WIN32
  for (ii = 0 ; ii < fileTemplate.size() ; ii++)
    {
      if (fileTemplate[ii] == '/')
        fileTemplate[ii] = '\\';
    }
  for (ii = 0 ; ii < gridTemplate.size() ; ii++)
    {
      if (gridTemplate[ii] == '/')
        gridTemplate[ii] = '\\';
    }
#endif

  if  (sFileVersion != "2.0") {
    for (ii = 0; ii < 3; ii++) {
      iInteriorSize[ii] = iBlockSize[ii] + 1; 
      iBoundarySize[ii] = iBlockSize[ii]; 
    }
  }
    
  for (int i=0; i<3; i++) {
    if (iGlobalDim[i] == iBlockSize[i]) {
      iNumBlocks[i] = 1; 
    } else {
      if (sFileVersion == "2.0" && bCurvilinear && bZonal) {
        iNumBlocks[i] = (iGlobalDim[i]-1) / (iBlockSize[i]-1);
      } else {
        iNumBlocks[i] = iGlobalDim[i] / iBlockSize[i];
      }
    }
  }
    
  //domainMap is only used if the file order is unspecified
  if (iFileOrder[0] == -1) {
    /* don't use domainMaps any more 
       domainMap.resize( iNumBlocks[0]*iNumBlocks[1]*iNumBlocks[2]*3, -1 );
    */ 
    iFileOrder[0] = 2; 
    iFileOrder[1] = 1; 
    iFileOrder[2] = 0;       
  }

  // Rearrange data if it is flat in one dimension
  if (iGlobalDim[2] == 1)
    {
      dim = 2;
      flatDim = 2;

      Fix2DFileOrder(0, 1, iFileOrder);  
    }
  else if (iGlobalDim[1] == 1)
    {
      dim = 2;
      flatDim = 1;
        
      double tmpOrigin = fOrigin[1];
      double tmpStride = fStride[1];
      double tmpInterior = iInteriorSize[1];
      double tmpBoundary = iBoundarySize[1];

      fOrigin[1] = fOrigin[2];
      fStride[1] = fStride[2];
      iNumBlocks[1] = iNumBlocks[2];
      iBlockSize[1] = iBlockSize[2];
      // version 2.0
      iInteriorSize[1] = iInteriorSize[2];
      iBoundarySize[1] = iBoundarySize[2];

      fOrigin[2] = tmpOrigin;
      fStride[2] = tmpStride;
      // version 2.0
      iInteriorSize[2] = tmpInterior; 
      iBoundarySize[2] = tmpBoundary;

      iNumBlocks[2] = 1;
      iBlockSize[2] = 1;

      Fix2DFileOrder(0, 2, iFileOrder);
    }
  else if (iGlobalDim[0] == 1)
    {
      dim = 2;
      flatDim = 0;
        
      double tmpOrigin = fOrigin[0];
      double tmpStride = fStride[0];
      double tmpInterior = iInteriorSize[0];
      double tmpBoundary = iBoundarySize[0];
        
      fOrigin[0] = fOrigin[1];
      fStride[0] = fStride[1];
      iNumBlocks[0] = iNumBlocks[1];
      iBlockSize[0] = iBlockSize[1];
      // version 2.0
      iInteriorSize[0] = iInteriorSize[1];
      iBoundarySize[0] = iBoundarySize[1];
        
      fOrigin[1] = fOrigin[2];
      fStride[1] = fStride[2];
      iNumBlocks[1] = iNumBlocks[2];
      iBlockSize[1] = iBlockSize[2];
      // version 2.0
      iInteriorSize[1] = iInteriorSize[2];
      iBoundarySize[1] = iBoundarySize[2];
   
      fOrigin[2] = tmpOrigin;
      fStride[2] = tmpStride;
      iInteriorSize[2] = tmpInterior; 
      iBoundarySize[2] = tmpBoundary;

      iNumBlocks[2] = 1;
      iBlockSize[2] = 1;

      Fix2DFileOrder(1, 2, iFileOrder);
    }
  debug5 << Vec2String("iGlobalDim",iGlobalDim,3) << endl; 

  if (bCurvilinear && iFileOrder[0] == -1)
    EXCEPTION1(InvalidDBTypeException, 
               "A curvilinear mesh must set the fileorder" );


}


// ****************************************************************************
//  Method: avtMirandaFileFormat::SkipToEndOfLine
//
//  Purpose:
//      Used by the constructor to parse the file.  This just doublechecks 
//      that no extra tokens are found in the area where only white space
//      and comments should be.
//
//  Programmer: Dave Bremer
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
// ****************************************************************************

void 
avtMirandaFileFormat::SkipToEndOfLine( ifstream &f, bool bCheckForBadTokens )
{
  char buf[512];
  f.getline(buf, 512);

  size_t len = strlen(buf);
  if (len >= 511)
    {
      EXCEPTION1(InvalidDBTypeException, "Error parsing file." );
    }

  if (bCheckForBadTokens)
    {
      for (size_t ii = 0 ; ii < len ; ii++)
        {
          if (buf[ii] == '#')
            {
              //The rest of the line is comments.  We're done
              break;
            }
          if ((buf[ii] != ' ') && (buf[ii] != '\t') && (buf[ii] != '\n'))
            {
              EXCEPTION1(InvalidDBTypeException, "Error parsing file." );
            }
        }
    }
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::GetFortranDouble
//
//  Purpose:
//      This function extracts the next double from the stream.  It also 
//      handles a fortran convention of sometimes writing D or d instead 
//      of E or e, to denote the exponent in a float or double.
//
//  Programmer: Dave Bremer
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
// ****************************************************************************

double
avtMirandaFileFormat::GetFortranDouble( ifstream &f )
{
  std::string s;
  f >> s;

  size_t ii;
  for (ii = 0 ; ii < s.size() ; ii++)
    {
      if (s[ii] == 'd' || s[ii] == 'D')
        {
          s[ii] = 'e';
        }
    }
  double r = atof(s.c_str());
  return r;
}



// ****************************************************************************
//  Method: avtMirandaFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
// ****************************************************************************

void
avtMirandaFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Feb 21 17:54:33 PST 2007
//    All components of a material set are also added as scalars.
//    Those scalars are returned unmodified, whereas a material set
//    is cleaned first.
//
//    Dave Bremer, June 25, 2009
//    Updated to set up curvilinear or rectilinear data.
//
//    Cyrus Harrison, Fri Aug  7 10:16:34 PDT 2009
//    Fixed an error with 2d mesh boundary info causing the domain boundary 
//    object calcuate a logical z-extent off by one.
//
// ****************************************************************************

void
avtMirandaFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, 
                                               int /*timeState*/)
{
  debug5 << "PopulateDatabaseMetaData called" << endl; 
  // Add the mesh
  std::string meshname("mesh");
  int nblocks = iNumBlocks[0] * iNumBlocks[1] * iNumBlocks[2];

  avtMeshMetaData *mesh = new avtMeshMetaData;
  mesh->name = meshname;
  mesh->numBlocks = nblocks;
  mesh->cellOrigin = 1;
  mesh->spatialDimension = dim;
  mesh->topologicalDimension = dim;
  mesh->blockTitle = "blocks";
  mesh->blockPieceName = "p%06d";
  mesh->groupPieceName = "global_index";

  debug5 << Vec2String("fOrigin",fOrigin,3) << endl; 
  debug5 << Vec2String("fStride",fStride,3) << endl; 
  debug5 << Vec2String("iBlockSize",iBlockSize,3) << endl; 
  debug5 << Vec2String("iBoundarySize",iBoundarySize,3) << endl; 
  debug5 << Vec2String("iInteriorSize",iInteriorSize,3) << endl; 
  if (!bCurvilinear)    
    {
      /* The extents of the mesh are from node to node, not zone centered */ 
      debug5 << "mesh->meshType = AVT_RECTILINEAR_MESH" << endl; 
      mesh->meshType = AVT_RECTILINEAR_MESH;
      double extents[6]; 
      for (int i=0; i< dim; i++) {
        if (bZonal) {  
          extents[2*i] = fOrigin[i] - 0.5*fStride[i]; 
          extents[2*i+1 ] =  extents[2*i] + (iNumBlocks[i]*iBlockSize[i])*fStride[i]; 
        } else {
          extents[2*i] = fOrigin[i];
          extents[2*i+1] =  fOrigin[i] + ((iNumBlocks[i]-1)*iBlockSize[i] + iBoundarySize[i] - 1)*fStride[i];
        }
      }
        
      debug5 << "Set " << Vec2String("extents",extents,2*dim) << endl; 
      mesh->SetExtents(extents);      
      mesh->hasSpatialExtents = true;
    }
  else  
    {
      debug5 << "mesh->meshType = AVT_CURVILINEAR_MESH" << endl; 
      mesh->meshType = AVT_CURVILINEAR_MESH;
      mesh->hasSpatialExtents = false;
    }
    
    
  if (dim == 2)
    {
      if (flatDim == 0)
        {
          mesh->xLabel = "Y";
          mesh->yLabel = "Z";
          mesh->zLabel = "";
        }
      if (flatDim == 1)
        {
          mesh->xLabel = "X";
          mesh->yLabel = "Z";
          mesh->zLabel = "";
        }
    }
    
    
  md->Add(mesh);
    
  // Add the variables    
  enum avtCentering centering = (bZonal) ? AVT_ZONECENT : AVT_NODECENT;
  // enum avtCentering centering = AVT_NODECENT;
  if (bZonal) 
    debug5 << "centering is zonal"<<endl;
  else 
    debug5 << "centering is nodal"<<endl;

  for (size_t ii = 0 ; ii < aVarNames.size() ; ii++)
    {
      if (aVarNumComps[ii] == 1)
        {
          AddScalarVarToMetaData(md, aVarNames[ii], meshname, centering);
        }
      else
        {
          AddVectorVarToMetaData(md, aVarNames[ii], meshname, 
                                 centering, aVarNumComps[ii]);
        }
    }

  if (aMatNames.size() > 0) {
    // Add material set components as scalar vars as well as a material set
    for (size_t ii = 0 ; ii < aMatNames.size() ; ii++)
      {
        AddScalarVarToMetaData(md, aMatNames[ii], meshname, centering);
      }
    avtMaterialMetaData *matmd = new avtMaterialMetaData;
    matmd->name = "materialset";
    matmd->meshName = meshname;
    matmd->numMaterials = (int)aMatNames.size();
    matmd->materialNames = aMatNames;
    md->Add(matmd);
  }
  debug5 << Vec2String("iNumBlocks", iNumBlocks, 3) << endl; 

  // Find logical domain boundaries
  if (!avtDatabase::OnlyServeUpMetaData() && nblocks > 1)
    {
      avtStructuredDomainBoundaries *rdb;
      if (bCurvilinear)
        rdb = new avtCurvilinearDomainBoundaries(true);
      else
        rdb = new avtRectilinearDomainBoundaries(true);

      rdb->SetNumDomains(nblocks);
      int bbox[6];
      for (int ii = 0 ; ii < nblocks ; ii++)
        {
          int iBlockIJK[3];           
          DomainToIJK( ii, iBlockIJK[0], iBlockIJK[1],  iBlockIJK[2]);
          for (int ijk=0; ijk < 3; ijk++) {
            if (sFileVersion == "2.0" && bCurvilinear && bZonal) {
              // in this particular instance, the block sizes are all like boundaries
              /* set lower bounds */ 
              bbox[2*ijk] = iBlockIJK[ijk] * (iBlockSize[ijk]-1); 
              /* set upper bound */ 
              bbox[2*ijk+1] = bbox[2*ijk] + iBlockSize[ijk] - 1; // parentheses are correct
            } else {
              /* set lower bounds */ 
              bbox[ijk*2] = iBlockIJK[ijk] * iBlockSize[ijk];
              /* set upper bound */ 
              if (iBlockIJK[ijk] == iNumBlocks[ijk]-1) {
                bbox[2*ijk+1] = bbox[2*ijk] + iBoundarySize[ijk] - 1; 
              } else {
                bbox[2*ijk+1] = bbox[2*ijk] + iInteriorSize[ijk] - 1; 
              }
            }
          }

          // VisIt expects the 2d case to have flat logical z extent (0,0).
          if(dim == 2)
            {
              bbox[4] = 0;
              bbox[5] = 0;
            }
          debug5 << "For domain "<< ii <<", "<<Vec2String("iBlockIJK",iBlockIJK,3)<<", "<<Vec2String("bounds",bbox,6) << endl; 
          rdb->SetIndicesForRectGrid(ii, bbox);
        }
      rdb->CalculateBoundaries();

      void_ref_ptr vr = void_ref_ptr(rdb,
                                     avtStructuredDomainBoundaries::Destruct);
      cache->CacheVoidRef("any_mesh",
                          AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
    }

   
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::DomainToIJK
//
//  Purpose:
//      Gets the location of a domain in block space, 
//      i.e. within 0 .. iNumBlocks-1.  
//
//  Programmer: Dave Bremer
//  Creation:   Feb 1, 2007
//
//  Modifications:
//    Dave Bremer, Wed Feb 20 15:25:12 PST 2008
//    Changed the computation of the mapping to use either iFileOrder, if set,
//    or the grid files.
// ****************************************************************************

void
avtMirandaFileFormat::DomainToIJK(int domain, int &i, int &j, int &k)
{

  if (iFileOrder[0] != -1)
    {
      int out[3];

      if (dim == 3)
        {
          out[iFileOrder[0]] = domain % iNumBlocks[iFileOrder[0]];
          out[iFileOrder[1]] = (domain / iNumBlocks[iFileOrder[0]]) % iNumBlocks[iFileOrder[1]];
          out[iFileOrder[2]] = domain / (iNumBlocks[iFileOrder[0]] * iNumBlocks[iFileOrder[1]]);
          i = out[0];
          j = out[1];
          k = out[2];
        }
      else
        {
          int lenInFastDir = iNumBlocks[iFileOrder[0]];

          out[iFileOrder[0]] = domain % lenInFastDir;
          out[iFileOrder[1]] = domain / lenInFastDir;
          i = out[0];
          j = out[1];
          k = 0;
        }
      /* This is only true if blocks come in ZYX order, which is normally
         true, but not guaranteed.
         k = domain % iNumBlocks[2];
         j = (domain / iNumBlocks[2]) % iNumBlocks[1];
         i = domain / (iNumBlocks[1] * iNumBlocks[2]);
      */
    }
  else
    {
      char buf[512] = "domainMaps are no longer supported"; 
      
      EXCEPTION1(InvalidFilesException, buf);
      /*
        if (domainMap[domain*3] == -1)
        {
        char filename[512];
        string tok;
        double blockOrigin[3];
        int ii;
            
        sprintf(filename, gridTemplate.c_str(), domain);
        ifstream  f(filename);
        if (!f.good())
        {
        EXCEPTION1(InvalidFilesException, filename);
        }
            
        f >> tok;
        if (tok != "origin:")
        {
        EXCEPTION1(InvalidFilesException, filename);        
        }
        blockOrigin[0] = GetFortranDouble( f );
        blockOrigin[1] = GetFortranDouble( f );
        blockOrigin[2] = GetFortranDouble( f );
        f.close();
    
        if (dim == 3)
        {
        for (ii = 0 ; ii < 3 ; ii++)
        {
        double dIndex = (blockOrigin[ii] - fOrigin[ii]) / 
        (fStride[ii]*iBlockSize[ii]);
        domainMap[domain*3 + ii] = (int)floor(0.5 + dIndex);
        }
        }
        else
        {
        if (flatDim == 0)
        {
        blockOrigin[0] = blockOrigin[1];
        blockOrigin[1] = blockOrigin[2];
        }
        else if (flatDim == 1)
        {
        blockOrigin[1] = blockOrigin[2];
        }
        for (ii = 0 ; ii < 2 ; ii++)
        {
        double dIndex = (blockOrigin[ii] - fOrigin[ii]) / 
        (fStride[ii]*iBlockSize[ii]);
        domainMap[domain*3 + ii] = (int)floor(0.5 + dIndex);
        }
        domainMap[domain*3 + 2] = 0;
        }
        }
        i = domainMap[domain*3 + 0];
        j = domainMap[domain*3 + 1];
        k = domainMap[domain*3 + 2];
      */
    }
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::GetMesh
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
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
//  Modifications:
//  Dave Bremer, June 25, 2009
//  Refactored into curvilinear and rectilinear methods.
// ****************************************************************************

vtkDataSet *
avtMirandaFileFormat::GetMesh(int /*timestate*/, int domain, const char * /*meshname*/)
{
  if (bCurvilinear)
    return GetCurvilinearMesh(domain);
  else
    return GetRectilinearMesh(domain);
}

// ****************************************************************************
//  Method: avtMirandaFileFormat::GetBlockDims
//
//  Purpose:
//      Gets the logical dimensions for the given block.  
//      Expressed in nodes if nodal data, zones if zonal.  
//      I.e., expressed in miranda terms, so that the grid returned 
//      contains the number of elements to read for that processor file.
//
//  Arguments:
//      domain  The index of the domain.
//
//  Programmer: rich Cook
//  Creation:   2012-05-25
//
// ****************************************************************************
void avtMirandaFileFormat::GetBlockDims(int domain, int dims[3]) {
  int ijk[3], axis=0; 
  DomainToIJK(domain, ijk[0], ijk[1], ijk[2]); 
  while (axis < 3) {
    if (ijk[axis] == iNumBlocks[axis]-1) {
      dims[axis] = iBoundarySize[axis]; 
    } else {
      dims[axis] = iInteriorSize[axis]; 
    }
    ++axis; 
  }
  return; 
}

// ****************************************************************************
//  Method: avtMirandaFileFormat::GetCurvilinearMesh2
//
//  Purpose:
//      Gets the curvilinear mesh associated with this file.
//      In version 2.0, this is much simpler -- just read a single file.  
//
//  Arguments:
//      domain  The index of the domain.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Jun 17 17:30:41 PDT 2009
//
//  Modified for version 2.0 by Rich Cook
//
//  Cyrus Harrison, Thu Oct  4 09:30:26 PDT 2012
//  Init alloced point memory buffer w/ zeros.
//  In the 2D case we may not touch these values, so we need
//  to make sure they are zero. 
//
// ****************************************************************************

vtkDataSet *
avtMirandaFileFormat::GetCurvilinearMesh2(int domain) {
  debug4 << "GetCurvilinearMesh2("<<domain<<")"<< endl; 
  TIMERSTART(); 

  char filename[512]; 
  if (SNPRINTF(filename, 512, gridTemplate.c_str(), domain) < 0)
    EXCEPTION1(InvalidFilesException, "");
  FILE *fd = fopen(filename, "rb");
  if (fd == NULL)
    EXCEPTION1(InvalidFilesException, filename);
  
  int domainsize[3]; 
  GetBlockDims(domain, domainsize); 
  int nSrcTuples = domainsize[0]*domainsize[1]*domainsize[2];
  vtkFloatArray *array = vtkFloatArray::New();
  array->SetNumberOfComponents(3);
  array->SetNumberOfTuples(nSrcTuples);  

  // We can just read the entire contents of the domain file in version 2.0
  // This is in fact the point of having a version 2.0 file format.  
  vector<float> v(nSrcTuples*3,0); // allocate and set to zero -- no delete needed
  float *bufp = &v[0];
  TRY
    {
      int comp = 0, bufcomp = 0; 
      for (comp=0; comp < 3; comp++)
        {
          if (dim == 2 && comp == flatDim) 
            continue;    
          ReadRawScalar(fd, comp, bufp, filename, domain, nSrcTuples); 
          debug5 << Vec2String("First five elements", bufp, 5) << endl; 
          bufp += nSrcTuples; 
          bufcomp++; 
        }
      fclose(fd); 
    }
  CATCHALL
    {
      fclose(fd);
      RETHROW;
    }
  ENDTRY

    InterleaveData((float *)(array->GetVoidPointer(0)), &v[0], domainsize, 3);

  vtkPoints *p = vtkPoints::New();
  p->SetData(array);
  array->Delete();

  vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
  sgrid->SetDimensions(domainsize);
  sgrid->SetPoints(p);

  int iBlockX, iBlockY, iBlockZ;
  DomainToIJK( domain, iBlockX, iBlockY, iBlockZ );
  vtkIntArray *arr = vtkIntArray::New();
  arr->SetNumberOfTuples(3);
  arr->SetValue(0, iBlockX*iBlockSize[0]+1);
  arr->SetValue(1, iBlockY*iBlockSize[1]+1);
  arr->SetValue(2, iBlockZ*iBlockSize[2]+1);
  arr->SetName("base_index");
  sgrid->GetFieldData()->AddArray(arr);
  arr->Delete();


  TIMERSTOP(str(format("GetCurvilinearMesh2(%1%)")%domain)); 
  return sgrid;

}
// ****************************************************************************
//  Method: avtMirandaFileFormat::GetCurvilinearMesh
//
//  Purpose:
//      Gets the curvilinear mesh associated with this file.
//      Note that the mesh is always expressed nodally here, even for zonal data.  
//
//  Arguments:
//      domain  The index of the domain.
//
//  Programmer: Dave Bremer
//  Creation:   Wed Jun 17 17:30:41 PDT 2009
//
// ****************************************************************************

vtkDataSet *
avtMirandaFileFormat::GetCurvilinearMesh(int domain)
{
  if (sFileVersion == "2.0") {
    return GetCurvilinearMesh2(domain); 
  }
  TIMERSTART(); 
  debug4 << "GetCurvilinearMesh("<<domain<<")"<< endl; 

  vtkStructuredGrid *sgrid = vtkStructuredGrid::New();

  int neighbors[8], realdim[3], ii, jj;
  float *aRawBlocks[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

  FindNeighborDomains(domain, neighbors, realdim);

  sgrid->SetDimensions(realdim);
  int nSrcTuples = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];

  // vectors provide auto initialization and scoping for garbage collection
  vector<vector<float> > floatvectors(8); 
  for (ii = 0; ii < 8; ii++)
    {
      if (neighbors[ii] == -1)
        continue;

      char filename[512];
      if (SNPRINTF(filename, 512, gridTemplate.c_str(), neighbors[ii]) < 0)
        EXCEPTION1(InvalidFilesException, "");

      FILE *fd = fopen(filename, "rb");
      if (fd == NULL)
        EXCEPTION1(InvalidFilesException, filename);
        
      floatvectors[ii].resize(nSrcTuples*3);
      aRawBlocks[ii] = &floatvectors[ii][0];

      int iDst = 0;
      for (jj = 0; jj < 3; jj++)
        {
          if (dim == 2 && jj == flatDim)
            continue;

          ReadRawScalar(fd, jj, aRawBlocks[ii] + iDst*nSrcTuples, filename, domain, nSrcTuples);
          iDst++;
        }

      // this is redundant -- vector initialized already
      /* if (dim == 2)
         memset(aRawBlocks[ii] + 2*nSrcTuples, 0, nSrcTuples*sizeof(float));
      */
      fclose(fd);
    }

  vtkFloatArray *array = vtkFloatArray::New();
  array->SetNumberOfComponents(3);
  array->SetNumberOfTuples(realdim[0]*realdim[1]*realdim[2]);

  PackData( (float *)(array->GetVoidPointer(0)), aRawBlocks, realdim, 3, true);
    
  vtkPoints *p = vtkPoints::New();
  p->SetData(array);
  array->Delete();

  sgrid->SetPoints(p);

  int iBlockX, iBlockY, iBlockZ;
  DomainToIJK( domain, iBlockX, iBlockY, iBlockZ );
  vtkIntArray *arr = vtkIntArray::New();
  arr->SetNumberOfTuples(3);
  arr->SetValue(0, iBlockX*iBlockSize[0]+1);
  arr->SetValue(1, iBlockY*iBlockSize[1]+1);
  arr->SetValue(2, iBlockZ*iBlockSize[2]+1);
  arr->SetName("base_index");
  sgrid->GetFieldData()->AddArray(arr);
  arr->Delete();
  TIMERSTOP(str(format("GetCurvilinearMesh(%1%)")%domain)); 

  return sgrid;
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::GetRectilinearMesh
//
//  Purpose:
//      Gets the rectilinear mesh associated with this file.  
//
//  Arguments:
//      domain  The index of the domain.
//
//  Programmer: Dave Bremer
//  Creation:   June 25, 2009
//
// ****************************************************************************

vtkDataSet *
avtMirandaFileFormat::GetRectilinearMesh(int domain)
{
  vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
  debug4 << "GetRectilinearMesh("<<domain<<")"<< endl; 
  debug5 << "bZonal is " << bZonal << endl; 

  int ii;    
  int iBlockIJK[3]; 
  DomainToIJK( domain, iBlockIJK[0], iBlockIJK[1], iBlockIJK[2] );
 
  int gridsize[3] = { iBlockSize[0]+1, iBlockSize[1]+1, iBlockSize[2]+1 }; 
  if (bZonal) {
    if (dim == 2) {
      gridsize[2] = 1; 
    }
  }
  else {   
    GetBlockDims(domain, gridsize); 
  }
  rgrid->SetDimensions(gridsize); 
  debug5 << "GetRectilinearMesh: " << Vec2String("gridsize",gridsize,3) << endl;
    
  rgrid->SetDimensions(gridsize);    

  vtkFloatArray *x = vtkFloatArray::New();
  vtkFloatArray *y = vtkFloatArray::New();
  vtkFloatArray *z = vtkFloatArray::New();
     
  x->SetNumberOfTuples(gridsize[0]);
  y->SetNumberOfTuples(gridsize[1]);
  z->SetNumberOfTuples(gridsize[2]);
 
  int extraElement = bZonal?1:0; 
  double zonalOffset[3] = {extraElement*0.5*fStride[0], extraElement*0.5*fStride[1], extraElement*0.5*fStride[2]};
  vtkFloatArray *array = NULL; 
  for (int ijk = 0; ijk<dim; ijk++) {
    if (ijk == 0) array = x; 
    else if (ijk == 1) array = y; 
    else if (ijk == 2) array = z; 
    double first, location;
    for (ii = 0 ; ii < gridsize[ijk] ; ii++) {
      location = fOrigin[ijk] + (iBlockIJK[ijk]*iBlockSize[ijk]+ii)*fStride[ijk] - zonalOffset[ijk];
      array->SetTuple1(ii, location);
      if (!ii) first = location; 
    }
    debug5 << "GetRectilinearMesh(domain="<<domain<<"): "<<ijk<<" direction has " << gridsize[ijk] << " samples and limits are "<<first << " to " << location << endl;  
  }
  if (dim == 2) {
    z->SetTuple1(0, 0.);
    debug5 << "GetRectilinearMesh("<<domain<<"): Z limits are 0 to 0" << endl; 
  }

  rgrid->SetXCoordinates(x);
  rgrid->SetYCoordinates(y);
  rgrid->SetZCoordinates(z);

  x->Delete();
  y->Delete();
  z->Delete();

  vtkIntArray *arr = vtkIntArray::New();
  arr->SetNumberOfTuples(3);
  for (ii=0; ii<3; ii++) {
    arr->SetValue(ii, iBlockIJK[ii]*iBlockSize[ii]); // always true, no boundary considerations
  }
  arr->SetName("base_index");
  rgrid->GetFieldData()->AddArray(arr);
  arr->Delete();

  return rgrid;
}



static void
ByteSwap32(void *val)
{
  char *v = (char *)val;
  char tmp;
  tmp = v[0]; v[0] = v[3]; v[3] = tmp;
  tmp = v[1]; v[1] = v[2]; v[2] = tmp;
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Feb 21 17:54:33 PST 2007
//    The components of a material set can be queried here, just like other 
//    scalars.  Those scalars are returned unmodified, whereas a material set
//    is cleaned first.
//
//    Dave Bremer, Mon Mar 19 19:04:11 PDT 2007
//    Fix fopen on windows.  Add error checking.
//
//    Dave Bremer, June 25, 2009
//    Updated to handle curvilinear meshes, which requires treating data as
//    nodal, and reading ghost data from adjacent domains.
//
// ****************************************************************************

vtkDataArray *
avtMirandaFileFormat::GetVar(int timestate, int domain, const char *varname)
{
  debug4 << "GetVar(timestate="<<timestate<<", domain="<<domain<<", var="<<varname<<")"<< endl; 
  TIMERSTART(); 

  int ii, iVar = -1, nPrevComp = 0;

  for (ii = 0 ; ii < (int)aVarNames.size() ; ii++) 
    {
      if (aVarNames[ii] == varname)
        {
          iVar = ii;
          break;
        }
      nPrevComp += aVarNumComps[ii];
    }
  // Search the materials if varname is not a regular variable
  if (iVar == -1)
    {
      for (ii = 0 ; ii < (int)aMatNames.size() ; ii++) 
        {
          if (aMatNames[ii] == varname)
            {
              iVar = ii;
              break;
            }
          nPrevComp += 1;
        }
    }
  if (iVar == -1)
    {
      EXCEPTION1(InvalidVariableException, varname);
    }

  vtkFloatArray *var = vtkFloatArray::New();
  if (sFileVersion == "2.0") {
    char filename[512];
    if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], domain) < 0)
      EXCEPTION1(InvalidFilesException, "");
      
    FILE *fd = fopen(filename, "rb");
    if (fd == NULL)
      EXCEPTION1(InvalidFilesException, filename);
      
    int dims[3]; 
    GetBlockDims(domain, dims); 
    if (bZonal) {
      for (int i=0; i<3; i++) {
        if (dims[i] > 1) dims[i]--;
      }
    }
    int nTuples = dims[0]*dims[1]*dims[2];
    var->SetNumberOfTuples( nTuples );
      
    float *buf = (float *)(var->GetVoidPointer(0));
    ReadRawScalar(fd, nPrevComp, buf, filename, domain, nTuples); 
    debug5 << "(2.0): Read "<< nTuples << " elements from " << filename << " for domain " << domain << endl;
    debug5 << Vec2String("First five elements", buf, 5) << endl; 
    fclose(fd); 
  } else {
    int neighbors[8], realdim[3];
    float *aRawBlocks[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    vector<vector<float> > floatvectors(8); 

    FindNeighborDomains(domain, neighbors, realdim);
    debug5 << "GetVar: For domain " << domain<<", "<<Vec2String("neighbors",neighbors,8)<<endl; 
    for (ii = 0; ii < 8; ii++)
      {
        if (neighbors[ii] == -1)
          continue;
          
        char filename[512];
        if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], neighbors[ii]) < 0)
          EXCEPTION1(InvalidFilesException, "");
        FILE *fd = fopen(filename, "rb");
        if (fd == NULL)
          EXCEPTION1(InvalidFilesException, filename);
        vector<vector<float> >::size_type numfloats =
          iBlockSize[0]*iBlockSize[1]*iBlockSize[2]; 
        floatvectors[ii].resize(numfloats); 
        aRawBlocks[ii] = &floatvectors[ii][0];
        ReadRawScalar(fd, nPrevComp, aRawBlocks[ii], filename, domain, (int)numfloats);
        debug5 << "Read "<<numfloats<< " elements from " << filename << " for neighbor " << ii << " of domain " << domain << endl;
          
        fclose(fd);
      }
      
    int nTuples = realdim[0]*realdim[1]*realdim[2];
    debug5 << "GetVar: nTuples = " << nTuples << endl; 
    var->SetNumberOfTuples( nTuples );
      
    PackData( (float *)(var->GetVoidPointer(0)), aRawBlocks, realdim, 1, false );
      
  }
  TIMERSTOP(str(format("GetVar(%1%, %2%, %3%)")%timestate%domain%varname));

  return var;
}



// ****************************************************************************
//  Method: avtMirandaFileFormat::GetVectorVar
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
//  Programmer: dbremer -- generated by xml2avt
//  Creation:   Tue Jan 23 17:00:13 PST 2007
//
//  Modifications:
//    Dave Bremer, Wed Feb 21 19:54:33 PST 2007
//    Patched for data that is treated as 2D.  For 3-component vectors, 
//    the two non-zero components are placed in the first two output components.
//
//    Dave Bremer, Mon Mar 19 19:04:11 PDT 2007
//    Fix fopen on windows.  Add error checking.
//
//    Dave Bremer, June 25, 2009
//    Updated to handle curvilinear meshes, which requires treating data as
//    nodal, and reading ghost data from adjacent domains.
//
// ****************************************************************************

vtkDataArray *
avtMirandaFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
  debug4 << "GetVectorVar("<<timestate<<", "<<domain<<", "<<varname<<")"<< endl; 
  TIMERSTART(); 

  int ii, jj, iVar = -1, nPrevComp = 0;

  for (ii = 0 ; ii < (int)aVarNames.size() ; ii++) 
    {
      if (aVarNames[ii] == varname)
        {
          iVar = ii;
          break;
        }
      nPrevComp += aVarNumComps[ii];
    }
  if (iVar == -1 || aVarNumComps[iVar] <= 1)
    {
      EXCEPTION1(InvalidVariableException, varname);
    }

  int nComps = aVarNumComps[iVar];

  bool bFlattenVec = false;
  if (aVarNumComps[iVar] == 3 && dim == 2 && 
      aVarMinMax[iVar][flatDim*2] == 0.0f &&
      aVarMinMax[iVar][flatDim*2+1] == 0.0f)
    {
      bFlattenVec = true;
    }

  vtkFloatArray *var = vtkFloatArray::New();
  var->SetNumberOfComponents( nComps );

  if (sFileVersion == "2.0") {
    int dims[3]; 
    GetBlockDims(domain, dims); 
    if (bZonal) {
      // zonal variables are always one less than mesh dimensions for version 2.0 data
      for (int i=0; i<3; i++) {
        if (dims[i] > 1) {
          dims[i]--; 
        }
      }
    }
    int nTuples = dims[0]*dims[1]*dims[2];
    var->SetNumberOfTuples( nTuples );

    char filename[512];
    if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], domain) < 0)
      EXCEPTION1(InvalidFilesException, "");
      
    FILE *fd = fopen(filename, "rb");
    if (fd == NULL)
      EXCEPTION1(InvalidFilesException, filename);
    vector<float> v(nTuples*nComps); // initialized to 0 by std
    float *bufp = &v[0];
    TRY
      {
        int iCurrComp = 0;
        for (jj = 0; jj < nComps; jj++)
          {
            if (bFlattenVec && jj==flatDim)
              continue;
          
            ReadRawScalar(fd, nPrevComp+jj, bufp + iCurrComp*nTuples, filename, domain, nTuples);
          
            iCurrComp++;
          }
      }
    CATCHALL
      {
        fclose(fd);
        RETHROW;
      }
    ENDTRY

      if (bFlattenVec)
        memset(bufp + 2*nTuples, 0, nTuples*sizeof(float));
      
    InterleaveData( (float *)(var->GetVoidPointer(0)), bufp, dims, nComps );      
         
    fclose(fd); 
  } else {
    int neighbors[8], realdim[3];
    float *aRawBlocks[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    vector<vector<float> > floatvectors(8); 

    FindNeighborDomains(domain, neighbors, realdim);
      
    int nDstTuples = realdim[0]*realdim[1]*realdim[2];
    int nSrcTuples = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
      
    var->SetNumberOfTuples( nDstTuples );
      
    for (ii = 0; ii < 8; ii++)
      {
        if (neighbors[ii] == -1)
          continue;
          
        char filename[512];
        if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], neighbors[ii]) < 0)
          EXCEPTION1(InvalidFilesException, "");
          
        FILE *fd = fopen(filename, "rb");
        if (fd == NULL)
          EXCEPTION1(InvalidFilesException, filename);
        floatvectors[ii].resize(nSrcTuples*nComps); 
        aRawBlocks[ii] = &floatvectors[ii][0];
          
        int iCurrComp = 0;
        for (jj = 0; jj < nComps; jj++)
          {
            if (bFlattenVec && jj==flatDim)
              continue;
              
            ReadRawScalar(fd, nPrevComp+jj, aRawBlocks[ii] + iCurrComp*nSrcTuples, filename, domain, nSrcTuples);
              
            iCurrComp++;
          }
        fclose(fd);
      }
    PackData( (float *)(var->GetVoidPointer(0)), aRawBlocks, realdim, nComps, true );
      
  }
  TIMERSTOP(str(format("GetVectorVar(%1%, %2%, %3%)")%timestate%domain%varname));
  return var;    
}
// ****************************************************************************
//  Method: avtMirandaFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets auxiliary data about the file format.
//
//  Programmer: Dave Bremer
//  Creation:   Jan 30, 2007
//
//  Modifications:
//    Dave Bremer, Wed Feb 21 19:54:33 PST 2007
//    Material data gets cleaned before it is returned.
//
//    Dave Bremer, Mon Mar 19 19:04:11 PDT 2007
//    Fix fopen on windows.  Add error checking.
//
//    Dave Bremer, June 25, 2009
//    Updated to handle curvilinear meshes, which requires treating data as
//    nodal, and reading ghost data from adjacent domains.
//
// ****************************************************************************
void *
avtMirandaFileFormat::GetAuxiliaryData(const char *var, int timestate,  
                                       int domain, const char *type, void *args,
                                       DestructorFunction &df)
{
  debug4 << "GetAuxiliaryData("<<var<<", timestate="<<timestate<<", domain="<<domain<<", type="<<type<<")"<< endl; 
  void *rv = NULL;
  int   nbricks = iNumBlocks[0] * iNumBlocks[1] * iNumBlocks[2];
  int   ii, jj, kk, mm;
  TIMERSTART(); 
  
  if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0 && !bCurvilinear)
    {
      if (strcmp(var, "mesh") != 0)
        {
          EXCEPTION1(InvalidVariableException, var);
        }
      avtIntervalTree *itree = new avtIntervalTree(nbricks, 3);
      for (ii = 0 ; ii < nbricks ; ii++)
        {
          int iBlockX, iBlockY, iBlockZ;
          DomainToIJK( ii, iBlockX, iBlockY, iBlockZ );
          
          //
          // Establish what the range is of this dataset.
          //
          double bounds[6];
          bounds[0] = fOrigin[0] + fStride[0]*iBlockSize[0]*iBlockX;
          bounds[2] = fOrigin[1] + fStride[1]*iBlockSize[1]*iBlockY;
          bounds[4] = fOrigin[2] + fStride[2]*iBlockSize[2]*iBlockZ;
          if (iBlockX != iNumBlocks[0]-1) {
            bounds[1] = bounds[0] + fStride[0]*iInteriorSize[0];
          } else {
            bounds[1] = bounds[0] + fStride[0]*iBoundarySize[0];
          }
          bounds[1] -= fStride[0]; 
          if (iBlockY != iNumBlocks[1]-1) {
            bounds[3] = bounds[2] + fStride[2]*iInteriorSize[2];
          } else {
            bounds[3] = bounds[2] + fStride[2]*iBoundarySize[2];
          }        
          bounds[3] -= fStride[1]; 
          if (iBlockZ != iNumBlocks[2]-1) {
            //bounds[5] = bounds[4] + fStride[4]*iInteriorSize[4];
            bounds[5] = bounds[4] + fStride[1]*iInteriorSize[1]; ///TODO: FIXME fStride & iInteriorSize are outside bounds size is only 3 I am setting it to 1 which is probably incorrect FIXME
              } else {
            //bounds[5] = bounds[4] + fStride[4]*iBoundarySize[4];
            bounds[5] = bounds[4] + fStride[1]*iBoundarySize[1]; ///TODO: FIXME fStride & iBoundarySize are outside bounds size is only 3 I am setting to 1 which is probably incorrect FIXME
              }        
          bounds[5] -= fStride[2]; 
          debug5 << "For domain "<<ii<<", "<<Vec2String("bounds", bounds, 6)<< endl;
          itree->AddElement(ii, bounds);
        }
      itree->Calculate(true);
      
      rv = (void *) itree;
      df = avtIntervalTree::Destruct;
    }
  else if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
      int     nMats    = (int)aMatNames.size();
      
      int    *matNums  = new int[nMats];
      char  **matNames = new char*[nMats];  //ewww.  
      float **volFracs = new float*[nMats];
      char    domainName[32];
      
      sprintf(domainName, "domain %d", domain);
      
      int iFirstComp = 0;
      for (ii = 0 ; ii < (int)aVarNumComps.size() ; ii++)
        iFirstComp += aVarNumComps[ii];
      
      
      int nDstTuples, nSrcTuples, nodaldims[3], zonaldims[3];
      int neighbors[8];
      float *aRawBlocks[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
      vector<vector<float> > floatvectors(8); 
      
      if (sFileVersion == "2.0") {
        GetBlockDims(domain, nodaldims); 
      } else {
        // file format < 2.0 ; data is nodal    
        FindNeighborDomains(domain, neighbors, nodaldims);
      }
      for (int i=0; i<3; i++) {
        zonaldims[i] = nodaldims[i]; 
        if (zonaldims[i] > 1) {
          zonaldims[i]--; 
        }
      }

        
      if (sFileVersion == "2.0") {
        if (bZonal) {
          nDstTuples = nSrcTuples = zonaldims[0]*zonaldims[1]*zonaldims[2];
        }            
        else {
          nDstTuples = nSrcTuples = nodaldims[0]*nodaldims[1]*nodaldims[2];
        }
      } else {
        // file format < 2.0 ; data is nodal    
          
        nSrcTuples = iBlockSize[0]*iBlockSize[1]*iBlockSize[2];
        nDstTuples = nodaldims[0]*nodaldims[1]*nodaldims[2];
          
      }

      vector<float> dstv(nDstTuples * nMats); 
      float *dst = &dstv[0];  

      if (sFileVersion == "2.0") {
          
        char filename[512];
        if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], domain) < 0)
          EXCEPTION1(InvalidFilesException, "");
          
        FILE *fd = fopen(filename, "rb");
        if (fd == NULL)
          EXCEPTION1(InvalidFilesException, filename);
          
        for (jj = 0; jj < nMats; jj++)
          {
            ReadRawScalar(fd, iFirstComp+jj, dst + jj*nDstTuples, filename, domain, nDstTuples);
            debug5 << "(2.0): Read "<< nDstTuples << " elements from " << filename << " for domain " << domain << endl;
            debug5 << Vec2String("First five elements", dst + jj*nDstTuples, 5) << endl; 
          }
        fclose(fd);

        // CHECK THIS:  for material data, I don't think we need PackData.  Just read into the actual array. 

      }
      else {
        // file format < 2.0       
        for (ii = 0; ii < 8; ii++)
          {
            if (neighbors[ii] == -1)
              continue;
              
            char filename[512];
            if (SNPRINTF(filename, 512, fileTemplate.c_str(), aCycles[timestate], neighbors[ii]) < 0)
              EXCEPTION1(InvalidFilesException, "");
              
            FILE *fd = fopen(filename, "rb");
            if (fd == NULL)
              EXCEPTION1(InvalidFilesException, filename);
              
            floatvectors[ii].resize(nSrcTuples*nMats); 
            aRawBlocks[ii] = &floatvectors[ii][0];
              
            for (jj = 0; jj < nMats; jj++)
              {
                ReadRawScalar(fd, iFirstComp+jj, aRawBlocks[ii] + jj*nSrcTuples, filename, domain, nSrcTuples);
              }
            fclose(fd);
          }
        PackData(dst, aRawBlocks, nodaldims, nMats, false);
      }
         
      for (jj = 0; jj < nMats; jj++)
        {
          matNums[jj] = jj;
          matNames[jj] = strdup( aMatNames[jj].c_str() );
            
          volFracs[jj] = dst + jj*nDstTuples;
        }
            

      /* For debugging, I'm going to clean up the data so that it
         can be processed.  I have zones with material fractions
         that range from -epsilon to 1+epsilon.  DJB */
        
      for (ii = 0 ; ii < nDstTuples ; ii++)
        {
          double sum = 0.00f;
          for (jj = 0 ; jj < nMats ; jj++)
            {
              if (volFracs[jj][ii] > 1.0 - 1e-5)
                volFracs[jj][ii] = 1.0f;
                
              if (volFracs[jj][ii] < 1e-5)   
                volFracs[jj][ii] = 0.0f;
                
              sum += volFracs[jj][ii];
            }
          for (jj = 0 ; jj < nMats ; jj++)
            {
              volFracs[jj][ii] /= sum;
            }
        }
        
      if (bCurvilinear && ! bZonal) {
        //In this case, we have node-centered material fractions 
        //that need to be averaged onto the zones.
        int numZIterations = (dim == 3) ? nodaldims[2]-1 : 1;
        
        for (mm = 0; mm < nMats; mm++) {
          int tt = 0;
          float *f = volFracs[mm];
          for (kk = 0; kk < numZIterations; kk++) {
            for (jj = 0; jj < nodaldims[1]-1; jj++) {
              for (ii = 0; ii < nodaldims[0]-1; ii++, tt++) {
                if (dim == 3) {
                  f[tt] = 
                    (f[ii   + jj*nodaldims[0]     + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + jj*nodaldims[0]     + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii   + (jj+1)*nodaldims[0] + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + (jj+1)*nodaldims[0] + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii   + jj*nodaldims[0]     + (kk+1)*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + jj*nodaldims[0]     + (kk+1)*nodaldims[0]*nodaldims[1]] +
                     f[ii   + (jj+1)*nodaldims[0] + (kk+1)*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + (jj+1)*nodaldims[0] + (kk+1)*nodaldims[0]*nodaldims[1]]) / 8.0;
                }
                else {
                  //dim == 2
                  f[tt] = 
                    (f[ii   + jj*nodaldims[0]     + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + jj*nodaldims[0]     + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii   + (jj+1)*nodaldims[0] + kk*nodaldims[0]*nodaldims[1]] +
                     f[ii+1 + (jj+1)*nodaldims[0] + kk*nodaldims[0]*nodaldims[1]]) / 4.0;
                }
              } /* end loop over ii */   
            } /* end loop over jj */   
          } /* end loop over numZIterations */   
        } /* end loop over nMats */                            
      } /* end of zonal averaging from nodes for nodal curvilinear case */ 
      rv = (void *)new avtMaterial( nMats,
                                    matNums,
                                    matNames,
                                    dim, 
                                    zonaldims,
                                    0 /*major_order*/,
                                    volFracs,
                                    domainName);
      df = avtMaterial::Destruct;
      
      for (ii = 0 ; ii < nMats ; ii++)
        {
          free(matNames[ii]);
        }
      delete[] matNums;
      delete[] matNames;
      delete[] volFracs;
    }
  TIMERSTOP(str(format("GetAuxiliaryData(%1%, %2%, %3%, %4%)")%var%timestate%domain%type));
  return rv;
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::FindNeighborDomains
//
//  Purpose:
//      For a given domain, find the neighbor domains from which ghost nodes 
//      should be read.  Rectilinear data will have no ghost nodes.
//
//  Arguments:
//      domain     domain of interest
//      neighbors  8 output values.  neighbors[0] = domain, others are neighbors
//                 in the increasing x, y, and z directions, or -1 if no 
//                 neighbor in that direction.
//      realdim    3 output values--the dimensions of the domain including any
//                 ghost nodes.
//
//  Programmer: Dave Bremer
//  Creation:   June 25, 2009
//
// ****************************************************************************
void
avtMirandaFileFormat::FindNeighborDomains(int domain, int *neighbors, 
                                          int *realdim)
{
  int ii, jj, kk;
  for (ii = 0; ii < 3; ii++)
    realdim[ii] = iBlockSize[ii];


  if (bZonal) {
    // don't need to read any neighbors for zonal case 
    neighbors[0] = domain;
    for (ii = 1; ii < 8; ii++)
      neighbors[ii] = -1;
  }
  else {
    int di, dj, dk;
    int incr[3];
    DomainToIJK(domain, di, dj, dk);
    
    if (di < iNumBlocks[0]-1)
      realdim[0]++;
    
    if (dj < iNumBlocks[1]-1)
      realdim[1]++;
    
    if (dk < iNumBlocks[2]-1)
      realdim[2]++;

    //fileorder flag must be set for curvilinear meshes
    if (iFileOrder[0] == -1)
      EXCEPTION1(InvalidFilesException, "");

    incr[iFileOrder[0]] = 1;
    incr[iFileOrder[1]] = iNumBlocks[iFileOrder[0]];
    incr[iFileOrder[2]] = iNumBlocks[iFileOrder[0]] * iNumBlocks[iFileOrder[1]];
    int *curr = neighbors;
    for (kk = 0; kk < 2; kk++) {
      for (jj = 0; jj < 2; jj++)  {
        for (ii = 0; ii < 2; ii++) {
          if ( di + ii >= iNumBlocks[0] ||
               dj + jj >= iNumBlocks[1] ||
               dk + kk >= iNumBlocks[2] ) {
            *curr = -1;
          }
          else {
            *curr = domain + ii*incr[0] + jj*incr[1] + kk*incr[2];
          }
          curr++;
        }
      }
    }
  }
}

// ****************************************************************************
//  Method: avtMirandaFileFormat::InterleaveData
//  Purpose:
//      Convert data from block format in source buffer into interleaved format
//       in destination
//
//  Arguments:
//      dst         output buffer
//      src         array of pointers to 8 neighbor buffers.  NULL val means no 
//                  neighbor on that side
//      dstDim      size of output buffer
//      nComp       number of components to pack.  If src is an xyz vec, all the
//                  x values come first, then all y, then all z.
//      interleave  if true, components in dst are in x, y, z tuples, otherwise
//                  separate into blocks of x, y, and z.  mesh and vec data are
//                  interleaved, mat data is separated.
//
//  Programmer: Dave Bremer
//  Creation:   June 25, 2009
//
// ****************************************************************************
void
avtMirandaFileFormat::InterleaveData( float * __restrict dst, float *__restrict src, int *dstDim, int nComp)
{
  debug4 << "InterleaveData(<"<<dstDim[0]<<", "<<dstDim[1]<<", "<<dstDim[2]<<">, "<<nComp<<") "<< endl;
  TIMERSTART(); 
  int numItems = dstDim[0]*dstDim[1]*dstDim[2]; 
  if (nComp == 1) {
    memcpy(dst, src, numItems*sizeof(float)); 
    return; 
  }
  //float *__restrict srcp = src, *__restrict dstp = dst; 
  int item, comp; 
  for (item=0; item < numItems; item++) {
    for (comp=0; comp < nComp; comp++) {
      dst[nComp*item+comp] = src[comp*numItems + item]; 
    }    
  }
  TIMERSTOP(str(format("InterleaveData(<%1%, %2%, %3%>, %4%)")%dstDim[0]%dstDim[1]%dstDim[2]%nComp));
  return; 
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::PackData
//
//  Purpose:
//      Takes 8, 4, 2, or 1 blocks--a domain and its neighbors, if any, in the 
//      increasing x, y, and z directions--and packs the block and one layer 
//      of ghost nodes into an output buffer.  This is used for mesh, var, vec,
//      and mat data.  Only used for versions less than 2.0.
//
//  Arguments:
//      dst         output buffer
//      src         array of pointers to 8 neighbor buffers.  NULL val means no 
//                  neighbor on that side
//      dstDim      size of output buffer
//      nComp       number of components to pack.  If src is an xyz vec, all the
//                  x values come first, then all y, then all z.
//      interleave  if true, components in dst are in x, y, z tuples, otherwise
//                  separate into blocks of x, y, and z.  mesh and vec data are
//                  interleaved, mat data is separated.
//
//  Programmer: Dave Bremer
//  Creation:   June 25, 2009
//
// ****************************************************************************
void
avtMirandaFileFormat::PackData(float *__restrict dst, const  float *  const * __restrict src, 
                               const int *dstDim, int nComp, bool interleave)
{
  debug4 << "PackData(<"<<dstDim[0]<<", "<<dstDim[1]<<", "<<dstDim[2]<<">, "<<nComp<<")" << endl;
  TIMERSTART(); 
  int bb, ii, jj, kk, cc;
  int srcDim[8][3] = {{iBlockSize[0], iBlockSize[1], iBlockSize[2]},
                      {1,             iBlockSize[1], iBlockSize[2]},
                      {iBlockSize[0], 1,             iBlockSize[2]},
                      {1,             1,             iBlockSize[2]},
                      {iBlockSize[0], iBlockSize[1], 1},
                      {1,             iBlockSize[1], 1},
                      {iBlockSize[0], 1,             1},
                      {1,             1,             1}};

  int dstOffset[8][3] = {{0,             0,             0},
                         {iBlockSize[0], 0,             0},
                         {0,             iBlockSize[1], 0},
                         {iBlockSize[0], iBlockSize[1], 0},
                         {0,             0,             iBlockSize[2]},
                         {iBlockSize[0], 0,             iBlockSize[2]},
                         {0,             iBlockSize[1], iBlockSize[2]},
                         {iBlockSize[0], iBlockSize[1], iBlockSize[2]} };

  for (bb = 0; bb < 8; bb++) {
    if (src[bb] == NULL)
      continue;
    
    for (kk = 0; kk < srcDim[bb][2]; kk++) {
      for (jj = 0; jj < srcDim[bb][1]; jj++) {
        for (ii = 0; ii < srcDim[bb][0]; ii++) {
          int iCurrDstTuple = ii + dstOffset[bb][0] + 
            (jj + dstOffset[bb][1]) * dstDim[0] +
            (kk + dstOffset[bb][2]) * dstDim[0] * dstDim[1];
          
          for (cc = 0; cc < nComp; cc++)  {
            float s = src[bb][ii + 
                              jj*iBlockSize[0] + 
                              kk*iBlockSize[0]*iBlockSize[1] + 
                              cc*iBlockSize[0]*iBlockSize[1]*iBlockSize[2]];
            if (interleave)
              dst[iCurrDstTuple*nComp + cc] = s;
            else
              dst[iCurrDstTuple + cc*dstDim[0]*dstDim[1]*dstDim[2]] = s;
          }
        }
      }
    }
  }
  TIMERSTOP(str(format("PackData(<%1%, %2%, %3%>, %4%)")%dstDim[0]%dstDim[1]%dstDim[2]%nComp));
}


// ****************************************************************************
//  Method: avtMirandaFileFormat::ReadRawScalar
//
//  Purpose:
//      Reads one block of scalar values out of a miranda file.
//
//  Arguments:
//      fd        open file descriptor
//      iComp     index of component to read
//      out       already allocated output buffer
//      filename  name of fd, only used for error reporting
//
//  Programmer: Dave Bremer
//  Creation:   June 25, 2009
//
// ****************************************************************************
void
avtMirandaFileFormat::ReadRawScalar(FILE *fd, int iComp, float *out, const char *filename, int domain, int nPoints)
{
  debug4 << "ReadRawScalar("<<iComp<<", "<<filename<<", "<<domain<<", "<<nPoints<<")"<< endl; 
  TIMERSTART(); 
  
  int header, nItemsRead, ii;
  int err;
  
  err = fseek(fd, iComp*(nPoints*sizeof(float) + 2*sizeof(int)), SEEK_SET);
  if (err)
    EXCEPTION1(InvalidFilesException, filename);
  
  // Fortran records have a 32 bit int that tells their length.  
  // We read that here to test endianism. 
  size_t res = fread(&header, sizeof(int), 1, fd); (void) res;
  nItemsRead = (int)fread(out, sizeof(float), nPoints, fd);
  if (nItemsRead != nPoints)
    EXCEPTION1(InvalidFilesException, filename);
  
  if ((size_t)header != nPoints*sizeof(float)) {
    //If this is true, we need to swap endian
    ByteSwap32(&header);
    if ((size_t)header != nPoints*sizeof(float))
      EXCEPTION1(InvalidFilesException, filename);
    
    float *f = out;
    for (ii = 0 ; ii < nPoints ; ii++, f++) {
      ByteSwap32(f);
    }
  }
  TIMERSTOP(str(format("ReadRawScalar(%1%, %2%, %3%)")%iComp% filename % domain));
}

