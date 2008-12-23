/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/*
 *  StreamlineAnalyzer.cc:
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2005
 *
 *  Copyright (C) 2005 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Containers/Handle.h>
#include <Core/Containers/StringUtil.h>

#include <Packages/FusionPSE/Dataflow/Modules/Fields/StreamlineAnalyzer.h>

namespace FusionPSE {

using namespace SCIRun;

class StreamlineAnalyzer : public Module
{
public:
  StreamlineAnalyzer(GuiContext* ctx);
  virtual ~StreamlineAnalyzer();

  virtual void execute();

protected:
  GuiString gui_PlanesStr_;
  GuiInt gui_PlanesInt_;
  GuiInt gui_Color_;
  GuiInt gui_MaxWindings_;
  GuiInt gui_Override_;
  GuiInt gui_Order_;
  GuiInt gui_CurveMesh_;
  GuiInt gui_ScalarField_;
  GuiInt gui_ShowIslands_;
  GuiInt gui_IslandCentroids_;
  GuiInt gui_Overlaps_;

  vector< double > planes_;

  FieldHandle sl_field_output_handle_;
  FieldHandle pcc_field_output_handle_;
  FieldHandle pcs_field_output_handle_;
};


DECLARE_MAKER(StreamlineAnalyzer)

StreamlineAnalyzer::StreamlineAnalyzer(GuiContext* context)
  : Module("StreamlineAnalyzer", context, Source, "Fields", "FusionPSE"),
    gui_PlanesStr_(context->subVar("planes-list"), "0.0"),
    gui_PlanesInt_(context->subVar("planes-quantity"), 0),
    gui_Color_(context->subVar("color"), 1),
    gui_MaxWindings_(context->subVar("maxWindings"), 30),
    gui_Override_(context->subVar("override"), 0),
    gui_Order_(context->subVar("order"), 0),
    gui_CurveMesh_(context->subVar("curve-mesh"), 1),
    gui_ScalarField_(context->subVar("scalar-field"), 1),
    gui_ShowIslands_(context->subVar("show-islands"), 0),
    gui_IslandCentroids_(context->subVar("island-centroids"), 1),
    gui_Overlaps_(context->subVar("overlaps"), 1),
    sl_field_output_handle_(0),
    pcc_field_output_handle_(0),
    pcs_field_output_handle_(0)
{
}

StreamlineAnalyzer::~StreamlineAnalyzer()
{
}

void
StreamlineAnalyzer::execute()
{
  // The streamline or poincare field input is required.
  FieldHandle sl_field_input_handle;

  if( get_input_handle( "Input Streamlines", sl_field_input_handle, true ) )
  {
    string if_name =
      sl_field_input_handle->get_type_description(Field::MESH_TD_E)->get_name();
    if (if_name.find("CurveMesh")       == string::npos &&
        if_name.find("StructCurveMesh") == string::npos &&
        if_name.find("PointCloudMesh")  == string::npos ) {
      error("Only available for (Struct)CurveFields or PointCloudFields.");
      return;
    }

    if (!sl_field_input_handle->query_scalar_interface(this).get_rep()) {
      error("Only available for Scalar data.");
      return;
    }
  }

  // The centroid field input is optional.
  FieldHandle pcc_field_input_handle;

  if (get_input_handle( "Input Centroids", pcc_field_input_handle, false )) 
  {
    string pc_name =
      pcc_field_input_handle->get_type_description(Field::MESH_TD_E)->get_name();
    string pc_type =
      pcc_field_input_handle->get_type_description(Field::FDATA_TD_E)->get_name();

    if (pc_name.find( "PointCloudMesh") == string::npos ||
        pc_type.find( "double")         == string::npos ) {
      error("Only available for Point Cloud Meshes of type double.");
      error("Found a mesh of " + pc_name + " and a type of " + pc_type );
      return;
    }

    if (!pcc_field_input_handle->query_scalar_interface(this).get_rep()) {
      error("Only available for Scalar data.");
      return;
    }
  }

  // The separatrices field input is optional.
  FieldHandle pcs_field_input_handle;

  if (get_input_handle( "Input Separatrices", pcs_field_input_handle, false ))
  {
    string pc_name =
      pcs_field_input_handle->get_type_description(Field::MESH_TD_E)->get_name();
    string pc_type =
      pcs_field_input_handle->get_type_description(Field::FDATA_TD_E)->get_name();

    if (pc_name.find( "PointCloudMesh") == string::npos ||
        pc_type.find( "double")         == string::npos ) {
      error("Only available for Point Cloud Meshes of type double.");
      error("Found a mesh of " + pc_name + " and a type of " + pc_type );
      return;
    }

    if (!pcs_field_input_handle->query_scalar_interface(this).get_rep()) {
      error("Only available for Scalar data.");
      return;
    }
  }


  cerr << "StreamlineAnalyzer getting gui " << endl;

  vector< double > planes(0);

  if( gui_PlanesInt_.get() ) {

    unsigned int nplanes = gui_PlanesInt_.get();

    for( unsigned int i=0; i<nplanes; i++ )
      planes.push_back(2.0 * M_PI * (double) i / (double) nplanes );

  } else {

    istringstream plist(gui_PlanesStr_.get());
    double plane;
    while(!plist.eof()) {
      plist >> plane;
      if (plist.fail()) {
        if (!plist.eof()) {
          plist.clear();
          warning("List of Planes was bad at character " +
                  to_string((int)(plist.tellg())) +
                  "('" + ((char)(plist.peek())) + "').");
        }
        break;

      } else if (!plist.eof() && plist.peek() == '%') {
        plist.get();
        plane = 0 + (2.0*M_PI - 0) * plane / 100.0;
      }

      if( 0 <= plane && plane <= 2.0*M_PI )
        planes.push_back(plane);
      else {
        error("Plane is not in the range of 0 to 2 PI.");
        return;
      }
    }
  }


  if( planes_.size() != planes.size() ){
    inputs_changed_ = true;

    planes_.resize(planes.size());

    for( unsigned int i=0; i<planes.size(); i++ )
      planes_[i] = planes[i];

  } else {
    for( unsigned int i=0; i<planes.size(); i++ ) {
      if( fabs( planes_[i] - planes[i] ) > 1.0e-4 ) {
        planes_[i] = planes[i];
        inputs_changed_ = true;
      }
    }
  }

  cerr << "StreamlineAnalyzer executing " << endl;

  // If no data or a changed recalcute.
  if( inputs_changed_ ||

      !sl_field_output_handle_.get_rep() ||

      gui_Color_.changed( true ) ||
      gui_MaxWindings_.changed( true ) ||
      gui_Override_.changed( true ) ||
      gui_Order_.changed( true ) ||
      gui_CurveMesh_.changed( true ) ||
      gui_ScalarField_.changed( true ) ||
      gui_ShowIslands_.changed( true ) ||
      gui_IslandCentroids_.changed( true ) ||
      gui_Overlaps_.changed( true ) ) {

    update_state( Executing );

    const TypeDescription *ftd = sl_field_input_handle->get_type_description();

    const TypeDescription *mtd =
      ( gui_CurveMesh_.get() ?
        get_type_description( (StreamlineAnalyzerAlgo::CMesh*) 0) : 
        get_type_description( (StreamlineAnalyzerAlgo::SQSMesh*) 0) );

    const TypeDescription *btd =
      ( gui_CurveMesh_.get() ?
        get_type_description( (StreamlineAnalyzerAlgo::CDatBasis*) 0) : 
        get_type_description( (StreamlineAnalyzerAlgo::SQSDatBasis*) 0) );

    const TypeDescription *dtd = ( gui_ScalarField_.get() ?
                                   get_type_description( (double*) 0) : 
                                   get_type_description( (Vector*) 0) );

    CompileInfoHandle ci =
      StreamlineAnalyzerAlgo::get_compile_info(ftd, mtd, btd, dtd);

    Handle<StreamlineAnalyzerAlgo> algo;
    if (!module_dynamic_compile(ci, algo)) return;

    vector< pair< unsigned int, unsigned int > > topology;

    algo->execute(sl_field_input_handle,   sl_field_output_handle_,
                  pcc_field_input_handle, pcc_field_output_handle_,
                  pcs_field_input_handle, pcs_field_output_handle_,
                  planes_,
                  gui_Color_.get(),
                  gui_ShowIslands_.get(), gui_IslandCentroids_.get(),
                  gui_Overlaps_.get(),
                  gui_MaxWindings_.get(), gui_Override_.get(), gui_Order_.get(),
                  topology);
  }

  // Send the data downstream
  send_output_handle( "Output Poincare",     sl_field_output_handle_,  true );
  send_output_handle( "Output Centroids",    pcc_field_output_handle_, true );
  send_output_handle( "Output Separatrices", pcs_field_output_handle_, true );

  cerr << "StreamlineAnalyzer done " << endl;
}


CompileInfoHandle
StreamlineAnalyzerAlgo::get_compile_info(const TypeDescription *ftd,
                                         const TypeDescription *mtd,
                                         const TypeDescription *btd,
                                         const TypeDescription *dtd)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name = 
    ( dtd->get_name() == string("Vector") ? 
      string("StreamlineAnalyzerAlgoTVector") :
      string("StreamlineAnalyzerAlgoTScalar") );
  static const string base_class_name("StreamlineAnalyzerAlgo");

  string dtype_str = dtd->get_name();
  string mesh_str  = mtd->get_name();

  string data_str;
  if (mesh_str.find("StructQuadSurfMesh") != string::npos) 
    data_str = "FData2d<" + dtype_str + "," + mtd->get_name() + " >";
  else if (mesh_str.find("CurveMesh") != string::npos) 
    data_str = "vector<" + dtype_str + ">";
 
  string of_name = "GenericField<" + mtd->get_name() + ", " + 
      btd->get_similar_name(dtype_str, 0) + ", " + 
      data_str + "  >"; 

  string pc_name =
    string( "GenericField< " ) +
    string( "PointCloudMesh<ConstantBasis<Point> >, " ) +
    string( "ConstantBasis<double>, " ) +
    string( "vector<double> > " );


  CompileInfo *rval = scinew CompileInfo( template_class_name + "." +
                                          ftd->get_filename() + "." +
                                          mtd->get_filename() + "." +
                                          btd->get_filename() + "." +
                                          dtd->get_filename() + ".",
                                          base_class_name, 
                                          template_class_name, 
                                          ftd->get_name() + ", " +
                                          of_name + ", " +
                                          pc_name );

  // Add in the include path to compile this obj
  rval->add_basis_include("../src/Core/Basis/NoData.h");
  rval->add_basis_include("../src/Core/Basis/Constant.h");
  rval->add_basis_include("../src/Core/Basis/CrvLinearLgn.h");
  rval->add_basis_include("../src/Core/Basis/QuadBilinearLgn.h");

  rval->add_mesh_include("../src/Core/Datatypes/CurveMesh.h");
  rval->add_mesh_include("../src/Core/Datatypes/PointCloudMesh.h");
  rval->add_mesh_include("../src/Core/Datatypes/StructQuadSurfMesh.h");

  rval->add_include(include_path);
  ftd->fill_compile_info(rval);
  rval->add_namespace("FusionPSE");
  return rval;
}

} // End namespace FusionPSE
