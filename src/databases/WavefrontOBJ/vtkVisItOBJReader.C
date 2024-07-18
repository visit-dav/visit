/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOBJReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItOBJReader.h"

#include <InvalidFilesException.h>
#include <FileFunctions.h>

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"

#include <cstring>

vtkStandardNewMacro(vtkVisItOBJReader);

// Description:
// Instantiate object with NULL filename.
vtkVisItOBJReader::vtkVisItOBJReader()
{
  this->FileName = NULL;

  this->SetNumberOfInputPorts(0);
}

vtkVisItOBJReader::~vtkVisItOBJReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}

/*--------------------------------------------------------

This is only partial support for the OBJ format, which is
quite complicated. To find a full specification,
search the net for "OBJ format", eg.:

http://netghost.narod.ru/gff/graphics/summary/waveobj.htm

We support the following types:

v <x> <y> <z>              vertex

vn <x> <y> <z>             vertex normal

vt <x> <y>                 texture coordinate

f <v_a> <v_b> <v_c> ...

polygonal face linking vertices v_a, v_b, v_c, etc. which
are 1-based indices into the vertex list

f <v_a>/<t_a> <v_b>/<t_b> ...

polygonal face as above, but with texture coordinates for
each vertex. t_a etc. are 1-based indices into the texture
coordinates list (from the vt lines)

f <v_a>/<t_a>/<n_a> <v_b>/<t_b>/<n_b> ...

polygonal face as above, with a normal at each vertex, as a
1-based index into the normals list (from the vn lines)

f <v_a>//<n_a> <v_b>//<n_b> ...

polygonal face as above but without texture coordinates.

Per-face tcoords and normals are supported by duplicating
the vertices on each face as necessary.


Mark C. Miller, Fri Oct 19 09:45:48 PDT 2018

I added logic to deal with relative node, normal and
texture-coordinate references (e.g. negative values). I
also added logic to handle groups, materials and material
colors. This involves the attachment of abstract and
ordinary cell data arrays to the returned VTK dataset.
However, these arrays are in no way related to the poly
data "mesh" object the dataset represents. These arrays
are re-interpreted by avtWavefrontOBJFileFormat class to
have the intended effect in VisIt.

---------------------------------------------------------*/

// a replacement for isspace()
int is_whitespace(char c)
{
  if ( c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\v' || c=='\f')
    return 1;
  else
    return 0;
}

// Parse an OBJ material file for material "names" and
// coloration. Only the ambient color (Ka) is examined.
static int
ParseMTLFile(char const *objFileName, char const *mtlFileName,
    vtkStringArray *colorNames, vtkFloatArray *rgbValues)
{
  const int MAX_LINE=8192;
  char line[MAX_LINE];
  float rgb[3];
  int everything_ok = 1;
  FILE *in;

  if (mtlFileName[0] == '/') // abs path case
    {
    in = fopen(mtlFileName,"r");
    }
  else
    {
    char tmpFileName[2048];
    snprintf(tmpFileName, sizeof(tmpFileName), "%s/%s",
      FileFunctions::Dirname(objFileName), mtlFileName);
    in = fopen(FileFunctions::Normalize(tmpFileName),"r");
    }
  if (!in) return 0;
  while (everything_ok && fgets(line,MAX_LINE,in)!=NULL)
    {
    if (strncmp(line,"newmtl ",7)==0)
      {
      int len = (int) strlen(line);
      line[len-1] = '\0'; // chop off newline char
      colorNames->InsertNextValue(&line[7]);
      }
    else if (strncmp(line,"Ka ",3)==0)
      {
      // this is ambient color definition, expect three floats, separated by whitespace:
      if (sscanf(line, "Ka %f %f %f", &rgb[0], &rgb[1], &rgb[2])==3)
        {
        rgbValues->InsertNextTypedTuple(rgb);
        }
      else
        {
        everything_ok=0; // (false)
        }
      }
    else
      {
      //vtkDebugMacro(<<"Ignoring line: "<<line);
      }
    }
  fclose(in);
  return everything_ok;
}

int vtkVisItOBJReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->FileName)
    {
    vtkErrorMacro(<< "A FileName must be specified.");
    return 0;
    }

  FILE *in = fopen(this->FileName,"r");

  if (in == NULL)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
    }

  vtkDebugMacro(<<"Reading file");

  // intialise some structures to store the file contents in
  vtkPoints *points = vtkPoints::New();
  vtkFloatArray *tcoords = vtkFloatArray::New();
  tcoords->SetNumberOfComponents(2);
  vtkFloatArray *normals = vtkFloatArray::New();
  normals->SetNumberOfComponents(3);
  vtkCellArray *polys = vtkCellArray::New();
  vtkCellArray *tcoord_polys = vtkCellArray::New();
  int hasTCoords=0; // (false)
  int tcoords_same_as_verts=1; // (true)
  vtkCellArray *normal_polys = vtkCellArray::New();
  int hasNormals=0; // (false)
  int normals_same_as_verts=1; // (true)

  vtkIntArray *groupIds = vtkIntArray::New();
  vtkStringArray *groupNames = vtkStringArray::New();
  vtkStringArray *groupColors = vtkStringArray::New();
  int currGroupId = -1;

  int everything_ok = 1; // (true)   (use of this flag avoids early return and associated memory leak)

  // -- work through the file line by line, assigning into the above six structures as appropriate --

  { // (make a local scope section to emphasise that the variables below are only used here)

  const int MAX_LINE=8192;
  char line[MAX_LINE],*pChar;
  float xyz[3];
  int iVert,iTCoord,iNormal;
  while (everything_ok && fgets(line,MAX_LINE,in)!=NULL)
    {

    // in the OBJ format the first characters determine how to interpret the line:
    if (strncmp(line,"v ",2)==0)
      {
      // this is a vertex definition, expect three floats, separated by whitespace:
      if (sscanf(line, "v %f %f %f", xyz, xyz + 1, xyz + 2)==3)
        {
        points->InsertNextPoint(xyz);
        }
      else
        {
        vtkErrorMacro(<<"Error in reading file");
        everything_ok=0; // (false)
        }
      }
    else if (strncmp(line,"vt ",3)==0)
      {
      // this is a tcoord, expect two floats, separated by whitespace:
      if (sscanf(line, "vt %f %f", xyz, xyz + 1)==2)
        {
        tcoords->InsertNextTuple(xyz);
        }
      else
        {
        vtkErrorMacro(<<"Error in reading file");
        everything_ok=0; // (false)
        }
      }
    else if (strncmp(line,"vn ",3)==0)
      {
      // this is a normal, expect three floats, separated by whitespace:
      if (sscanf(line, "vn %f %f %f", xyz, xyz + 1, xyz + 2)==3)
        {
        normals->InsertNextTuple(xyz);
        }
      else
        {
        vtkErrorMacro(<<"Error in reading file");
        everything_ok=0; // (false)
        }
      }
    else if (strncmp(line,"f ",2)==0 || strncmp(line,"fo ",3)==0)   // not sure why "fo" here
      {
      // this is a face definition, consisting of 1-based indices separated by whitespace and /

      polys->InsertNextCell(0); // we don't yet know how many points are to come
      tcoord_polys->InsertNextCell(0);
      normal_polys->InsertNextCell(0);

      if (groupNames->GetNumberOfTuples())
        groupIds->InsertNextValue(currGroupId);

      int numPoints = points->GetNumberOfPoints();
      int numNormals = normals->GetNumberOfTuples();
      int numTCoords = tcoords->GetNumberOfTuples();

      int nVerts=0,nTCoords=0,nNormals=0; // keep a count of how many of each there are

      pChar = line + 2;
      const char *pEnd = line + strlen(line);

      while (everything_ok && pChar<pEnd)
        {
        // find the first non-whitespace character
        while (is_whitespace(*pChar) && pChar<pEnd) { pChar++; }

        if (pChar<pEnd)         // there is still data left on this line
          {
          if (sscanf(pChar,"%d/%d/%d",&iVert,&iTCoord,&iNormal)==3)
            {
                polys->InsertCellPoint(iVert<0?numPoints+iVert:iVert-1); // convert to 0-based index
                nVerts++;
                tcoord_polys->InsertCellPoint(iTCoord<0?numTCoords+iTCoord:iTCoord-1);
                nTCoords++;
                if (iTCoord!=iVert && tcoords_same_as_verts)
                    tcoords_same_as_verts = 0; // (false)
                normal_polys->InsertCellPoint(iNormal<0?numNormals+iNormal:iNormal-1);
                nNormals++;
                if (iNormal!=iVert && normals_same_as_verts)
                    normals_same_as_verts = 0; // (false)
            }
          else if (sscanf(pChar,"%d//%d",&iVert,&iNormal)==2)
            {
                polys->InsertCellPoint(iVert<0?numPoints+iVert:iVert-1);
                nVerts++;
                normal_polys->InsertCellPoint(iNormal<0?numNormals+iNormal:iNormal-1);
                nNormals++;
                if (iNormal!=iVert && normals_same_as_verts)
                    normals_same_as_verts = 0; // (false)
            }
          else if (sscanf(pChar,"%d/%d",&iVert,&iTCoord)==2)
            {
                polys->InsertCellPoint(iVert<0?numPoints+iVert:iVert-1);
                nVerts++;
                tcoord_polys->InsertCellPoint(iTCoord<0?numTCoords+iTCoord:iTCoord-1);
                nTCoords++;
                if (iTCoord!=iVert && tcoords_same_as_verts)
                    tcoords_same_as_verts = 0; // (false)
            }
          else if (sscanf(pChar,"%d",&iVert)==1)
            {
                polys->InsertCellPoint(iVert<0?numPoints+iVert:iVert-1);
                nVerts++;
            }
          else
            {
            vtkErrorMacro(<<"Error in reading file");
            everything_ok=0; // (false)
            }
          // skip over what we just read
          // (find the first whitespace character)
          while (!is_whitespace(*pChar) && pChar<pEnd)
            {
            pChar++;
            }
          }
        }

      // count of tcoords and normals must be equal to number of vertices or zero
      if (nVerts==0 || (nTCoords>0 && nTCoords!=nVerts) || (nNormals>0 && nNormals!=nVerts))
        {
        vtkErrorMacro(<<"Error in reading file");
        everything_ok=0; // (false)
        }

      // now we know how many points there were in this cell
      polys->UpdateCellCount(nVerts);
      tcoord_polys->UpdateCellCount(nTCoords);
      normal_polys->UpdateCellCount(nNormals);

      // also make a note of whether any cells have tcoords, and whether any have normals
      if (nTCoords>0 && !hasTCoords) { hasTCoords=1; }
      if (nNormals>0 && !hasNormals) { hasNormals=1; }
      }
    else if (strncmp(line,"g ",2)==0) // group name(s) multiple names handled by consumer
      {
      currGroupId = groupNames->LookupValue(&line[2]);
      if (currGroupId < 0)
        {
        int len = (int) strlen(line);
        line[len-1] = '\0'; // chop off newline char
        currGroupId = groupNames->InsertNextValue(&line[2]);
        }
      }
    else if (strncmp(line,"mtllib ",7)==0)
      {
      vtkFloatArray *rgbValues = vtkFloatArray::New();
      rgbValues->SetNumberOfComponents(3);
      vtkStringArray *colorNames = vtkStringArray::New();
      int len = (int) strlen(line);
      line[len-1] = '\0'; // chop off newline char
      if (ParseMTLFile(this->FileName, &line[7], colorNames, rgbValues))
        {
        colorNames->SetName("_vtkVisItOBJReader_ColorNames");
        rgbValues->SetName("_vtkVisItOBJReader_RGBValues");
        output->GetCellData()->AddArray(colorNames);
        output->GetCellData()->AddArray(rgbValues);
        }
      rgbValues->Delete();
      colorNames->Delete();
      }
    else if (strncmp(line,"usemtl ",7)==0)
      {
        int len = (int) strlen(line);
        line[len-1] = '\0'; // chop off newline char
        groupColors->InsertNextValue(&line[7]);
      }
    else
      {
      //vtkDebugMacro(<<"Ignoring line: "<<line);
      }

    } // (end of while loop)

  } // (end of local scope section)

  // we have finished with the file
  fclose(in);

  if (everything_ok)   // (otherwise just release allocated memory and return)
    {
    // -- now turn this lot into a useable vtkPolyData --

    // if there are no tcoords or normals or they match exactly
    // then we can just copy the data into the output (easy!)
    if ((!hasTCoords||tcoords_same_as_verts) && (!hasNormals||normals_same_as_verts))
      {
      vtkDebugMacro(<<"Copying file data into the output directly");

      output->SetPoints(points);
      output->SetPolys(polys);
      if (groupNames->GetNumberOfTuples())
        {
        groupIds->SetName("_vtkVisItOBJReader_AggregateGroupIDs");
        groupNames->SetName("_vtkVisItOBJReader_AggregateGroupNames");
        output->GetCellData()->AddArray(groupIds);
        output->GetCellData()->AddArray(groupNames);
        }
      if (groupColors->GetNumberOfTuples())
        {
        groupColors->SetName("_vtkVisItOBJReader_AggregateGroupColors");
        output->GetCellData()->AddArray(groupColors);
        }

      // if there is an exact correspondence between tcoords and vertices then can simply
      // assign the tcoords points as point data
      if (hasTCoords && tcoords_same_as_verts)
        output->GetPointData()->SetTCoords(tcoords);

      // if there is an exact correspondence between normals and vertices then can simply
      // assign the normals as point data
      if (hasNormals && normals_same_as_verts)
        {
        output->GetPointData()->SetNormals(normals);
        }
      output->Squeeze();
      }
    // otherwise we can duplicate the vertices as necessary (a bit slower)
    else
      {
      vtkDebugMacro(<<"Duplicating vertices so that tcoords and normals are correct");

      vtkPoints *new_points = vtkPoints::New();
      vtkFloatArray *new_tcoords = vtkFloatArray::New();
      new_tcoords->SetNumberOfComponents(2);
      vtkFloatArray *new_normals = vtkFloatArray::New();
      new_normals->SetNumberOfComponents(3);
      vtkCellArray *new_polys = vtkCellArray::New();
      vtkIntArray *new_groupIds = groupIds->GetNumberOfTuples()>0?vtkIntArray::New():0;

      // for each poly, copy its vertices into new_points (and point at them)
      // also copy its tcoords into new_tcoords
      // also copy its normals into new_normals
      polys->InitTraversal();
      tcoord_polys->InitTraversal();
      normal_polys->InitTraversal();
      vtkIdType n_pts=-1, n_tcoord_pts=-1, n_normal_pts=-1;
      const vtkIdType *pts=nullptr, *tcoord_pts=nullptr, *normal_pts=nullptr;
      for (int i=0;i<polys->GetNumberOfCells();i++)
        {
        polys->GetNextCell(n_pts,pts);
        tcoord_polys->GetNextCell(n_tcoord_pts,tcoord_pts);
        normal_polys->GetNextCell(n_normal_pts,normal_pts);

        // If some vertices have tcoords and not others (likewise normals)
        // then we must do something else VTK will complain. (crash on render attempt)
        // Easiest solution is to delete polys that don't have complete tcoords (if there
        // are any tcoords in the dataset) or normals (if there are any normals in the dataset).

        if ( (n_pts!=n_tcoord_pts && hasTCoords) || (n_pts!=n_normal_pts && hasNormals) )
          {
          // skip this poly
          vtkDebugMacro(<<"Skipping poly "<<i+1<<" (1-based index)");
          }
        else
          {
          vtkNew<vtkIdList> newCellPts;
          newCellPts->SetNumberOfIds(n_pts);
          // copy the corresponding points, tcoords and normals across
          for (vtkIdType j=0;j<n_pts;j++)
            {
            // copy the tcoord for this point across (if there is one)
            if (n_tcoord_pts>0)
              {
              new_tcoords->InsertNextTuple(tcoords->GetTuple(tcoord_pts[j]));
              }
            // copy the normal for this point across (if there is one)
            if (n_normal_pts>0)
              {
              new_normals->InsertNextTuple(normals->GetTuple(normal_pts[j]));
              }
            // copy the vertex into the new structure and update
            // the vertex index in the polys structure (pts is a pointer into it)
            newCellPts->SetId(j, new_points->InsertNextPoint(points->GetPoint(pts[j])));
            }
          // copy this poly (pointing at the new points) into the new polys list
          new_polys->InsertNextCell(newCellPts);
          new_groupIds->InsertNextValue(groupIds->GetValue(i));
          }
        }

      // use the new structures for the output
      output->SetPoints(new_points);
      output->SetPolys(new_polys);
      if (groupNames->GetNumberOfTuples())
        {
        new_groupIds->SetName("_vtkVisItOBJReader_AggregateGroupIDs");
        groupNames->SetName("_vtkVisItOBJReader_AggregateGroupNames");
        output->GetCellData()->AddArray(new_groupIds);
        output->GetCellData()->AddArray(groupNames);
        new_groupIds->Delete();
        }
      if (groupColors->GetNumberOfTuples())
        {
        groupColors->SetName("_vtkVisItOBJReader_AggregateGroupColors");
        output->GetCellData()->AddArray(groupColors);
        }
      if (hasTCoords)
        {
        output->GetPointData()->SetTCoords(new_tcoords);
        }
      if (hasNormals)
        {
        output->GetPointData()->SetNormals(new_normals);
        }
      output->Squeeze();

      new_points->Delete();
      new_polys->Delete();
      new_tcoords->Delete();
      new_normals->Delete();
      }
    }

  points->Delete();
  tcoords->Delete();
  normals->Delete();
  polys->Delete();
  tcoord_polys->Delete();
  normal_polys->Delete();
  groupIds->Delete();
  groupNames->Delete();
  groupColors->Delete();

  return 1;
}

void vtkVisItOBJReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";

}
