// ************************************************************************* //
//                          avtHistogramPlot.C                               //
// ************************************************************************* //

#include <avtHistogramPlot.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkUnsignedCharArray.h>

#include <LineAttributes.h>

#include <avtHistogramFilter.h>
#include <avtUserDefinedMapper.h>


// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
// HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK*HACK
//
// The Histogram plot needs to create some expressions.  Expressions are only
// located on the engine.  This is fine, because that's the only spot the
// histogram plot needs them.  However, the way our linking works, this means
// that expressions would also need to be linked into the viewer.  This causes
// some serious problems.  So, as a work-around, the expressions needed are
// being included directly in this file.  When the linking issues are
// straightened out, this hack can be removed.
//
// BEGIN HACK:
// ************************************************************************* //
//                            avtExprFilterNoNamespaceConflict.h                          //
// ************************************************************************* //

/*    verdict.h
 *
 *
 *    Header file for verdict library that calculates metrics for finite elements
 *
 *
 */

/* This directive is used by the AVT code to determine if we actually have 
 * Verdict. */
#define HAVE_VERDICT


#ifndef VERDICT_INC_LIB
#define VERDICT_INC_LIB

#define VERDICT_DBL_MIN 1.0E-30
#define VERDICT_DBL_MAX 1.0E+30
#define VERDICT_PI 3.1415926535897932384626

#include <assert.h>
#include <math.h>

/* note:  the VERDICT_USE_FLOAT must be consistent with the build of the library */

#ifdef VERDICT_USE_FLOAT
   #define VERDICT_REAL float
#else
   #define VERDICT_REAL double
#endif


#ifdef __cplusplus
   #define C_FUNC_DEF extern "C"
#else
   #define C_FUNC_DEF
#endif


/* typedef for the user if they want to use
   function pointers */
typedef VERDICT_REAL(*VerdictFunction)(int, VERDICT_REAL[][3]);


/*  definition of structs that hold the results of the calculations */

struct HexMetricVals
{
  VERDICT_REAL aspect ;
  VERDICT_REAL skew ;
  VERDICT_REAL taper ;
  VERDICT_REAL volume ;
  VERDICT_REAL stretch ;
  VERDICT_REAL diagonal ;
  VERDICT_REAL dimension ;
  VERDICT_REAL oddy ;
  VERDICT_REAL condition ;
  VERDICT_REAL jacobian ;
  VERDICT_REAL scaled_jacobian ;
  VERDICT_REAL shear ;
  VERDICT_REAL shape ;
  VERDICT_REAL relative_size ;
  VERDICT_REAL shape_and_size ; 
  VERDICT_REAL shear_and_size ; 
  VERDICT_REAL distortion; 
};


struct EdgeMetricVals
{
  VERDICT_REAL length ; 
};


struct KnifeMetricVals
{
  VERDICT_REAL volume ; 
};

struct QuadMetricVals
{
  VERDICT_REAL aspect ;
  VERDICT_REAL skew ;
  VERDICT_REAL taper ;
  VERDICT_REAL warpage ;
  VERDICT_REAL area ;
  VERDICT_REAL stretch ;
  VERDICT_REAL smallest_angle ;
  VERDICT_REAL largest_angle ;
  VERDICT_REAL oddy ;
  VERDICT_REAL condition ;
  VERDICT_REAL jacobian ;
  VERDICT_REAL scaled_jacobian ;
  VERDICT_REAL shear ;
  VERDICT_REAL shape ;
  VERDICT_REAL relative_size ;
  VERDICT_REAL shape_and_size ; 
  VERDICT_REAL shear_and_size ;
  VERDICT_REAL distortion; 
};

struct PyramidMetricVals
{
  VERDICT_REAL volume ; 
};
   
struct WedgeMetricVals
{
  VERDICT_REAL volume ; 
};
     
     
struct TetMetricVals
{
  VERDICT_REAL aspect ;
  VERDICT_REAL aspect_gamma ;
  VERDICT_REAL volume ;
  VERDICT_REAL condition ;
  VERDICT_REAL jacobian ;
  VERDICT_REAL scaled_jacobian ;
  VERDICT_REAL shear ;
  VERDICT_REAL shape ;
  VERDICT_REAL relative_size ;
  VERDICT_REAL shape_and_size ; 
  VERDICT_REAL distortion; 
};
       
struct TriMetricVals
{
  VERDICT_REAL aspect ;
  VERDICT_REAL area ;
  VERDICT_REAL smallest_angle ;
  VERDICT_REAL largest_angle ;
  VERDICT_REAL condition ;
  VERDICT_REAL scaled_jacobian ;
  VERDICT_REAL shear ;
  VERDICT_REAL shape ;
  VERDICT_REAL relative_size ;
  VERDICT_REAL shape_and_size ; 
  VERDICT_REAL distortion; 
};



/* definition of bit fields to determine which metrics to calculate */

#define V_HEX_ASPECT                 1
#define V_HEX_SKEW                   2
#define V_HEX_TAPER                  4
#define V_HEX_VOLUME                 8
#define V_HEX_STRETCH                16
#define V_HEX_DIAGONAL               32
#define V_HEX_DIMENSION              64
#define V_HEX_ODDY                   128
#define V_HEX_CONDITION              256
#define V_HEX_JACOBIAN               512
#define V_HEX_SCALED_JACOBIAN        1024
#define V_HEX_SHEAR                  2048
#define V_HEX_SHAPE                  4096
#define V_HEX_RELATIVE_SIZE          8192
#define V_HEX_SHAPE_AND_SIZE         16384
#define V_HEX_SHEAR_AND_SIZE         32768
#define V_HEX_DISTORTION             65536
#define V_HEX_ALL                    131071
#define V_HEX_TRADITIONAL            V_HEX_ASPECT          + \
                                     V_HEX_SKEW            + \
                                     V_HEX_TAPER           + \
                                     V_HEX_STRETCH         + \
                                     V_HEX_DIAGONAL        + \
                                     V_HEX_ODDY            + \
                                     V_HEX_CONDITION       + \
                                     V_HEX_JACOBIAN        + \
                                     V_HEX_SCALED_JACOBIAN + \
                                     V_HEX_DIMENSION
#define V_HEX_DIAGNOSTIC             V_HEX_VOLUME
#define V_HEX_ALGEBRAIC              V_HEX_SHAPE          + \
                                     V_HEX_SHEAR          + \
                                     V_HEX_RELATIVE_SIZE  + \
                                     V_HEX_SHAPE_AND_SIZE + \
                                     V_HEX_SHEAR_AND_SIZE
#define V_HEX_ROBINSON               V_HEX_SKEW + \
                                     V_HEX_TAPER    

#define V_TET_ASPECT                 1
#define V_TET_ASPECT_GAMMA           2
#define V_TET_VOLUME                 4
#define V_TET_CONDITION              8
#define V_TET_JACOBIAN               16
#define V_TET_SCALED_JACOBIAN        32
#define V_TET_SHEAR                  64
#define V_TET_SHAPE                  128
#define V_TET_RELATIVE_SIZE          256
#define V_TET_SHAPE_AND_SIZE         512
#define V_TET_DISTORTION             1024
#define V_TET_ALL                    2047
#define V_TET_TRADITIONAL            V_TET_ASPECT + \
                                     V_TET_ASPECT_GAMMA + \
                                     V_TET_CONDITION + \
                                     V_TET_JACOBIAN + \
                                     V_TET_SCALED_JACOBIAN  
#define V_TET_DIAGNOSTIC             V_TET_VOLUME
#define V_TET_ALGEBRAIC              V_TET_SHEAR + \
                                     V_TET_SHAPE + \
                                     V_TET_RELATIVE_SIZE + \
                                     V_TET_SHAPE_AND_SIZE

 
#define V_PYRAMID_VOLUME             1
 
#define V_WEDGE_VOLUME               1
 
#define V_KNIFE_VOLUME               1
 
#define V_QUAD_ASPECT                1
#define V_QUAD_SKEW                  2
#define V_QUAD_TAPER                 4
#define V_QUAD_WARPAGE               8
#define V_QUAD_AREA                  16
#define V_QUAD_STRETCH               32
#define V_QUAD_SMALLEST_ANGLE        64
#define V_QUAD_LARGEST_ANGLE         128
#define V_QUAD_ODDY                  256
#define V_QUAD_CONDITION             512
#define V_QUAD_JACOBIAN              1024
#define V_QUAD_SCALED_JACOBIAN       2048
#define V_QUAD_SHEAR                 4096
#define V_QUAD_SHAPE                 8192
#define V_QUAD_RELATIVE_SIZE         16384
#define V_QUAD_SHAPE_AND_SIZE        32768
#define V_QUAD_SHEAR_AND_SIZE        65536
#define V_QUAD_DISTORTION            131072
#define V_QUAD_ALL                   262143
#define V_QUAD_TRADITIONAL           V_QUAD_ASPECT         + \
                                     V_QUAD_SKEW           + \
                                     V_QUAD_TAPER          + \
                                     V_QUAD_WARPAGE        + \
                                     V_QUAD_STRETCH        + \
                                     V_QUAD_SMALLEST_ANGLE + \
                                     V_QUAD_LARGEST_ANGLE  + \
                                     V_QUAD_ODDY           + \
                                     V_QUAD_CONDITION      + \
                                     V_QUAD_JACOBIAN       + \
                                     V_QUAD_SCALED_JACOBIAN 
#define V_QUAD_DIAGNOSTIC            V_QUAD_AREA
#define V_QUAD_ALGEBRAIC             V_QUAD_SHEAR         + \
                                     V_QUAD_SHAPE         + \
                                     V_QUAD_RELATIVE_SIZE + \
                                     V_QUAD_SHAPE_AND_SIZE     
#define V_QUAD_ROBINSON              V_QUAD_ASPECT + \
                                     V_QUAD_SKEW   + \
                                     V_QUAD_TAPER



 
#define V_TRI_ASPECT                 1
#define V_TRI_AREA                   2
#define V_TRI_SMALLEST_ANGLE         4
#define V_TRI_LARGEST_ANGLE          8
#define V_TRI_CONDITION              16
#define V_TRI_SCALED_JACOBIAN        32
#define V_TRI_SHEAR                  64
#define V_TRI_SHAPE                  128
#define V_TRI_RELATIVE_SIZE          256
#define V_TRI_SHAPE_AND_SIZE         512
#define V_TRI_DISTORTION             1024
#define V_TRI_ALL                    2047
#define V_TRI_TRADITIONAL            V_TRI_ASPECT + \
                                     V_TRI_SMALLEST_ANGLE + \
                                     V_TRI_LARGEST_ANGLE + \
                                     V_TRI_CONDITION + \
                                     V_TRI_SCALED_JACOBIAN + \
                                     V_TRI_SHEAR 
#define V_TRI_DIAGNOSTIC             V_TRI_AREA
#define V_TRI_ALGEBRAIC              V_TRI_SHAPE + \
                                     V_TRI_SHAPE_AND_SIZE + \
                                     V_TRI_RELATIVE_SIZE
 
#define V_EDGE_LENGTH                1





/* quality checks for types of mesh elements
 
  num_nodes is number of nodes in element
  coordinates is the x,y,z data of the nodes
  metrics is a bitfield to define which metrics to calculate ( see definitions above )
  metric_vals store the resultant values
 */


    C_FUNC_DEF void v_hex_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct HexMetricVals *metric_vals ); 
    
    C_FUNC_DEF void v_tet_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct TetMetricVals *metric_vals ); 
    
    C_FUNC_DEF void v_pyramid_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct PyramidMetricVals *metric_vals ); 

    C_FUNC_DEF void v_wedge_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct WedgeMetricVals *metric_vals ); 

    C_FUNC_DEF void v_knife_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct KnifeMetricVals *metric_vals ); 

    C_FUNC_DEF void v_quad_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct QuadMetricVals *metric_vals ); 

    C_FUNC_DEF void v_tri_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct TriMetricVals *metric_vals ); 

    C_FUNC_DEF void v_edge_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
        unsigned int metrics_request_flag, struct EdgeMetricVals *metric_vals ); 



/* individual quality functions for hex elements */


    C_FUNC_DEF void v_set_hex_size( VERDICT_REAL size );
    C_FUNC_DEF VERDICT_REAL v_hex_aspect( int num_nodes, VERDICT_REAL coordinates[][3] );
    C_FUNC_DEF VERDICT_REAL v_hex_skew( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_taper( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_volume( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_stretch( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_diagonal( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_dimension( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_oddy( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_condition( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_scaled_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_shear( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_shape( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_relative_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_hex_shear_and_size( int num_nodes, VERDICT_REAL coordinates[][3] );
    C_FUNC_DEF VERDICT_REAL v_hex_distortion( int num_nodes, VERDICT_REAL coordinates[][3] );

/* individual quality functions for tet elements */

    C_FUNC_DEF void v_set_tet_size( VERDICT_REAL size );
    C_FUNC_DEF VERDICT_REAL v_tet_aspect( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_aspect_gamma( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_volume( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_condition( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_scaled_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_shear( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_shape( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_relative_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tet_distortion( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    
/* individual quality functions for pyramid elements */ 

    C_FUNC_DEF VERDICT_REAL v_pyramid_volume( int num_nodes, VERDICT_REAL coordinates[][3] ); 


/* individual quality functions for wedge elements */

    C_FUNC_DEF VERDICT_REAL v_wedge_volume( int num_nodes, VERDICT_REAL coordinates[][3] ); 

   
/* individual quality functions for knife elements */

    C_FUNC_DEF VERDICT_REAL v_knife_volume( int num_nodes, VERDICT_REAL coordinates[][3] ); 

    
/* individual quality functions for edge elements */

    C_FUNC_DEF VERDICT_REAL v_edge_length( int num_nodes, VERDICT_REAL coordinates[][3] ); 

    
/* individual quality functions for quad elements */

    C_FUNC_DEF void v_set_quad_size( VERDICT_REAL size );
    C_FUNC_DEF VERDICT_REAL v_quad_aspect( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_skew( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_taper( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_warpage( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_area( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_stretch( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_smallest_angle( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_largest_angle( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_oddy( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_condition( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_scaled_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_shear( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_shape( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_relative_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_shear_and_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_quad_distortion( int num_nodes, VERDICT_REAL coordinates[][3] ); 



 
/* individual quality functions for tri elements */

    C_FUNC_DEF void  v_set_tri_size( VERDICT_REAL size );
    C_FUNC_DEF VERDICT_REAL v_tri_aspect( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_area( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_smallest_angle( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_largest_angle( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_condition( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_scaled_jacobian( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_shear( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_shape( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_relative_size( int num_nodes, VERDICT_REAL coordinates[][3] ); 
    C_FUNC_DEF VERDICT_REAL v_tri_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] );
    C_FUNC_DEF VERDICT_REAL v_tri_distortion( int num_nodes, VERDICT_REAL coordinates[][3] );



//- Class: GaussIntegration
//- Owner: Lingyun Pan
//- Description: class for gauss integration operation
//- Checked By:

// GaussIntegration.hpp: interface for the GaussIntegration class.
//
//////////////////////////////////////////////////////////////////////


#ifndef GAUSS_INTEGRATION_HPP
#define GAUSS_INTEGRATION_HPP

#define maxTotalNumberGaussPoints 27
#define maxNumberNodes 20
#define maxNumberGaussPoints 3
#define maxNumberGaussPointsTri 6
#define maxNumberGaussPointsTet 4


class GaussIntegration  
{

private:
   int numberGaussPoints;
   int numberNodes;
   int numberDims;
   VERDICT_REAL gaussPointY[maxNumberGaussPoints];
   VERDICT_REAL gaussWeight[maxNumberGaussPoints];
   VERDICT_REAL shapeFunction[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy1GaussPts[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy2GaussPts[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy3GaussPts[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL totalGaussWeight[maxTotalNumberGaussPoints];
   int totalNumberGaussPts;
   VERDICT_REAL y1Area[maxNumberGaussPointsTri];
   VERDICT_REAL y2Area[maxNumberGaussPointsTri];
   VERDICT_REAL y1Volume[maxNumberGaussPointsTet];
   VERDICT_REAL y2Volume[maxNumberGaussPointsTet];
   VERDICT_REAL y3Volume[maxNumberGaussPointsTet];
   VERDICT_REAL y4Volume[maxNumberGaussPointsTet];  

   void get_signs_for_node_local_coord_hex(int node_id, VERDICT_REAL &sign_y1, 
                                           VERDICT_REAL &sign_y2, VERDICT_REAL &sign_y3);
   //- to get the signs for  coordinates of hex nodes in the local computational space 

public:
   //constructors
   GaussIntegration(int n=2, int m=4, int dim=2, int tri=0);
    
   virtual ~GaussIntegration();

   //manipulators
   void get_gauss_pts_and_weight();
   //- get gauss point locations and weights

   void get_tri_rule_pts_and_weight();
   //- get integration points and weights for triangular rules

   void calculate_shape_function_2d_tri();
   //- calculate the shape functions and derivatives of shape functions 
   //- at integration points for 2D triangular elements

   void calculate_shape_function_2d_quad();
   //- calculate the shape functions and derivatives of shape functions 
   //- at gaussian points for 2D quad elements

   void get_shape_func(VERDICT_REAL shape_function[], VERDICT_REAL dndy1_at_gauss_pts[], VERDICT_REAL dndy2_at_gauss_ptsp[], VERDICT_REAL gauss_weight[]) const;
   //- get shape functions and the derivatives

   void get_shape_func(VERDICT_REAL shape_function[], VERDICT_REAL dndy1_at_gauss_pts[], 
                       VERDICT_REAL dndy2_at_gauss_pts[], VERDICT_REAL dndy3_at_gauss_pts[],
                       VERDICT_REAL gauss_weight[]) const; 
   //- get shape functions and the derivatives for 3D elements

   void calculate_derivative_at_nodes(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                      VERDICT_REAL dndy2_at_nodes[][maxNumberNodes]);
   //- calculate shape function derivatives at nodes

   void calculate_shape_function_3d_hex();
   //- calculate shape functions and derivatives of shape functions 
   //- at gaussian points for 3D hex elements

   void calculate_derivative_at_nodes_3d(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                         VERDICT_REAL dndy2_at_nodes[][maxNumberNodes],
                                         VERDICT_REAL dndy3_at_nodes[][maxNumberNodes]);
   //- calculate shape function derivatives at nodes for hex elements

   void calculate_derivative_at_nodes_2d_tri(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                             VERDICT_REAL dndy2_at_nodes[][maxNumberNodes]);
   //- calculate shape function derivatives at nodes for triangular elements

   void calculate_shape_function_3d_tet();
   //- calculate shape functions and derivatives of shape functions 
   //- at integration points for 3D tet elements

   void get_tet_rule_pts_and_weight();
   //- get integration points and weights for tetrhedron rules

   void calculate_derivative_at_nodes_3d_tet(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes], 
                                             VERDICT_REAL dndy2_at_nodes[][maxNumberNodes],
                                             VERDICT_REAL dndy3_at_nodes[][maxNumberNodes]);
   //- calculate shape function derivatives at nodes for tetrahedron elements
   
   void get_node_local_coord_tet(int node_id, VERDICT_REAL &y1, VERDICT_REAL &y2, 
                                                VERDICT_REAL &y3, VERDICT_REAL &y4);
   //- get nodal volume coordinates for tetrahedron element
};

#endif 
/*

VerdictVector.hpp contains definitions of vector operations

*/



#ifndef VERDICTVECTOR_HPP
#define VERDICTVECTOR_HPP

class VerdictVector;
typedef void ( VerdictVector::*transform_function )( VERDICT_REAL gamma,
                                                   VERDICT_REAL gamma2);
// a pointer to some function that transforms the point,
// taking a VERDICT_REAL parameter.  e.g. blow_out, rotate, and scale_angle

class VerdictVector
{
public:
  
    //- Heading: Constructors and Destructor
  VerdictVector();  //- Default constructor.
  
  VerdictVector(const VERDICT_REAL x, const VERDICT_REAL y, const VERDICT_REAL z);
    //- Constructor: create vector from three components
  
  VerdictVector( const VERDICT_REAL xyz[3] );
    //- Constructor: create vector from tuple

  VerdictVector (const VerdictVector& tail, const VerdictVector& head);
    //- Constructor for a VerdictVector starting at tail and pointing
    //- to head.
  
  VerdictVector(const VerdictVector& copy_from);  //- Copy Constructor
  
    //- Heading: Set and Inquire Functions
  void set(const VERDICT_REAL x, const VERDICT_REAL y, const VERDICT_REAL z);
    //- Change vector components to {x}, {y}, and {z}
  
  void set( const VERDICT_REAL xyz[3] );
    //- Change vector components to xyz[0], xyz[1], xyz[2]

  void set(const VerdictVector& tail, const VerdictVector& head);
    //- Change vector to go from tail to head.
  
  void set(const VerdictVector& to_copy);
    //- Same as operator=(const VerdictVector&)
  
  VERDICT_REAL x() const; //- Return x component of vector
  
  VERDICT_REAL y() const; //- Return y component of vector
  
  VERDICT_REAL z() const; //- Return z component of vector
  
  void get_xyz( VERDICT_REAL &x, VERDICT_REAL &y, VERDICT_REAL &z ); //- Get x, y, z components
  void get_xyz( VERDICT_REAL xyz[3] ); //- Get xyz tuple
  
  VERDICT_REAL &r(); //- Return r component of vector, if (r,theta) format
  
  VERDICT_REAL &theta();  //- Return theta component of vector, if (r,theta) format
  
  void x( const VERDICT_REAL x ); //- Set x component of vector
  
  void y( const VERDICT_REAL y ); //- Set y component of vector
  
  void z( const VERDICT_REAL z ); //- Set z component of vector
  
  void r( const VERDICT_REAL x ); //- Set r component of vector, if (r,theta) format
  
  void theta( const VERDICT_REAL y ); //- Set theta component of vector, if (r,theta) format
  
  void xy_to_rtheta();
    //- convert from cartesian to polar coordinates, just 2d for now
    //- theta is in [0,2 PI)
  
  void rtheta_to_xy();
    //- convert from  polar to cartesian coordinates, just 2d for now
  
  void scale_angle(VERDICT_REAL gamma, VERDICT_REAL );
    //- tranform_function.
    //- transform  (x,y) to (r,theta) to (r,gamma*theta) to (x',y')
    //- plus some additional scaling so long chords won't cross short ones
  
  void blow_out(VERDICT_REAL gamma, VERDICT_REAL gamma2 = 0.0);
    //- transform_function
    //- blow points radially away from the origin, 
  
  void rotate(VERDICT_REAL angle, VERDICT_REAL );
    //- transform function.
    //- transform  (x,y) to (r,theta) to (r,theta+angle) to (x',y')
  
  void reflect_about_xaxis(VERDICT_REAL dummy, VERDICT_REAL );
    //- dummy argument to make it a transform function
  
  VERDICT_REAL normalize();
    //- Normalize (set magnitude equal to 1) vector - return the magnitude

  VerdictVector& length(const VERDICT_REAL new_length);
    //- Change length of vector to {new_length}. Can be used to move a
    //- location a specified distance from the origin in the current
    //- orientation.
  
  VERDICT_REAL length() const;
    //- Calculate the length of the vector.
    //- Use {length_squared()} if only comparing lengths, not adding.
  
  VERDICT_REAL distance_between(const VerdictVector& test_vector);
    //- Calculate the distance from the head of one vector
    //  to the head of the test_vector.
  
  VERDICT_REAL length_squared() const;
    //- Calculate the squared length of the vector.
    //- Faster than {length()} since it eliminates the square root if
    //- only comparing other lengths.
  
  VERDICT_REAL interior_angle(const VerdictVector &otherVector);
    //- Calculate the interior angle: acos((a%b)/(|a||b|))
    //- Returns angle in degrees.
  
  VERDICT_REAL vector_angle_quick(const VerdictVector& vec1, const VerdictVector& vec2);
    //- Calculate the interior angle between the projections of
    //- {vec1} and {vec2} onto the plane defined by the {this} vector.
    //- The angle returned is the right-handed angle around the {this}
    //- vector from {vec1} to {vec2}. Angle is in radians.
  
  VERDICT_REAL vector_angle( const VerdictVector &vector1,
                       const VerdictVector &vector2) const;
    //- Compute the angle between the projections of {vector1} and {vector2}
    //- onto the plane defined by *this. The angle is the
    //- right-hand angle, in radians, about *this from {vector1} to {vector2}.
  
  void perpendicular_z();
    //- Transform this vector to a perpendicular one, leaving
    //- z-component alone. Rotates clockwise about the z-axis by pi/2.

  void print_me();
    //- Prints out the coordinates of this vector.
  
  void orthogonal_vectors( VerdictVector &vector2, VerdictVector &vector3 );
    //- Finds 2 (arbitrary) vectors that are orthogonal to this one
  
  void next_point( const VerdictVector &direction, VERDICT_REAL distance, 
                   VerdictVector& out_point );
    //- Finds the next point in space based on *this* point (starting point), 
    //- a direction and the distance to extend in the direction. The
    //- direction vector need not be a unit vector.  The out_point can be
    //- "*this" (i.e., modify point in place).
  
  bool within_tolerance( const VerdictVector &vectorPtr2,
                                VERDICT_REAL tolerance) const;
    //- Compare two vectors to see if they are spatially equal.  They
    //- compare if x, y, and z are all within tolerance.
  
    //- Heading: Operator Overloads  *****************************
  VerdictVector&  operator+=(const VerdictVector &vec);
    //- Compound Assignment: addition: {this = this + vec}
  
  VerdictVector& operator-=(const VerdictVector &vec);
    //- Compound Assignment: subtraction: {this = this - vec}
  
  VerdictVector& operator*=(const VerdictVector &vec);
    //- Compound Assignment: cross product: {this = this * vec},
    //- non-commutative
  
  VerdictVector& operator*=(const VERDICT_REAL scalar);
    //- Compound Assignment: multiplication: {this = this * scalar}
  
  VerdictVector& operator/=(const VERDICT_REAL scalar);
    //- Compound Assignment: division: {this = this / scalar}
  
  VerdictVector operator-() const;
    //- unary negation.
  
  friend VerdictVector operator~(const VerdictVector &vec);
    //- normalize. Returns a new vector which is a copy of {vec},
    //- scaled such that {|vec|=1}. Uses overloaded bitwise NOT operator.
  
  friend VerdictVector operator+(const VerdictVector &v1, 
                               const VerdictVector &v2);
    //- vector addition
  
  friend VerdictVector operator-(const VerdictVector &v1, 
                               const VerdictVector &v2);
    //- vector subtraction
  
  friend VerdictVector operator*(const VerdictVector &v1, 
                               const VerdictVector &v2);
    //- vector cross product, non-commutative
  
  friend VerdictVector operator*(const VerdictVector &v1, const VERDICT_REAL sclr);
    //- vector * scalar
  
  friend VerdictVector operator*(const VERDICT_REAL sclr, const VerdictVector &v1);
    //- scalar * vector
  
  friend VERDICT_REAL operator%(const VerdictVector &v1, const VerdictVector &v2);
    //- dot product
  
  friend VerdictVector operator/(const VerdictVector &v1, const VERDICT_REAL sclr);
    //- vector / scalar
  
  friend int operator==(const VerdictVector &v1, const VerdictVector &v2);
    //- Equality operator
  
  friend int operator!=(const VerdictVector &v1, const VerdictVector &v2);
    //- Inequality operator
  
  friend VerdictVector interpolate(const VERDICT_REAL param, const VerdictVector &v1,
                                 const VerdictVector &v2);
    //- Interpolate between two vectors. Returns (1-param)*v1 + param*v2


  VerdictVector &operator=(const VerdictVector& from);
  
private:
  
  VERDICT_REAL xVal;  //- x component of vector.
  VERDICT_REAL yVal;  //- y component of vector.
  VERDICT_REAL zVal;  //- z component of vector.
};

VerdictVector vectorRotate(const VERDICT_REAL angle, 
                         const VerdictVector &normalAxis,
                         const VerdictVector &referenceAxis);
  //- A new coordinate system is created with the xy plane corresponding
  //- to the plane normal to {normalAxis}, and the x axis corresponding to
  //- the projection of {referenceAxis} onto the normal plane.  The normal
  //- plane is the tangent plane at the root point.  A unit vector is
  //- constructed along the local x axis and then rotated by the given
  //- ccw angle to form the new point.  The new point, then is a unit
  //- distance from the global origin in the tangent plane.
  //- {angle} is in radians.

inline VERDICT_REAL VerdictVector::x() const
{ return xVal; }
inline VERDICT_REAL VerdictVector::y() const
{ return yVal; }
inline VERDICT_REAL VerdictVector::z() const
{ return zVal; }
inline void VerdictVector::get_xyz(VERDICT_REAL xyz[3])
{
  xyz[0] = xVal;
  xyz[1] = yVal;
  xyz[2] = zVal;
}
inline void VerdictVector::get_xyz(VERDICT_REAL &x, VERDICT_REAL &y, VERDICT_REAL &z)
{
  x = xVal; 
  y = yVal; 
  z = zVal;
}
inline VERDICT_REAL &VerdictVector::r()
{ return xVal; }
inline VERDICT_REAL &VerdictVector::theta()
{ return yVal; }
inline void VerdictVector::x( const VERDICT_REAL x )
{ xVal = x; }
inline void VerdictVector::y( const VERDICT_REAL y )
{ yVal = y; }
inline void VerdictVector::z( const VERDICT_REAL z )
{ zVal = z; }
inline void VerdictVector::r( const VERDICT_REAL x )
{ xVal = x; }
inline void VerdictVector::theta( const VERDICT_REAL y )
{ yVal = y; }
inline VerdictVector& VerdictVector::operator+=(const VerdictVector &vec)
{
  xVal += vec.x();
  yVal += vec.y();
  zVal += vec.z();
  return *this;
}

inline VerdictVector& VerdictVector::operator-=(const VerdictVector &vec)
{
  xVal -= vec.x();
  yVal -= vec.y();
  zVal -= vec.z();
  return *this;
}

inline VerdictVector& VerdictVector::operator*=(const VerdictVector &vec)
{
  VERDICT_REAL xcross, ycross, zcross;
  xcross = yVal * vec.z() - zVal * vec.y();
  ycross = zVal * vec.x() - xVal * vec.z();
  zcross = xVal * vec.y() - yVal * vec.x();
  xVal = xcross;
  yVal = ycross;
  zVal = zcross;
  return *this;
}

inline VerdictVector::VerdictVector(const VerdictVector& copy_from)
    : xVal(copy_from.xVal), yVal(copy_from.yVal), zVal(copy_from.zVal)
{}

inline VerdictVector::VerdictVector()
    : xVal(0), yVal(0), zVal(0)
{}

inline VerdictVector::VerdictVector (const VerdictVector& tail,
                                 const VerdictVector& head)
    : xVal(head.xVal - tail.xVal),
      yVal(head.yVal - tail.yVal),
      zVal(head.zVal - tail.zVal)
{}

inline VerdictVector::VerdictVector(const VERDICT_REAL xIn,
                                const VERDICT_REAL yIn,
                                const VERDICT_REAL zIn)
    : xVal(xIn), yVal(yIn), zVal(zIn)
{}

// This sets the vector to be perpendicular to it's current direction.
// NOTE:
//      This is a 2D function.  It only works in the XY Plane.
inline void VerdictVector::perpendicular_z()
{
  VERDICT_REAL temp = x();
  x( y() );
  y( -temp );
}

inline void VerdictVector::set(const VERDICT_REAL x,
                      const VERDICT_REAL y,
                      const VERDICT_REAL z)
{
  xVal = x;
  yVal = y;
  zVal = z;
}

inline void VerdictVector::set(const VERDICT_REAL xyz[3])
{
  xVal = xyz[0];
  yVal = xyz[1];
  zVal = xyz[2];
}

inline void VerdictVector::set(const VerdictVector& tail,
                             const VerdictVector& head)
{
  xVal = head.xVal - tail.xVal;
  yVal = head.yVal - tail.yVal;
  zVal = head.zVal - tail.zVal;
}

inline VerdictVector& VerdictVector::operator=(const VerdictVector &from)  
{
  xVal = from.xVal; 
  yVal = from.yVal; 
  zVal = from.zVal; 
  return *this;
}

inline void VerdictVector::set(const VerdictVector& to_copy)
{
  *this = to_copy;
}

// Scale all values by scalar.
inline VerdictVector& VerdictVector::operator*=(const VERDICT_REAL scalar)
{
  xVal *= scalar;
  yVal *= scalar;
  zVal *= scalar;
  return *this;
}

// Scales all values by 1/scalar
inline VerdictVector& VerdictVector::operator/=(const VERDICT_REAL scalar)
{
  assert (scalar != 0);
  xVal /= scalar;
  yVal /= scalar;
  zVal /= scalar;
  return *this;
}

// Returns the normalized 'this'.
inline VerdictVector operator~(const VerdictVector &vec)
{
  VERDICT_REAL mag = sqrt(vec.xVal*vec.xVal +
                    vec.yVal*vec.yVal +
                    vec.zVal*vec.zVal);
  
  VerdictVector temp = vec;
  if (mag != 0.0)
  {
    temp /= mag;
  }
  return temp;
}

// Unary minus.  Negates all values in vector.
inline VerdictVector VerdictVector::operator-() const
{
  return VerdictVector(-xVal, -yVal, -zVal);
}

inline VerdictVector operator+(const VerdictVector &vector1,
                      const VerdictVector &vector2)
{
  VERDICT_REAL xv = vector1.x() + vector2.x();
  VERDICT_REAL yv = vector1.y() + vector2.y();
  VERDICT_REAL zv = vector1.z() + vector2.z();
  return VerdictVector(xv,yv,zv);
//  return VerdictVector(vector1) += vector2;
}

inline VerdictVector operator-(const VerdictVector &vector1,
                      const VerdictVector &vector2)
{
  VERDICT_REAL xv = vector1.x() - vector2.x();
  VERDICT_REAL yv = vector1.y() - vector2.y();
  VERDICT_REAL zv = vector1.z() - vector2.z();
  return VerdictVector(xv,yv,zv);
//  return VerdictVector(vector1) -= vector2;
}

// Cross products.
// vector1 cross vector2
inline VerdictVector operator*(const VerdictVector &vector1,
                      const VerdictVector &vector2)
{
  return VerdictVector(vector1) *= vector2;
}

// Returns a scaled vector.
inline VerdictVector operator*(const VerdictVector &vector1,
                      const VERDICT_REAL scalar)
{
  return VerdictVector(vector1) *= scalar;
}

// Returns a scaled vector
inline VerdictVector operator*(const VERDICT_REAL scalar,
                             const VerdictVector &vector1)
{
  return VerdictVector(vector1) *= scalar;
}

// Returns a vector scaled by 1/scalar
inline VerdictVector operator/(const VerdictVector &vector1,
                             const VERDICT_REAL scalar)
{
  return VerdictVector(vector1) /= scalar;
}

inline int operator==(const VerdictVector &v1, const VerdictVector &v2)
{
  return (v1.xVal == v2.xVal && v1.yVal == v2.yVal && v1.zVal == v2.zVal);
}

inline int operator!=(const VerdictVector &v1, const VerdictVector &v2)
{
  return (v1.xVal != v2.xVal || v1.yVal != v2.yVal || v1.zVal != v2.zVal);
}

inline VERDICT_REAL VerdictVector::length_squared() const
{
  return( xVal*xVal + yVal*yVal + zVal*zVal );
}

inline VERDICT_REAL VerdictVector::length() const
{
  return( sqrt(xVal*xVal + yVal*yVal + zVal*zVal) );
}

inline VERDICT_REAL VerdictVector::normalize()
{
  VERDICT_REAL mag = length();
  if (mag != 0)
  {
    xVal = xVal / mag;
    yVal = yVal / mag;
    zVal = zVal / mag;
  }
  return mag;
}


// Dot Product.
inline VERDICT_REAL operator%(const VerdictVector &vector1,
                        const VerdictVector &vector2)
{
  return( vector1.x() * vector2.x() +
          vector1.y() * vector2.y() +
          vector1.z() * vector2.z() );
}

#endif
#ifndef VERDICT_DEFINES
#define VERDICT_DEFINES

//VERDICT_REAL VERDICT_DBL_MIN = 1.0E-30;

enum VerdictBoolean { VERDICT_FALSE = 0, VERDICT_TRUE = 1} ;


#define VERDICT_MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#define VERDICT_MAX(a,b)  ( (a) > (b) ? (a) : (b) )

inline VERDICT_REAL  determinant(VERDICT_REAL a,
    VERDICT_REAL b,
    VERDICT_REAL c,
    VERDICT_REAL d)
{ 
  return ((a)*(d) - (b)*(c));
}

inline VERDICT_REAL determinant( VerdictVector v1,
    VerdictVector v2,
    VerdictVector v3 )
{
  return v1 % (v2 * v3);
}

#define jacobian_matrix(a,b,c,d,e,f,g) \
        VERDICT_REAL jac_mat_tmp; \
        jac_mat_tmp = sqrt(a); \
        if(jac_mat_tmp == 0) { d = 0; e = 0; f = 0; g = 0; } \
        else { d = jac_mat_tmp; e = 0; f = b/jac_mat_tmp; g = c/jac_mat_tmp; }


// this assumes that detmw != 0
#define form_t(m11,m21,m12,m22,mw11,mw21,mw12,mw22,detmw,xm11,xm21,xm12,xm22) \
        xm11= (m11*mw22-m12*mw21)/detmw; \
        xm21= (m21*mw22-m22*mw21)/detmw; \
        xm12= (m12*mw11-m11*mw12)/detmw; \
        xm22= (m22*mw11-m21*mw12)/detmw;


inline VERDICT_REAL normalize_jacobian( VERDICT_REAL jacobi,
    VerdictVector& v1,
    VerdictVector& v2,
    VerdictVector& v3,
    int tet_flag = 0 )
{
  VERDICT_REAL return_value = 0.0;

  static const VERDICT_REAL rt2 = sqrt(2.0);
  
  if ( jacobi != 0.0 )
  {
    
    VERDICT_REAL l1, l2, l3, length_product;
    // Note: there may be numerical problems if one is a lot shorter
    // than the others this way. But scaling each vector before the
    // triple product would involve 3 square roots instead of just
    // one.
    l1 = v1.length_squared();
    l2 = v2.length_squared();
    l3 = v3.length_squared();
    length_product = sqrt( l1 * l2 * l3 );
    
    // if some numerical scaling problem, or just plain roundoff,
    // then push back into range [-1,1].
    if ( length_product < fabs(jacobi) )
    {
      length_product = fabs(jacobi);
    }
    
    if( tet_flag == 1)
      return_value = rt2 * jacobi / length_product;
    else
      return_value = jacobi / length_product;
    
  }
  return return_value;
  
}


inline VERDICT_REAL  norm_squared( VERDICT_REAL m11,
    VERDICT_REAL m21,
    VERDICT_REAL m12,
    VERDICT_REAL m22 )
{
  return m11*m11+m21*m21+m12*m12+m22*m22;
}

#define metric_matrix(m11,m21,m12,m22,gm11,gm12,gm22) \
        gm11 = m11*m11+m21*m21; \
        gm12 = m11*m12+m21*m22; \
        gm22 = m12*m12+m22*m22;


inline int skew_matrix(VERDICT_REAL gm11, VERDICT_REAL gm12, VERDICT_REAL gm22, VERDICT_REAL det, VERDICT_REAL &qm11, VERDICT_REAL &qm21, VERDICT_REAL &qm12, VERDICT_REAL &qm22 )
{
  VERDICT_REAL tmp = sqrt(gm11*gm22);
  if( tmp == 0 ) {return false;}
  
  qm11=1;
  qm21=0;
  qm12=gm12/tmp;
  qm22=det/tmp;
  return true;
}


inline void inverse(VerdictVector x1,
    VerdictVector x2,
    VerdictVector x3,
    VerdictVector& u1,
    VerdictVector& u2,
    VerdictVector& u3 )
{ 
  VERDICT_REAL  detx = determinant(x1, x2, x3);
  VerdictVector rx1, rx2, rx3;
  
  rx1.set(x1.x(), x2.x(), x3.x());
  rx2.set(x1.y(), x2.y(), x3.y());
  rx3.set(x1.z(), x2.z(), x3.z());
  
  u1 = rx2 * rx3;
  u2 = rx3 * rx1;
  u3 = rx1 * rx2;
  
  u1 /= detx;
  u2 /= detx;
  u3 /= detx;
} 

/*
inline void form_T(VERDICT_REAL a1[3],
  VERDICT_REAL a2[3],
  VERDICT_REAL a3[3],
  VERDICT_REAL w1[3],
  VERDICT_REAL w2[3],
  VERDICT_REAL w3[3],
  VERDICT_REAL t1[3],
  VERDICT_REAL t2[3],
  VERDICT_REAL t3[3] )
{

  VERDICT_REAL x1[3], x2[3], x3[3];
  VERDICT_REAL ra1[3], ra2[3], ra3[3];
  
  x1[0] = a1[0];
  x1[1] = a2[0];
  x1[2] = a3[0];
  
  x2[0] = a1[1];
  x2[1] = a2[1];
  x2[2] = a3[1];
  
  x3[0] = a1[2];
  x3[1] = a2[2];
  x3[2] = a3[2];
  
  inverse(w1,w2,w3,x1,x2,x3);
  
  t1[0] = dot_product(ra1, x1);
  t1[1] = dot_product(ra1, x2);
  t1[2] = dot_product(ra1, x3);

  t2[0] = dot_product(ra2, x1);
  t2[0] = dot_product(ra2, x2);
  t2[0] = dot_product(ra2, x3);
  
  t3[0] = dot_product(ra3, x1);
  t3[0] = dot_product(ra3, x2);
  t3[0] = dot_product(ra3, x3);

}
*/

inline void form_Q( const VerdictVector& v1,
    const VerdictVector& v2,
    const VerdictVector& v3,
    VerdictVector& q1,
    VerdictVector& q2,
    VerdictVector& q3 )
{
  
  VERDICT_REAL g11, g12, g13, g22, g23, g33;
  
  g11 = v1 % v1;
  g12 = v1 % v2;
  g13 = v1 % v3;
  g22 = v2 % v2;
  g23 = v2 % v3;
  g33 = v3 % v3;
  
  VERDICT_REAL rtg11 = sqrt(g11);
  VERDICT_REAL rtg22 = sqrt(g22);
  VERDICT_REAL rtg33 = sqrt(g33);
  VerdictVector temp1;
  
  temp1 = v1 * v2;
  
  VERDICT_REAL cross = sqrt( temp1 % temp1 );
  
  VERDICT_REAL q11,q21,q31;
  VERDICT_REAL q12,q22,q32;
  VERDICT_REAL q13,q23,q33;
  
  q11=1;
  q21=0;
  q31=0;
  
  q12 = g12 / rtg11 / rtg22;
  q22 = cross / rtg11 / rtg22;
  q32 = 0;
  
  q13 = g13 / rtg11 / rtg33;
  q23 = ( g11*g23-g12*g13 )/ rtg11 / rtg33 / cross;
  temp1 = v2 * v3;
  q33 = ( v1 % temp1  ) / rtg33 / cross;
  
  q1.set( q11, q21, q31 );
  q2.set( q12, q22, q32 );
  q3.set( q13, q23, q33 );
  
}

inline void product( VerdictVector& a1,
    VerdictVector& a2,
    VerdictVector& a3,
    VerdictVector& b1,
    VerdictVector& b2,
    VerdictVector& b3,
    VerdictVector& c1,
    VerdictVector& c2,
    VerdictVector& c3 )
{
  
  VerdictVector x1, x2, x3;
  
  x1.set( a1.x(), a2.x(), a3.x() );
  x2.set( a1.y(), a2.y(), a3.y() );
  x3.set( a1.z(), a2.z(), a3.z() );
  
  c1.set( x1 % b1, x2 % b1, x3 % b1 );
  c2.set( x1 % b2, x2 % b2, x3 % b2 );
  c3.set( x1 % b3, x2 % b3, x3 % b3 );
}


inline VERDICT_REAL norm_squared( VerdictVector& x1,
    VerdictVector& x2,
    VerdictVector& x3 )

{
  return  (x1 % x1) + (x2 % x2) + (x3 % x3);
}


inline VERDICT_REAL skew_x( VerdictVector& q1,
    VerdictVector& q2,
    VerdictVector& q3,
    VerdictVector& qw1,
    VerdictVector& qw2,
    VerdictVector& qw3 )
{
  VERDICT_REAL normsq1, normsq2, kappa;
  VerdictVector u1, u2, u3;
  VerdictVector x1, x2, x3;
  
  inverse(qw1,qw2,qw3,u1,u2,u3);
  product(q1,q2,q3,u1,u2,u3,x1,x2,x3);
  inverse(x1,x2,x3,u1,u2,u3);
  normsq1 = norm_squared(x1,x2,x3);
  normsq2 = norm_squared(u1,u2,u3);
  kappa = sqrt( normsq1 * normsq2 );
  
  VERDICT_REAL skew = 0;
  if ( kappa > VERDICT_DBL_MIN )
    skew = 3/kappa;
  
  return skew;
}

#endif


#ifndef VERDICT_VECTOR
#define VERDICT_VECTOR


// computes the dot product of 3d vectors
//VERDICT_REAL dot_product( VERDICT_REAL vec1[], VERDICT_REAL vec2[] );

// computes the cross product
//VERDICT_REAL *cross_product( VERDICT_REAL vec1[], VERDICT_REAL vec2[], VERDICT_REAL answer[] = 0);

// computes the interior angle between 2 vectors in degrees
//VERDICT_REAL interior_angle ( VERDICT_REAL vec1[], VERDICT_REAL vec2[] );

// computes the length of a vector
//VERDICT_REAL length ( VERDICT_REAL vec[] );

//VERDICT_REAL length_squared (VERDICT_REAL vec[] );


inline VERDICT_REAL dot_product( VERDICT_REAL vec1[], VERDICT_REAL vec2[] )
{
    VERDICT_REAL answer =  vec1[0] * vec2[0] +
                           vec1[1] * vec2[1] +
                           vec1[2] * vec2[2];
    return answer;
}
inline void normalize( VERDICT_REAL vec[] )
{
    VERDICT_REAL x = sqrt( vec[0]*vec[0] +
                           vec[1]*vec[1] +
                           vec[2]*vec[2] );

    vec[0] /= x;
    vec[1] /= x;
    vec[2] /= x;
}


inline VERDICT_REAL * cross_product( VERDICT_REAL vec1[], VERDICT_REAL vec2[], VERDICT_REAL answer[] )
{
    answer[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    answer[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    answer[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
    return answer;
}

inline VERDICT_REAL length ( VERDICT_REAL vec[] )
{
    return (sqrt ( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] ));
}

inline VERDICT_REAL length_squared (VERDICT_REAL vec[] )
{
    return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
}


inline VERDICT_REAL interior_angle( VERDICT_REAL vec1[], VERDICT_REAL vec2[] )
{
    VERDICT_REAL len1, len2, cosAngle, angleRad;

    if (  ((len1 = length(vec1)) > 0 ) && ((len2 = length(vec2)) > 0 ) )
    {
        cosAngle = dot_product(vec1, vec2) / (len1 * len2);
    }
    else
    {
        assert(len1 > 0);
        assert(len2 > 0);
    }

    if ((cosAngle > 1.0) && (cosAngle < 1.0001))
    {
        cosAngle = 1.0;
        angleRad = acos(cosAngle);
    }
    else if (cosAngle < -1.0 && cosAngle > -1.0001)
    {
        cosAngle = -1.0;
        angleRad = acos(cosAngle);
    }
    else if (cosAngle >= -1.0 && cosAngle <= 1.0)
        angleRad = acos(cosAngle);
    else
    {
        assert(cosAngle < 1.0001 && cosAngle > -1.0001);
    }

    return( (angleRad * 180.) / VERDICT_PI );
}

#endif


#endif  // VERDICT_INC_LIB
#include <memory.h>

/*

EdgeMetric.cpp contains quality calcultions for edges

*/

#include <math.h>

/*! 

  length of and edge
  length is calculated by taking the distance between the end nodes

 */
C_FUNC_DEF VERDICT_REAL v_edge_length( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VERDICT_REAL x = coordinates[1][0] - coordinates[0][0];
  VERDICT_REAL y = coordinates[1][1] - coordinates[0][1];
  VERDICT_REAL z = coordinates[1][2] - coordinates[0][2];
  return ( sqrt (x*x + y*y + z*z) );
}

/*!
  
  higher order function for calculating multiple metrics at once.

  for an edge, there is only one metric, edge length.
*/

C_FUNC_DEF void edge_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, struct EdgeMetricVals *metric_vals )
{
  if(metrics_request_flag & V_EDGE_LENGTH)
    metric_vals->length = v_edge_length(num_nodes, coordinates);
}



// GaussIntegration.cpp: implementation of the GaussIntegration class.
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
GaussIntegration::GaussIntegration(int n, int m, int dim, int tri)
{
   numberGaussPoints = n;     
   numberNodes = m;
   numberDims = dim;

   if (tri==1)
   //triangular element
   {
       if ( numberDims == 2)
          totalNumberGaussPts = numberGaussPoints;
       else if (numberDims ==3)
          totalNumberGaussPts =numberGaussPoints ;
   }
   else if (tri == 0)
   {
      if ( numberDims == 2)
         totalNumberGaussPts = numberGaussPoints*numberGaussPoints;
      else if (numberDims ==3)
         totalNumberGaussPts = numberGaussPoints*numberGaussPoints*numberGaussPoints;
   }


}

GaussIntegration::~GaussIntegration(){}

void GaussIntegration::get_shape_func(VERDICT_REAL shape_function[], VERDICT_REAL dndy1_at_gauss_pts[], 
                             VERDICT_REAL dndy2_at_gauss_pts[], VERDICT_REAL gauss_weight[]) const
{
   int i, j;
   for (i=0;i<totalNumberGaussPts; i++)
   {
      for ( j =0;j<numberNodes;j++)
      {
         shape_function[i*maxNumberNodes+j] = 
            shapeFunction[i][j];
         dndy1_at_gauss_pts[i*maxNumberNodes+j]  = dndy1GaussPts[i][j];
         dndy2_at_gauss_pts[i*maxNumberNodes+j]  = dndy2GaussPts[i][j];
      }
   }

   for (  i=0;i<totalNumberGaussPts; i++)
      gauss_weight[i] = totalGaussWeight[i];     
}

void GaussIntegration::get_shape_func(VERDICT_REAL shape_function[], VERDICT_REAL dndy1_at_gauss_pts[], 
                                      VERDICT_REAL dndy2_at_gauss_pts[], VERDICT_REAL dndy3_at_gauss_pts[], 
                                      VERDICT_REAL gauss_weight[]) const
{
   int i, j;
   for ( i =0;i<totalNumberGaussPts;i++)
   {
      for ( j=0;j<numberNodes; j++)
      {
         shape_function[i*maxNumberNodes+j] = 
            shapeFunction[i][j];
         dndy1_at_gauss_pts[i*maxNumberNodes+j] = dndy1GaussPts[i][j];
         dndy2_at_gauss_pts[i*maxNumberNodes+j] = dndy2GaussPts[i][j];
         dndy3_at_gauss_pts[i*maxNumberNodes+j] = dndy3GaussPts[i][j];
      }
   }

   for ( i=0;i<totalNumberGaussPts; i++)
      gauss_weight[i] = totalGaussWeight[i];     
}


void GaussIntegration::get_gauss_pts_and_weight()
{

   switch( numberGaussPoints )
   {
      case 1:
         gaussPointY[0]=  0.0;
         gaussWeight[0] = 2.0;
         break;
      case 2:
         gaussPointY[0] = -0.577350269189626;
         gaussPointY[1] =  0.577350269189626;
         gaussWeight[0] = 1.0;
         gaussWeight[1] = 1.0;
         break;
      case 3:
         gaussPointY[0]= -0.774596669241483;
         gaussPointY[1] = 0.0;
         gaussPointY[2] = 0.774596669241483;
         gaussWeight[0] = 0.555555555555555;
         gaussWeight[1] = 0.888888888888889;
         gaussWeight[2] = 0.555555555555555;
         break;
   }
}

void GaussIntegration::calculate_shape_function_2d_quad()
{
   int ife=0, i, j;
   VERDICT_REAL y1,y2;
   get_gauss_pts_and_weight();

    switch( numberNodes ){
      case 4:
         for ( i=0; i<numberGaussPoints; i++)
         {
            for ( j=0;j<numberGaussPoints;j++)
            {
               y1 = gaussPointY[i];
               y2 = gaussPointY[j];
               shapeFunction[ife][0]= 0.25*(1-y1)*(1-y2);
               shapeFunction[ife][1]= 0.25*(1+y1)*(1-y2);
               shapeFunction[ife][2] = 0.25*(1+y1)*(1+y2);
               shapeFunction[ife][3] = 0.25*(1-y1)*(1+y2);

               dndy1GaussPts[ife][0] = -0.25*(1-y2);
               dndy1GaussPts[ife][1] =  0.25*(1-y2);
               dndy1GaussPts[ife][2] =  0.25*(1+y2);
               dndy1GaussPts[ife][3] = -0.25*(1+y2);

               dndy2GaussPts[ife][0] = -0.25*(1-y1);
               dndy2GaussPts[ife][1] = -0.25*(1+y1);
               dndy2GaussPts[ife][2] =  0.25*(1+y1);
               dndy2GaussPts[ife][3] =  0.25*(1-y1);

               totalGaussWeight[ife] = gaussWeight[i]*gaussWeight[j];
               ife++;
            }
         }
         break;
      case 8:
         for ( i=0; i<numberGaussPoints; i++)
         {
            for ( j=0;j<numberGaussPoints;j++)
            {
               y1 = gaussPointY[i];
               y2 = gaussPointY[j];
               shapeFunction[ife][0] = 0.25*(1-y1)*(1-y2)*(-y1-y2-1);
               shapeFunction[ife][1] = 0.25*(1+y1)*(1-y2)*(y1-y2-1);
               shapeFunction[ife][2] = 0.25*(1+y1)*(1+y2)*(y1+y2-1);
               shapeFunction[ife][3] = 0.25*(1-y1)*(1+y2)*(-y1+y2-1);
               shapeFunction[ife][4] = 0.5*(1-y1*y1)*(1-y2);
               shapeFunction[ife][5] = 0.5*(1-y2*y2)*(1+y1);
               shapeFunction[ife][6] = 0.5*(1-y1*y1)*(1+y2);
               shapeFunction[ife][7] = 0.5*(1-y2*y2)*(1-y1);


               dndy1GaussPts[ife][0] =  0.25*(1-y2)*(2.0*y1+y2);
               dndy1GaussPts[ife][1] =  0.25*(1-y2)*(2.0*y1-y2);
               dndy1GaussPts[ife][2] =  0.25*(1+y2)*(2.0*y1+y2);
               dndy1GaussPts[ife][3] =  0.25*(1+y2)*(2.0*y1-y2);

               dndy1GaussPts[ife][4] = -y1*(1-y2);
               dndy1GaussPts[ife][5] =  0.5*(1-y2*y2);
               dndy1GaussPts[ife][6] = -y1*(1+y2);
               dndy1GaussPts[ife][7] = -0.5*(1-y2*y2);

               dndy2GaussPts[ife][0] =  0.25*(1-y1)*(2.0*y2+y1);
               dndy2GaussPts[ife][1] =  0.25*(1+y1)*(2.0*y2-y1);
               dndy2GaussPts[ife][2] =  0.25*(1+y1)*(2.0*y2+y1);
               dndy2GaussPts[ife][3] =  0.25*(1-y1)*(2.0*y2-y1);

               dndy2GaussPts[ife][4] = -0.5*(1-y1*y1);
               dndy2GaussPts[ife][5] = -y2*(1+y1);
               dndy2GaussPts[ife][6] =  0.5*(1-y1*y1);
               dndy2GaussPts[ife][7] = -y2*(1-y1);

               totalGaussWeight[ife] = gaussWeight[i]*gaussWeight[j];
               ife++;
            }
         }
         break;
   }
}

void GaussIntegration::calculate_shape_function_3d_hex()
{
   int ife=0, i, j, k, node_id;
   VERDICT_REAL y1,y2, y3, sign_node_y1, sign_node_y2, sign_node_y3;
   VERDICT_REAL y1_term, y2_term, y3_term, y123_temp;

   get_gauss_pts_and_weight();

    switch( numberNodes )
    {
    case 8:
       for ( i=0; i<numberGaussPoints; i++)
       {
          for ( j=0;j<numberGaussPoints;j++)
          {
             for ( k=0;k<numberGaussPoints;k++)
             {
                y1 = gaussPointY[i];
                y2 = gaussPointY[j];
                y3 = gaussPointY[k];

                for ( node_id =0; node_id <numberNodes; node_id++)
                {
                   get_signs_for_node_local_coord_hex(node_id, sign_node_y1, 
                      sign_node_y2, sign_node_y3);
   
                   y1_term = 1+sign_node_y1*y1;
                   y2_term = 1+sign_node_y2*y2;
                   y3_term = 1+sign_node_y3*y3;

                   shapeFunction[ife][node_id] = 0.125*y1_term
                      *y2_term*y3_term;
                   dndy1GaussPts[ife][node_id] = 0.125*sign_node_y1
                      *y2_term*y3_term;
                   dndy2GaussPts[ife][node_id] = 0.125*sign_node_y2
                      *y1_term*y3_term;
                   dndy3GaussPts[ife][node_id] = 0.125*sign_node_y3
                      *y1_term*y2_term;
                }
                totalGaussWeight[ife] = gaussWeight[i]*gaussWeight[j]*gaussWeight[k];
                ife++;
             }
          }
       }
       break;
    case 20:
       for ( i=0; i<numberGaussPoints; i++)
       {
          for ( j=0;j<numberGaussPoints;j++)
          {
             for ( k=0;k<numberGaussPoints;k++)
             {
                y1 = gaussPointY[i];
                y2 = gaussPointY[j];
                y3 = gaussPointY[k];

                for ( node_id =0; node_id <numberNodes; node_id++)
                {
                   get_signs_for_node_local_coord_hex(node_id, sign_node_y1, 
                      sign_node_y2, sign_node_y3);
                   
                   y1_term = 1+sign_node_y1*y1;
                   y2_term = 1+sign_node_y2*y2;
                   y3_term = 1+sign_node_y3*y3;
                   y123_temp = sign_node_y1*y1+sign_node_y2*y2+sign_node_y3*y3-2.;

                   switch (node_id)
                   {
                   case 0: case 1: case 2: case 3:
                   case 4: case 5: case 6: case 7:
                      {
                         shapeFunction[ife][node_id] = 0.125*y1_term
                            *y2_term*y3_term
                            *y123_temp;
                         dndy1GaussPts[ife][node_id] = 0.125*sign_node_y1
                            *y123_temp
                            *y2_term*y3_term
                            +0.125*y1_term
                            *y2_term*y3_term*sign_node_y1;
                         dndy2GaussPts[ife][node_id] = 0.125*sign_node_y2
                            *y1_term*y3_term
                            *y123_temp
                            +0.125*y1_term
                            *y2_term*y3_term*sign_node_y2;
                         dndy3GaussPts[ife][node_id] = 0.125*sign_node_y3
                            *y1_term*y2_term
                            *y123_temp
                            +0.125*y1_term
                            *y2_term*y3_term*sign_node_y3;
                         break;
                      }
                   case 8: case 10: case 16: case 18:
                      {
                         shapeFunction[ife][node_id] = 0.25*(1-y1*y1)
                            *y2_term*y3_term;
                         dndy1GaussPts[ife][node_id] = -0.5*y1
                            *y2_term*y3_term;
                         dndy2GaussPts[ife][node_id] = 0.25*(1-y1*y1)
                            *sign_node_y2*y3_term;
                         dndy3GaussPts[ife][node_id] = 0.25*(1-y1*y1)
                            *y2_term*sign_node_y3;
                         break;
                      }
                   case 9: case 11: case 17: case 19:
                      {
                         shapeFunction[ife][node_id] = 0.25*(1-y2*y2)
                            *y1_term*y3_term;
                         dndy1GaussPts[ife][node_id] = 0.25*(1-y2*y2)
                            *sign_node_y1*y3_term;
                         dndy2GaussPts[ife][node_id] = -0.5*y2
                            *y1_term*y3_term;
                         dndy3GaussPts[ife][node_id] = 0.25*(1-y2*y2)
                            *y1_term*sign_node_y3;
                         break;
                      }
                   case 12: case 13: case 14: case 15:
                      {
                         shapeFunction[ife][node_id] = 0.25*(1-y3*y3)
                            *y1_term*y2_term;
                         dndy1GaussPts[ife][node_id] = 0.25*(1-y3*y3)
                            *sign_node_y1*y2_term;
                         dndy2GaussPts[ife][node_id] = 0.25*(1-y3*y3)
                            *y1_term*sign_node_y2;
                         dndy3GaussPts[ife][node_id] = -0.5*y3
                            *y1_term*y2_term;
                         break;
                      }
                   }
                   
                }
                totalGaussWeight[ife] = gaussWeight[i]*gaussWeight[j]*gaussWeight[k];
                ife++;
             }
          }
       }
       break;

   }
}

void GaussIntegration::calculate_derivative_at_nodes(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes], 
                                                     VERDICT_REAL dndy2_at_nodes[][maxNumberNodes])
{
   VERDICT_REAL y1=0., y2=0.;
   int i;
   for(i=0;i<numberNodes; i++) 
   {
      switch( i )
      {
         case 0:
            y1 = -1.;
            y2 = -1.;
            break;
         case 1:
            y1 = 1.;
            y2 = -1.;
            break;
         case 2:
            y1 = 1.;
            y2 = 1.;
            break;
         case 3:
            y1 = -1.;
            y2 = 1.;
            break;

         // midside nodes if there is any

         case 4:
            y1 = 0. ;
            y2 = -1. ;
            break;

         case 5:
            y1 = 1. ;
            y2 = 0. ;
            break;

         case 6:
            y1 = 0. ;
            y2 = 1. ;
            break;

         case 7:
            y1 = -1. ;
            y2 = 0. ;
            break;

      }

      switch( numberNodes )
      {
         case 4:
            //dn_i/dy1 evaluated at node i
            dndy1_at_nodes[i][0] = -0.25*(1-y2);
            dndy1_at_nodes[i][1] = 0.25*(1-y2);
            dndy1_at_nodes[i][2] = 0.25*(1+y2);
            dndy1_at_nodes[i][3] = -0.25*(1+y2);

            // dn_i/dy2 evaluated at node i
            dndy2_at_nodes[i][0] = -0.25*(1-y1);
            dndy2_at_nodes[i][1] = -0.25*(1+y1);
            dndy2_at_nodes[i][2] = 0.25*(1+y1);
            dndy2_at_nodes[i][3] = 0.25*(1-y1);
            break;

         case 8:

            dndy1_at_nodes[i][0]  =  0.25*(1-y2)*(2.0*y1+y2);
            dndy1_at_nodes[i][1]  =  0.25*(1-y2)*(2.0*y1-y2);
            dndy1_at_nodes[i][2]  =  0.25*(1+y2)*(2.0*y1+y2);
            dndy1_at_nodes[i][3]  =  0.25*(1+y2)*(2.0*y1-y2);

            dndy1_at_nodes[i][4]  = -y1*(1-y2);
            dndy1_at_nodes[i][5]  =  0.5*(1-y2*y2);
            dndy1_at_nodes[i][6]  = -y1*(1+y2);
            dndy1_at_nodes[i][7]  = -0.5*(1-y2*y2);

            dndy2_at_nodes[i][0]  =  0.25*(1-y1)*(2.0*y2+y1);
            dndy2_at_nodes[i][1]  =  0.25*(1+y1)*(2.0*y2-y1);
            dndy2_at_nodes[i][2]  =  0.25*(1+y1)*(2.0*y2+y1);
            dndy2_at_nodes[i][3]  =  0.25*(1-y1)*(2.0*y2-y1);

            dndy2_at_nodes[i][4] = -0.5*(1-y1*y1);
            dndy2_at_nodes[i][5] = -y2*(1+y1);
            dndy2_at_nodes[i][6] =  0.5*(1-y1*y1);
            dndy2_at_nodes[i][7] = -y2*(1-y1);
            break;
         }
   }
}

void GaussIntegration::calculate_derivative_at_nodes_3d(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                                        VERDICT_REAL dndy2_at_nodes[][maxNumberNodes],
                                                        VERDICT_REAL dndy3_at_nodes[][maxNumberNodes])
{
   VERDICT_REAL y1, y2, y3,sign_node_y1,sign_node_y2,sign_node_y3 ;
   VERDICT_REAL y1_term, y2_term, y3_term, y123_temp;
   int node_id, node_id_2;
   for(node_id=0;node_id<numberNodes; node_id++) 
   {
      get_signs_for_node_local_coord_hex(node_id, y1,y2,y3);
      

      switch( numberNodes )
      {
      case 8:
         for ( node_id_2 =0; node_id_2 <numberNodes; node_id_2++)
         {
            get_signs_for_node_local_coord_hex(node_id_2, sign_node_y1,
               sign_node_y2,sign_node_y3);
            y1_term = 1+sign_node_y1*y1;
            y2_term = 1+sign_node_y2*y2;
            y3_term = 1+sign_node_y3*y3;

            dndy1_at_nodes[node_id][node_id_2] = 0.125*sign_node_y1
                                                 *y2_term*y3_term;

            dndy2_at_nodes[node_id][node_id_2] = 0.125*sign_node_y2
                                                 *y1_term*y3_term;

            dndy3_at_nodes[node_id][node_id_2] = 0.125*sign_node_y3
                                                 *y1_term*y2_term;
         }
         break;
      case 20:
         for ( node_id_2 =0; node_id_2 <numberNodes; node_id_2++)
         {
            get_signs_for_node_local_coord_hex(node_id_2, sign_node_y1,
               sign_node_y2,sign_node_y3);

            y1_term = 1+sign_node_y1*y1;
            y2_term = 1+sign_node_y2*y2;
            y3_term = 1+sign_node_y3*y3;
            y123_temp = sign_node_y1*y1+sign_node_y2*y2+sign_node_y3*y3-2.;
            switch (node_id_2)
            {
               case 0: case 1: case 2: case 3:
               case 4: case 5: case 6: case 7:
                  {
                     dndy1_at_nodes[node_id][node_id_2] = 0.125*sign_node_y1
                        *y2_term*y3_term
                        *y123_temp
                        +0.125*y1_term
                        *y2_term*y3_term*sign_node_y1;
                     dndy2_at_nodes[node_id][node_id_2] = 0.125*sign_node_y2
                        *y1_term*y3_term
                        *y123_temp
                        +0.125*y1_term
                        *y2_term*y3_term*sign_node_y2;
                     dndy3_at_nodes[node_id][node_id_2] = 0.125*sign_node_y3
                        *y1_term*y2_term
                        *y123_temp
                        +0.125*y1_term
                        *y2_term*y3_term*sign_node_y3;
                     break;
                  }
               case 8: case 10: case 16: case 18:
                  {
                     dndy1_at_nodes[node_id][node_id_2] = -0.5*y1
                        *y2_term*y3_term;
                     dndy2_at_nodes[node_id][node_id_2] = 0.25*(1-y1*y1)
                        *sign_node_y2*y3_term;
                     dndy3_at_nodes[node_id][node_id_2] = 0.25*(1-y1*y1)
                        *y2_term*sign_node_y3;
                     break;
                  }
               case 9: case 11: case 17: case 19:
                  {
                     dndy1_at_nodes[node_id][node_id_2] = 0.25*(1-y2*y2)
                        *sign_node_y1*y3_term;
                     dndy2_at_nodes[node_id][node_id_2] = -0.5*y2
                        *y1_term*y3_term;
                     dndy3_at_nodes[node_id][node_id_2] = 0.25*(1-y2*y2)
                        *y1_term*sign_node_y3;
                     break;
                  }
               case 12: case 13: case 14: case 15:
                  {
                     dndy1_at_nodes[node_id][node_id_2] = 0.25*(1-y3*y3)
                        *sign_node_y1*y2_term;
                     dndy2_at_nodes[node_id][node_id_2] = 0.25*(1-y3*y3)
                        *y1_term*sign_node_y2;
                     dndy3_at_nodes[node_id][node_id_2] = -0.5*y3
                        *y1_term*y2_term;
                     break;
                  }
            }
         }
         break;
         
      }
   }
}



void GaussIntegration::get_signs_for_node_local_coord_hex(int node_id, VERDICT_REAL &sign_node_y1, VERDICT_REAL &sign_node_y2, VERDICT_REAL &sign_node_y3)
{
   switch (node_id)
   {
   case 0:
      sign_node_y1 = -1.;
      sign_node_y2 = -1.;
      sign_node_y3 = -1.;
      break;
   case 1:
      sign_node_y1 = 1.;
      sign_node_y2 = -1.;
      sign_node_y3 = -1.;
      break;
   case 2:
      sign_node_y1 = 1.;
      sign_node_y2 = 1.;
      sign_node_y3 = -1.;
      break;
   case 3:
      sign_node_y1 = -1.;
      sign_node_y2 = 1.;
      sign_node_y3 = -1.;
      break;
   case 4:
      sign_node_y1 = -1.;
      sign_node_y2 = -1.;
      sign_node_y3 = 1.;
      break;
   case 5:
      sign_node_y1 = 1.;
      sign_node_y2 = -1.;
      sign_node_y3 = 1.;
      break;
   case 6:
      sign_node_y1 = 1.;
      sign_node_y2 = 1.;
      sign_node_y3 = 1.;
      break;
   case 7:
      sign_node_y1 = -1.;
      sign_node_y2 = 1.;
      sign_node_y3 = 1.;
      break;
   case 8:
      sign_node_y1 = 0;
      sign_node_y2 = -1.;
      sign_node_y3 = -1.;
      break;
   case 9:
      sign_node_y1 = 1.;
      sign_node_y2 = 0;
      sign_node_y3 = -1.;
      break;
   case 10:
      sign_node_y1 = 0;
      sign_node_y2 = 1.;
      sign_node_y3 = -1.;
      break;
   case 11:
      sign_node_y1 = -1.;
      sign_node_y2 = 0.;
      sign_node_y3 = -1.;
      break;
   case 12:
      sign_node_y1 = -1.;
      sign_node_y2 = -1.;
      sign_node_y3 = 0.;
      break;
   case 13:
      sign_node_y1 = 1.;
      sign_node_y2 = -1.;
      sign_node_y3 = 0.;
      break;
   case 14:
      sign_node_y1 = 1.;
      sign_node_y2 = 1.;
      sign_node_y3 = 0.;
      break;
   case 15:
      sign_node_y1 = -1.;
      sign_node_y2 = 1.;
      sign_node_y3 = 0.;
      break;
   case 16:
      sign_node_y1 = 0;
      sign_node_y2 = -1.;
      sign_node_y3 = 1.;
      break;
   case 17:
      sign_node_y1 = 1.;
      sign_node_y2 = 0;
      sign_node_y3 = 1.;
      break;
   case 18:
      sign_node_y1 = 0;
      sign_node_y2 = 1.;
      sign_node_y3 = 1.;
      break;
   case 19:
      sign_node_y1 = -1.;
      sign_node_y2 = 0.;
      sign_node_y3 = 1.;
      break;
   }
}

void GaussIntegration::get_tri_rule_pts_and_weight()
{
   // get triangular rule integration points and weight

   switch( numberGaussPoints )
   {
      case 6:
         y1Area[0] = 0.09157621;
         y2Area[0] = 0.09157621;

         y1Area[1] = 0.09157621;
         y2Area[1] = 0.8168476;

         y1Area[2] = 0.8168476;
         y2Area[2] = 0.09157621;

         y1Area[3] = 0.4459485;
         y2Area[3] = 0.4459485;

         y1Area[4] = 0.4459485;
         y2Area[4] = 0.1081030;

         y1Area[5] = 0.1081030;
         y2Area[5] = 0.4459485;

         int i;
         for (i=0;i<3;i++)
         {
            totalGaussWeight[i] = 0.06348067;
         }
         for (i=3;i<6;i++)
         {
            totalGaussWeight[i] = 0.1289694;
         }   
         break;
   }
}

void GaussIntegration::calculate_shape_function_2d_tri()
{
   int ife=0;
   VERDICT_REAL y1,y2, y3;
   get_tri_rule_pts_and_weight();

   for (ife=0; ife<totalNumberGaussPts; ife++)
   {
      y1 =  y1Area[ife];
      y2 =  y2Area[ife];
      y3 = 1.0 -y1 -y2;

      shapeFunction[ife][0] = y1*(2.*y1-1.);
      shapeFunction[ife][1] = y2*(2.*y2-1.);
      shapeFunction[ife][2] = y3*(2.*y3-1.);

      shapeFunction[ife][3] = 4.*y1*y2;
      shapeFunction[ife][4] = 4.*y2*y3;
      shapeFunction[ife][5] = 4.*y1*y3;
      

      dndy1GaussPts[ife][0] = 4*y1-1.;
      dndy1GaussPts[ife][1] = 0;
      dndy1GaussPts[ife][2] = 1-4.*y3;
      
      dndy1GaussPts[ife][3] = 4.*y2;
      dndy1GaussPts[ife][4] = -4.*y2;
      dndy1GaussPts[ife][5] = 4.*(1-2*y1-y2);

      dndy2GaussPts[ife][0] = 0.0;
      dndy2GaussPts[ife][1] = 4.*y2-1.;
      dndy2GaussPts[ife][2] = 1-4.*y3;
      
      dndy2GaussPts[ife][3] = 4.*y1;
      dndy2GaussPts[ife][4] = 4.*(1-y1-2.*y2);
      dndy2GaussPts[ife][5] = -4.*y1;
   }
}


void GaussIntegration::calculate_derivative_at_nodes_2d_tri(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                                            VERDICT_REAL dndy2_at_nodes[][maxNumberNodes])
{
   VERDICT_REAL y1=0., y2=0., y3=0.;
   int i;
   for(i=0;i<numberNodes; i++) 
   {
      switch( i )
      {
      case 0:
         y1 = 1.;
         y2 = 0.;
         break;
      case 1:
         y1 = 0.;
         y2 = 1.;
         break;
      case 2:
         y1 = 0.;
         y2 = 0.;
         break;
      case 3:
         y1 = 0.5;
         y2 = 0.5;
         break;
      case 4:
         y1 = 0.;
         y2 = 0.5;
         break;
      case 5:
         y1 = 0.5;
         y2 = 0.0;
         break;
      }

      y3 = 1. -y1-y2;

      dndy1_at_nodes[i][0] = 4*y1-1.;
      dndy1_at_nodes[i][1]= 0;
      dndy1_at_nodes[i][2] = 1-4.*y3;
      
      dndy1_at_nodes[i][3] = 4.*y2;
      dndy1_at_nodes[i][4] = -4.*y2;
      dndy1_at_nodes[i][5] = 4.*(1-2*y1-y2);

      dndy2_at_nodes[i][0] = 0.0;
      dndy2_at_nodes[i][1] = 4.*y2-1.;
      dndy2_at_nodes[i][2] = 1-4.*y3;
      
      dndy2_at_nodes[i][3] = 4.*y1;
      dndy2_at_nodes[i][4] = 4.*(1-y1-2.*y2);
      dndy2_at_nodes[i][5] = -4.*y1;
   }
}
void GaussIntegration::get_tet_rule_pts_and_weight()
{
   // get tetrahedron rule integration points and weight

   VERDICT_REAL a, b;
   switch( numberGaussPoints )
   {
   case 1:
      // 1 integration point formula, degree of precision 1
      y1Volume[0] = 0.25;
      y2Volume[0] = 0.25;
      y3Volume[0] = 0.25;
      y4Volume[0] = 0.25;
      totalGaussWeight[0] = 1.;
      break;
   case 4:
      // 4 integration points formula, degree of precision 2
      a = 0.58541020;
      b = 0.13819660;

      y1Volume[0] = a;
      y2Volume[0] = b;
      y3Volume[0] = b;
      y4Volume[0] = b;

      y1Volume[1] = b;
      y2Volume[1] = a;
      y3Volume[1] = b;
      y4Volume[1] = b;

      y1Volume[2] = b;
      y2Volume[2] = b;
      y3Volume[2] = a;
      y4Volume[2] = b;

      y1Volume[3] = b;
      y2Volume[3] = b;
      y3Volume[3] = b;
      y4Volume[3] = a;

      int i;
      for (i=0;i<4;i++)
      {
         totalGaussWeight[i] = 0.25;
       }
      break;
   }
}

void GaussIntegration::calculate_shape_function_3d_tet()
{
   int ife=0;
   VERDICT_REAL y1,y2, y3, y4;
   get_tet_rule_pts_and_weight();

   switch (numberNodes)
   {
   case 10: // 10 nodes quadratic tet
      {
         for (ife=0; ife<totalNumberGaussPts; ife++)
         {
            // y1,y2,y3,y4 are the volume coordinates
            y1 =  y1Volume[ife];
            y2 =  y2Volume[ife];
            y3 =  y3Volume[ife];
            y4 =  y4Volume[ife];

            // shape function is the same as in ABAQUS
            // it is different from that in all the FEA book
            // in which node is the first node
            // here at node 1 y4=1
            shapeFunction[ife][0] = y4*(2.*y4-1.);
            shapeFunction[ife][1] = y1*(2.*y1-1.);
            shapeFunction[ife][2] = y2*(2.*y2-1.);
            shapeFunction[ife][3] = y3*(2.*y3-1.);

            shapeFunction[ife][4] = 4.*y1*y4;
            shapeFunction[ife][5] = 4.*y1*y2;            
            shapeFunction[ife][6] = 4.*y2*y4;
            shapeFunction[ife][7] = 4.*y3*y4;
            shapeFunction[ife][8] = 4.*y1*y3;
            shapeFunction[ife][9] = 4.*y2*y3;

            dndy1GaussPts[ife][0] = 1-4*y4;
            dndy1GaussPts[ife][1] = 4*y1-1.;
            dndy1GaussPts[ife][2] = 0;
            dndy1GaussPts[ife][3] = 0;
            
            dndy1GaussPts[ife][4] = 4.*(y4-y1);
            dndy1GaussPts[ife][5] = 4.*y2;
            dndy1GaussPts[ife][6] = -4.*y2;
            dndy1GaussPts[ife][7] = -4.*y3;    
            dndy1GaussPts[ife][8] = 4.*y3;
            dndy1GaussPts[ife][9] = 0;
            
            dndy2GaussPts[ife][0] = 1-4*y4;
            dndy2GaussPts[ife][1] = 0;
            dndy2GaussPts[ife][2] = 4.*y2-1.;
            dndy2GaussPts[ife][3] = 0;

            dndy2GaussPts[ife][4] = -4.*y1; 
            dndy2GaussPts[ife][5] = 4.*y1;       
            dndy2GaussPts[ife][6] = 4.*(y4-y2);
            dndy2GaussPts[ife][7] = -4.*y3;
            dndy2GaussPts[ife][8] = 0.;
            dndy2GaussPts[ife][9] = 4.*y3;

            dndy3GaussPts[ife][0] = 1-4*y4;
            dndy3GaussPts[ife][1] = 0;
            dndy3GaussPts[ife][2] = 0;
            dndy3GaussPts[ife][3] = 4.*y3-1.;
            
            dndy3GaussPts[ife][4] = -4.*y1;
            dndy3GaussPts[ife][5] = 0;     
            dndy3GaussPts[ife][6] = -4.*y2;
            dndy3GaussPts[ife][7] = 4.*(y4-y3);
            dndy3GaussPts[ife][8] = 4.*y1;
            dndy3GaussPts[ife][9] = 4.*y2;
         }
         break;
      }
   case 4: // four node linear tet for debug purpose
      {
         for (ife=0; ife<totalNumberGaussPts; ife++)
         {
            y1 =  y1Volume[ife];
            y2 =  y2Volume[ife];
            y3 =  y3Volume[ife];
            y4 =  y4Volume[ife];

            shapeFunction[ife][0] = y4;
            shapeFunction[ife][1] = y1;
            shapeFunction[ife][2] = y2;
            shapeFunction[ife][3] = y3;

            dndy1GaussPts[ife][0] = -1.;
            dndy1GaussPts[ife][1] = 1;
            dndy1GaussPts[ife][2] = 0;
            dndy1GaussPts[ife][3] = 0;

            dndy2GaussPts[ife][0] = -1.;
            dndy2GaussPts[ife][1] = 0;
            dndy2GaussPts[ife][2] = 1;
            dndy2GaussPts[ife][3] = 0;

            dndy3GaussPts[ife][0] = -1.;
            dndy3GaussPts[ife][1] = 0;
            dndy3GaussPts[ife][2] = 0;
            dndy3GaussPts[ife][3] = 1;
            
         }
         break;
      }
   }

}

void GaussIntegration::calculate_derivative_at_nodes_3d_tet(VERDICT_REAL dndy1_at_nodes[][maxNumberNodes],
                                                            VERDICT_REAL dndy2_at_nodes[][maxNumberNodes],
                                                            VERDICT_REAL dndy3_at_nodes[][maxNumberNodes])
{
   VERDICT_REAL y1, y2, y3, y4;
   int i;

   switch (numberNodes)
   {
   case 10:
      {
         for(i=0;i<numberNodes; i++) 
         {
            get_node_local_coord_tet(i, y1, y2, y3, y4);

            dndy1_at_nodes[i][0] = 1-4*y4;
            dndy1_at_nodes[i][1] = 4*y1-1.;
            dndy1_at_nodes[i][2] = 0;
            dndy1_at_nodes[i][3] = 0;

            dndy1_at_nodes[i][4] = 4.*(y4-y1);
            dndy1_at_nodes[i][5] = 4.*y2;
            dndy1_at_nodes[i][6] = -4.*y2;
            dndy1_at_nodes[i][7] = -4.*y3;
            dndy1_at_nodes[i][8] = 4.*y3;
            dndy1_at_nodes[i][9] = 0;

            dndy2_at_nodes[i][0] = 1-4*y4;
            dndy2_at_nodes[i][1] = 0;
            dndy2_at_nodes[i][2] = 4.*y2-1.;
            dndy2_at_nodes[i][3] = 0;
            dndy2_at_nodes[i][4] = -4.*y1;
            dndy2_at_nodes[i][5] = 4.*y1;
            dndy2_at_nodes[i][6] = 4.*(y4-y2);
            dndy2_at_nodes[i][7] = -4.*y3;
            dndy2_at_nodes[i][8] = 0.;
            dndy2_at_nodes[i][9] = 4.*y3;

            dndy3_at_nodes[i][0] = 1-4*y4;
            dndy3_at_nodes[i][1] = 0;
            dndy3_at_nodes[i][2] = 0;
            dndy3_at_nodes[i][3] = 4.*y3-1.;

            dndy3_at_nodes[i][4] = -4.*y1;
            dndy3_at_nodes[i][5] = 0;
            dndy3_at_nodes[i][6] = -4.*y2;
            dndy3_at_nodes[i][7] = 4.*(y4-y3);
            dndy3_at_nodes[i][8] = 4.*y1;
            dndy3_at_nodes[i][9] = 4.*y2;
         }
         break;
      }
   case 4:
      {
         for(i=0;i<numberNodes; i++)
         {
            get_node_local_coord_tet(i, y1, y2, y3, y4);
            dndy1_at_nodes[i][0] = -1.;
            dndy1_at_nodes[i][1] = 1;
            dndy1_at_nodes[i][2] = 0;
            dndy1_at_nodes[i][3] = 0;

            dndy2_at_nodes[i][0] = -1.;
            dndy2_at_nodes[i][1] = 0;
            dndy2_at_nodes[i][2] = 1;
            dndy2_at_nodes[i][3] = 0;
 
            dndy3_at_nodes[i][0] = -1.;
            dndy3_at_nodes[i][1] = 0;
            dndy3_at_nodes[i][2] = 0;
            dndy3_at_nodes[i][3] = 1;
            
         }
         break;
      }
   }
}
      

void GaussIntegration::get_node_local_coord_tet(int node_id, VERDICT_REAL &y1, VERDICT_REAL &y2, 
                                                VERDICT_REAL &y3, VERDICT_REAL &y4)
{
   switch( node_id )
   {
   case 0:
      y1 = 0.;
      y2 = 0.;
      y3 = 0.;
      y4 = 1.;
      break;
   case 1:
      y1 = 1.;
      y2 = 0.;
      y3 = 0.;
      y4 = 0.;
      break;
   case 2:
      y1 = 0.;
      y2 = 1.;
      y3 = 0.;
      y4 = 0.;
      break;
   case 3:
      y1 = 0.;
      y2 = 0.;
      y3 = 1.;
      y4 = 0.;
      break;
   case 4:
      y1 = 0.5;
      y2 = 0.;
      y3 = 0.;
      y4 = 0.5;
      break;
   case 5:
      y1 = 0.5;
      y2 = 0.5;
      y3 = 0.;
      y4 = 0.;
      break;
   case 6:
      y1 = 0.;
      y2 = 0.5;
      y3 = 0.;
      y4 = 0.5;
      break;
   case 7:
      y1 = 0.;
      y2 = 0.0;
      y3 = 0.5;
      y4 = 0.5;
      break;
   case 8:
      y1 = 0.5;
      y2 = 0.;
      y3 = 0.5;
      y4 = 0.0;
      break;
   case 9:
      y1 = 0.;
      y2 = 0.5;
      y3 = 0.5;
      y4 = 0.;
      break;
   }
}
/*

HexMetric.cpp contains quality calculations for hexes

*/
//! the average volume of a hex
VERDICT_REAL verdict_hex_size = 0;

//! weights based on the average size of a hex
int v_hex_get_weight( VerdictVector &v1, 
    VerdictVector &v2,
    VerdictVector &v3 )
{

  if( verdict_hex_size == 0)
    return 0;

  v1.set(1,0,0);
  v2.set(0,1,0);
  v3.set(0,0,1);

  VERDICT_REAL scale = pow(verdict_hex_size/ (v1 % (v2 * v3 )), 0.33333333333); 
  v1 *= scale;
  v2 *= scale;
  v3 *= scale;

  return 1;
}

//! returns the average volume of a hex
C_FUNC_DEF void v_set_hex_size( VERDICT_REAL size )
{
  verdict_hex_size = size;
}

#define make_hex_nodes(coord, pos)                      \
  for (int mhcii = 0; mhcii < 8; mhcii++ )              \
  {                                                     \
    pos[mhcii].set( coord[mhcii][0],                    \
        coord[mhcii][1],                                \
        coord[mhcii][2] );                              \
  }


#define make_edge_length_squares(edges, lengths)        \
{                                                       \
  for(int melii=0; melii<12; melii++)                   \
    lengths[melii] = edges[melii].length_squared();     \
}
  

//! make VerdictVectors from coordinates
void make_hex_edges( VERDICT_REAL coordinates[][3], VerdictVector edges[12] )
{
  edges[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  edges[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  edges[2].set(
      coordinates[3][0] - coordinates[2][0],
      coordinates[3][1] - coordinates[2][1],
      coordinates[3][2] - coordinates[2][2]
      );
  edges[3].set(
      coordinates[0][0] - coordinates[3][0],
      coordinates[0][1] - coordinates[3][1],
      coordinates[0][2] - coordinates[3][2]
      );
  edges[4].set(
      coordinates[5][0] - coordinates[4][0],
      coordinates[5][1] - coordinates[4][1],
      coordinates[5][2] - coordinates[4][2]
      );
  edges[5].set(
      coordinates[6][0] - coordinates[5][0],
      coordinates[6][1] - coordinates[5][1],
      coordinates[6][2] - coordinates[5][2]
      );
  edges[6].set(
      coordinates[7][0] - coordinates[6][0],
      coordinates[7][1] - coordinates[6][1],
      coordinates[7][2] - coordinates[6][2]
      );
  edges[7].set(
      coordinates[4][0] - coordinates[7][0],
      coordinates[4][1] - coordinates[7][1],
      coordinates[4][2] - coordinates[7][2]
      );
  edges[8].set(
      coordinates[4][0] - coordinates[0][0],
      coordinates[4][1] - coordinates[0][1],
      coordinates[4][2] - coordinates[0][2]
      );
  edges[9].set(
      coordinates[5][0] - coordinates[1][0],
      coordinates[5][1] - coordinates[1][1],
      coordinates[5][2] - coordinates[1][2]
      );
  edges[10].set(
      coordinates[6][0] - coordinates[2][0],
      coordinates[6][1] - coordinates[2][1],
      coordinates[6][2] - coordinates[2][2]
      );
  edges[11].set(
      coordinates[7][0] - coordinates[3][0],
      coordinates[7][1] - coordinates[3][1],
      coordinates[7][2] - coordinates[3][2]
      );
}

/*!
  localizes hex coordinates
*/
void localize_hex_coordinates(VERDICT_REAL coordinates[][3], VerdictVector position[8] )
{

  int ii;
  for ( ii = 0; ii < 8; ii++ )
  {
    position[ii].set( coordinates[ii][0],
        coordinates[ii][1],
        coordinates[ii][2] );
  }
  
  // ... Make centroid of element the center of coordinate system
  VerdictVector point_1256 = position[1];
  point_1256 += position[2];
  point_1256 += position[5];
  point_1256 += position[6];

  VerdictVector point_0374 = position[0];
  point_0374 += position[3];
  point_0374 += position[7];
  point_0374 += position[4];

  VerdictVector centroid = point_1256;
  centroid += point_0374;
  centroid /= 8.0;

  int i;
  for ( i = 0; i < 8; i++)
    position[i] -= centroid;

  // ... Rotate element such that center of side 2-3 and 4-1 define X axis
  VERDICT_REAL DX = point_1256.x() - point_0374.x();
  VERDICT_REAL DY = point_1256.y() - point_0374.y();
  VERDICT_REAL DZ = point_1256.z() - point_0374.z();

  VERDICT_REAL AMAGX = sqrt(DX*DX + DZ*DZ);
  VERDICT_REAL AMAGY = sqrt(DX*DX + DY*DY + DZ*DZ);
  VERDICT_REAL FMAGX = AMAGX == 0.0 ? 1.0 : 0.0;
  VERDICT_REAL FMAGY = AMAGY == 0.0 ? 1.0 : 0.0;

  VERDICT_REAL CZ = DX / (AMAGX + FMAGX) + FMAGX;
  VERDICT_REAL SZ = DZ / (AMAGX + FMAGX);
  VERDICT_REAL CY = AMAGX / (AMAGY + FMAGY) + FMAGY;
  VERDICT_REAL SY = DY / (AMAGY + FMAGY);

  VERDICT_REAL temp;
 
  for (i = 0; i < 8; i++) 
  {
    temp =  CY * CZ * position[i].x() + CY * SZ * position[i].z() +
      SY * position[i].y();
    position[i].y( -SY * CZ * position[i].x() - SY * SZ * position[i].z() +
        CY * position[i].y());
    position[i].z( -SZ * position[i].x() + CZ * position[i].z());
    position[i].x(temp);
  }
 
  // ... Now, rotate about Y
  VerdictVector delta = -position[0];
  delta -= position[1];
  delta += position[2];
  delta += position[3];
  delta -= position[4];
  delta -= position[5];
  delta += position[6];
  delta += position[7];

  DX = delta.x();
  DY = delta.y();
  DZ = delta.z();

  AMAGY = sqrt(DY*DY + DZ*DZ);
  FMAGY = AMAGY == 0.0 ? 1.0 : 0.0;

  VERDICT_REAL CX = DY / (AMAGY + FMAGY) + FMAGY;
  VERDICT_REAL SX = DZ / (AMAGY + FMAGY);
  
  for (i = 0; i < 8; i++) 
  {
    temp =  CX * position[i].y() + SX * position[i].z();
    position[i].z(-SX * position[i].y() + CX * position[i].z());
    position[i].y(temp);
  }
}


VERDICT_REAL safe_ratio3( const VERDICT_REAL numerator, 
    const VERDICT_REAL denominator,
    const VERDICT_REAL max_ratio )
{
    // this filter is essential for good running time in practice
  VERDICT_REAL return_value = 0.0;

  const VERDICT_REAL filter_n = max_ratio * 1.0e-16;
  const VERDICT_REAL filter_d = 1.0e-16;
  if ( fabs( numerator ) <= filter_n &&
      fabs( denominator ) >= filter_d )
    return_value = numerator / denominator;
  
  else
    return_value = fabs(numerator) / max_ratio >= fabs(denominator) ?
      ( numerator >= 0.0 && denominator >= 0.0 ||
        numerator < 0.0 && denominator < 0.0 ?
        max_ratio : -max_ratio )
      : numerator / denominator;
  
  return return_value;
}


VERDICT_REAL safe_ratio( const VERDICT_REAL numerator, 
    const VERDICT_REAL denominator )
{

  VERDICT_REAL return_value = 0.0;
  const VERDICT_REAL max_ratio = VERDICT_DBL_MAX;
  const VERDICT_REAL filter_n = max_ratio * 1.0e-16;
  const VERDICT_REAL filter_d = 1.0e-16;
  if ( fabs( numerator ) <= filter_n &&
      fabs( denominator ) >= filter_d )
    return_value = numerator / denominator;
  
  else
    return_value = fabs(numerator) / max_ratio >= fabs(denominator) ?
      ( numerator >= 0.0 && denominator >= 0.0 ||
        numerator < 0.0 && denominator < 0.0 ?
        max_ratio : -max_ratio )
      : numerator / denominator;
  
  return return_value;
  
}



VERDICT_REAL condition_comp( const VerdictVector &xxi, 
    const VerdictVector &xet, 
    const VerdictVector &xze )
{
  VERDICT_REAL det =  xxi % (xet * xze);
  
  if( det <= VERDICT_DBL_MIN) { return 1.e+06; }
  
  
  VERDICT_REAL term1 = xxi % xxi + xet % xet + xze % xze;
  VERDICT_REAL term2 = ( xxi * xet ) % ( xxi * xet ) + ( xet * xze ) % ( xet * xze ) + ( xze * xxi ) % ( xze * xxi );
  
  return sqrt( term1 * term2 ) / det;
}



VERDICT_REAL oddy_comp( const VerdictVector &xxi, 
    const VerdictVector &xet, 
    const VerdictVector &xze )
{
  static const VERDICT_REAL third=1.0/3.0;
  
  VERDICT_REAL g11, g12, g13, g22, g23, g33, rt_g;
  
  g11 = xxi % xxi;
  g12 = xxi % xet;
  g13 = xxi % xze;
  g22 = xet % xet;
  g23 = xet % xze;
  g33 = xze % xze;
  rt_g = xxi % (xet * xze);
  
  VERDICT_REAL oddy_metric;
  if( rt_g > 0. ) 
  {
    VERDICT_REAL norm_G_squared = g11*g11 + 2.0*g12*g12 + 2.0*g13*g13 + g22*g22 + 2.0*g23*g23 +g33*g33;
    
    VERDICT_REAL norm_J_squared = g11 + g22 + g33;
    
    oddy_metric = ( norm_G_squared - third*norm_J_squared*norm_J_squared ) / pow( rt_g, 4.*third );
    
  }
  
  else
    oddy_metric = 1.e+06;
  
  return oddy_metric;
  
}


//! calcualates edge lengths of a hex
VERDICT_REAL hex_edge_length(int max_min, VERDICT_REAL coordinates[][3])
{
  VERDICT_REAL temp[3], edge[12];
  int i;
  
  //lengths^2 of edges
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[1][i] - coordinates[0][i];
  edge[0] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[2][i] - coordinates[1][i];
  edge[1] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[3][i] - coordinates[2][i];
  edge[2] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[0][i] - coordinates[3][i];
  edge[3] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[5][i] - coordinates[4][i];
  edge[4] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[6][i] - coordinates[5][i];
  edge[5] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[7][i] - coordinates[6][i];
  edge[6] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[4][i] - coordinates[7][i];
  edge[7] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[4][i] - coordinates[0][i];
  edge[8] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[5][i] - coordinates[1][i];
  edge[9] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[6][i] - coordinates[2][i];
  edge[10] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[7][i] - coordinates[3][i];
  edge[11] = length_squared(temp);
  
  VERDICT_REAL _edge = edge[0];
  
  if(max_min == 0)
  {
    for( i = 1; i<12; i++) 
      _edge = VERDICT_MIN( _edge, edge[i] ); 
    return _edge;
  }  
  else
  {
    for( i = 1; i<12; i++) 
      _edge = VERDICT_MAX( _edge, edge[i] );
    return _edge;
  }
  
}


VERDICT_REAL diag_length(int max_min, VERDICT_REAL coordinates[][3]) 
{
  VERDICT_REAL temp[3], diag[4];
  int i = 0;
  
  //lengths^2  f diag nals
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[6][i] - coordinates[0][i];
  diag[0] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[7][i] - coordinates[1][i];
  diag[1] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[4][i] - coordinates[2][i];
  diag[2] = length_squared(temp);
  
  for (i = 0; i < 3; i++ )
    temp[i] = coordinates[5][i] - coordinates[3][i];
  diag[3] = length_squared(temp);
  
       
  VERDICT_REAL diagonal = diag[0];
  if(max_min == 0 )  //Return max diagonal
  { 
    for( i = 1; i<4; i++)
      diagonal = VERDICT_MIN( diagonal, diag[i] );
    return diagonal;
  }
  else
  {
    for( i = 1; i<4; i++)
      diagonal = VERDICT_MAX( diagonal, diag[i] );
    return diagonal;  
  }

}

//! calculates efg values
VerdictVector calc_hex_efg( int efg_index, VerdictVector coordinates[8])
{
  
  VerdictVector efg;
  
  switch(efg_index) {
    
    case 2:
      efg = -coordinates[0];
      efg += coordinates[1];
      efg += coordinates[2];
      efg -= coordinates[3];
      efg -= coordinates[4];
      efg += coordinates[5];
      efg += coordinates[6];
      efg -= coordinates[7];
      break;
      
    case 3:
      efg = -coordinates[0];
      efg -= coordinates[1];
      efg += coordinates[2];
      efg += coordinates[3];
      efg -= coordinates[4];
      efg -= coordinates[5];
      efg += coordinates[6];
      efg += coordinates[7];
      break;
      
    case 4:
      efg = -coordinates[0];
      efg -= coordinates[1];
      efg -= coordinates[2];
      efg -= coordinates[3];
      efg += coordinates[4];
      efg += coordinates[5];
      efg += coordinates[6];
      efg += coordinates[7];
      break;

    case 5:
      efg =  coordinates[0];
      efg += coordinates[1];
      efg -= coordinates[2];
      efg -= coordinates[3];
      efg -= coordinates[4];
      efg -= coordinates[5];
      efg += coordinates[6];
      efg += coordinates[7];
      break;
      
    case 6:
      efg =  coordinates[0];
      efg += coordinates[3];
      efg -= coordinates[4];
      efg -= coordinates[7];
      efg -= coordinates[2];
      efg -= coordinates[1];
      efg += coordinates[5];
      efg += coordinates[6];
      break;

   case 7:
      efg =  coordinates[0];
      efg += coordinates[4];
      efg -= coordinates[7];
      efg -= coordinates[3];
      efg -= coordinates[1];
      efg -= coordinates[5];
      efg += coordinates[6];
      efg += coordinates[2];
      break;
      
   default:
      efg.set(0,0,0);
      
  }
  
  return efg;
}

/*!
  the aspect ratio of a hexahedral

  Maximum edge length ratios at hex center
*/
C_FUNC_DEF VERDICT_REAL v_hex_aspect (int /*num_nodes*/, VERDICT_REAL coordinates[][3])
{
  
  VERDICT_REAL aspect;
  VerdictVector node_pos[8];
  
  localize_hex_coordinates( coordinates, node_pos );
  
  VerdictVector efg2, efg3, efg4;
  VERDICT_REAL aspect_xy, aspect_xz, aspect_yz;
  
  efg2 = calc_hex_efg( 2, node_pos);
  efg3 = calc_hex_efg( 3, node_pos);
  efg4 = calc_hex_efg( 4, node_pos);
  
  aspect_xy = safe_ratio( VERDICT_MAX( efg2.x(), efg3.y() ) , VERDICT_MIN( efg2.x(), efg3.y() ) );
  aspect_xz = safe_ratio( VERDICT_MAX( efg2.x(), efg4.z() ) , VERDICT_MIN( efg2.x(), efg4.z() ) );
  aspect_yz = safe_ratio( VERDICT_MAX( efg3.y(), efg4.z() ) , VERDICT_MIN( efg3.y(), efg4.z() ) );

  aspect = VERDICT_MAX( aspect_xy, VERDICT_MAX( aspect_xz, aspect_yz ) );
  
  return aspect;
}

/*!
  skew of a hex

  Maximum ||cosA|| where A is the angle between edges at hex center.
*/
C_FUNC_DEF VERDICT_REAL v_hex_skew( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VerdictVector node_pos[8];
  localize_hex_coordinates ( coordinates, node_pos );
  
  VerdictVector efg4, efg3, efg2;
  VERDICT_REAL skewx, skewy, skewz;
  
  efg4 = calc_hex_efg( 4, node_pos);
  efg3 = calc_hex_efg( 3, node_pos);
  efg2 = calc_hex_efg( 2, node_pos);
  
  
  skewx = fabs(efg4.y()/efg4.z()) /
    sqrt((efg4.y()*efg4.y()) / (efg4.z()*efg4.z()) + 1.0);
  
  skewy = fabs(efg4.x()/efg4.z()) /
    sqrt((efg4.x()*efg4.x()) / (efg4.z()*efg4.z()) + 1.0);
  
  skewz = fabs(efg3.x()/efg3.y()) /
    sqrt((efg3.x()*efg3.x()) / (efg3.y()*efg3.y()) + 1.0);
  
  VERDICT_REAL answer = (VERDICT_MAX( skewx, VERDICT_MAX( skewy, skewz ) ));
  return answer;
  
}

/*!
  taper of a hex

  Maximum ratio of lengths derived from opposite edges.
*/
C_FUNC_DEF VERDICT_REAL v_hex_taper( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector node_pos[8];
  localize_hex_coordinates ( coordinates, node_pos );
  
  VerdictVector efg2 = calc_hex_efg( 2, node_pos);
  VerdictVector efg3 = calc_hex_efg( 3, node_pos);
  VerdictVector efg4 = calc_hex_efg( 4, node_pos);
  VerdictVector efg5 = calc_hex_efg( 5, node_pos);

  VERDICT_REAL taperx = fabs( safe_ratio( efg5.x(), efg2.x() ) );
  VERDICT_REAL tapery = fabs( safe_ratio( efg5.y(), efg3.y() ) );
  VERDICT_REAL taperz = fabs( safe_ratio( efg5.z(), efg4.z() ) );
  
  return VERDICT_MAX(taperx, VERDICT_MAX(tapery, taperz));  
  
}

/*!
  volume of a hex

  Jacobian at hex center
*/
C_FUNC_DEF VERDICT_REAL v_hex_volume( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VerdictVector vec01, vec02, vec03, vec04, vec05, vec06, vec07;
  VERDICT_REAL volume = 0;
  
  vec01.set( coordinates[1][0] - coordinates[0][0],
             coordinates[1][1] - coordinates[0][1],
             coordinates[1][2] - coordinates[0][2] );

  vec02.set( coordinates[2][0] - coordinates[0][0],
             coordinates[2][1] - coordinates[0][1],
             coordinates[2][2] - coordinates[0][2] );

  vec03.set( coordinates[3][0] - coordinates[0][0],
             coordinates[3][1] - coordinates[0][1],
             coordinates[3][2] - coordinates[0][2] );

  vec04.set( coordinates[4][0] - coordinates[0][0],
             coordinates[4][1] - coordinates[0][1],
             coordinates[4][2] - coordinates[0][2] );

  vec05.set( coordinates[5][0] - coordinates[0][0],
             coordinates[5][1] - coordinates[0][1],
             coordinates[5][2] - coordinates[0][2] );

  vec06.set( coordinates[6][0] - coordinates[0][0],
             coordinates[6][1] - coordinates[0][1],
             coordinates[6][2] - coordinates[0][2] );

  vec07.set( coordinates[7][0] - coordinates[0][0],
             coordinates[7][1] - coordinates[0][1],
             coordinates[7][2] - coordinates[0][2] );

  volume += (vec05 % (vec01 * vec02 ) );
  volume += (vec04 % (vec05 * vec07 ) );
  volume += (vec05 % (vec02 * vec06 ) );
  volume += (vec07 % (vec05 * vec06 ) );
  volume += (vec07 % (vec06 * vec03 ) );
  volume += (vec06 % (vec02 * vec03 ) );

  volume /= 6;
  return volume;
}

/*!
  stretch of a hex

  sqrt(3) * minimum edge length / maximum diagonal length
*/
C_FUNC_DEF VERDICT_REAL v_hex_stretch( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  static const VERDICT_REAL HEX_STRETCH_SCALE_FACTOR = sqrt(3.0);
  
  VERDICT_REAL min_edge = hex_edge_length( 0, coordinates );
  VERDICT_REAL max_diag = diag_length( 1, coordinates );  
  
  return HEX_STRETCH_SCALE_FACTOR * sqrt( safe_ratio(min_edge, max_diag)  );
  
}

/*!
  diagonal ratio of a hex
  
  Minimum diagonal length / maximum diagonal length
*/
C_FUNC_DEF VERDICT_REAL v_hex_diagonal( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VERDICT_REAL min_diag = diag_length( 0, coordinates ); 
  VERDICT_REAL max_diag = diag_length( 1, coordinates );
  
  return sqrt (min_diag / max_diag );
}

#define SQR(x) ((x) * (x))

/*!
  dimension of a hex

  Pronto-specific characteristic length for stable time step calculation. 
  Char_length = Volume / 2 grad Volume
*/
C_FUNC_DEF VERDICT_REAL v_hex_dimension( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VERDICT_REAL gradop[9][4];

  VERDICT_REAL x1 = coordinates[0][0];
  VERDICT_REAL x2 = coordinates[1][0];
  VERDICT_REAL x3 = coordinates[2][0];
  VERDICT_REAL x4 = coordinates[3][0];
  VERDICT_REAL x5 = coordinates[4][0];
  VERDICT_REAL x6 = coordinates[5][0];
  VERDICT_REAL x7 = coordinates[6][0];
  VERDICT_REAL x8 = coordinates[7][0];

  VERDICT_REAL y1 = coordinates[0][1];
  VERDICT_REAL y2 = coordinates[1][1];
  VERDICT_REAL y3 = coordinates[2][1];
  VERDICT_REAL y4 = coordinates[3][1];
  VERDICT_REAL y5 = coordinates[4][1];
  VERDICT_REAL y6 = coordinates[5][1];
  VERDICT_REAL y7 = coordinates[6][1];
  VERDICT_REAL y8 = coordinates[7][1];

  VERDICT_REAL z1 = coordinates[0][2];
  VERDICT_REAL z2 = coordinates[1][2];
  VERDICT_REAL z3 = coordinates[2][2];
  VERDICT_REAL z4 = coordinates[3][2];
  VERDICT_REAL z5 = coordinates[4][2];
  VERDICT_REAL z6 = coordinates[5][2];
  VERDICT_REAL z7 = coordinates[6][2];
  VERDICT_REAL z8 = coordinates[7][2];

  VERDICT_REAL z24 = z2 - z4;
  VERDICT_REAL z52 = z5 - z2;
  VERDICT_REAL z45 = z4 - z5;
  gradop[1][1] = ( y2*(z6-z3-z45) + y3*z24 + y4*(z3-z8-z52)
      + y5*(z8-z6-z24) + y6*z52 + y8*z45 ) / 12.0;
         
        VERDICT_REAL z31 = z3 - z1;
  VERDICT_REAL z63 = z6 - z3;
  VERDICT_REAL z16 = z1 - z6;
  gradop[2][1] = ( y3*(z7-z4-z16) + y4*z31 + y1*(z4-z5-z63)
      + y6*(z5-z7-z31) + y7*z63 + y5*z16 ) / 12.0;

  VERDICT_REAL z42 = z4 - z2;
  VERDICT_REAL z74 = z7 - z4;
  VERDICT_REAL z27 = z2 - z7;
  gradop[3][1] = ( y4*(z8-z1-z27) + y1*z42 + y2*(z1-z6-z74)
      + y7*(z6-z8-z42) + y8*z74 + y6*z27 ) / 12.0;
             
  VERDICT_REAL z13 = z1 - z3;
  VERDICT_REAL z81 = z8 - z1;
  VERDICT_REAL z38 = z3 - z8;
  gradop[4][1] = ( y1*(z5-z2-z38) + y2*z13 + y3*(z2-z7-z81)
      + y8*(z7-z5-z13) + y5*z81 + y7*z38 ) / 12.0;
   
  VERDICT_REAL z86 = z8 - z6;
  VERDICT_REAL z18 = z1 - z8;
  VERDICT_REAL z61 = z6 - z1;
  gradop[5][1] = ( y8*(z4-z7-z61) + y7*z86 + y6*(z7-z2-z18)
      + y1*(z2-z4-z86) + y4*z18 + y2*z61 ) / 12.0;
     
  VERDICT_REAL z57 = z5 - z7;
  VERDICT_REAL z25 = z2 - z5;
  VERDICT_REAL z72 = z7 - z2;
  gradop[6][1] = ( y5*(z1-z8-z72) + y8*z57 + y7*(z8-z3-z25)
      + y2*(z3-z1-z57) + y1*z25 + y3*z72 ) / 12.0;
       
  VERDICT_REAL z68 = z6 - z8;
  VERDICT_REAL z36 = z3 - z6;
  VERDICT_REAL z83 = z8 - z3;
  gradop[7][1] = ( y6*(z2-z5-z83) + y5*z68 + y8*(z5-z4-z36)
      + y3*(z4-z2-z68) + y2*z36 + y4*z83 ) / 12.0;
         
  VERDICT_REAL z75 = z7 - z5;
  VERDICT_REAL z47 = z4 - z7;
  VERDICT_REAL z54 = z5 - z4;
  gradop[8][1] = ( y7*(z3-z6-z54) + y6*z75 + y5*(z6-z1-z47)
      + y4*(z1-z3-z75) + y3*z47 + y1*z54 ) / 12.0;
           
  VERDICT_REAL x24 = x2 - x4;
  VERDICT_REAL x52 = x5 - x2;
  VERDICT_REAL x45 = x4 - x5;
  gradop[1][2] = ( z2*(x6-x3-x45) + z3*x24 + z4*(x3-x8-x52)
      + z5*(x8-x6-x24) + z6*x52 + z8*x45 ) / 12.0;

  VERDICT_REAL x31 = x3 - x1;
  VERDICT_REAL x63 = x6 - x3;
  VERDICT_REAL x16 = x1 - x6;
  gradop[2][2] = ( z3*(x7-x4-x16) + z4*x31 + z1*(x4-x5-x63)
      + z6*(x5-x7-x31) + z7*x63 + z5*x16 ) / 12.0;

  VERDICT_REAL x42 = x4 - x2;
  VERDICT_REAL x74 = x7 - x4;
  VERDICT_REAL x27 = x2 - x7;
  gradop[3][2] = ( z4*(x8-x1-x27) + z1*x42 + z2*(x1-x6-x74)
      + z7*(x6-x8-x42) + z8*x74 + z6*x27 ) / 12.0;

  VERDICT_REAL x13 = x1 - x3;
  VERDICT_REAL x81 = x8 - x1;
  VERDICT_REAL x38 = x3 - x8;
  gradop[4][2] = ( z1*(x5-x2-x38) + z2*x13 + z3*(x2-x7-x81)
      + z8*(x7-x5-x13) + z5*x81 + z7*x38 ) / 12.0;

  VERDICT_REAL x86 = x8 - x6;
  VERDICT_REAL x18 = x1 - x8;
  VERDICT_REAL x61 = x6 - x1;
  gradop[5][2] = ( z8*(x4-x7-x61) + z7*x86 + z6*(x7-x2-x18)
      + z1*(x2-x4-x86) + z4*x18 + z2*x61 ) / 12.0;
                     
  VERDICT_REAL x57 = x5 - x7;
  VERDICT_REAL x25 = x2 - x5;
  VERDICT_REAL x72 = x7 - x2;
  gradop[6][2] = ( z5*(x1-x8-x72) + z8*x57 + z7*(x8-x3-x25)
      + z2*(x3-x1-x57) + z1*x25 + z3*x72 ) / 12.0;

  VERDICT_REAL x68 = x6 - x8;
  VERDICT_REAL x36 = x3 - x6;
  VERDICT_REAL x83 = x8 - x3;
  gradop[7][2] = ( z6*(x2-x5-x83) + z5*x68 + z8*(x5-x4-x36)
      + z3*(x4-x2-x68) + z2*x36 + z4*x83 ) / 12.0;

  VERDICT_REAL x75 = x7 - x5;
  VERDICT_REAL x47 = x4 - x7;
  VERDICT_REAL x54 = x5 - x4;
  gradop[8][2] = ( z7*(x3-x6-x54) + z6*x75 + z5*(x6-x1-x47)
      + z4*(x1-x3-x75) + z3*x47 + z1*x54 ) / 12.0;
           
  VERDICT_REAL y24 = y2 - y4;
  VERDICT_REAL y52 = y5 - y2;
  VERDICT_REAL y45 = y4 - y5;
  gradop[1][3] = ( x2*(y6-y3-y45) + x3*y24 + x4*(y3-y8-y52)
      + x5*(y8-y6-y24) + x6*y52 + x8*y45 ) / 12.0;
             
  VERDICT_REAL y31 = y3 - y1;
  VERDICT_REAL y63 = y6 - y3;
  VERDICT_REAL y16 = y1 - y6;
  gradop[2][3] = ( x3*(y7-y4-y16) + x4*y31 + x1*(y4-y5-y63)
      + x6*(y5-y7-y31) + x7*y63 + x5*y16 ) / 12.0;
               
  VERDICT_REAL y42 = y4 - y2;
  VERDICT_REAL y74 = y7 - y4;
  VERDICT_REAL y27 = y2 - y7;
  gradop[3][3] = ( x4*(y8-y1-y27) + x1*y42 + x2*(y1-y6-y74)
      + x7*(y6-y8-y42) + x8*y74 + x6*y27 ) / 12.0;
                 
  VERDICT_REAL y13 = y1 - y3;
  VERDICT_REAL y81 = y8 - y1;
  VERDICT_REAL y38 = y3 - y8;
  gradop[4][3] = ( x1*(y5-y2-y38) + x2*y13 + x3*(y2-y7-y81)
      + x8*(y7-y5-y13) + x5*y81 + x7*y38 ) / 12.0;
                   
  VERDICT_REAL y86 = y8 - y6;
  VERDICT_REAL y18 = y1 - y8;
  VERDICT_REAL y61 = y6 - y1;
  gradop[5][3] = ( x8*(y4-y7-y61) + x7*y86 + x6*(y7-y2-y18)
      + x1*(y2-y4-y86) + x4*y18 + x2*y61 ) / 12.0;
                     
  VERDICT_REAL y57 = y5 - y7;
  VERDICT_REAL y25 = y2 - y5;
  VERDICT_REAL y72 = y7 - y2;
  gradop[6][3] = ( x5*(y1-y8-y72) + x8*y57 + x7*(y8-y3-y25)
      + x2*(y3-y1-y57) + x1*y25 + x3*y72 ) / 12.0;
                       
  VERDICT_REAL y68 = y6 - y8;
  VERDICT_REAL y36 = y3 - y6;
  VERDICT_REAL y83 = y8 - y3;
  gradop[7][3] = ( x6*(y2-y5-y83) + x5*y68 + x8*(y5-y4-y36)
      + x3*(y4-y2-y68) + x2*y36 + x4*y83 ) / 12.0;
                         
  VERDICT_REAL y75 = y7 - y5;
  VERDICT_REAL y47 = y4 - y7;
  VERDICT_REAL y54 = y5 - y4;
  gradop[8][3] = ( x7*(y3-y6-y54) + x6*y75 + x5*(y6-y1-y47)
      + x4*(y1-y3-y75) + x3*y47 + x1*y54 ) / 12.0;

  //     calculate element volume and characteristic element aspect ratio
  //     (used in time step and hourglass control) - 



  VERDICT_REAL volume =  coordinates[0][0] * gradop[1][1]
    + coordinates[1][0] * gradop[2][1]
    + coordinates[2][0] * gradop[3][1]
    + coordinates[3][0] * gradop[4][1]
    + coordinates[4][0] * gradop[5][1]
    + coordinates[5][0] * gradop[6][1]
    + coordinates[6][0] * gradop[7][1]
    + coordinates[7][0] * gradop[8][1];
  VERDICT_REAL aspect = .5*SQR(volume) /
    ( SQR(gradop[1][1]) + SQR(gradop[2][1])
      + SQR(gradop[3][1]) + SQR(gradop[4][1])
      + SQR(gradop[5][1]) + SQR(gradop[6][1])
      + SQR(gradop[7][1]) + SQR(gradop[8][1])
      + SQR(gradop[1][2]) + SQR(gradop[2][2])
      + SQR(gradop[3][2]) + SQR(gradop[4][2])
      + SQR(gradop[5][2]) + SQR(gradop[6][2])
      + SQR(gradop[7][2]) + SQR(gradop[8][2])
      + SQR(gradop[1][3]) + SQR(gradop[2][3])
      + SQR(gradop[3][3]) + SQR(gradop[4][3])
      + SQR(gradop[5][3]) + SQR(gradop[6][3])
      + SQR(gradop[7][3]) + SQR(gradop[8][3]) );
     
  return sqrt(aspect);
  
}

/*!
  oddy of a hex

  General distortion measure based on left Cauchy-Green Tensor
*/
C_FUNC_DEF VERDICT_REAL v_hex_oddy( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VERDICT_REAL oddy = 0.0, current_oddy = 0.0;
  VerdictVector xxi, xet, xze;
  
  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );
  
  xxi = calc_hex_efg(2, node_pos);
  xet = calc_hex_efg(3, node_pos);
  xze = calc_hex_efg(4, node_pos);
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  xet.set(coordinates[3][0] - coordinates[0][0],
      coordinates[3][1] - coordinates[0][1],
      coordinates[3][2] - coordinates[0][2] );
  
  xze.set(coordinates[4][0] - coordinates[0][0],
      coordinates[4][1] - coordinates[0][1],
      coordinates[4][2] - coordinates[0][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  xet.set(coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2] );
  
  xze.set(coordinates[5][0] - coordinates[1][0],
      coordinates[5][1] - coordinates[1][1],
      coordinates[5][2] - coordinates[1][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[2][0] - coordinates[3][0],
      coordinates[2][1] - coordinates[3][1],
      coordinates[2][2] - coordinates[3][2] );
  
  xet.set(coordinates[3][0] - coordinates[0][0],
      coordinates[3][1] - coordinates[0][1],
      coordinates[3][2] - coordinates[0][2] );
  
  xze.set(coordinates[7][0] - coordinates[3][0],
      coordinates[7][1] - coordinates[3][1],
      coordinates[7][2] - coordinates[3][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[5][0] - coordinates[4][0],
      coordinates[5][1] - coordinates[4][1],
      coordinates[5][2] - coordinates[4][2] );
  
  xet.set(coordinates[7][0] - coordinates[4][0],
      coordinates[7][1] - coordinates[4][1],
      coordinates[7][2] - coordinates[4][2] );
  
  xze.set(coordinates[4][0] - coordinates[0][0],
      coordinates[4][1] - coordinates[0][1],
      coordinates[4][2] - coordinates[0][2] );
 
 
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[2][0] - coordinates[3][0],
      coordinates[2][1] - coordinates[3][1],
      coordinates[2][2] - coordinates[3][2] );
  
  xet.set(coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2] );
  
  xze.set(coordinates[6][0] - coordinates[2][0],
      coordinates[6][1] - coordinates[2][1],
      coordinates[6][2] - coordinates[2][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[5][0] - coordinates[4][0],
      coordinates[5][1] - coordinates[4][1],
      coordinates[5][2] - coordinates[4][2] );
  
  xet.set(coordinates[6][0] - coordinates[5][0],
      coordinates[6][1] - coordinates[5][1],
      coordinates[6][2] - coordinates[5][2] );
  
  xze.set(coordinates[5][0] - coordinates[1][0],
      coordinates[5][1] - coordinates[1][1],
      coordinates[5][2] - coordinates[1][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[6][0] - coordinates[7][0],
      coordinates[6][1] - coordinates[7][1],
      coordinates[6][2] - coordinates[7][2] );
  
  xet.set(coordinates[7][0] - coordinates[4][0],
      coordinates[7][1] - coordinates[4][1],
      coordinates[7][2] - coordinates[4][2] );
  
  xze.set(coordinates[7][0] - coordinates[3][0],
      coordinates[7][1] - coordinates[3][1],
      coordinates[7][2] - coordinates[3][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  xxi.set(coordinates[6][0] - coordinates[7][0],
      coordinates[6][1] - coordinates[7][1],
      coordinates[6][2] - coordinates[7][2] );
  
  xet.set(coordinates[6][0] - coordinates[5][0],
      coordinates[6][1] - coordinates[5][1],
      coordinates[6][2] - coordinates[5][2] );
  
  xze.set(coordinates[6][0] - coordinates[2][0],
      coordinates[6][1] - coordinates[2][1],
      coordinates[6][2] - coordinates[2][2] );
  
  
  current_oddy = oddy_comp( xxi, xet, xze);
  if ( current_oddy > oddy ) { oddy = current_oddy; }
  
  return oddy;  

}

/*!
  condition number of a hex

  Maximum condition number of the Jacobian matrix at 8 corners
*/
C_FUNC_DEF VERDICT_REAL v_hex_condition( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );

  VERDICT_REAL condition = 0.0, current_condition = 0.0; 
  VerdictVector xxi, xet, xze;

  xxi = calc_hex_efg(2, node_pos );
  xet = calc_hex_efg(3, node_pos );
  xze = calc_hex_efg(4, node_pos );

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  current_condition = condition_comp( xxi, xet, xze );
  if ( current_condition > condition ) { condition = current_condition; }

  return condition /= 3.0;

}

/*!
  jacobian of a hex

  Minimum pointwise volume of local map at 8 corners & center of hex
*/
C_FUNC_DEF VERDICT_REAL v_hex_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );

  VERDICT_REAL jacobian = VERDICT_DBL_MAX;
  VERDICT_REAL current_jacobian = VERDICT_DBL_MAX; 
  VerdictVector xxi, xet, xze;

  xxi = calc_hex_efg(2, node_pos );
  xet = calc_hex_efg(3, node_pos );
  xze = calc_hex_efg(4, node_pos );


  current_jacobian = xxi % (xet * xze) / 64.0;
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  current_jacobian = xxi % (xet * xze);
  if ( current_jacobian < jacobian ) { jacobian = current_jacobian; }

  return jacobian;

}

/*!
  scaled jacobian of a hex

  Minimum Jacobian divided by the lengths of the 3 edge vectors
*/
C_FUNC_DEF VERDICT_REAL v_hex_scaled_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VERDICT_REAL jacobi, min_norm_jac = VERDICT_DBL_MAX;  
  VERDICT_REAL min_jacobi = VERDICT_DBL_MAX;
  VERDICT_REAL temp_norm_jac, lengths;
  VerdictVector xxi, xet, xze;

  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );

  xxi = calc_hex_efg(2, node_pos );
  xet = calc_hex_efg(3, node_pos );
  xze = calc_hex_efg(4, node_pos );


  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else  
    temp_norm_jac = jacobi;

        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }

  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else  
    temp_norm_jac = jacobi;


        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else  
    temp_norm_jac = jacobi;

  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else  
    temp_norm_jac = jacobi;

  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else  
    temp_norm_jac = jacobi;

  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else 
    temp_norm_jac = jacobi;

  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else
    temp_norm_jac = jacobi;

  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else
    temp_norm_jac = jacobi;

  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  jacobi = xxi % ( xet * xze );
  if( jacobi < min_jacobi ) { min_jacobi = jacobi; }
  
  lengths = sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
  temp_norm_jac = jacobi / lengths;
  if( temp_norm_jac < min_norm_jac)
    min_norm_jac = temp_norm_jac;  
  else 
    temp_norm_jac = jacobi;

  
  return min_norm_jac;

}

/*!
  shear of a hex
  
  3/Condition number of Jacobian Skew matrix
*/
C_FUNC_DEF VERDICT_REAL v_hex_shear( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VERDICT_REAL shear;
  VERDICT_REAL min_shear = 1.0; 
  VerdictVector xxi, xet, xze;
  VERDICT_REAL det;


  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );

        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );


  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  if ( det < VERDICT_DBL_MIN ) { return 0; }
    shear = det / sqrt( (xxi%xxi)*(xet%xet)*(xze%xze) );

  min_shear = VERDICT_MIN( shear, min_shear );
  return min_shear;

}

/*!
  shape of a hex

  3/Condition number of weighted Jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_hex_shape( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{


  VERDICT_REAL det, shape;
  VERDICT_REAL min_shape = 1.0; 
  static const VERDICT_REAL two_thirds = 2.0/3.0;

  VerdictVector xxi, xet, xze;

  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );


        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }
  

  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  if ( det > VERDICT_DBL_MIN ) 
    shape = 3 * pow( det, two_thirds) / (xxi%xxi + xet%xet + xze%xze);
  else
    return 0;
  
  if( shape < min_shape ) { min_shape = shape; }

  
  return min_shape;

}

/*!
  relative size of a hex

  Min( J, 1/J ), where J is determinant of weighted Jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_hex_relative_size( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL size = 0;
  VERDICT_REAL tau; 

  VerdictVector xxi, xet, xze;
  VERDICT_REAL det, det_sum = 0;

  v_hex_get_weight( xxi, xet, xze );
  
  VERDICT_REAL detw = xxi % (xet * xze);


  if ( detw < VERDICT_DBL_MIN ) 
  { 
    return 0; 
   }

  VerdictVector node_pos[8];
  make_hex_nodes ( coordinates, node_pos );

        // J(0,0,0):

  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[4] - node_pos[0];
  
  det = xxi % (xet * xze);
  det_sum += det;  


        // J(1,0,0):
  
  xxi = node_pos[1] - node_pos[0];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  det_sum += det;  


  // J(0,1,0):
  
  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[3] - node_pos[0];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  det_sum += det;  

  // J(0,0,1):
  
  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[4] - node_pos[0];

  det = xxi % (xet * xze);
  det_sum += det;  


  // J(1,1,0):

  xxi = node_pos[2] - node_pos[3];
  xet = node_pos[2] - node_pos[1];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  det_sum += det;  


  // J(1,0,1):

  xxi = node_pos[5] - node_pos[4];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[5] - node_pos[1];

  det = xxi % (xet * xze);
  det_sum += det;  


  // J(0,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[7] - node_pos[4];
  xze = node_pos[7] - node_pos[3];

  det = xxi % (xet * xze);
  det_sum += det;  


  // J(1,1,1):

  xxi = node_pos[6] - node_pos[7];
  xet = node_pos[6] - node_pos[5];
  xze = node_pos[6] - node_pos[2];

  det = xxi % (xet * xze);
  det_sum += det;  


  if( det_sum > VERDICT_DBL_MIN )
  {
    tau = det_sum / ( 8*detw);

    size = VERDICT_MIN( tau*tau, 1.0/tau/tau);
  }


  return size;

}

/*!
  shape and size of a hex

  Product of Shape and Relative Size
*/
C_FUNC_DEF VERDICT_REAL v_hex_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL size = v_hex_relative_size( num_nodes, coordinates );
  VERDICT_REAL shape = v_hex_shape( num_nodes, coordinates );

  return size * shape;

}



/*!
  shear and size of a hex

  Product of Shear and Relative Size
*/
C_FUNC_DEF VERDICT_REAL v_hex_shear_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL size = v_hex_relative_size( num_nodes, coordinates );
  VERDICT_REAL shear = v_hex_shear( num_nodes, coordinates );

  return size * shear;

}

/*!
  distortion of a hex
*/
VERDICT_REAL v_hex_distortion( int num_nodes, VERDICT_REAL coordinates[][3] )
{

   //use 2x2 gauss points for linear hex and 3x3 for 2nd order hex
   int number_of_gauss_points=0;
   if (num_nodes ==8)
      //2x2 quadrature rule
      number_of_gauss_points = 2;
   else if (num_nodes ==20)
      //3x3 quadrature rule
      number_of_gauss_points = 3;

   int number_dimension = 3;
   int total_number_of_gauss_points = number_of_gauss_points
      *number_of_gauss_points*number_of_gauss_points;
   VERDICT_REAL distortion = VERDICT_DBL_MAX;

   // maxTotalNumberGaussPoints =27, maxNumberNodes = 20
   // they are defined in GaussIntegration.hpp
   // This is used to make these arrays static.
   // I tried dynamically allocated arrays but the new and delete
   // was very expensive

   VERDICT_REAL shape_function[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy1[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy2[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy3[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL weight[maxTotalNumberGaussPoints];


   //create an object of GaussIntegration
   GaussIntegration gauss_integration(number_of_gauss_points,num_nodes,number_dimension );
   gauss_integration.calculate_shape_function_3d_hex();
   gauss_integration.get_shape_func(shape_function[0], dndy1[0], dndy2[0], dndy3[0],weight);


   VerdictVector xxi, xet, xze, xin;

   VERDICT_REAL jacobian, minimum_jacobian;
   VERDICT_REAL element_volume =0.0;
   minimum_jacobian = VERDICT_DBL_MAX;
   // calculate element volume
   int ife, ja;
   for (ife=0;ife<total_number_of_gauss_points; ife++)
   {

      xxi.set(0.0,0.0,0.0);
      xet.set(0.0,0.0,0.0);
      xze.set(0.0,0.0,0.0);

      for (ja=0;ja<num_nodes;ja++)
      {
   xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
         xxi += dndy1[ife][ja]*xin;
         xet += dndy2[ife][ja]*xin;
         xze += dndy3[ife][ja]*xin;
      }

      jacobian = xxi % (xet * xze);
      if (minimum_jacobian > jacobian)
         minimum_jacobian = jacobian;

      element_volume += weight[ife]*jacobian;
      }

   // loop through all nodes
   VERDICT_REAL dndy1_at_node[maxNumberNodes][maxNumberNodes];
   VERDICT_REAL dndy2_at_node[maxNumberNodes][maxNumberNodes];
   VERDICT_REAL dndy3_at_node[maxNumberNodes][maxNumberNodes];

   gauss_integration.calculate_derivative_at_nodes_3d( dndy1_at_node, dndy2_at_node, dndy3_at_node);
   int node_id;
   for (node_id=0;node_id<num_nodes; node_id++)
   {

      xxi.set(0.0,0.0,0.0);
      xet.set(0.0,0.0,0.0);
      xze.set(0.0,0.0,0.0);

      for (ja=0;ja<num_nodes;ja++)
      {
   xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
         xxi += dndy1_at_node[node_id][ja]*xin;
         xet += dndy2_at_node[node_id][ja]*xin;
         xze += dndy3_at_node[node_id][ja]*xin;
      }

      jacobian = xxi % (xet * xze);
      if (minimum_jacobian > jacobian)
         minimum_jacobian = jacobian;

      }
   distortion = minimum_jacobian/element_volume*8.;
   return distortion;
}





/*
C_FUNC_DEF VERDICT_REAL hex_jac_normjac_oddy_cond( int choices[], 
                      VERDICT_REAL coordinates[][3],
                      VERDICT_REAL answers[4]  )
{

  //Define variables
  int i;
  
  VERDICT_REAL xxi[3], xet[3], xze[3];
  VERDICT_REAL norm_jacobian = 0.0, current_norm_jac = 0.0;
        VERDICT_REAL jacobian = 0.0, current_jacobian = 0.0;
  VERDICT_REAL oddy = 0.0, current_oddy = 0.0;  
  VERDICT_REAL condition = 0.0, current_condition = 0.0;


        for( i=0; i<3; i++)
          xxi[i] = calc_hex_efg( 2, i, coordinates );
        for( i=0; i<3; i++)
          xet[i] = calc_hex_efg( 3, i, coordinates );
        for( i=0; i<3; i++)
          xze[i] = calc_hex_efg( 6, i, coordinates );

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze  );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 

    current_jacobian /= 64.0;
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }


  for( i=0; i<3; i++)
  {
    xxi[i] = coordinates[1][i] - coordinates[0][i];
    xet[i] = coordinates[3][i] - coordinates[0][i];
    xze[i] = coordinates[4][i] - coordinates[0][i];
  } 

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze  );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }

  
  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[1][i] - coordinates[0][i];
          xet[i] = coordinates[2][i] - coordinates[1][i];
          xze[i] = coordinates[5][i] - coordinates[1][i];
  }

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi,  xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }


  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[2][i] - coordinates[3][i];
          xet[i] = coordinates[3][i] - coordinates[0][i];
          xze[i] = coordinates[7][i] - coordinates[3][i];
  }

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }

  
  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[5][i] - coordinates[4][i];
          xet[i] = coordinates[7][i] - coordinates[4][i];
          xze[i] = coordinates[4][i] - coordinates[0][i];
  }  


  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }

  
  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[2][i] - coordinates[3][i];
          xet[i] = coordinates[2][i] - coordinates[1][i];
          xze[i] = coordinates[6][i] - coordinates[2][i];
  }  

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }


  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[5][i] - coordinates[4][i];
          xet[i] = coordinates[6][i] - coordinates[5][i];
          xze[i] = coordinates[5][i] - coordinates[1][i];
  }


  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }

    
  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[6][i] - coordinates[7][i];
          xet[i] = coordinates[7][i] - coordinates[4][i];
          xze[i] = coordinates[7][i] - coordinates[3][i];
  }


  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }
  }

  
  for( i=0; i<3; i++)
  {
          xxi[i] = coordinates[6][i] - coordinates[7][i];
          xet[i] = coordinates[6][i] - coordinates[5][i];
          xze[i] = coordinates[6][i] - coordinates[2][i];
  }

  // norm jacobian and jacobian
  if( choices[0] || choices[1] )
  {
    current_jacobian = determinant( xxi, xet, xze );
    current_norm_jac = normalize_jacobian( current_jacobian,
            xxi, xet, xze );
    
    if (current_norm_jac < norm_jacobian) { norm_jacobian = current_norm_jac; } 
    if (current_jacobian < jacobian) { jacobian = current_jacobian; }
  }

  // oddy 
  if( choices[2] )
  {
    current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
  }

  // condition
  if( choices[3] )
  {
    current_condition = condition_comp( xxi, xet, xze );
    if ( current_condition > condition ) { condition = current_condition; }

    condition /= 3.0;
  }

  
  answers[0] = jacobian;
  answers[1] = norm_jacobian;
  answers[2] = oddy;
  answers[3] = condition;

  return 1.0;

}
*/

/*!
  multiple quality metrics of a hex
*/
C_FUNC_DEF void v_hex_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
  unsigned int metrics_request_flag, HexMetricVals *metric_vals )
{ 
  memset( metric_vals, 0, sizeof(HexMetricVals) );

  // aspect, skew, taper, and volume
  if(metrics_request_flag & (V_HEX_ASPECT | V_HEX_SKEW | V_HEX_TAPER ) ) 
  {
    VerdictVector node_pos[8];
    // coordinates need to be localized
    localize_hex_coordinates( coordinates, node_pos );
    VerdictVector efg2, efg3, efg4;
    efg2 = calc_hex_efg( 2, node_pos);
    efg3 = calc_hex_efg( 3, node_pos);
    efg4 = calc_hex_efg( 4, node_pos);

    if(metrics_request_flag & V_HEX_ASPECT)
    {
      VERDICT_REAL aspect_xy, aspect_xz, aspect_yz;
      aspect_xy = safe_ratio( VERDICT_MAX( efg2.x(), efg3.y() ) , VERDICT_MIN( efg2.x(), efg3.y() ) );
      aspect_xz = safe_ratio( VERDICT_MAX( efg2.x(), efg4.z() ) , VERDICT_MIN( efg2.x(), efg4.z() ) );
      aspect_yz = safe_ratio( VERDICT_MAX( efg3.y(), efg4.z() ) , VERDICT_MIN( efg3.y(), efg4.z() ) );
      
      metric_vals->aspect = VERDICT_MAX( aspect_xy, VERDICT_MAX( aspect_xz, aspect_yz ) );
    }
    
    if(metrics_request_flag & V_HEX_SKEW)
    {
      VERDICT_REAL skewx, skewy, skewz;
      skewx = fabs(efg4.y()/efg4.z()) /
        sqrt((efg4.y()*efg4.y()) / (efg4.z()*efg4.z()) + 1.0);
      
      skewy = fabs(efg4.x()/efg4.z()) /
        sqrt((efg4.x()*efg4.x()) / (efg4.z()*efg4.z()) + 1.0);
      
      skewz = fabs(efg3.x()/efg3.y()) /
        sqrt((efg3.x()*efg3.x()) / (efg3.y()*efg3.y()) + 1.0);
      
      metric_vals->skew = (VERDICT_MAX( skewx, VERDICT_MAX( skewy, skewz ) ));
    }
  
    if(metrics_request_flag & V_HEX_TAPER)
    {
      VerdictVector efg5 = calc_hex_efg( 5, node_pos);
      VERDICT_REAL taperx = fabs( safe_ratio( efg5.x(), efg2.x() ) );
      VERDICT_REAL tapery = fabs( safe_ratio( efg5.y(), efg3.y() ) );
      VERDICT_REAL taperz = fabs( safe_ratio( efg5.z(), efg4.z() ) );
      
      metric_vals->taper = VERDICT_MAX(taperx, VERDICT_MAX(tapery, taperz));  

    }
  }
  
  if(metrics_request_flag & V_HEX_VOLUME)
  {
    metric_vals->volume = v_hex_volume(6, coordinates ); 
  }

  if(metrics_request_flag & ( V_HEX_JACOBIAN | V_HEX_SCALED_JACOBIAN | V_HEX_CONDITION |
        V_HEX_SHEAR | V_HEX_SHAPE | V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE |
        V_HEX_STRETCH ))
  {

    static const VERDICT_REAL two_thirds = 2.0/3.0;
    VerdictVector edges[12];
    // the length squares
    VERDICT_REAL length_squared[12];
    // make vectors from coordinates
    make_hex_edges(coordinates, edges);

    // calculate the length squares if we need to
    if(metrics_request_flag & ( V_HEX_JACOBIAN | V_HEX_SHEAR | V_HEX_SCALED_JACOBIAN | V_HEX_SHAPE | 
          V_HEX_SHAPE_AND_SIZE | V_HEX_RELATIVE_SIZE | V_HEX_SHEAR_AND_SIZE | V_HEX_STRETCH))
      make_edge_length_squares(edges, length_squared);

    VERDICT_REAL jacobian = VERDICT_DBL_MAX, scaled_jacobian = VERDICT_DBL_MAX,
      condition = 0.0, shear=1.0, shape=1.0, oddy = 0.0;
    VERDICT_REAL current_jacobian, current_scaled_jacobian, current_condition, current_shape,
      detw=0, det_sum=0, current_oddy;
    VerdictBoolean rel_size_error = VERDICT_FALSE;
    
    VerdictVector xxi, xet, xze;

    // get weights if we need based on average size of a hex
    if(metrics_request_flag & (V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      v_hex_get_weight(xxi, xet, xze);
      detw = xxi % (xet * xze);
      if(detw < VERDICT_DBL_MIN)
        rel_size_error = VERDICT_TRUE;
    }

    xxi = edges[0] - edges[2] + edges[4] - edges[6];
    xet = edges[1] - edges[3] + edges[5] - edges[7];
    xze = edges[8] + edges[9] + edges[10] + edges[11];

    current_jacobian = xxi % (xet * xze) / 64.0;
    if ( current_jacobian < jacobian ) 
      jacobian = current_jacobian;


    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_jacobian *= 64.0;
      current_scaled_jacobian = current_jacobian / 
        sqrt(xxi.length_squared() * xet.length_squared() * xze.length_squared());
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }

    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( xxi, xet, xze );
      if ( current_condition > condition ) { condition = current_condition; }
    }

    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( xxi, xet, xze );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }
    

    // J(0,0,0)
    current_jacobian = edges[0] % (-edges[3] * edges[8]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;

    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[0] * length_squared[3] * length_squared[8]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }

    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( edges[0], -edges[3], edges[8] );
      if ( current_condition > condition ) { condition = current_condition; }
    }

    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( edges[0], -edges[3], edges[8] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[0] + length_squared[3] + length_squared[8]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(1,0,0)
    current_jacobian = edges[0] % (edges[1] * edges[9]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[0] * length_squared[1] * length_squared[9]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }

    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( edges[0], edges[1], edges[9] );
      if ( current_condition > condition ) { condition = current_condition; }
    }

    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( edges[0], edges[1], edges[9] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }
    
    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[0] + length_squared[1] + length_squared[9]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(0,1,0)
    current_jacobian = -edges[2] % (-edges[3] * edges[11]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[2] * length_squared[3] * length_squared[11]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }
    
    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( -edges[2], -edges[3], edges[11] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( -edges[2], -edges[3], edges[11] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[2] + length_squared[3] + length_squared[11]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }


    // J(0,0,1)
    current_jacobian = edges[4] % (-edges[7] * edges[8]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[4] * length_squared[7] * length_squared[8]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }

    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( edges[4], -edges[7], edges[8] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( edges[4], -edges[7], edges[8] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[4] + length_squared[7] + length_squared[8]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(1,1,0)
    current_jacobian = -edges[2] % (edges[1] * edges[10]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[2] * length_squared[1] * length_squared[10]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }

    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( -edges[2], edges[1], edges[10] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( -edges[2], edges[1], edges[10] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[2] + length_squared[1] + length_squared[10]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(1,0,1)
    current_jacobian = edges[4] % (edges[5] * edges[9]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[4] * length_squared[5] * length_squared[9]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }
    
    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( edges[4], edges[5], edges[9] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( edges[4], edges[5], edges[9] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[4] + length_squared[5] + length_squared[9]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(0,1,1)
    current_jacobian = -edges[6] % (-edges[7] * edges[11]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[6] * length_squared[7] * length_squared[11]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }
    
    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( -edges[6], -edges[7], edges[11] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( -edges[6], -edges[7], edges[11] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[6] + length_squared[7] + length_squared[11]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    // J(1,1,1)
    current_jacobian = -edges[6] % (edges[5] * edges[10]);
    if ( current_jacobian < jacobian )
      jacobian = current_jacobian;
    
    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      det_sum += current_jacobian;
    }
    
    if(metrics_request_flag & ( V_HEX_SCALED_JACOBIAN | V_HEX_SHEAR | V_HEX_SHEAR_AND_SIZE ))
    {
      current_scaled_jacobian = current_jacobian / 
        sqrt(length_squared[6] * length_squared[5] * length_squared[10]);
      if(current_scaled_jacobian < scaled_jacobian)
        shear = scaled_jacobian = current_scaled_jacobian;
      else
        current_scaled_jacobian = current_jacobian;
    }
    
    if(metrics_request_flag & V_HEX_CONDITION)
    {
      current_condition = condition_comp( -edges[6], edges[5], edges[10] );
      if ( current_condition > condition ) { condition = current_condition; }
    }
    
    if(metrics_request_flag & V_HEX_ODDY)
    {
      current_oddy = oddy_comp( -edges[6], edges[5], edges[10] );
      if ( current_oddy > oddy ) { oddy = current_oddy; }
    }

    if(metrics_request_flag & ( V_HEX_SHAPE | V_HEX_SHAPE_AND_SIZE ))
    {
      if(current_jacobian > VERDICT_DBL_MIN)
        current_shape = 3 * pow( current_jacobian, two_thirds) / 
          (length_squared[6] + length_squared[5] + length_squared[10]);
      else
        current_shape = 0;

      if(current_shape < shape) { shape = current_shape; }

    }

    if(metrics_request_flag & ( V_HEX_RELATIVE_SIZE | V_HEX_SHAPE_AND_SIZE | V_HEX_SHEAR_AND_SIZE ))
    {
      if(det_sum > VERDICT_DBL_MIN && rel_size_error != VERDICT_TRUE)
      {
        VERDICT_REAL tau = det_sum / ( 8 * detw );
        metric_vals->relative_size = VERDICT_MIN( tau*tau, 1.0/tau/tau);
      }
      else
        metric_vals->relative_size = 0;
    }

    // set values from above calculations
    if(metrics_request_flag & V_HEX_JACOBIAN)
      metric_vals->jacobian = jacobian;

    if(metrics_request_flag & V_HEX_SCALED_JACOBIAN)
      metric_vals->scaled_jacobian = scaled_jacobian;

    if(metrics_request_flag & V_HEX_CONDITION)
      metric_vals->condition = condition/3.0;

    if(metrics_request_flag & V_HEX_SHEAR)
      metric_vals->shear = shear;

    if(metrics_request_flag & V_HEX_SHAPE)
      metric_vals->shape = shape; 
  
    if(metrics_request_flag & V_HEX_SHAPE_AND_SIZE)
      metric_vals->shape_and_size = shape * metric_vals->relative_size;
  
    if(metrics_request_flag & V_HEX_SHEAR_AND_SIZE)
      metric_vals->shear_and_size = shear * metric_vals->relative_size;
  
    if(metrics_request_flag & V_HEX_ODDY)
      metric_vals->oddy = oddy;

    if(metrics_request_flag & V_HEX_STRETCH)
    {
      static const VERDICT_REAL HEX_STRETCH_SCALE_FACTOR = sqrt(3.0);
      VERDICT_REAL min_edge=length_squared[0];
      for(int j=1; j<12; j++)
        min_edge = VERDICT_MIN(min_edge, length_squared[j]);

      VERDICT_REAL max_diag = diag_length(1, coordinates);
        
      metric_vals->stretch = HEX_STRETCH_SCALE_FACTOR * sqrt( safe_ratio(min_edge, max_diag)  );
    }
  }


  if(metrics_request_flag & V_HEX_DIAGONAL)
    metric_vals->diagonal = v_hex_diagonal(num_nodes, coordinates);
  if(metrics_request_flag & V_HEX_DIMENSION)
    metric_vals->dimension = v_hex_dimension(num_nodes, coordinates);
  if(metrics_request_flag & V_HEX_DISTORTION)
                metric_vals->distortion = v_hex_distortion(num_nodes, coordinates);
}




/*

KnifeMetrics.cpp contains quality calculations for knives

*/


/*  a knife element

          3
         _/\_
       _/  | \_
   0 _/        \_ 2
    |\_    | ___/|
    |  \  __/    |
    |  1\/ |     |
    |    \       |
    |_____\|_____|
   4       5      6      


    (edge 3,5  is is a hidden line if you will)

    if this is hard to visualize, consider a hex
    with nodes 5 and 7 becoming the same node


*/




/*!
  calculates the volume of a knife element

  this is done by dividing the knife into 4 tets
  and summing the volumes of each.
*/

C_FUNC_DEF VERDICT_REAL v_knife_volume( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL volume = 0;
  VerdictVector side1, side2, side3;
  
  if (num_nodes == 7)
  {
    
    // divide the knife into 4 tets and calculate the volume
    
    side1.set(
        coordinates[1][0] - coordinates[0][0],
        coordinates[1][1] - coordinates[0][1],
        coordinates[1][2] - coordinates[0][2]
        );
    side2.set(
        coordinates[3][0] - coordinates[0][0],
        coordinates[3][1] - coordinates[0][1],
        coordinates[3][2] - coordinates[0][2]
        );
    side3.set(
        coordinates[4][0] - coordinates[0][0],
        coordinates[4][1] - coordinates[0][1],
        coordinates[4][2] - coordinates[0][2]
        );

    volume = side3 % (side1 * side2) / 6;
   

    side1.set(
        coordinates[5][0] - coordinates[1][0],
        coordinates[5][1] - coordinates[1][1],
        coordinates[5][2] - coordinates[1][2]
        );
    side2.set(
        coordinates[3][0] - coordinates[1][0],
        coordinates[3][1] - coordinates[1][1],
        coordinates[3][2] - coordinates[1][2]
        );
    side3.set(
        coordinates[4][0] - coordinates[1][0],
        coordinates[4][1] - coordinates[1][1],
        coordinates[4][2] - coordinates[1][2]
        );

    volume += side3 % (side1 * side2) / 6;
    
    
    side1.set(
        coordinates[2][0] - coordinates[1][0],
        coordinates[2][1] - coordinates[1][1],
        coordinates[2][2] - coordinates[1][2]
        );
    side2.set(
        coordinates[3][0] - coordinates[1][0],
        coordinates[3][1] - coordinates[1][1],
        coordinates[3][2] - coordinates[1][2]
        );
    side3.set(
        coordinates[6][0] - coordinates[1][0],
        coordinates[6][1] - coordinates[1][1],
        coordinates[6][2] - coordinates[1][2]
        );

    volume += side3 % (side1 * side2) / 6;
   
    
    side1.set(
        coordinates[3][0] - coordinates[1][0],
        coordinates[3][1] - coordinates[1][1],
        coordinates[3][2] - coordinates[1][2]
        );
    side2.set(
        coordinates[5][0] - coordinates[1][0],
        coordinates[5][1] - coordinates[1][1],
        coordinates[5][2] - coordinates[1][2]
        );
    side3.set(
        coordinates[6][0] - coordinates[1][0],
        coordinates[6][1] - coordinates[1][1],
        coordinates[6][2] - coordinates[1][2]
        );

    volume += side3 % (side1 * side2) / 6;
  
  }

  return volume;
    
}


/*!
 
  calculate the quality metrics of a knife element.

  There is only one, but we put this here to be consistent with
  functions for other element types.  Who knows if we'll add
  more metrics.
*/

C_FUNC_DEF void v_knife_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, KnifeMetricVals *metric_vals )
{
  memset( metric_vals, 0, sizeof(KnifeMetricVals) );

  if(metrics_request_flag & V_KNIFE_VOLUME)
    metric_vals->volume = v_knife_volume(num_nodes, coordinates);
}



/*

PyramidMetrics.cpp contains quality calculations for Pyramids
 
*/


/*
  the pyramid element

       5
       ^
       |\ 
      /| \\_
     |  \   \
     |  | \_ \_
     /   \/4\  \
    |   /|    \ \_
    | /  \      \ \
    /     |       \
  1 \_    |      _/3
      \_   \   _/
        \_ | _/
          \_/
          2

    a quadrilateral base and a pointy peak like a pyramid
          
*/


/*!
  the volume of a pyramid

  the volume is calculated by dividing the pyramid into
  2 tets and summing the volumes of the 2 tets.
*/

C_FUNC_DEF VERDICT_REAL v_pyramid_volume( int num_nodes, VERDICT_REAL coordinates[][3] )
{
    
  VERDICT_REAL volume = 0;
  VerdictVector side1, side2, side3;
  
  if (num_nodes == 5)
  {
    // divide the pyramid into 2 tets and calculate each

    side1.set( coordinates[1][0] - coordinates[0][0],
        coordinates[1][1] - coordinates[0][1],
        coordinates[1][2] - coordinates[0][2] );
    
    side2.set( coordinates[3][0] - coordinates[0][0],
        coordinates[3][1] - coordinates[0][1],
        coordinates[3][2] - coordinates[0][2] );
    
    side3.set( coordinates[4][0] - coordinates[0][0],
        coordinates[4][1] - coordinates[0][1], 
        coordinates[4][2] - coordinates[0][2] );
    
    // volume of the first tet
    volume = (side3 % (side1 * side2 ))/6.0;
    
    
    side1.set( coordinates[3][0] - coordinates[2][0],
        coordinates[3][1] - coordinates[2][1],
        coordinates[3][2] - coordinates[2][2] );
    
    side2.set( coordinates[1][0] - coordinates[2][0],
        coordinates[1][1] - coordinates[2][1],
        coordinates[1][2] - coordinates[2][2] );
    
    side3.set( coordinates[4][0] - coordinates[2][0],
        coordinates[4][1] - coordinates[2][1],
        coordinates[4][2] - coordinates[2][2] );
    
    // volume of the second tet
    volume += (side3 % (side1 * side2 ))/6.0;
 
  }   
  return volume;
    
}



C_FUNC_DEF void v_pyramid_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, PyramidMetricVals *metric_vals )
{
  memset( metric_vals, 0, sizeof( PyramidMetricVals ) );

  if(metrics_request_flag & V_PYRAMID_VOLUME)
    metric_vals->volume = v_pyramid_volume(num_nodes, coordinates);
}




/*
 
QuadMetric.cpp contains quality calculations for Quads
 
*/ 

//! the average area of a quad
VERDICT_REAL verdict_quad_size = 0;

/*!
  weights based on the average size of a quad
*/
int get_weight ( VERDICT_REAL &m11,
    VERDICT_REAL &m21,
    VERDICT_REAL &m12,
    VERDICT_REAL &m22 )
{
  
  m11=1;
  m21=0;
  m12=0;
  m22=1;
  
  VERDICT_REAL scale = sqrt( verdict_quad_size/(m11*m22-m21*m12));

  m11 *= scale;
  m21 *= scale;
  m12 *= scale;
  m22 *= scale;
  
  return 1;

}

//! return the average area of a quad
C_FUNC_DEF void v_set_quad_size( VERDICT_REAL size )
{
  verdict_quad_size = size;
}

//! returns whether the quad is collapsed or not
VerdictBoolean is_collapsed_quad ( VERDICT_REAL coordinates[][3] )
{
  if( coordinates[3][0] == coordinates[2][0] &&
      coordinates[3][1] == coordinates[2][1] &&
      coordinates[3][2] == coordinates[2][2] )
    return VERDICT_TRUE;
  
  else
    return VERDICT_FALSE;
}

/*!
  localize the coordinates of a quad

  localizing puts the centriod of the quad
  at the orgin and also rotates the quad
  such that edge (0,1) is aligned with the x axis 
  and the quad normal lines up with the y axis.

*/
void localize_quad_coordinates(VerdictVector nodes[4])
{
  int i;
  VerdictVector global[4] = { nodes[0], nodes[1], nodes[2], nodes[3] };
  
  VerdictVector center = (global[0] + global[1] + global[2] + global[3]) / 4.0;
  for(i=0; i<4; i++)
    global[i] -= center;
  
  VerdictVector vector_diff;
  VerdictVector vector_sum;
  VerdictVector ref_point(0.0,0.0,0.0);
  VerdictVector tmp_vector, normal(0.0,0.0,0.0);
  VerdictVector vector1, vector2;
  
  for(i=0; i<4; i++)
  {
    vector1 = global[i];
    vector2 = global[(i+1)%4];
    vector_diff = vector2 - vector1;
    ref_point += vector1;
    vector_sum = vector1 + vector2;
    
    tmp_vector.set(vector_diff.y() * vector_sum.z(),
        vector_diff.z() * vector_sum.x(),
        vector_diff.x() * vector_sum.y());
    normal += tmp_vector;
  }
  
  normal.normalize();
  normal *= -1.0;
  
  
  VerdictVector local_x_axis = global[1] - global[0];
  local_x_axis.normalize();
  
  VerdictVector local_y_axis = normal * local_x_axis;
  local_y_axis.normalize();
  
  for (i=0; i < 4; i++)
  {
    nodes[i].x(global[i] % local_x_axis);
    nodes[i].y(global[i] % local_y_axis);
    nodes[i].z(global[i] % normal);
  }
}

/*! 
  moves and rotates the quad such that it enables us to 
  use components of ef's
*/
void localize_quad_for_ef( VerdictVector node_pos[4])
{

  VerdictVector centroid(node_pos[0]);
  centroid += node_pos[1];
  centroid += node_pos[2];
  centroid += node_pos[3];
  
  centroid /= 4.0;

  node_pos[0] -= centroid;
  node_pos[1] -= centroid;
  node_pos[2] -= centroid;
  node_pos[3] -= centroid;

  VerdictVector rotate = node_pos[1] + node_pos[2] - node_pos[3] - node_pos[0];
  rotate.normalize();

  VERDICT_REAL cosine = rotate.x();
  VERDICT_REAL   sine = rotate.y();
 
  VERDICT_REAL xnew;
 
  for (int i=0; i < 4; i++) 
  {
    xnew =  cosine * node_pos[i].x() +   sine * node_pos[i].y();
    node_pos[i].y( -sine * node_pos[i].x() + cosine * node_pos[i].y() );
    node_pos[i].x(xnew);
  }
}

/*!
  returns the normal vector of a quad
*/
VerdictVector quad_normal( VERDICT_REAL coordinates[][3] )
{
  // get normal at node 0
  VerdictVector edge0, edge1;
  
  edge0.set( coordinates[1][0] - coordinates[0][0], 
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  
  edge1.set( coordinates[3][0] - coordinates[0][0], 
      coordinates[3][1] - coordinates[0][1],
      coordinates[3][2] - coordinates[0][2] );
  
  VerdictVector norm0 = edge0 * edge1 ;
  norm0.normalize();
  
  // because some faces may have obtuse angles, check another normal at
  // node 2 for consistent sense


  edge0.set ( coordinates[2][0] - coordinates[3][0], 
      coordinates[2][1] - coordinates[3][1],
      coordinates[2][2] - coordinates[3][2] );
  
  
  edge1.set ( coordinates[2][0] - coordinates[1][0], 
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2] );
  
  VerdictVector norm2 = edge0 * edge1 ;
  norm2.normalize();
  
  // if these two agree, we are done, else test a third to decide

  if ( (norm0 % norm2) > 0.0 )
  {
    norm0 += norm2;
    norm0 *= 0.5;
    return norm0;
  }
  
  // test normal at node1


  edge0.set ( coordinates[1][0] - coordinates[2][0], 
      coordinates[1][1] - coordinates[2][1],
      coordinates[1][2] - coordinates[2][2] );
  
  
  edge1.set ( coordinates[1][0] - coordinates[0][0], 
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  VerdictVector norm1 = edge0 * edge1 ;
  norm1.normalize();
  
  if ( (norm0 % norm1) > 0.0 )
  {
    norm0 += norm1;
    norm0 *= 0.5;
    return norm0;
  }
  else
  {
    norm2 += norm1;
    norm2 *= 0.5;
    return norm2;
  }

}

/*!
  aspect ratio of a quad

  maximum edge length ratios at quad center
*/
C_FUNC_DEF VERDICT_REAL v_quad_aspect( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector node_pos[4];

  for(int i = 0; i < 4; i++ )
  {
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
  }

  localize_quad_coordinates( node_pos );

  localize_quad_for_ef( node_pos );

  VerdictVector ef2 = -node_pos[0] + node_pos[1] + node_pos[2] - node_pos[3];
  VerdictVector ef3 = -node_pos[0] - node_pos[1] + node_pos[2] + node_pos[3];

  VERDICT_REAL aspect1 = ef2.x() / ef3.y();
  aspect1 = (aspect1 < 1.0 ? 1.0/ aspect1 : aspect1);

  // handle another permutation of node order
  VerdictVector node_pos2[4];
  node_pos2[0] = node_pos[1];
  node_pos2[1] = node_pos[2];
  node_pos2[2] = node_pos[3];
  node_pos2[3] = node_pos[0];
  
  localize_quad_for_ef(node_pos2);

  ef2 = -node_pos2[0] + node_pos2[1] + node_pos2[2] - node_pos2[3];
  ef3 = -node_pos2[0] - node_pos2[1] + node_pos2[2] + node_pos2[3];

  VERDICT_REAL aspect2 = ef2.x() / ef3.y();
  aspect2 = (aspect2 < 1.0 ? 1.0/ aspect2 : aspect2);

  return VERDICT_MAX(aspect1, aspect2);

}

/*!
  skew of a quad

  maximum ||cos A|| where A is the angle between edges at quad center
*/
C_FUNC_DEF VERDICT_REAL v_quad_skew( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector node_pos[4];
  
  for(int i = 0; i < 4; i++ )
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
  
  localize_quad_coordinates( node_pos );
  localize_quad_for_ef(node_pos);

  VerdictVector ef2 = -node_pos[0] - node_pos[1] +
    node_pos[2] + node_pos[3]; 
  
  VerdictVector ef3 = -node_pos[0] + node_pos[1] +
    node_pos[2] - node_pos[3]; 
  
  VERDICT_REAL skew = fabs( ef2 % ef3)/ (ef2.length() * ef3.length() );

  return skew;

}

/*! 
  taper of a quad

  maximum ratio of lengths derived from opposite edges
*/
C_FUNC_DEF VERDICT_REAL v_quad_taper( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector node_pos[4];

  for(int i = 0; i < 4; i++ )
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
    
  localize_quad_coordinates( node_pos );

  localize_quad_for_ef(node_pos);

  VerdictVector ef2 = -node_pos[0] + node_pos[1] + node_pos[2] - node_pos[3];
  VerdictVector ef3 = -node_pos[0] - node_pos[1] + node_pos[2] + node_pos[3];
  VerdictVector ef4 =  node_pos[0] - node_pos[1] + node_pos[2] - node_pos[3];
  
  VERDICT_REAL taper1 = VERDICT_MAX( fabs(ef4.y() / ef3.y()) , fabs(ef4.x() / ef2.x()) );
  
  // handle another permutation of node order
  VerdictVector node_pos2[4];
  node_pos2[0] = node_pos[1];
  node_pos2[1] = node_pos[2];
  node_pos2[2] = node_pos[3];
  node_pos2[3] = node_pos[0];

  localize_quad_for_ef(node_pos2);

  ef2 = -node_pos2[0] + node_pos2[1] + node_pos2[2] - node_pos2[3];
  ef3 = -node_pos2[0] - node_pos2[1] + node_pos2[2] + node_pos2[3];
  ef4 =  node_pos2[0] - node_pos2[1] + node_pos2[2] - node_pos2[3];
  
  VERDICT_REAL taper2 = VERDICT_MAX( fabs(ef4.y() / ef3.y()) , fabs(ef4.x() / ef2.x()) );

  return VERDICT_MIN(taper1, taper2);

}

/*!
  warpage of a quad

  deviation of element from planarity
*/
C_FUNC_DEF VERDICT_REAL v_quad_warpage( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector xxi, xet, unit_normal[4];
  
  VERDICT_REAL min_value = 0;
  VERDICT_REAL min_mag = VERDICT_DBL_MAX;
  
  for ( int i=0; i<4; i++ ) 
  {
    xxi.set(coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    xet.set(coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    unit_normal[i] = xxi * xet;
    
    VERDICT_REAL mag = unit_normal[i].length();
    min_mag = VERDICT_MIN( min_mag, mag );
    
    if (mag > VERDICT_DBL_MIN )
      unit_normal[i] /= mag;
    
  }
  
  if ( min_mag > VERDICT_DBL_MIN ) 
  {
    VERDICT_REAL dot1 = unit_normal[0] % unit_normal[2];
    VERDICT_REAL dot2 = unit_normal[1] % unit_normal[3];
    
    if ( (dot1>VERDICT_DBL_MIN) && (dot1>VERDICT_DBL_MIN) ) 
      min_value = pow( VERDICT_MIN( dot1, dot2 ), 4 );
  }
  
  return min_value;
}

/*!
  the area of a quad

  jacobian at quad center
*/
C_FUNC_DEF VERDICT_REAL v_quad_area( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{    

  VerdictVector node_pos[4];
  for(int i = 0; i < 4; i++ )
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
  
  localize_quad_coordinates( node_pos );
  localize_quad_for_ef( node_pos );

  VerdictVector ef3 = -node_pos[0] - node_pos[1] +
           node_pos[2] + node_pos[3]; 

  VerdictVector ef2 = -node_pos[0] + node_pos[1] +
          node_pos[2] - node_pos[3]; 

  VERDICT_REAL area1 = ef2.x() * ef3.y() / 4.0;
  VERDICT_REAL area2 = -ef3.x() * ef2.y() / 4.0;
  return area1 + area2;
}

/*!
  the stretch of a quad

  sqrt(2) * minimum edge length / maximum diagonal length
*/
C_FUNC_DEF VERDICT_REAL v_quad_stretch( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector node_pos[4];
  
  for(int i = 0; i < 4; i++ )
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);


  VerdictVector temp;
  temp = node_pos[1] - node_pos[0];
  VERDICT_REAL side01 = temp.length_squared();

  temp = node_pos[2] - node_pos[1];
  VERDICT_REAL side12 = temp.length_squared();

  temp = node_pos[3] - node_pos[2];
  VERDICT_REAL side23 = temp.length_squared();

  temp = node_pos[0] - node_pos[3];
  VERDICT_REAL side30 = temp.length_squared();

  temp = node_pos[2] - node_pos[0];
  VERDICT_REAL diag02 = temp.length_squared();

  temp = node_pos[3] - node_pos[1];
  VERDICT_REAL diag13 = temp.length_squared();
  
  static const VERDICT_REAL QUAD_STRETCH_FACTOR = sqrt(2.0);

  return  sqrt( 
      VERDICT_MIN(VERDICT_MIN(VERDICT_MIN(side01,side12),side23),side30) / 
        VERDICT_MAX(diag02, diag13)
      ) * QUAD_STRETCH_FACTOR ; 
  

}

/*!
  the largest angle of a quad

  largest included quad area (degrees)
*/
C_FUNC_DEF VERDICT_REAL v_quad_largest_angle( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  // if this is a collapsed quad, just pass it on to 
  // the tri_largest_angle routine
  if( is_collapsed_quad(coordinates) == VERDICT_TRUE )
    return v_tri_largest_angle(3, coordinates);

  VERDICT_REAL angle;
  VERDICT_REAL max_angle = 0.0;
  VerdictVector face_normal = quad_normal(coordinates);
  
  VerdictVector edges[4];
  edges[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  edges[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  edges[2].set(
      coordinates[3][0] - coordinates[2][0],
      coordinates[3][1] - coordinates[2][1],
      coordinates[3][2] - coordinates[2][2]
      );
  edges[3].set(
      coordinates[0][0] - coordinates[3][0],
      coordinates[0][1] - coordinates[3][1],
      coordinates[0][2] - coordinates[3][2]
      );

  // go around each node and calculate the angle
  // at each node

  angle = face_normal.vector_angle(edges[0], -edges[3] ); 
  max_angle = VERDICT_MAX(angle, max_angle);

  angle = face_normal.vector_angle(edges[1], -edges[0] ); 
  max_angle = VERDICT_MAX(angle, max_angle);

  angle = face_normal.vector_angle(edges[2], -edges[1] ); 
  max_angle = VERDICT_MAX(angle, max_angle);

  angle = face_normal.vector_angle(edges[3], -edges[2] ); 
  max_angle = VERDICT_MAX(angle, max_angle);

  return max_angle *180.0/VERDICT_PI;

}

/*!
  the smallest angle of a quad

  smallest included quad angle (degrees)
*/
C_FUNC_DEF VERDICT_REAL v_quad_smallest_angle( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
 
  // if this quad is a collapsed quad, then just
  // send it to the tri_smallest_angle routine 
  if ( is_collapsed_quad(coordinates) == VERDICT_TRUE )
    return v_tri_smallest_angle(3, coordinates);

  VERDICT_REAL angle;
  VERDICT_REAL min_angle = 6.5;
  VerdictVector face_normal = quad_normal(coordinates);
  
  VerdictVector edges[4];
  edges[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  edges[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  edges[2].set(
      coordinates[3][0] - coordinates[2][0],
      coordinates[3][1] - coordinates[2][1],
      coordinates[3][2] - coordinates[2][2]
      );
  edges[3].set(
      coordinates[0][0] - coordinates[3][0],
      coordinates[0][1] - coordinates[3][1],
      coordinates[0][2] - coordinates[3][2]
      );

  // go around each node and calculate the angle
  // at each node

  angle = face_normal.vector_angle(edges[0], -edges[3] ); 
  min_angle = VERDICT_MIN(angle, min_angle);

  angle = face_normal.vector_angle(edges[1], -edges[0] ); 
  min_angle = VERDICT_MIN(angle, min_angle);

  angle = face_normal.vector_angle(edges[2], -edges[1] ); 
  min_angle = VERDICT_MIN(angle, min_angle);

  angle = face_normal.vector_angle(edges[3], -edges[2] ); 
  min_angle = VERDICT_MIN(angle, min_angle);

  return min_angle *180.0/VERDICT_PI;
}

/*!
  the oddy of a quad

  general distortion measure based on left Cauchy-Green Tensor
*/
C_FUNC_DEF VERDICT_REAL v_quad_oddy( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  
  VERDICT_REAL max_oddy = 0.;
  
  VerdictVector first, second, node_pos[4];
  
  VERDICT_REAL g, g11, g12, g22, cur_oddy;
  int i;
  
  for(i = 0; i < 4; i++ )
    node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
  
  for ( i = 0; i < 4; i++ )
  {
    first  = node_pos[i] - node_pos[(i+1)%4];
    second = node_pos[i] - node_pos[(i+3)%4];
    
    g11 = first % first;
    g12 = first % second;
    g22 = second % second;
    g = g11*g22 - g12*g12;
    
    if ( g != 0. ) {
      cur_oddy = ( (g11-g22)*(g11-g22) + 4.*g12*g12 ) / 2. / g;
    }
    else {
      cur_oddy = 1.e+06;
    }
    
    max_oddy = VERDICT_MAX(max_oddy, cur_oddy);
  }
  
  return max_oddy;
}


/*!
  the condition of a quad

  maximum condition number of the Jacobian matrix at 4 corners
*/
C_FUNC_DEF VERDICT_REAL v_quad_condition( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  if ( is_collapsed_quad( coordinates ) == VERDICT_TRUE ) 
    return v_tri_condition(3,coordinates);
  
  VerdictVector face_normal = quad_normal( coordinates );
  face_normal.normalize();
  
  VERDICT_REAL max_condition = 0.;
  
  VerdictVector xxi, xet;
  
  VERDICT_REAL condition, det;
  
  for ( int i=0; i<4; i++ ) 
  {
    
    xxi.set( coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    xet.set( coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    det = face_normal % ( xxi * xet );
    
    if ( det <  VERDICT_DBL_MIN )
      condition = 1.e+06;
    else 
      condition = ( xxi % xxi + xet % xet ) / det;
    
    max_condition = VERDICT_MAX(max_condition, condition);
  }
  
  return max_condition /= 2;
}

/*!
  the jacobian of a quad

  minimum pointwise volume of local map at 4 corners and center of quad
*/
C_FUNC_DEF VERDICT_REAL v_quad_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
   
  if ( is_collapsed_quad( coordinates ) == VERDICT_TRUE )
    return v_tri_area(3, coordinates) * 2.0;
  
  VerdictVector face_normal = quad_normal( coordinates );
  VERDICT_REAL min_jac = VERDICT_DBL_MAX;
  VerdictVector first, second;
  
  for ( int i=0; i<4; i++ ) 
  {
    first.set( coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    second.set( coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    VERDICT_REAL cur_jac = face_normal % ( first * second );
    min_jac = VERDICT_MIN(min_jac, cur_jac);
  }
  return min_jac;
}


/*!
  scaled jacobian of a quad

  Minimum Jacobian divided by the lengths of the 2 edge vector
*/
C_FUNC_DEF VERDICT_REAL v_quad_scaled_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  if ( is_collapsed_quad( coordinates ) == VERDICT_TRUE ) 
    return v_tri_scaled_jacobian(3, coordinates);
  
  VerdictVector face_normal = quad_normal( coordinates );
  
  VERDICT_REAL min_scaled_jac = VERDICT_DBL_MAX;
  
  VerdictVector first, second;
  
  for ( int i=0; i<4; i++ ) 
  { 
    first.set( coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    second.set( coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    VERDICT_REAL rt_g11 = first.length();
    VERDICT_REAL rt_g22 = second.length();
    
    VERDICT_REAL cur_jac = 0.0;
    
    if ( rt_g11 != 0. && rt_g22 != 0. )
      cur_jac = face_normal % ( first * second ) / rt_g11 / rt_g22;
    
    min_scaled_jac = VERDICT_MIN(min_scaled_jac, cur_jac);
    
  }
  return min_scaled_jac;

}

/*!
  the shear of a quad

  2/Condition number of Jacobian Skew matrix
*/
C_FUNC_DEF VERDICT_REAL v_quad_shear( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector face_normal = quad_normal( coordinates );
  face_normal.normalize();
  
  VERDICT_REAL min_shear = 1.0;
  
  VERDICT_REAL shear;
  VerdictVector xxi, xet;
  
  VERDICT_REAL deta;
  VERDICT_REAL g11, g22;
  
  for ( int i=0; i<4; i++ ) 
  {
    
    xxi.set( coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    xet.set( coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    g11 = xxi % xxi;
    g22 = xet % xet;
    deta = face_normal % ( xxi * xet );
    
    VERDICT_REAL den = sqrt( g11 * g22 );
    if ( den > VERDICT_DBL_MIN ) 
    {
      shear = deta / den;
      min_shear = VERDICT_MIN(min_shear, shear);
    }
    else 
      min_shear = 0;
    
  }
  
  return VERDICT_MAX(min_shear,0);
}

/*!
  the shape of a quad

   2/Condition number of weighted Jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_quad_shape( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector face_normal = quad_normal( coordinates );
  face_normal.normalize();
  
  VERDICT_REAL min_shape = 1.0;
  
  VERDICT_REAL shape;
  VerdictVector xxi, xet;
  
  VERDICT_REAL deta;
  VERDICT_REAL g11, g22;
  
  for ( int i=0; i<4; i++ ) 
  {
    
    xxi.set( coordinates[i][0] - coordinates[(i+1)%4][0],
        coordinates[i][1] - coordinates[(i+1)%4][1],
        coordinates[i][2] - coordinates[(i+1)%4][2] );
    
    xet.set( coordinates[i][0] - coordinates[(i+3)%4][0],
        coordinates[i][1] - coordinates[(i+3)%4][1],
        coordinates[i][2] - coordinates[(i+3)%4][2] );
    
    g11 = xxi % xxi;
    g22 = xet % xet;
    deta = face_normal % ( xxi * xet );
    
    VERDICT_REAL den = g11 + g22;
    if ( den >VERDICT_DBL_MIN ) {
      shape = 2 * deta / den;
      min_shape = VERDICT_MIN(min_shape, shape);
    }
    else 
      min_shape = 0;
    
  }
  
  return VERDICT_MAX(min_shape,0);

}

/*!
  the relative size of a quad

  Min( J, 1/J ), where J is determinant of weighted Jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_quad_relative_size( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector face_normal = quad_normal( coordinates );
  face_normal.normalize();
  
  VERDICT_REAL rel_size = 0;
  
  VERDICT_REAL w11,w21,w12,w22;
  VerdictVector xxi, xet;
  
  get_weight(w11,w21,w12,w22);
  VERDICT_REAL detw = determinant(w11,w21,w12,w22);
  
  if ( detw > VERDICT_DBL_MIN ) 
  {
    
    VERDICT_REAL deta0, deta2;
    
    xxi.set( coordinates[1][0] - coordinates[0][0],
        coordinates[1][1] - coordinates[0][1],
        coordinates[1][2] - coordinates[0][2] );
    
    xet.set( coordinates[3][0] - coordinates[0][0],
        coordinates[3][1] - coordinates[0][1],
        coordinates[3][2] - coordinates[0][2] );
    
    deta0 = face_normal % ( xxi * xet );
    
    xxi.set( coordinates[2][0] - coordinates[3][0],
        coordinates[2][1] - coordinates[3][1],
        coordinates[2][2] - coordinates[3][2] );
    
    xet.set( coordinates[2][0] - coordinates[1][0],
        coordinates[2][1] - coordinates[1][1],
        coordinates[2][2] - coordinates[1][2] );
    
    deta2 = face_normal % ( xxi * xet );
    
    VERDICT_REAL tau = 0.5*( deta0 + deta2 ) / detw;
    
    if ( tau > VERDICT_DBL_MIN )
      rel_size = VERDICT_MIN( tau*tau, 1/tau/tau );
    
  }
  
  return rel_size;
}

/*!
  the relative shape and size of a quad

  Product of Shape and Relative Size
*/
C_FUNC_DEF VERDICT_REAL v_quad_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL shape, size;
  size = v_quad_relative_size( num_nodes, coordinates );
  shape = v_quad_shape( num_nodes, coordinates );

  return shape * size;
}

/*!
  the shear and size of a quad

  product of shear and relative size
*/
C_FUNC_DEF VERDICT_REAL v_quad_shear_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL shear, size;
  shear = v_quad_shear( num_nodes, coordinates );
  size = v_quad_relative_size( num_nodes, coordinates );

  return shear * size;
}

/*!
  the distortion of a quad
*/
C_FUNC_DEF VERDICT_REAL v_quad_distortion( int num_nodes, VERDICT_REAL coordinates[][3] )
{
// To calculate distortion for linear and 2nd order quads
   // distortion = {min(|J|)/actual area}*{parent area}
   // parent area = 4 for a quad.
   // min |J| is the minimum over nodes and gaussian integration points
   // created by Ling Pan, CAT on 4/30/01

   VERDICT_REAL element_area =0.0,distrt,thickness_gauss;
   VERDICT_REAL cur_jacobian=0., sign_jacobian, jacobian ;
   VerdictVector  aa, bb, cc,normal_at_point, xin;


   //use 2x2 gauss points for linear quads and 3x3 for 2nd order quads
   int   number_of_gauss_points=0;
   if (num_nodes ==4)
      //2x2 quadrature rule
      number_of_gauss_points = 2;
   else if (num_nodes ==8)
      //3x3 quadrature rule
      number_of_gauss_points = 3;


   int total_number_of_gauss_points = number_of_gauss_points*number_of_gauss_points;

   VerdictVector face_normal = quad_normal( coordinates );

   VERDICT_REAL distortion = VERDICT_DBL_MAX;

   VerdictVector first, second;

   int i;
    //Will work out the case for collapsed quad later
   if ( is_collapsed_quad( coordinates ) == VERDICT_TRUE )
   {
      for (  i=0; i<3; i++ )
      {
 
         first.set( coordinates[i][0] - coordinates[(i+1)%3][0],
        coordinates[i][1] - coordinates[(i+1)%3][1],
        coordinates[i][2] - coordinates[(i+1)%3][2] );
 
         second.set( coordinates[i][0] - coordinates[(i+2)%3][0],
         coordinates[i][1] - coordinates[(i+2)%3][1],
         coordinates[i][2] - coordinates[(i+2)%3][2] );

         sign_jacobian = (face_normal % ( first * second )) > 0? 1.:-1.;
         cur_jacobian = sign_jacobian*(first * second).length();
         distortion = VERDICT_MIN(distortion, cur_jacobian);
      }
      element_area = (first*second).length()/2.0;
      distortion /= element_area;
   }
else
   {
      VERDICT_REAL shape_function[maxTotalNumberGaussPoints][maxNumberNodes];
      VERDICT_REAL dndy1[maxTotalNumberGaussPoints][maxNumberNodes];
      VERDICT_REAL dndy2[maxTotalNumberGaussPoints][maxNumberNodes];
      VERDICT_REAL weight[maxTotalNumberGaussPoints];

      //create an object of GaussIntegration
      GaussIntegration gauss_integration(number_of_gauss_points,num_nodes );
      gauss_integration.calculate_shape_function_2d_quad();
      gauss_integration.get_shape_func(shape_function[0], dndy1[0], dndy2[0], weight);

      // calculate element area
      int ife,ja;
      for ( ife=0;ife<total_number_of_gauss_points; ife++)
      {
         aa.set(0.0,0.0,0.0);
         bb.set(0.0,0.0,0.0);

         for (ja=0;ja<num_nodes;ja++)
         {
            xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
            aa += dndy1[ife][ja]*xin;
            bb += dndy2[ife][ja]*xin;
         }
         normal_at_point = aa*bb;
         VERDICT_REAL jacobian = normal_at_point.length();
         element_area += weight[ife]*jacobian;
      }


      VERDICT_REAL dndy1_at_node[maxNumberNodes][maxNumberNodes];
      VERDICT_REAL dndy2_at_node[maxNumberNodes][maxNumberNodes];

      gauss_integration.calculate_derivative_at_nodes( dndy1_at_node,  dndy2_at_node);

      VerdictVector *normal_at_nodes = new VerdictVector[num_nodes];


      //evaluate normal at nodes and distortion values at nodes
      int jai;
      for (ja =0; ja<num_nodes; ja++)
      {
         aa.set(0.0,0.0,0.0);
         bb.set(0.0,0.0,0.0);
         for (jai =0; jai<num_nodes; jai++)
         {
            xin.set(coordinates[jai][0], coordinates[jai][1], coordinates[jai][2]);
            aa += dndy1_at_node[ja][jai]*xin;
            bb += dndy2_at_node[ja][jai]*xin;
         }
         normal_at_nodes[ja] = aa*bb;
         normal_at_nodes[ja].normalize();

      }

      //determine if element is flat
      bool flat_element =true;
      VERDICT_REAL dot_product;

      for ( ja=0; ja<num_nodes;ja++)
      {
         dot_product = normal_at_nodes[0]%normal_at_nodes[ja];
         if (fabs(dot_product) <0.99)
         {
     flat_element = false;
            break;
         }
      }

      // take into consideration of the thickness of the element
      VERDICT_REAL thickness;
      //get_quad_thickness(face, element_area, thickness );
      thickness = 0.001*sqrt(element_area);

      //set thickness gauss point location
      VERDICT_REAL zl = 0.5773502691896;
      if (flat_element) zl =0.0;

      int no_gauss_pts_z = (flat_element)? 1 : 2;
      VERDICT_REAL thickness_z;
      int igz;
      //loop on Gauss points
      for (ife=0;ife<total_number_of_gauss_points;ife++)
      {
         //loop on the thickness direction gauss points
         for ( igz=0;igz<no_gauss_pts_z;igz++)
         {
            zl = -zl;
            thickness_z = zl*thickness/2.0;

            aa.set(0.0,0.0,0.0);
            bb.set(0.0,0.0,0.0);
            cc.set(0.0,0.0,0.0);

            for (ja=0;ja<num_nodes;ja++)
            {
               xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
               xin += thickness_z*normal_at_nodes[ja];
               aa  += dndy1[ife][ja]*xin;
               bb  += dndy2[ife][ja]*xin;
               thickness_gauss = shape_function[ife][ja]*thickness/2.0;
               cc  += thickness_gauss*normal_at_nodes[ja];
            }

            normal_at_point = aa*bb;
            jacobian = normal_at_point.length();
            distrt = cc%normal_at_point;
            if (distrt < distortion) distortion = distrt;
         }
      }

      //loop through nodal points
      for ( ja =0; ja<num_nodes; ja++)
      {
         for ( igz=0;igz<no_gauss_pts_z;igz++)
         {
            zl = -zl;
            thickness_z = zl*thickness/2.0;

            aa.set(0.0,0.0,0.0);
            bb.set(0.0,0.0,0.0);
            cc.set(0.0,0.0,0.0);

            for ( jai =0; jai<num_nodes; jai++)
            {
               xin.set(coordinates[jai][0], coordinates[jai][1], coordinates[jai][2]);
               xin += thickness_z*normal_at_nodes[ja];
               aa += dndy1_at_node[ja][jai]*xin;
               bb += dndy2_at_node[ja][jai]*xin;
               if (jai == ja)
     thickness_gauss = thickness/2.0;
               else
                  thickness_gauss = 0.;
               cc  += thickness_gauss*normal_at_nodes[jai];
            }

         }
         normal_at_point = aa*bb;
         VERDICT_REAL sign_jacobian = (face_normal % normal_at_point) > 0? 1.:-1.;
         distrt = sign_jacobian  * (cc%normal_at_point);

         if (distrt < distortion) distortion = distrt;
      }

      if (element_area*thickness !=0)
         distortion *=8./( element_area*thickness);
      else
         distortion *=8.;

      delete [] normal_at_nodes;
   }

   return distortion;
}

/*!
  multiple quality measures of a quad
*/
C_FUNC_DEF void v_quad_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, QuadMetricVals *metric_vals )
{

  memset( metric_vals, 0, sizeof(QuadMetricVals) );

  // for starts, lets set up some basic and common information

  /*  node numbers and side numbers used below

                  2
            3 +--------- 2
             /         +
            /          |
         3 /           | 1
          /            |
         +             |
       0 -------------+ 1
             0
  */
  
  // vectors for each side
  VerdictVector edges[4];
  edges[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  edges[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  edges[2].set(
      coordinates[3][0] - coordinates[2][0],
      coordinates[3][1] - coordinates[2][1],
      coordinates[3][2] - coordinates[2][2]
      );
  edges[3].set(
      coordinates[0][0] - coordinates[3][0],
      coordinates[0][1] - coordinates[3][1],
      coordinates[0][2] - coordinates[3][2]
      );

  // two normal vectors used by some metrics
  VerdictVector normal = quad_normal(coordinates);
  VerdictVector normal_ized = normal;
  normal_ized.normalize();

  VerdictBoolean is_collapsed = is_collapsed_quad(coordinates);

  // handle collapsed quads metrics here
  if(is_collapsed == VERDICT_TRUE && metrics_request_flag & 
      ( V_QUAD_SMALLEST_ANGLE | V_QUAD_LARGEST_ANGLE | V_QUAD_JACOBIAN |
        V_QUAD_SCALED_JACOBIAN ))
  {
    if(metrics_request_flag & V_QUAD_SMALLEST_ANGLE)
      metric_vals->smallest_angle = v_tri_smallest_angle(3, coordinates);
    if(metrics_request_flag & V_QUAD_LARGEST_ANGLE)
      metric_vals->largest_angle = v_tri_largest_angle(3, coordinates);
    if(metrics_request_flag & V_QUAD_JACOBIAN)
      metric_vals->jacobian = v_tri_area(3, coordinates) * 2.0;
    if(metrics_request_flag & V_QUAD_SCALED_JACOBIAN)
      metric_vals->jacobian = v_tri_scaled_jacobian(3, coordinates) * 2.0;
  }
  
  // calculate both largest and smallest angles
  if(metrics_request_flag & (V_QUAD_SMALLEST_ANGLE | V_QUAD_LARGEST_ANGLE) 
      && is_collapsed == VERDICT_FALSE )
  {
    // gather the angles
    VERDICT_REAL angles[4];
    angles[0] = normal.vector_angle(edges[0], -edges[3]);
    angles[1] = normal.vector_angle(edges[1], -edges[0]);
    angles[2] = normal.vector_angle(edges[2], -edges[1]);
    angles[3] = normal.vector_angle(edges[3], -edges[2]);

    // if smallest angle, find the smallest angle
    if(metrics_request_flag & V_QUAD_SMALLEST_ANGLE)
    {
      metric_vals->smallest_angle = 6.5;
      for(int i = 0; i<4; i++)
        metric_vals->smallest_angle = VERDICT_MIN(angles[i], metric_vals->smallest_angle);
      metric_vals->smallest_angle *= (180.0 / VERDICT_PI);
    }
    // if largest angle, find the largest angle
    if(metrics_request_flag & V_QUAD_LARGEST_ANGLE)
    {
      metric_vals->largest_angle = 0.0;
      for(int i = 0; i<4; i++)
        metric_vals->largest_angle = VERDICT_MAX(angles[i], metric_vals->largest_angle);
      metric_vals->largest_angle *= (180.0 / VERDICT_PI);
    }
  }

  // handle aspect, skew, taper, and area together
  if( metrics_request_flag & ( V_QUAD_ASPECT | V_QUAD_SKEW | V_QUAD_TAPER | V_QUAD_AREA ))
  {
    //localized coordinates are required by these metrics
    VerdictVector node_pos[4];
    for(int i = 0; i < 4; i++ )
      node_pos[i].set(coordinates[i][0], coordinates[i][1], coordinates[i][2]);
   
    localize_quad_coordinates(node_pos);
    localize_quad_for_ef(node_pos);
  
    // these ef's are used by aspect, skew, taper, and area
    VerdictVector ef2, ef3, ef4;
    ef2 = -node_pos[0] + node_pos[1] + node_pos[2] - node_pos[3];
    ef3 = -node_pos[0] - node_pos[1] + node_pos[2] + node_pos[3];
    ef4 =  node_pos[0] - node_pos[1] + node_pos[2] - node_pos[3];

    // calculate the aspect
    if(metrics_request_flag & (V_QUAD_ASPECT | V_QUAD_TAPER))
    {
      // handle another permutation of node order
      VerdictVector node_pos2[4];
      node_pos2[0] = node_pos[1];
      node_pos2[1] = node_pos[2];
      node_pos2[2] = node_pos[3];
      node_pos2[3] = node_pos[0];
      
      localize_quad_for_ef(node_pos2);
      
      VerdictVector ef2_a = -node_pos2[0] + node_pos2[1] + node_pos2[2] - node_pos2[3];
      VerdictVector ef3_a = -node_pos2[0] - node_pos2[1] + node_pos2[2] + node_pos2[3];
      VerdictVector ef4_a =  node_pos2[0] - node_pos2[1] + node_pos2[2] - node_pos2[3];

      if(metrics_request_flag & V_QUAD_ASPECT)
      {
        VERDICT_REAL aspect1 = ef2.x() / ef3.y();
        aspect1 = (aspect1 < 1.0 ? 1.0/ aspect1 : aspect1);
        
        VERDICT_REAL aspect2 = ef2_a.x() / ef3_a.y();
        aspect2 = (aspect2 < 1.0 ? 1.0/ aspect2 : aspect2);

        metric_vals->aspect = VERDICT_MAX(aspect1, aspect2);
      }
    
      // calculate the taper
      if(metrics_request_flag & V_QUAD_TAPER)
      {
        VERDICT_REAL taper1 = VERDICT_MAX( fabs(ef4.y() / ef3.y()) , fabs(ef4.x() / ef2.x()) );
        VERDICT_REAL taper2 = VERDICT_MAX( fabs(ef4_a.y() / ef3_a.y()) , fabs(ef4_a.x() / ef2_a.x()) );
        
        metric_vals->taper = VERDICT_MIN(taper1, taper2);
      }
      // calculate the area
      if(metrics_request_flag & V_QUAD_AREA)
      {
        VERDICT_REAL area1 = ef2.x() * ef3.y() / 4.0;
        VERDICT_REAL area2 = -ef3.x() * ef2.y() / 4.0;
        metric_vals->area = area1 + area2;
      }

    }
    // calculate the skew
    if(metrics_request_flag & V_QUAD_SKEW)
    {
      metric_vals->skew = fabs( ef2 % ef3)/ (ef2.length() * ef3.length() );
    }

  }


  // calcualte the jacobian and/or the scaled jacobian
  if(metrics_request_flag & (V_QUAD_JACOBIAN | V_QUAD_SCALED_JACOBIAN ))
  {
    VERDICT_REAL min_jac = VERDICT_DBL_MAX;
    VERDICT_REAL min_sc_jac = VERDICT_DBL_MAX;

    // go through each node
    for(int i=0; i<4; i++)
    {
      VerdictVector &first = edges[i];
      VerdictVector second = -edges[(i+3)%4];
      
      // calculate the jacobian at this node
      VERDICT_REAL jac = normal % ( first * second );
      min_jac = VERDICT_MIN(min_jac, jac);
    
      // calculate the scaled jacobian at this node 
      if(metrics_request_flag & V_QUAD_SCALED_JACOBIAN)
      {
        VERDICT_REAL rt_g11 = first.length();
        VERDICT_REAL rt_g22 = second.length();
        VERDICT_REAL sc_jac = jac / rt_g11 / rt_g22;
        min_sc_jac = VERDICT_MIN(min_sc_jac, sc_jac);
      }
    }
    // set the jacobian value
    if(metrics_request_flag & V_QUAD_SCALED_JACOBIAN)
      metric_vals->scaled_jacobian = min_sc_jac;
    // set the scaled jacobian value
    if(metrics_request_flag & V_QUAD_JACOBIAN)
      metric_vals->jacobian = min_jac;
  }

  // calculate shape, condition, shear and oddy
  // other metrics also depend on these results
  if(metrics_request_flag & (V_QUAD_SHAPE | V_QUAD_SHEAR | V_QUAD_SHEAR_AND_SIZE |
        V_QUAD_ODDY | V_QUAD_CONDITION | V_QUAD_SHAPE_AND_SIZE ) )
  {
    VERDICT_REAL min_shape = 1.0, max_oddy = 0.0, max_condition = 0.0, min_shear = 1.0;
    VERDICT_REAL deta=0, den, g11, g12, g22, shape, oddy, condition, shear;
    // go through each node on the quad
    for(int i=0; i<4; i++)
    {
      VerdictVector &xxi = edges[i];
      VerdictVector xet = -edges[(i+3)%4];
      g11 = xxi % xxi;
      g22 = xet % xet;
      // calculate deta if needed
      if(metrics_request_flag & (V_QUAD_SHAPE | V_QUAD_SHAPE_AND_SIZE | 
            V_QUAD_CONDITION | V_QUAD_SHEAR |V_QUAD_SHEAR_AND_SIZE ))
        deta = normal_ized % ( xxi * xet );

      // calculate the shear
      if(metrics_request_flag & (V_QUAD_SHEAR | V_QUAD_SHEAR_AND_SIZE ))
      {
        VERDICT_REAL den = sqrt(g11*g22);
        if ( den > VERDICT_DBL_MIN ) 
        {
          shear = deta / den;
          min_shear = VERDICT_MIN(min_shear, shear);
        }
        else 
          min_shear = 0;
      }

      // calculate the condition number
      if(metrics_request_flag & V_QUAD_CONDITION)
      {
        if(deta < VERDICT_DBL_MIN)
          condition = 1.e+06;
        else
          condition = ( g11 + g22 ) / deta;
        max_condition = VERDICT_MAX(max_condition, condition);
      }

      // calculate the shape
      if(metrics_request_flag & (V_QUAD_SHAPE | V_QUAD_SHAPE_AND_SIZE))
      {
        den = ( xxi % xxi ) + ( xet % xet );
        if(den > VERDICT_DBL_MIN )
        {
          shape = 2 * deta / den;
          min_shape = VERDICT_MIN ( min_shape, shape );
        }
        else
          min_shape = 0;
      }

      // calculate the oddy
      if(metrics_request_flag & V_QUAD_ODDY)
      {
        g12 = xxi % xet;
        VERDICT_REAL g = g11*g22 - g12*g12;
        if ( g != 0. )
          oddy = ( (g11-g22)*(g11-g22) + 4.*g12*g12 ) / 2. / g;
        else
          oddy = 1.e+06;
        
        max_oddy = VERDICT_MAX(max_oddy, oddy);
      }
    }
    
    // set the value of shape
    if(metrics_request_flag & (V_QUAD_SHAPE | V_QUAD_SHAPE_AND_SIZE))
      metric_vals->shape = min_shape;
    // set the value of oddy
    if(metrics_request_flag & V_QUAD_ODDY)
      metric_vals->oddy = max_oddy;
    // set the value of condition
    if(metrics_request_flag & V_QUAD_CONDITION)
      metric_vals->condition = max_condition / 2;
    // set the value of shear
    if(metrics_request_flag & (V_QUAD_SHEAR | V_QUAD_SHEAR_AND_SIZE ))
      metric_vals->shear = VERDICT_MAX(min_shear, 0);
  }
  
  //calculate the stretch of the quad
  if(metrics_request_flag & V_QUAD_STRETCH)
  {
    static const VERDICT_REAL root_of_2 = sqrt(2.0);
    // take the lengths squared of each side and the
    // diagonals
    VERDICT_REAL lengths[6] =
    {
      edges[0].length_squared(),
      edges[1].length_squared(),
      edges[2].length_squared(),
      edges[3].length_squared(),
      (edges[0] + edges[1]).length_squared(),
      (edges[1] + edges[2]).length_squared()
    };
    VERDICT_REAL min_edge = 
      VERDICT_MIN(VERDICT_MIN(VERDICT_MIN(lengths[0],lengths[1]),lengths[2]),lengths[3]);
    VERDICT_REAL max_diag = VERDICT_MAX(lengths[4], lengths[5]);
    metric_vals->stretch = sqrt( min_edge / max_diag ) * root_of_2;
  }

  // calculate the warpage
  if(metrics_request_flag & V_QUAD_WARPAGE)
  {
    VERDICT_REAL warpage=0, min_mag = VERDICT_DBL_MAX, mag;
    VerdictVector unit_normal[4];

    for(int i=0; i<4; i++)
    {
      VerdictVector &xxi = edges[i];
      VerdictVector xet = -edges[(i+3)%4];
      unit_normal[i] = xxi * xet;
      mag = unit_normal[i].length();
      min_mag = VERDICT_MIN(min_mag, mag);
      if (mag > VERDICT_DBL_MIN )
        unit_normal[i] /= mag;
    }
    if(min_mag > VERDICT_DBL_MIN)
    {
      VERDICT_REAL dot1 = unit_normal[0] % unit_normal[2];
      VERDICT_REAL dot2 = unit_normal[1] % unit_normal[3];
      
      if ( (dot1>VERDICT_DBL_MIN) && (dot1>VERDICT_DBL_MIN) ) 
        warpage = pow( VERDICT_MIN( dot1, dot2 ), 4 );
    }
    metric_vals->warpage = warpage;
  }
  
  // calculate the relative size
  // some other metrics depend on this one
  if(metrics_request_flag & (V_QUAD_RELATIVE_SIZE | V_QUAD_SHAPE_AND_SIZE | 
        V_QUAD_SHEAR_AND_SIZE ))
  {
    VERDICT_REAL rel_size = 0;
    VERDICT_REAL w11,w21,w12,w22;
    get_weight(w11,w21,w12,w22);
    VERDICT_REAL detw = determinant(w11,w21,w12,w22);
    
    if ( detw > VERDICT_DBL_MIN ) 
    {
      VERDICT_REAL deta0, deta2;
      deta0 = normal_ized % ( edges[0] * -edges[3] );
      deta2 = normal_ized % ( edges[2] * -edges[1] );
      
      VERDICT_REAL tau = 0.5*( deta0 + deta2 ) / detw;
      
     if ( tau > VERDICT_DBL_MIN )
       rel_size = VERDICT_MIN( tau*tau, 1/tau/tau );
    }
    metric_vals->relative_size = rel_size;
  }

  // calculate shape and size
  // reuse values from above
  if(metrics_request_flag & V_QUAD_SHAPE_AND_SIZE)
  {
    metric_vals->shape_and_size = metric_vals->shape * metric_vals->relative_size;
  }

  // calculate shear and size
  // reuse values from above
  if(metrics_request_flag & V_QUAD_SHEAR_AND_SIZE)
  {
    metric_vals->shear_and_size = metric_vals->shear * metric_vals->relative_size;
  }

  // calculate distortion
  if(metrics_request_flag & V_QUAD_DISTORTION)
    metric_vals->distortion = v_quad_distortion(num_nodes, coordinates);

}

 
/*
 
TetMetric.cpp contains quality calculations for Tets
 
*/
     
//! the average volume of a tet
VERDICT_REAL verdict_tet_size = 0;

/*! 
  set the average volume of a tet
*/
C_FUNC_DEF void v_set_tet_size( VERDICT_REAL size )
{
  verdict_tet_size = size;
}

/*!
  get the weights based on the average size
  of a tet
*/
int get_weight ( VerdictVector &w1,
                 VerdictVector &w2,
                 VerdictVector &w3 )
{
  static const VERDICT_REAL rt3 = sqrt(3.0);
  static const VERDICT_REAL root_of_2 = sqrt(2.0);
  
  w1.set(1,0,0);
  w2.set(0.5, 0.5*rt3, 0 );
  w3.set(0.5, rt3/6.0, root_of_2/rt3); 

  VERDICT_REAL scale = pow( 6.*verdict_tet_size/determinant(w1,w2,w3),0.3333333333333);   

  w1 *= scale;
  w2 *= scale;
  w3 *= scale;
  
  return 1;
}

/*!
  the scaled jacobian of a tet

  minimum of the jacobian divided by the lengths of 3 edge vectors

*/
C_FUNC_DEF VERDICT_REAL v_tet_scaled_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VerdictVector side0, side1, side2, side3, side4, side5;

  side0.set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2] );
  
  side1.set( coordinates[2][0] - coordinates[1][0],
       coordinates[2][1] - coordinates[1][1],
       coordinates[2][2] - coordinates[1][2] );
  
  side2.set( coordinates[0][0] - coordinates[2][0],
       coordinates[0][1] - coordinates[2][1],
       coordinates[0][2] - coordinates[2][2] );

  side3.set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2] );
  
  side4.set( coordinates[3][0] - coordinates[1][0],
       coordinates[3][1] - coordinates[1][1],
       coordinates[3][2] - coordinates[1][2] );
  
  side5.set( coordinates[3][0] - coordinates[2][0],
       coordinates[3][1] - coordinates[2][1],
       coordinates[3][2] - coordinates[2][2] );
  
  VERDICT_REAL jacobi;

  jacobi = side3 % ( side0 * -side2 );

  // products of lengths squared of each edge attached to a node.
  VERDICT_REAL length_squared[4] = {
    side0.length_squared() * side2.length_squared() * side3.length_squared(),
    side0.length_squared() * side1.length_squared() * side4.length_squared(),
    side1.length_squared() * side2.length_squared() * side5.length_squared(),
    side3.length_squared() * side4.length_squared() * side5.length_squared()
  };
  int which_node = 0;
  if(length_squared[1] > length_squared[which_node])
    which_node = 1;
  if(length_squared[2] > length_squared[which_node])
    which_node = 2;
  if(length_squared[3] > length_squared[which_node])
    which_node = 3;
  
  VERDICT_REAL length_product = sqrt( length_squared[which_node] );
  if(length_product < fabs(jacobi))
    length_product = fabs(jacobi);

  static const VERDICT_REAL root_of_2 = sqrt(2.0);

  return root_of_2 * jacobi / length_product;

}

/*!
  the aspect of a tet

  CR / (3.0*IR) where CR is the circumsphere radius and IR is the inscribed sphere radius
*/
C_FUNC_DEF VERDICT_REAL v_tet_aspect( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  //Determine side vectors
  VerdictVector side[6];

  side[0].set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2] );
  
  side[1].set( coordinates[2][0] - coordinates[1][0],
       coordinates[2][1] - coordinates[1][1],
       coordinates[2][2] - coordinates[1][2] );
  
  side[2].set( coordinates[0][0] - coordinates[2][0],
       coordinates[0][1] - coordinates[2][1],
       coordinates[0][2] - coordinates[2][2] );

  side[3].set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2] );
  
  side[4].set( coordinates[3][0] - coordinates[1][0],
       coordinates[3][1] - coordinates[1][1],
       coordinates[3][2] - coordinates[1][2] );
  
  side[5].set( coordinates[3][0] - coordinates[2][0],
       coordinates[3][1] - coordinates[2][1],
       coordinates[3][2] - coordinates[2][2] );

  
  

  VERDICT_REAL area_sum = 0.0;
  
  area_sum = ((side[0] * -side[2]).length() + (side[3] * side[0]).length() +
       (side[4] * side[1]).length() + (side[3] * -side[2]).length() ) * 0.5;
  
  VERDICT_REAL volume = v_tet_volume(4, coordinates);
  
  VERDICT_REAL inscribed_rad = 1.0;
  VERDICT_REAL circum_rad = 0.0;

  if( volume != 0.0 && area_sum != 0.0 )
  {
    
    inscribed_rad = 3.0 * volume / area_sum;
    VerdictVector top = side[3].length_squared() * ( side[0] * -side[2]) +
      side[2].length_squared() * (side[3] * side[0]) +
      side[0].length_squared() * (-side[2] * side[3]);
    
    
    top = top / (2.0 * volume * 6.0);
    circum_rad = top.length();
    
    VERDICT_REAL aspect_ratio;
    
    if (circum_rad == 0.0) 
      aspect_ratio = pow(VERDICT_DBL_MAX,0.333);
    else
      aspect_ratio = circum_rad / (3.0 * inscribed_rad);
    
    return aspect_ratio;
  }
  else 
    return pow(VERDICT_DBL_MAX, 0.333);

}

/*!
  the aspect gamma of a tet

  srms^3 / (8.479670*V) where srms = sqrt(sum(Si^2)/6), where Si is the edge length
*/
C_FUNC_DEF VERDICT_REAL v_tet_aspect_gamma( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  //Determine side vectors
  VerdictVector side0, side1, side2, side3, side4, side5;

  side0.set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2] );
  
  side1.set( coordinates[2][0] - coordinates[1][0],
       coordinates[2][1] - coordinates[1][1],
       coordinates[2][2] - coordinates[1][2] );
  
  side2.set( coordinates[0][0] - coordinates[2][0],
       coordinates[0][1] - coordinates[2][1],
       coordinates[0][2] - coordinates[2][2] );

  side3.set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2] );
  
  side4.set( coordinates[3][0] - coordinates[1][0],
       coordinates[3][1] - coordinates[1][1],
       coordinates[3][2] - coordinates[1][2] );
  
  side5.set( coordinates[3][0] - coordinates[2][0],
       coordinates[3][1] - coordinates[2][1],
       coordinates[3][2] - coordinates[2][2] );
  

  VERDICT_REAL volume = v_tet_volume(4, coordinates);

  if( volume != 0.0)
  {
    VERDICT_REAL srms = sqrt((side0.length_squared() + side1.length_squared() +
                              side2.length_squared() + side3.length_squared() +
                              side4.length_squared() + side5.length_squared()) / 6.0 );

     VERDICT_REAL aspect_ratio_gamma = pow(srms, 3) / (8.47967 * fabs(volume) );  
    return aspect_ratio_gamma;
  }
  else { return pow(VERDICT_DBL_MAX, 0.333); }

}

/*!
  the volume of a tet

  1/6 * jacobian at a corner node
*/
C_FUNC_DEF VERDICT_REAL v_tet_volume( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  //Determine side vectors
  VerdictVector side0, side2, side3;

  side0.set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2] );
  
  side2.set( coordinates[2][0] - coordinates[0][0],
       coordinates[2][1] - coordinates[0][1],
       coordinates[2][2] - coordinates[0][2] );
  
  side3.set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2] );

  return  (side3 % (side0 * side2)) / 6.0;

}

/*!
  the condition of a tet

  condition number of the jacobian matrix at any corner
*/
C_FUNC_DEF VERDICT_REAL v_tet_condition( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  VERDICT_REAL condition, term1, term2, det;
  VERDICT_REAL max_condition = -VERDICT_DBL_MAX;
  VERDICT_REAL rt3 = sqrt(3.0);
  VERDICT_REAL rt6 = sqrt(6.0);

  VerdictVector side1, side2, side3;
  
  side1.set(coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2]);

  side2.set(coordinates[0][0] - coordinates[2][0],
       coordinates[0][1] - coordinates[2][1],
       coordinates[0][2] - coordinates[2][2]);


  side3.set(coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2]);

  VerdictVector xxi, xet, xze;

  xxi = side1;
  xet = (-2*side2-side1)/rt3;
  xze = (3*side3+side2-side1)/rt6;


  term1 = xxi % xxi + xet % xet + xze % xze;
  term2 = ( xxi * xet ) % ( xxi * xet ) + ( xet * xze ) % ( xet * xze ) + ( xze * xxi ) % ( xze * xxi );
  det = xxi % ( xet * xze );
  
  if ( det <= VERDICT_DBL_MIN )
    return 1.e+06;
  
  else 
    condition = sqrt( term1 * term2 ) / det;
  
  max_condition = VERDICT_MAX( condition, max_condition );
  
  return max_condition/3.0;
}


/*!
  the jacobian of a tet

  TODO
*/
C_FUNC_DEF VERDICT_REAL v_tet_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector side0, side2, side3;

  side0.set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
       coordinates[1][2] - coordinates[0][2] );
  
  side2.set( coordinates[2][0] - coordinates[0][0],
       coordinates[2][1] - coordinates[0][1],
       coordinates[2][2] - coordinates[0][2] );

  side3.set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
       coordinates[3][2] - coordinates[0][2] );
  

  return side0 % (side2 * side3);

}


/*!
  the shear of a tet

  Condition number of weighted skew matrices
*/
C_FUNC_DEF VERDICT_REAL v_tet_shear( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  //Determine side vectors
  VerdictVector side0, side1, side2, side3, side4, side5;

  side0.set( coordinates[1][0] - coordinates[0][0],
       coordinates[1][1] - coordinates[0][1],
         coordinates[1][2] - coordinates[0][2] );

  side1.set( coordinates[2][0] - coordinates[1][0],
       coordinates[2][1] - coordinates[1][1],
         coordinates[2][2] - coordinates[1][2] );

  side2.set( coordinates[0][0] - coordinates[2][0],
       coordinates[0][1] - coordinates[2][1],
         coordinates[0][2] - coordinates[2][2] );

  side3.set( coordinates[3][0] - coordinates[0][0],
       coordinates[3][1] - coordinates[0][1],
         coordinates[3][2] - coordinates[0][2] );

  side4.set( coordinates[3][0] - coordinates[1][0],
       coordinates[3][1] - coordinates[1][1],
       coordinates[3][2] - coordinates[1][2] );
  
  side5.set( coordinates[3][0] - coordinates[2][0],
       coordinates[3][1] - coordinates[2][1],
       coordinates[3][2] - coordinates[2][2] );

  
  VERDICT_REAL min_skew = VERDICT_DBL_MAX;
  VERDICT_REAL skew;
  VerdictVector w1, w2, w3; 

  get_weight(w1, w2, w3);
  
  VERDICT_REAL detw = determinant(w1, w2, w3);
  
  if( detw > VERDICT_DBL_MIN && determinant(side0, -side2, side3) > VERDICT_DBL_MIN)
  {
    VerdictVector q1, q2, q3;
          VerdictVector qw1, qw2, qw3;
    form_Q(side0,-side2,side3,q1,q2,q3);
    form_Q(w1,w2,w3,qw1,qw2,qw3);

     skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
    min_skew = VERDICT_MIN(skew, min_skew);    

    form_Q(side1,-side0,side4,q1,q2,q3);
    form_Q(w2-w1,-w1,w3-w1,qw1,qw2,qw3);
          skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
          min_skew = VERDICT_MIN( skew, min_skew );

    form_Q(side2,-side1,side5,q1,q2,q3);
          form_Q(-w2,w1-w2,w3-w2,qw1,qw2,qw3);
          skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
          min_skew = VERDICT_MIN( skew, min_skew );

          form_Q(side3,side4,side5,q1,q2,q3);
          form_Q(w3,w1-w3,w2-w1,qw1,qw2,qw3);
          skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
          min_skew = VERDICT_MIN( skew, min_skew );

  }
  else { return 0; }
  
  return min_skew;

}


/*!
  the shape of a tet

  3/ condition number of weighted jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_tet_shape( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

   VERDICT_REAL shape=0;
   static const VERDICT_REAL two_thirds = 2.0/3.0;
   static const VERDICT_REAL root_of_2 = sqrt(2.0);

   VerdictVector xxi, xet, xze;


  xxi.set(coordinates[1][0] - coordinates[0][0],
    coordinates[1][1] - coordinates[0][1],
    coordinates[1][2] - coordinates[0][2]);

  xet.set(coordinates[2][0] - coordinates[0][0],
    coordinates[2][1] - coordinates[0][1],
    coordinates[2][2] - coordinates[0][2]);


  xze.set(coordinates[3][0] - coordinates[0][0],
    coordinates[3][1] - coordinates[0][1],
    coordinates[3][2] - coordinates[0][2]);


   VERDICT_REAL alp = xxi % (xet *xze);

   if ( alp > VERDICT_DBL_MIN ) {

     VERDICT_REAL num = 3 * pow( root_of_2 * alp, two_thirds );
     VERDICT_REAL den = 1.5*(xxi%xxi+xet%xet+xze%xze)-(xxi%xet+xet%xze+xze%xxi);

     if ( den > VERDICT_DBL_MIN ) {
       shape = num / den;
     }
   }
   return shape;

}



/*!
  the relative size of a tet

  Min(J,1/J), where J is the determinant of the weighted Jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_tet_relative_size( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{


   VERDICT_REAL size=0;
   VerdictVector w1, w2, w3;

   get_weight(w1,w2,w3);


   VERDICT_REAL detw = w1 % (w2 *w3);

  VerdictVector xxi, xet, xze;
  
  xxi.set(coordinates[0][0] - coordinates[1][0],
    coordinates[0][1] - coordinates[1][1],
    coordinates[0][2] - coordinates[1][2]);

  xet.set(coordinates[0][0] - coordinates[2][0],
    coordinates[0][1] - coordinates[2][1],
    coordinates[0][2] - coordinates[2][2]);


  xze.set(coordinates[3][0] - coordinates[0][0],
    coordinates[3][1] - coordinates[0][1],
    coordinates[3][2] - coordinates[0][2]);


   VERDICT_REAL deta = xxi % ( xet * xze );

   if ( detw >VERDICT_DBL_MIN && deta > VERDICT_DBL_MIN ) {

      size = pow(deta/detw,2);
      if ( size > 1 ) { size = (VERDICT_REAL)(1)/size; }

   }
   return size;


}


/*!
  the shape and size of a tet

  Product of the shape and relative size
*/
C_FUNC_DEF VERDICT_REAL v_tet_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  
  VERDICT_REAL shape, size;
  shape = v_tet_shape( num_nodes, coordinates );
  size = v_tet_relative_size (num_nodes, coordinates );  
  
  return shape * size;

}



/*!
  the distortion of a tet
*/
C_FUNC_DEF VERDICT_REAL v_tet_distortion( int num_nodes, VERDICT_REAL coordinates[][3] )
{

   VERDICT_REAL distortion = VERDICT_DBL_MAX;
   int   number_of_gauss_points=0;
   if (num_nodes ==4)
      // for linear tet, the distortion is always 1 because
      // straight edge tets are the target shape for tet
      return 1.0;

   else if (num_nodes ==10)
      //use four integration points for quadratic tet
      number_of_gauss_points = 4;

   int number_dims = 3;
   int total_number_of_gauss_points = number_of_gauss_points;
   // use is_tri=1 to indicate this is for tet in 3D
   int is_tri =1;

   VERDICT_REAL shape_function[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy1[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy2[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy3[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL weight[maxTotalNumberGaussPoints];

   // create an object of GaussIntegration for tet
   GaussIntegration gauss_integration(number_of_gauss_points,num_nodes, number_dims, is_tri);
   gauss_integration.calculate_shape_function_3d_tet();
   gauss_integration.get_shape_func(shape_function[0], dndy1[0], dndy2[0], dndy3[0],weight);

   // vector xxi is the derivative vector of coordinates w.r.t local xi coordinate in the
   // computation space
   // vector xet is the derivative vector of coordinates w.r.t local et coordinate in the
   // computation space
   // vector xze is the derivative vector of coordinates w.r.t local ze coordinate in the
   // computation space
   VerdictVector xxi, xet, xze, xin;

   VERDICT_REAL jacobian, minimum_jacobian;
   VERDICT_REAL element_volume =0.0;
   minimum_jacobian = VERDICT_DBL_MAX;

   // calculate element volume
   int ife, ja;
   for (ife=0;ife<total_number_of_gauss_points; ife++)
   {
      xxi.set(0.0,0.0,0.0);
      xet.set(0.0,0.0,0.0);
      xze.set(0.0,0.0,0.0);

      for (ja=0;ja<num_nodes;ja++)
      {
         xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
         xxi += dndy1[ife][ja]*xin;
         xet += dndy2[ife][ja]*xin;
         xze += dndy3[ife][ja]*xin;
      }

      // determinant
      jacobian = xxi % ( xet * xze );
      if (minimum_jacobian > jacobian)
         minimum_jacobian = jacobian;

      element_volume += weight[ife]*jacobian;
      }//element_volume is 6 times the actual volume

   // loop through all nodes
   VERDICT_REAL dndy1_at_node[maxNumberNodes][maxNumberNodes];
   VERDICT_REAL dndy2_at_node[maxNumberNodes][maxNumberNodes];
   VERDICT_REAL dndy3_at_node[maxNumberNodes][maxNumberNodes];


   gauss_integration.calculate_derivative_at_nodes_3d_tet( dndy1_at_node,
                                                           dndy2_at_node,
                                                           dndy3_at_node);
   int node_id;
   for (node_id=0;node_id<num_nodes; node_id++)
   {
      xxi.set(0.0,0.0,0.0);
      xet.set(0.0,0.0,0.0);
      xze.set(0.0,0.0,0.0);

      for (ja=0;ja<num_nodes;ja++)
      {
         xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
         xxi += dndy1_at_node[node_id][ja]*xin;
         xet += dndy2_at_node[node_id][ja]*xin;
         xze += dndy3_at_node[node_id][ja]*xin;
      }

      jacobian = xxi % ( xet * xze );
      if (minimum_jacobian > jacobian)
         minimum_jacobian = jacobian;

      }
   distortion = minimum_jacobian/element_volume;

   return distortion;
}


/*!
  the quality metrics of a tet
*/
C_FUNC_DEF void v_tet_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, TetMetricVals *metric_vals )
{

  memset( metric_vals, 0, sizeof(TetMetricVals) );

  /*
  
    node numbers and edge numbers below


    
             3 
             +            edge 0 is node 0 to 1
            +|+           edge 1 is node 1 to 2
          3/ | \5         edge 2 is node 0 to 2
          / 4|  \         edge 3 is node 0 to 3
        0 - -|- + 2       edge 4 is node 1 to 3
          \  |  +         edge 5 is node 2 to 3
          0\ | /1
            +|/           edge 2 is behind edge 4
             1 

             
  */

  // lets start with making the vectors
  VerdictVector edges[6];
  edges[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  edges[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  edges[5].set(
      coordinates[3][0] - coordinates[2][0],
      coordinates[3][1] - coordinates[2][1],
      coordinates[3][2] - coordinates[2][2]
      );

  edges[2] = edges[0] + edges[1];
  edges[3] = edges[5] + edges[2];
  edges[4] = edges[3] - edges[0];

  // common numbers
  static const VERDICT_REAL root_of_2 = sqrt(2.0);
 
  // calculate the jacobian 
  static const int do_jacobian = V_TET_JACOBIAN | V_TET_VOLUME | 
    V_TET_ASPECT | V_TET_ASPECT_GAMMA | V_TET_SHAPE | V_TET_RELATIVE_SIZE | 
    V_TET_SHAPE_AND_SIZE | V_TET_SCALED_JACOBIAN | V_TET_CONDITION;
  if(metrics_request_flag & do_jacobian )
  {
    metric_vals->jacobian = edges[3] % (edges[0] * edges[2]);
  }
 
  // calculate the volume 
  if(metrics_request_flag & V_TET_VOLUME)
  {
    metric_vals->volume = metric_vals->jacobian / 6.0;
  }
  
  // calculate aspect ratio
  if(metrics_request_flag & V_TET_ASPECT)
  {
    VERDICT_REAL surface_area = 
      ((edges[0] * edges[2]).length() + (edges[3] * edges[0]).length() +
       (edges[4] * edges[1]).length() + (edges[3] * edges[2]).length() ) * 0.5;
    VerdictVector top = 
      edges[3].length_squared() * ( edges[0] * edges[2]) +
      edges[2].length_squared() * (edges[3] * edges[0]) +
      edges[0].length_squared() * (edges[2] * edges[3]);

    VERDICT_REAL volume = metric_vals->jacobian / 6.0;

    top /= (12.0 * volume);
    VERDICT_REAL circum_rad = top.length();
    VERDICT_REAL inscribed_rad = 9.0 * volume / surface_area;

    metric_vals->aspect = circum_rad / inscribed_rad;
  }
   

  // calculate the aspect gamma 
  if(metrics_request_flag & V_TET_ASPECT_GAMMA)
  {
    
    // srms
    VERDICT_REAL srms = sqrt((
          edges[0].length_squared() + edges[1].length_squared() +
          edges[2].length_squared() + edges[3].length_squared() +
          edges[4].length_squared() + edges[5].length_squared()
          ) / 6.0 );

    // cube the srms
    srms *= (srms * srms);

    metric_vals->aspect_gamma = 6.0 * srms / (8.47967 * metric_vals->jacobian );

  }

  // calculate the shape of the tet
  if(metrics_request_flag & ( V_TET_SHAPE | V_TET_SHAPE_AND_SIZE ) )
  {

    static const VERDICT_REAL two_thirds = 2.0/3.0;
    VERDICT_REAL num = 3.0 * pow(root_of_2 * metric_vals->jacobian, two_thirds);
    VERDICT_REAL den = 1.5 *
      (edges[0] % edges[0] + edges[2] % edges[2] + edges[3] % edges[3]) -
      (edges[0] % edges[2] + edges[2] % edges[3] + edges[3] % edges[0]);

    metric_vals->shape = num/den;
  }
  
  // calculate the relative size of the tet
  if(metrics_request_flag & (V_TET_RELATIVE_SIZE | V_TET_SHAPE_AND_SIZE ))
  {
    VerdictVector w1, w2, w3;
    get_weight(w1,w2,w3);
    VERDICT_REAL detw = w1 % (w2 *w3);
    VERDICT_REAL tmp = metric_vals->jacobian / detw;
    tmp *= tmp;
    metric_vals->relative_size = VERDICT_MIN(tmp, 1/tmp);
  }
  
  // calculate the shape and size
  if(metrics_request_flag & V_TET_SHAPE_AND_SIZE)
  {
    metric_vals->shape_and_size = metric_vals->shape * metric_vals->relative_size;
  }
  
  // calculate the scaled jacobian
  if(metrics_request_flag & V_TET_SCALED_JACOBIAN)
  {

    //find out which node the normalized jacobian can be calculated at
    //and it will be the smaller than at other nodes
    VERDICT_REAL length_squared[4] = {
      edges[0].length_squared() * edges[2].length_squared() * edges[3].length_squared(),
      edges[0].length_squared() * edges[1].length_squared() * edges[4].length_squared(),
      edges[1].length_squared() * edges[2].length_squared() * edges[5].length_squared(),
      edges[3].length_squared() * edges[4].length_squared() * edges[5].length_squared()
    };
    
    int which_node = 0;
    if(length_squared[1] > length_squared[which_node])
      which_node = 1;
    if(length_squared[2] > length_squared[which_node])
      which_node = 2;
    if(length_squared[3] > length_squared[which_node])
      which_node = 3;

    // find the scaled jacobian at this node
    VERDICT_REAL length_product = sqrt( length_squared[which_node] );
    if(length_product < fabs(metric_vals->jacobian))
      length_product = fabs(metric_vals->jacobian);
    
    metric_vals->scaled_jacobian = root_of_2 * metric_vals->jacobian / length_product;
  }
  
  // calculate the condition number
  if(metrics_request_flag & V_TET_CONDITION)
  {
    static const VERDICT_REAL root_of_3 = sqrt(3.0);
    static const VERDICT_REAL root_of_6 = sqrt(6.0);

    VerdictVector xxi, xet, xze;
    xxi = edges[0];
    xet = (2*edges[2] - edges[0])/root_of_3;
    xze = (3*edges[3] - edges[2] - edges[0])/root_of_6;
    VERDICT_REAL term1 =  xxi % xxi + xet % xet + xze % xze;
    VERDICT_REAL term2 = 
      ( xxi * xet ) % ( xxi * xet ) + 
      ( xet * xze ) % ( xet * xze ) + 
      ( xze * xxi ) % ( xze * xxi );
    if(metric_vals->jacobian < VERDICT_DBL_MIN)
      metric_vals->condition = 1.e+06;
    else
      metric_vals->condition = sqrt(term1 * term2) / (3.0 * (xxi % ( xet * xze)));
  }
    
  // calculate the shear of the tet
  if(metrics_request_flag & V_TET_SHEAR)
  {
    
    VERDICT_REAL min_skew = VERDICT_DBL_MAX;
    VERDICT_REAL skew;
    VerdictVector w1, w2, w3; 
    get_weight(w1, w2, w3);

    VERDICT_REAL detw = determinant(w1,w2,w3);
    if( detw > VERDICT_DBL_MIN && determinant(edges[0], edges[2], edges[3]) > VERDICT_DBL_MIN)
    {
      VerdictVector q1, q2, q3;
      VerdictVector qw1, qw2, qw3;
      form_Q(edges[0],edges[2],edges[3],q1,q2,q3);
      form_Q(w1,w2,w3,qw1,qw2,qw3);
      
      skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
      min_skew = VERDICT_MIN(skew, min_skew);    
      
      form_Q(edges[1],-edges[0],edges[4],q1,q2,q3);
      form_Q(w2-w1,-w1,w3-w1,qw1,qw2,qw3);
      skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
      min_skew = VERDICT_MIN( skew, min_skew );
      
      form_Q(-edges[2],-edges[1],edges[5],q1,q2,q3);
      form_Q(-w2,w1-w2,w3-w2,qw1,qw2,qw3);
      skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
      min_skew = VERDICT_MIN( skew, min_skew );
      
      form_Q(edges[3],edges[4],edges[5],q1,q2,q3);
      form_Q(w3,w1-w3,w2-w1,qw1,qw2,qw3);
      skew = skew_x(q1,q2,q3,qw1,qw2,qw3);
      min_skew = VERDICT_MIN( skew, min_skew );
    }
    else
      min_skew = 0;

    metric_vals->shear = min_skew;
  }

  // calculate the distortion
  if(metrics_request_flag & V_TET_DISTORTION)
  {
    metric_vals->distortion = v_tet_distortion(num_nodes, coordinates);
  }
}



/*
 
TriMetric.cpp contains quality calculations for Tris
 
*/
   
// the average area of a tri
VERDICT_REAL verdict_tri_size = 0;

/*! 
  get weights based on the average area of a set of
  tris
*/
int v_tri_get_weight ( VERDICT_REAL &m11,
    VERDICT_REAL &m21,
    VERDICT_REAL &m12,
    VERDICT_REAL &m22 )
{
  static const VERDICT_REAL rootOf3 = sqrt(3.0);
  m11=1;
  m21=0;
  m12=0.5;
  m22=0.5*rootOf3;
  VERDICT_REAL scale = sqrt(2.0*verdict_tri_size/(m11*m22-m21*m12));
  
  m11 *= scale;
  m21 *= scale;
  m12 *= scale;
  m22 *= scale;
  
  return 1;
}


/*! sets the average area of a tri */
C_FUNC_DEF void v_set_tri_size( VERDICT_REAL size )
{
  verdict_tri_size = size;
}


/*!
   the aspect ratio of a tri

   srms^2/(2 * sqrt(3.0) * area)
   where srms^2 is sum of the lengths squared
   
*/

C_FUNC_DEF VERDICT_REAL v_tri_aspect( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  static const VERDICT_REAL two_times_root_of_3 = 2*sqrt(3.0);

  // three vectors for each side 
  VerdictVector side1( coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  VerdictVector side2( coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2] );
  
  VerdictVector side3( coordinates[2][0] - coordinates[0][0],
      coordinates[2][1] - coordinates[0][1],
      coordinates[2][2] - coordinates[0][2] );
 
  //sum the lengths squared of each side
  VERDICT_REAL srms = (side1.length_squared() + side2.length_squared() 
      + side3.length_squared());
  
  // find two times the area of the triangle by cross product
  VERDICT_REAL areaX2 = ((side1 * (-side3)).length());
 
  return  srms / (two_times_root_of_3 * (areaX2));

}

/*!
  The area of a tri

  0.5 * jacobian at a node
*/
C_FUNC_DEF VERDICT_REAL v_tri_area( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  // two vectors for two sides
  VerdictVector side1( coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2] );
  
  VerdictVector side3( coordinates[2][0] - coordinates[0][0],
      coordinates[2][1] - coordinates[0][1],
      coordinates[2][2] - coordinates[0][2] );
 
  // the cross product of the two vectors representing two sides of the
  // triangle 
  VerdictVector tmp = side1 * side3;
  
  // return the magnitude of the vector divided by two
  return 0.5 * tmp.length() ;         
  
}


/*!
  The smallest angle of a tri

  The smallest angle of a tri is the smallest angle between 
  two adjacents sides out of all three corners of the triangle.
*/
C_FUNC_DEF VERDICT_REAL v_tri_smallest_angle( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  // vectors for all the sides
  VerdictVector sides[4];
  sides[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  sides[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  sides[2].set(
      coordinates[2][0] - coordinates[0][0],
      coordinates[2][1] - coordinates[0][1],
      coordinates[2][2] - coordinates[0][2]
      );

  // in case we need to find the interior angle
  // between sides 0 and 1
  sides[3] = -sides[1];

  // calculate the lengths squared of the sides
  VERDICT_REAL sides_lengths[3];
  sides_lengths[0] = sides[0].length_squared();
  sides_lengths[1] = sides[1].length_squared();
  sides_lengths[2] = sides[2].length_squared();
  
  // using the law of sines, we know that the smallest
  // angle is opposite of the shortest side

  // find the shortest side
  int short_side=0;
  if(sides_lengths[1] < sides_lengths[0])
    short_side = 1;
  if(sides_lengths[2] < sides_lengths[short_side])
    short_side = 2;

  // from the shortest side, calculate the angle of the 
  // opposite angle
  VERDICT_REAL min_angle = 0.;
  if(short_side == 0)
    min_angle = sides[2].interior_angle(sides[1]);
  else if(short_side == 1)
    min_angle = sides[0].interior_angle(sides[2]);
  else
    min_angle = sides[0].interior_angle(sides[3]);

  return min_angle;
  
}

/*!
  The largest angle of a tri

  The largest angle of a tri is the largest angle between 
  two adjacents sides out of all three corners of the triangle.
*/
C_FUNC_DEF VERDICT_REAL v_tri_largest_angle( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{

  // vectors for all the sides
  VerdictVector sides[4];
  sides[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  sides[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  sides[2].set(
      coordinates[2][0] - coordinates[0][0],
      coordinates[2][1] - coordinates[0][1],
      coordinates[2][2] - coordinates[0][2]
      );

  // in case we need to find the interior angle
  // between sides 0 and 1
  sides[3] = -sides[1];

  // calculate the lengths squared of the sides
  VERDICT_REAL sides_lengths[3];
  sides_lengths[0] = sides[0].length_squared();
  sides_lengths[1] = sides[1].length_squared();
  sides_lengths[2] = sides[2].length_squared();
  
  // using the law of sines, we know that the largest
  // angle is opposite of the longest side

  // find the longest side
  int short_side=0;
  if(sides_lengths[1] > sides_lengths[0])
    short_side = 1;
  if(sides_lengths[2] > sides_lengths[short_side])
    short_side = 2;

  // from the longest side, calculate the angle of the 
  // opposite angle
  VERDICT_REAL min_angle = 0.;
  if(short_side == 0)
    min_angle = sides[2].interior_angle(sides[1]);
  else if(short_side == 1)
    min_angle = sides[0].interior_angle(sides[2]);
  else
    min_angle = sides[0].interior_angle(sides[3]);

  return min_angle;
  
}



/*!
  The condition of a tri

  Condition number of the jacobian matrix at any corner
*/
C_FUNC_DEF VERDICT_REAL v_tri_condition( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  static const VERDICT_REAL rt3 = sqrt(3.0);
  
  VerdictVector xxi(coordinates[0][0] - coordinates[1][0],
      coordinates[0][1] - coordinates[1][1],
      coordinates[0][2] - coordinates[1][2] );
  
  
  VerdictVector xet(coordinates[0][0] - coordinates[2][0],
      coordinates[0][1] - coordinates[2][1],
      coordinates[0][2] - coordinates[2][2] );
  
  
  xet = (2*xet - xxi)/rt3;
  
  VerdictVector tri_normal = xxi * xet;  
  VERDICT_REAL det = tri_normal.length();
  
  if (det <= VERDICT_DBL_MIN)
  {
    return 1.e+06;
  }
  else
  {
    return 0.5 * ( xxi % xxi + xet % xet ) / det; 
    
  }
  
}

/*!
  The scaled jacobian of a tri

  minimum of the jacobian divided by the lengths of 2 edge vectors
*/
C_FUNC_DEF VERDICT_REAL v_tri_scaled_jacobian( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  static const VERDICT_REAL detw = 2./sqrt(3.0);
  VerdictVector first, second;
  VERDICT_REAL curr_jac, min_jac = VERDICT_DBL_MAX;
  
  for(int i=0; i<3; i++)
  {
    first.set(coordinates[(i+1)%3][0] - coordinates[i%3][0],
              coordinates[(i+1)%3][1] - coordinates[i%3][1],
              coordinates[(i+1)%3][2] - coordinates[i%3][2]);
  
    second.set(coordinates[(i+2)%3][0] - coordinates[i%3][0],
               coordinates[(i+2)%3][1] - coordinates[i%3][1],
               coordinates[(i+2)%3][2] - coordinates[i%3][2]);
  
    VerdictVector tmp = first * second;
    curr_jac = tmp.length();
  
    curr_jac *= detw;
  
    curr_jac /= first.length();
    curr_jac /= second.length();

    if( curr_jac < min_jac )
      min_jac = curr_jac;

  }

  return min_jac;
}


/*!
  The shear of a tri

  2 / condition number of weighted skew matrix
*/
C_FUNC_DEF VERDICT_REAL v_tri_shear( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector xxi, xet, tri_normal;
  VERDICT_REAL w11, w21, w12, w22;
  VERDICT_REAL qw11, qw21, qw12, qw22;
  VERDICT_REAL q11, q21, q12, q22;
  VERDICT_REAL x11, x21, x12, x22;
  VERDICT_REAL skew, min_skew = 1.e100;
  
  v_tri_get_weight(w11, w21, w12, w22);
  
  VERDICT_REAL gw11,gw12,gw22;
  metric_matrix(w11,w21,w12,w22,gw11,gw12,gw22);
  
  VERDICT_REAL detw = determinant(w11,w21,w12,w22);
  
  int result = skew_matrix(gw11,gw12,gw22,detw,qw11,qw21,qw12,qw22);
  
  VERDICT_REAL detqw = determinant(qw11,qw21,qw12,qw22);
  
  if( (detqw != 0) && (result))
  {
    for(int i=0; i<3; i++)
    {
      xxi.set( coordinates[i][0] - coordinates[(i+1)%3][0],
          coordinates[i][1] - coordinates[(i+1)%3][1],
          coordinates[i][2] - coordinates[(i+1)%3][2] );
      
      xet.set( coordinates[i][0] - coordinates[(i+2)%3][0],
          coordinates[i][1] - coordinates[(i+2)%3][1],
          coordinates[i][2] - coordinates[(i+2)%3][2] );
      
      tri_normal = xxi * xet;
      VERDICT_REAL  det = tri_normal.length();
      
      VERDICT_REAL g11, g12, g22;  
      
      g11 = xxi % xxi;
      g12 = xxi % xet;
      g22 = xet % xet;
      
      result = skew_matrix(g11,g12,g22,det,q11,q21,q12,q22);
      if( result == false ) {break;}
      
      form_t(q11,q21,q12,q22,qw11,qw21,qw12,qw22,detqw,x11,x21,x12,x22);
      
      VERDICT_REAL detx = determinant(x11,x21,x12,x22);
      VERDICT_REAL normsq = norm_squared(x11,x21,x12,x22);
      
      if(normsq <= VERDICT_DBL_MIN ) { skew = 0; }
      else { skew = 2*detx/normsq; }
      
      min_skew = VERDICT_MIN(skew, min_skew);
    }
  }
  return min_skew;
}



/*!
  The shape of a tri

  2 / condition number of weighted jacobian matrix
*/
C_FUNC_DEF VERDICT_REAL v_tri_shape( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VerdictVector xxi, xet, tri_normal;
  static const VERDICT_REAL rt3 = sqrt(3.0);
  
  xxi.set(coordinates[0][0] - coordinates[1][0],
      coordinates[0][1] - coordinates[1][1],
      coordinates[0][2] - coordinates[1][2]);
  
  xet.set(coordinates[0][0] - coordinates[2][0],
      coordinates[0][1] - coordinates[2][1],
      coordinates[0][2] - coordinates[2][2]);
  
  tri_normal = xxi * xet;
  VERDICT_REAL det = tri_normal.length(); 
  
  VERDICT_REAL g11, g12, g22;
  
  g11 = xxi % xxi;
  g12 = xxi % xet;
  g22 = xet % xet;
  
  VERDICT_REAL den = g11 + g22 - g12;
  
  if ( den > VERDICT_DBL_MIN )
    return rt3 * det /den;
  else
    return 0.0;
}

/*!
  The relative size of a tri

  Min(J,1/J) where J is the determinant of the weighted jacobian matrix.
*/
C_FUNC_DEF VERDICT_REAL v_tri_relative_size( int /*num_nodes*/, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL w11, w21, w12, w22;

  VerdictVector xxi, xet, tri_normal;
  
  v_tri_get_weight(w11,w21,w12,w22);

  VERDICT_REAL detw = determinant(w11,w21,w12,w22);

  xxi.set(coordinates[0][0] - coordinates[1][0],
    coordinates[0][1] - coordinates[1][1],
    coordinates[0][2] - coordinates[1][2]);

  xet.set(coordinates[0][0] - coordinates[2][0],
    coordinates[0][1] - coordinates[2][1],
    coordinates[0][2] - coordinates[2][2]);

  tri_normal = xxi * xet;

  VERDICT_REAL deta = tri_normal.length();

  VERDICT_REAL size = pow( deta/detw, 2 );
  
  return VERDICT_MIN(size, 1.0/size );  
  
}

/*!
  The shape and size of a tri
  
  Product of the Shape and Relative Size
*/
C_FUNC_DEF VERDICT_REAL v_tri_shape_and_size( int num_nodes, VERDICT_REAL coordinates[][3] )
{
  VERDICT_REAL size, shape;  

  size = v_tri_relative_size( num_nodes, coordinates );
  shape = v_tri_shape( num_nodes, coordinates );
  
  return size * shape;

}


/*!
  The distortion of a tri

TODO:  make a short definition of the distortion and comment below
*/
C_FUNC_DEF VERDICT_REAL v_tri_distortion( int num_nodes, VERDICT_REAL coordinates[][3] )
{

   VERDICT_REAL distortion;
   int total_number_of_gauss_points=0;
   VerdictVector  aa, bb, cc,normal_at_point, xin;
   VERDICT_REAL element_area = 0.;

   aa.set(coordinates[1][0] - coordinates[0][0], 
    coordinates[1][1] - coordinates[0][1],
    coordinates[1][2] - coordinates[0][2] );
  
   bb.set(coordinates[2][0] - coordinates[0][0], 
    coordinates[2][1] - coordinates[0][1],
    coordinates[2][2] - coordinates[0][2] );
  

   VerdictVector tri_normal = aa * bb;
 
   int number_of_gauss_points=0;
   if (num_nodes ==3)
   {
      distortion = 1.0;
      return distortion;
   }
   
   else if (num_nodes ==6)
   {
      total_number_of_gauss_points = 6;
      number_of_gauss_points = 6;
   }

   distortion = VERDICT_DBL_MAX;
   VERDICT_REAL shape_function[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy1[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL dndy2[maxTotalNumberGaussPoints][maxNumberNodes];
   VERDICT_REAL weight[maxTotalNumberGaussPoints];

   //create an object of GaussIntegration
   int number_dims = 2;
   int is_tri = 1;
   GaussIntegration gauss_integration(number_of_gauss_points,num_nodes, number_dims, is_tri);
   gauss_integration.calculate_shape_function_2d_tri();
   gauss_integration.get_shape_func(shape_function[0], dndy1[0], dndy2[0], weight);

         // calculate element area
   int ife, ja;
   for (ife=0;ife<total_number_of_gauss_points; ife++)
   {
      aa.set(0.0,0.0,0.0);
      bb.set(0.0,0.0,0.0);

      for (ja=0;ja<num_nodes;ja++)
      {
         xin.set(coordinates[ja][0], coordinates[ja][1], coordinates[ja][2]);
         aa += dndy1[ife][ja]*xin;
         bb += dndy2[ife][ja]*xin;
      }
         normal_at_point = aa*bb;
         VERDICT_REAL jacobian = normal_at_point.length();
         element_area += weight[ife]*jacobian;
   }

   element_area *= 0.8660254;
   VERDICT_REAL dndy1_at_node[maxNumberNodes][maxNumberNodes];
   VERDICT_REAL dndy2_at_node[maxNumberNodes][maxNumberNodes];


   gauss_integration.calculate_derivative_at_nodes_2d_tri( dndy1_at_node,  dndy2_at_node);

   VerdictVector *normal_at_nodes = new VerdictVector[num_nodes];



   //evaluate normal at nodes and distortion values at nodes
   int  jai=0;
   for (ja =0; ja<num_nodes; ja++)
   {
      aa.set(0.0,0.0,0.0);
      bb.set(0.0,0.0,0.0);
      for (jai =0; jai<num_nodes; jai++)
      {
         xin.set(coordinates[jai][0], coordinates[jai][1], coordinates[jai][2]);
         aa += dndy1_at_node[ja][jai]*xin;
         bb += dndy2_at_node[ja][jai]*xin;
      }
      normal_at_nodes[ja] = aa*bb;
      normal_at_nodes[ja].normalize();
   }

   //determine if element is flat
   bool flat_element =true;
   VERDICT_REAL dot_product;

   for ( ja=0; ja<num_nodes;ja++)
   {
      dot_product = normal_at_nodes[0]%normal_at_nodes[ja];
      if (fabs(dot_product) <0.99)
      {
         flat_element = false;
         break;
      }
   }

   // take into consideration of the thickness of the element
   VERDICT_REAL thickness, thickness_gauss;
   VERDICT_REAL jacobian, distrt;
   //get_tri_thickness(tri, element_area, thickness );
     thickness = 0.001*sqrt(element_area);

   //set thickness gauss point location
   VERDICT_REAL zl = 0.5773502691896;
   if (flat_element) zl =0.0;

   int no_gauss_pts_z = (flat_element)? 1 : 2;
   VERDICT_REAL thickness_z;

   //loop on integration points
   int igz;
   for (ife=0;ife<total_number_of_gauss_points;ife++)
   {
      //loop on the thickness direction gauss points
      for (igz=0;igz<no_gauss_pts_z;igz++)
      {
  zl = -zl;
         thickness_z = zl*thickness/2.0;

         aa.set(0.0,0.0,0.0);
         bb.set(0.0,0.0,0.0);
         cc.set(0.0,0.0,0.0);

         for (ja=0;ja<num_nodes;ja++)
         {
            xin.set(coordinates[jai][0], coordinates[jai][1], coordinates[jai][2]);
            xin += thickness_z*normal_at_nodes[ja];
            aa  += dndy1[ife][ja]*xin;
            bb  += dndy2[ife][ja]*xin;
            thickness_gauss = shape_function[ife][ja]*thickness/2.0;
            cc  += thickness_gauss*normal_at_nodes[ja];
         }

         normal_at_point = aa*bb;
         jacobian = normal_at_point.length();
         distrt = cc%normal_at_point;
         if (distrt < distortion) distortion = distrt;
      }
   }

   //loop through nodal points
   for ( ja =0; ja<num_nodes; ja++)
   {
      for ( igz=0;igz<no_gauss_pts_z;igz++)
      {
         zl = -zl;
         thickness_z = zl*thickness/2.0;

         aa.set(0.0,0.0,0.0);
         bb.set(0.0,0.0,0.0);
         cc.set(0.0,0.0,0.0);

         for ( jai =0; jai<num_nodes; jai++)
         {
            xin.set(coordinates[jai][0], coordinates[jai][1], coordinates[jai][2]);
            xin += thickness_z*normal_at_nodes[ja];
            aa += dndy1_at_node[ja][jai]*xin;
            bb += dndy2_at_node[ja][jai]*xin;
            if (jai == ja)
               thickness_gauss = thickness/2.0;
            else
               thickness_gauss = 0.;
            cc  += thickness_gauss*normal_at_nodes[jai];
         }
      }

      normal_at_point = aa*bb;
      VERDICT_REAL sign_jacobian = (tri_normal % normal_at_point) > 0? 1.:-1.;
      distrt = sign_jacobian  * (cc%normal_at_point);

      if (distrt < distortion) distortion = distrt;
   }     
   if (element_area*thickness !=0)
      distortion *=1./( element_area*thickness);
   else
      distortion *=1.;

   delete [] normal_at_nodes;
   
   return distortion;
}


/*! 
  tri_quality for calculating multiple tri metrics at once

  using this method is generally faster than using the individual 
  method multiple times.

*/
C_FUNC_DEF void v_tri_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, TriMetricVals *metric_vals )
{

  memset( metric_vals, 0, sizeof(TriMetricVals) );

  // for starts, lets set up some basic and common information

  /*  node numbers and side numbers used below

             2
             ++
            /  \ 
         2 /    \ 1
          /      \
         /        \
       0 ---------+ 1
             0
  */
  
  // vectors for each side
  VerdictVector sides[3];
  sides[0].set(
      coordinates[1][0] - coordinates[0][0],
      coordinates[1][1] - coordinates[0][1],
      coordinates[1][2] - coordinates[0][2]
      );
  sides[1].set(
      coordinates[2][0] - coordinates[1][0],
      coordinates[2][1] - coordinates[1][1],
      coordinates[2][2] - coordinates[1][2]
      );
  sides[2].set(
      coordinates[2][0] - coordinates[0][0],
      coordinates[2][1] - coordinates[0][1],
      coordinates[2][2] - coordinates[0][2]
      );
  
  // lengths squared of each side
  VERDICT_REAL sides_lengths_squared[3];
  sides_lengths_squared[0] = sides[0].length_squared();
  sides_lengths_squared[1] = sides[1].length_squared();
  sides_lengths_squared[2] = sides[2].length_squared();
 

  // if we are doing angle calcuations
  if( metrics_request_flag & (V_TRI_SMALLEST_ANGLE | V_TRI_LARGEST_ANGLE) )
  {
    // which is short and long side
    int short_side=0, long_side=0;

    if(sides_lengths_squared[1] < sides_lengths_squared[0])
      short_side = 1;
    if(sides_lengths_squared[2] < sides_lengths_squared[short_side])
      short_side = 2;
  
    if(sides_lengths_squared[1] > sides_lengths_squared[0])
      long_side = 1;
    if(sides_lengths_squared[2] > sides_lengths_squared[long_side])
      long_side = 2;


    // calculate the smallest angle of the tri
    if( metrics_request_flag & V_TRI_SMALLEST_ANGLE )
    {
      if(short_side == 0)
        metric_vals->smallest_angle = sides[2].interior_angle(sides[1]);
      else if(short_side == 1)
        metric_vals->smallest_angle = sides[0].interior_angle(sides[2]);
      else
        metric_vals->smallest_angle = sides[0].interior_angle(-sides[1]);
    }
    
    // calculate the largest angle of the tri
    if( metrics_request_flag & V_TRI_LARGEST_ANGLE )
    {
      if(long_side == 0)
        metric_vals->largest_angle = sides[2].interior_angle(sides[1]);
      else if(long_side == 1)
        metric_vals->largest_angle = sides[0].interior_angle(sides[2]);
      else
        metric_vals->largest_angle = sides[0].interior_angle(-sides[1]);
    }
  }


  // calculate the area of the tri
  // the following metrics depend on area
  if( metrics_request_flag & (V_TRI_AREA | V_TRI_SCALED_JACOBIAN | 
    V_TRI_SHAPE | V_TRI_RELATIVE_SIZE | V_TRI_SHAPE_AND_SIZE ) )
  {
    metric_vals->area = (sides[0] * sides[2]).length() * 0.5;
  }

  // calculate the aspect ratio
  if(metrics_request_flag & V_TRI_ASPECT)
  {
    // sum the lengths squared
    VERDICT_REAL srms = 
      sides_lengths_squared[0] +
      sides_lengths_squared[1] +
      sides_lengths_squared[2] ;

    // calculate once and reuse
    static const VERDICT_REAL twoTimesRootOf3 = 2*sqrt(3.0);

    metric_vals->aspect = srms / (twoTimesRootOf3 * 
        ( (sides[0] * (-sides[2])).length() ));
  }

  // calculate the scaled jacobian
  if(metrics_request_flag & V_TRI_SCALED_JACOBIAN)
  {
    // calculate once and reuse
    static const VERDICT_REAL fourOverRootOf3 = 4/sqrt(3.0);
    // use the area from above
    VERDICT_REAL tmp = fourOverRootOf3 * metric_vals->area;

    // now scale it by the lengths of the sides
    VERDICT_REAL min_scaled_jac = VERDICT_DBL_MAX;
    VERDICT_REAL temp_scaled_jac;
    for(int i=0; i<3; i++)
    {
      temp_scaled_jac = tmp / sqrt(sides_lengths_squared[i%3]) / sqrt(sides_lengths_squared[(i+2)%3]);
      if( temp_scaled_jac < min_scaled_jac )
        min_scaled_jac = temp_scaled_jac;
    }

    metric_vals->scaled_jacobian = min_scaled_jac;

  }

  // calculate the condition number
  if(metrics_request_flag & V_TRI_CONDITION)
  {
    // calculate once and reuse
    static const VERDICT_REAL rootOf3 = sqrt(3.0);
    VerdictVector tmp = (2*sides[2] - sides[0])/rootOf3;
    VERDICT_REAL det = (sides[0] * tmp).length();
    if(det <= VERDICT_DBL_MIN)
      metric_vals->condition = 1.e+06;
    else
      metric_vals->condition = 0.5 * 
        ( sides[0] % sides[0] + tmp % tmp) / det;   
  }

  // calculate the shape
  if(metrics_request_flag & V_TRI_SHAPE || metrics_request_flag & V_TRI_SHAPE_AND_SIZE)
  {
    // calculate once and reuse
    static const VERDICT_REAL rootOf3 = sqrt(3.0);
    // reuse area from before
    VERDICT_REAL area2x = metric_vals->area * 2;
    // dot products
    VERDICT_REAL dots[3] = { 
      sides[0] % sides[0],
      sides[0] % sides[2],
      sides[2] % sides[2]
    };

    // add the dots
    VERDICT_REAL sum_dots = dots[0] + dots[2] - dots[1];

    // then the finale
    if( sum_dots > VERDICT_DBL_MIN )
      metric_vals->shape = rootOf3 * area2x / sum_dots;
    else
      metric_vals->shape = 0.;
  }

  // calculate relative size
  if(metrics_request_flag & V_TRI_RELATIVE_SIZE || metrics_request_flag & V_TRI_SHAPE_AND_SIZE)
  {
    // get weights
    VERDICT_REAL w11, w21, w12, w22;
    v_tri_get_weight(w11,w21,w12,w22);
    // get the determinant
    VERDICT_REAL detw = determinant(w11,w21,w12,w22);
    // use the area from above and divide with the determinant
    VERDICT_REAL size = metric_vals->area * 2.0 / detw;
    // square the size
    size *= size;
    // value ranges between 0 to 1
    metric_vals->relative_size = VERDICT_MIN(size, 1.0/size );
  }

  // calculate shape and size
  if(metrics_request_flag & V_TRI_SHAPE_AND_SIZE)
  {
    metric_vals->shape_and_size = 
      metric_vals->relative_size * metric_vals->shape;
  }

  // calculate shear
  if(metrics_request_flag & V_TRI_SHEAR)
    metric_vals->shear = v_tri_shear(num_nodes, coordinates);

  // calculate distortion
  if(metrics_request_flag & V_TRI_DISTORTION)
    metric_vals->distortion = v_tri_distortion(num_nodes, coordinates);

}




/*

WedgeMetric.cpp contains quality calculations for wedges

*/

/*
  the wedge element


        5
        ^
       / \  
      / | \   
     / /2\ \       
   6/_______\4
    | /   \ |
    |/_____\|
   3         1
  
*/



/*!
  
  calculate the volume of a wedge

  this is done by dividing the wedge into 3 tets
  and summing the volume of each tet

*/

C_FUNC_DEF VERDICT_REAL v_wedge_volume( int num_nodes, VERDICT_REAL coordinates[][3] )
{

  VERDICT_REAL volume = 0;
  VerdictVector side1, side2, side3;

  if ( num_nodes == 6 )
  {

    // divide the wedge into 3 tets and calculate each volume

    side1.set( coordinates[1][0] - coordinates[0][0],
               coordinates[1][1] - coordinates[0][1],
               coordinates[1][2] - coordinates[0][2]);

    side2.set( coordinates[2][0] - coordinates[0][0],
               coordinates[2][1] - coordinates[0][1],
               coordinates[2][2] - coordinates[0][2]);


    side3.set( coordinates[3][0] - coordinates[0][0],
               coordinates[3][1] - coordinates[0][1],
               coordinates[3][2] - coordinates[0][2]);

    volume = side3 % (side1 * side2)  / 6;

    side1.set( coordinates[4][0] - coordinates[1][0],
               coordinates[4][1] - coordinates[1][1],
               coordinates[4][2] - coordinates[1][2]);

    side2.set( coordinates[5][0] - coordinates[1][0],
               coordinates[5][1] - coordinates[1][1],
               coordinates[5][2] - coordinates[1][2]);


    side3.set( coordinates[3][0] - coordinates[1][0],
               coordinates[3][1] - coordinates[1][1],
               coordinates[3][2] - coordinates[1][2]);

    volume += side3 % (side1 * side2)  / 6;

    side1.set( coordinates[5][0] - coordinates[1][0],
               coordinates[5][1] - coordinates[1][1],
               coordinates[5][2] - coordinates[1][2]);

    side2.set( coordinates[2][0] - coordinates[1][0],
               coordinates[2][1] - coordinates[1][1],
               coordinates[2][2] - coordinates[1][2]);


    side3.set( coordinates[3][0] - coordinates[1][0],
               coordinates[3][1] - coordinates[1][1],
               coordinates[3][2] - coordinates[1][2]);

    volume += side3 % (side1 * side2)  / 6;

  }

  return volume;

}



C_FUNC_DEF void v_wedge_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, WedgeMetricVals *metric_vals )
{
  memset( metric_vals, 0, sizeof(WedgeMetricVals) );

  if(metrics_request_flag & V_WEDGE_VOLUME)
    metric_vals->volume = v_wedge_volume(num_nodes, coordinates);
}


/*

VerdictVector.cpp contians implementation of Vector operations

*/

const VERDICT_REAL TWO_VERDICT_PI = 2.0 * VERDICT_PI;


VerdictVector &VerdictVector::length(const VERDICT_REAL new_length)
{
  VERDICT_REAL length = this->length();
  xVal *= new_length / length;
  yVal *= new_length / length;
  zVal *= new_length / length;
  return *this;
}


VERDICT_REAL VerdictVector::distance_between(const VerdictVector& test_vector)
{
  VERDICT_REAL x = xVal - test_vector.x();
  VERDICT_REAL y = yVal - test_vector.y();
  VERDICT_REAL z = zVal - test_vector.z();
  
  return(sqrt(x*x + y*y + z*z));
}

/*
void VerdictVector::print_me()
{
  PRINT_INFO("X: %f\n",xVal);
  PRINT_INFO("Y: %f\n",yVal);
  PRINT_INFO("Z: %f\n",zVal);
  
}
*/

VERDICT_REAL VerdictVector::interior_angle(const VerdictVector &otherVector)
{
  VERDICT_REAL cosAngle=0., angleRad=0., len1=0., len2=0.;
  
  if (((len1 = this->length()) > 0) && ((len2 = otherVector.length()) > 0))
    cosAngle = (*this % otherVector)/(len1 * len2);
  else
  {
    assert(len1 > 0);
    assert(len2 > 0);
  }
  
  if ((cosAngle > 1.0) && (cosAngle < 1.0001))
  {
    cosAngle = 1.0;
    angleRad = acos(cosAngle);
  }
  else if (cosAngle < -1.0 && cosAngle > -1.0001)
  {
    cosAngle = -1.0;
    angleRad = acos(cosAngle);
  }
  else if (cosAngle >= -1.0 && cosAngle <= 1.0)
    angleRad = acos(cosAngle);
  else
  {
    assert(cosAngle < 1.0001 && cosAngle > -1.0001);
  }
  
  return( (angleRad * 180.) / VERDICT_PI );
}


// Interpolate between two vectors.
// Returns (1-param)*v1 + param*v2
VerdictVector interpolate(const VERDICT_REAL param, const VerdictVector &v1,
                        const VerdictVector &v2)
{
  VerdictVector temp = (1.0 - param) * v1;
  temp += param * v2;
  return temp;
}

void VerdictVector::xy_to_rtheta()
{
    //careful about overwriting
  VERDICT_REAL r_ = length();
  VERDICT_REAL theta_ = atan2( y(), x() );
  if (theta_ < 0.0) 
    theta_ += TWO_VERDICT_PI;
  
  r( r_ );
  theta( theta_ );
}

void VerdictVector::rtheta_to_xy()
{
    //careful about overwriting
  VERDICT_REAL x_ =  r() * cos( theta() );
  VERDICT_REAL y_ =  r() * sin( theta() );
  
  x( x_ );
  y( y_ );
}

void VerdictVector::rotate(VERDICT_REAL angle, VERDICT_REAL )
{
  xy_to_rtheta();
  theta() += angle;
  rtheta_to_xy();
}

void VerdictVector::blow_out(VERDICT_REAL gamma, VERDICT_REAL rmin)
{
    // if gamma == 1, then 
    // map on a circle : r'^2 = sqrt( 1 - (1-r)^2 )
    // if gamma ==0, then map back to itself
    // in between, linearly interpolate
  xy_to_rtheta();
//  r() = sqrt( (2. - r()) * r() ) * gamma  + r() * (1-gamma);
  assert(gamma > 0.0);
    // the following limits should really be roundoff-based
  if (r() > rmin*1.001 && r() < 1.001) {
    r() = rmin + pow(r(), gamma) * (1.0 - rmin);
  }
  rtheta_to_xy();
}

void VerdictVector::reflect_about_xaxis(VERDICT_REAL, VERDICT_REAL )
{
  yVal = -yVal;
}

void VerdictVector::scale_angle(VERDICT_REAL gamma, VERDICT_REAL )
{
  const VERDICT_REAL r_factor = 0.3;
  const VERDICT_REAL theta_factor = 0.6;
  
  xy_to_rtheta();
  
    // if neary 2pi, treat as zero
    // some near zero stuff strays due to roundoff
  if (theta() > TWO_VERDICT_PI - 0.02)
    theta() = 0;
    // the above screws up on big sheets - need to overhaul at the sheet level
  
  if ( gamma < 1 )
  {
      //squeeze together points of short radius so that
      //long chords won't cross them
    theta() += (VERDICT_PI-theta())*(1-gamma)*theta_factor*(1-r());
    
      //push away from center of circle, again so long chords won't cross
    r( (r_factor + r()) / (1 + r_factor) );
    
      //scale angle by gamma
    theta() *= gamma;
  }
  else
  {
      //scale angle by gamma, making sure points nearly 2pi are treated as zero
    VERDICT_REAL new_theta = theta() * gamma;
    if ( new_theta < 2.5 * VERDICT_PI || r() < 0.2) 
      theta( new_theta );
  }
  rtheta_to_xy();
}

VERDICT_REAL VerdictVector::vector_angle_quick(const VerdictVector& vec1, 
                                               const VerdictVector& vec2)
{
  //- compute the angle between two vectors in the plane defined by this vector
  // build yAxis and xAxis such that xAxis is the projection of
  // vec1 onto the normal plane of this vector

  // NOTE: vec1 and vec2 are Vectors from the vertex of the angle along
  //       the two sides of the angle.
  //       The angle returned is the right-handed angle around this vector
  //       from vec1 to vec2.

  // NOTE: vector_angle_quick gives exactly the same answer as vector_angle below
  //       providing this vector is normalized.  It does so with two fewer
  //       cross-product evaluations and two fewer vector normalizations.
  //       This can be a substantial time savings if the function is called
  //       a significant number of times (e.g Hexer) ... (jrh 11/28/94)
  // NOTE: vector_angle() is much more robust. Do not use vector_angle_quick()
  //       unless you are very sure of the safety of your input vectors.

  VerdictVector ry = (*this) * vec1;
  VerdictVector rx = ry * (*this);

  VERDICT_REAL x = vec2 % rx;
  VERDICT_REAL y = vec2 % ry;

  VERDICT_REAL angle;
  assert(x != 0.0 || y != 0.0);

  angle = atan2(y, x);

  if (angle < 0.0)
  {
    angle += TWO_VERDICT_PI;
  }
  return angle;
}

VerdictVector vectorRotate(const VERDICT_REAL angle,
                         const VerdictVector &normalAxis,
                         const VerdictVector &referenceAxis)
{
    // A new coordinate system is created with the xy plane corresponding
    // to the plane normal to the normal axis, and the x axis corresponding to
    // the projection of the reference axis onto the normal plane.  The normal
    // plane is the tangent plane at the root point.  A unit vector is
    // constructed along the local x axis and then rotated by the given
    // ccw angle to form the new point.  The new point, then is a unit
    // distance from the global origin in the tangent plane.
  
  VERDICT_REAL x, y;
  
    // project a unit distance from root along reference axis
  
  VerdictVector yAxis = normalAxis * referenceAxis;
  VerdictVector xAxis = yAxis * normalAxis;
  yAxis.normalize();
  xAxis.normalize();
  
  x = cos(angle);
  y = sin(angle);
  
  xAxis *= x;
  yAxis *= y;
  return VerdictVector(xAxis + yAxis);
}

VERDICT_REAL VerdictVector::vector_angle(const VerdictVector &vector1,
                                 const VerdictVector &vector2) const
{
    // This routine does not assume that any of the input vectors are of unit
    // length. This routine does not normalize the input vectors.
    // Special cases:
    //     If the normal vector is zero length:
    //         If a new one can be computed from vectors 1 & 2:
    //             the normal is replaced with the vector cross product
    //         else the two vectors are colinear and zero or 2PI is returned.
    //     If the normal is colinear with either (or both) vectors
    //         a new one is computed with the cross products
    //         (and checked again).
  
    // Check for zero length normal vector
  VerdictVector normal = *this;
  VERDICT_REAL normal_lensq = normal.length_squared();
  VERDICT_REAL len_tol = 0.0000001;
  if( normal_lensq <= len_tol )
  {
      // null normal - make it the normal to the plane defined by vector1
      // and vector2. If still null, the vectors are colinear so check
      // for zero or 180 angle.
    normal = vector1 * vector2;
    normal_lensq = normal.length_squared();
    if( normal_lensq <= len_tol )
    {
      VERDICT_REAL cosine = vector1 % vector2;
      if( cosine > 0.0 ) return 0.0;
      else               return VERDICT_PI;
    }
  }
  
    //Trap for normal vector colinear to one of the other vectors. If so,
    //use a normal defined by the two vectors.
  VERDICT_REAL dot_tol = 0.985;
  VERDICT_REAL dot = vector1 % normal;
  if( dot * dot >= vector1.length_squared() * normal_lensq * dot_tol )
  {
    normal = vector1 * vector2;
    normal_lensq = normal.length_squared();
    
      //Still problems if all three vectors were colinear
    if( normal_lensq <= len_tol )
    {
      VERDICT_REAL cosine = vector1 % vector2;
      if( cosine >= 0.0 ) return 0.0;
      else                return VERDICT_PI;
    }
  }
  else
  {
      //The normal and vector1 are not colinear, now check for vector2
    dot = vector2 % normal;
    if( dot * dot >= vector2.length_squared() * normal_lensq * dot_tol )
    {
      normal = vector1 * vector2;
    }
  }
  
    // Assume a plane such that the normal vector is the plane's normal.
    // Create yAxis perpendicular to both the normal and vector1. yAxis is
    // now in the plane. Create xAxis as the perpendicular to both yAxis and
    // the normal. xAxis is in the plane and is the projection of vector1
    // into the plane.
  
  normal.normalize();
  VerdictVector yAxis = normal;
  yAxis *= vector1;
  VERDICT_REAL y = vector2 % yAxis;
    //  yAxis memory slot will now be used for xAxis
  yAxis *= normal;
  VERDICT_REAL x = vector2 % yAxis;
  
  
    //  assert(x != 0.0 || y != 0.0);
  if( x == 0.0 && y == 0.0 )
  {
    return 0.0;
  }
  VERDICT_REAL angle = atan2(y, x);
  
  if (angle < 0.0)
  {
    angle += TWO_VERDICT_PI;
  }
  return angle;
}

bool VerdictVector::within_tolerance( const VerdictVector &vectorPtr2,
                                            VERDICT_REAL tolerance) const
{
  if (( fabs (this->x() - vectorPtr2.x()) < tolerance) &&
      ( fabs (this->y() - vectorPtr2.y()) < tolerance) &&
      ( fabs (this->z() - vectorPtr2.z()) < tolerance)
      )
  {
    return true;
  }
  
  return false;
}

void VerdictVector::orthogonal_vectors( VerdictVector &vector2, 
                                      VerdictVector &vector3 )
{
  VERDICT_REAL x[3];
  unsigned short i=0;
  unsigned short imin=0;
  VERDICT_REAL rmin = 1.0E20;
  unsigned short iperm1[3];
  unsigned short iperm2[3];
  unsigned short cont_flag = 1;
  VERDICT_REAL vec1[3], vec2[3];
  VERDICT_REAL rmag;
  
    // Copy the input vector and normalize it
  VerdictVector vector1 = *this;
  vector1.normalize();
  
    // Initialize perm flags
  iperm1[0] = 1; iperm1[1] = 2; iperm1[2] = 0;
  iperm2[0] = 2; iperm2[1] = 0; iperm2[2] = 1;
  
    // Get into the array format we can work with
  vector1.get_xyz( vec1 );
  
  while (i<3 && cont_flag )
  {
    if (fabs(vec1[i]) < 1e-6)
    {
      vec2[i] = 1.0;
      vec2[iperm1[i]] = 0.0;
      vec2[iperm2[i]] = 0.0;
      cont_flag = 0;
    }
    
    if (fabs(vec1[i]) < rmin)
    {
      imin = i;
      rmin = fabs(vec1[i]);
    }
    ++i;
  }
  
  if (cont_flag)
  {
    x[imin] = 1.0;
    x[iperm1[imin]] = 0.0;
    x[iperm2[imin]] = 0.0;
    
      // Determine cross product
    vec2[0] = vec1[1] * x[2] - vec1[2] * x[1];
    vec2[1] = vec1[2] * x[0] - vec1[0] * x[2];
    vec2[2] = vec1[0] * x[1] - vec1[1] * x[0];
    
      // Unitize
    rmag = sqrt(vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2]);
    vec2[0] /= rmag;
    vec2[1] /= rmag;
    vec2[2] /= rmag;
  }
  
    // Copy 1st orthogonal vector into VerdictVector vector2
  vector2.set( vec2 );
  
    // Cross vectors to determine last orthogonal vector
  vector3 = vector1 * vector2;
  
  return;
}

//- Find next point from this point using a direction and distance
void VerdictVector::next_point( const VerdictVector &direction,
                              VERDICT_REAL distance, VerdictVector& out_point )
{
  VerdictVector my_direction = direction;
  my_direction.normalize();
  
    // Determine next point in space
  out_point.x( xVal + (distance * my_direction.x()) );     
  out_point.y( yVal + (distance * my_direction.y()) );     
  out_point.z( zVal + (distance * my_direction.z()) ); 
  
  return;
}

VerdictVector::VerdictVector(const VERDICT_REAL xyz[3]) 
  : xVal(xyz[0]), yVal(xyz[1]), zVal(xyz[2])
{}



#ifndef AVT_EXPRESSION_FILTER_H
#define AVT_EXPRESSION_FILTER_H

#include <expression_exports.h>
#include <avtStreamer.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtExprFilterNoNamespaceConflict
//
//  Purpose:
//      This is a base class that lets derived types not worry about how to set
//      up a derived variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:18:07 PDT 2003
//      Added the virtual function NumVariableArguments that lets an
//      expression declare how many of its arguments are variables.
//
// ****************************************************************************

class avtExprFilterNoNamespaceConflict : public avtStreamer
{
  public:
                             avtExprFilterNoNamespaceConflict();
    virtual                 ~avtExprFilterNoNamespaceConflict();

    void                     SetOutputVariableName(const char *);
    virtual void             AddInputVariableName(const char *var)
                                {SetActiveVariable(var);}

    virtual void             ProcessArguments(ArgsExpr *, ExprPipelineState *);
    virtual int              NumVariableArguments() = 0;

  protected:
    char                    *outputVariableName;

    virtual bool             IsPointVariable();

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual void             RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                             PerformRestriction(avtPipelineSpecification_p);

    virtual int              GetVariableDimension() { return 1; };
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *) = 0;
};


#endif


// ************************************************************************* //
//                           avtExprFilterNoNamespaceConflict.C                           //
// ************************************************************************* //

#include <avtExprNode.h>

#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <DebugStream.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict constructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

avtExprFilterNoNamespaceConflict::avtExprFilterNoNamespaceConflict()
{
    outputVariableName = NULL;
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict destructor
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

avtExprFilterNoNamespaceConflict::~avtExprFilterNoNamespaceConflict()
{
    if (outputVariableName != NULL)
    {
        delete [] outputVariableName;
        outputVariableName = NULL;
    }
}

void
avtExprFilterNoNamespaceConflict::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    std::vector<ArgExpr*>::iterator i;
    for (i=arguments->begin(); i != arguments->end(); i++)
        dynamic_cast<avtExprNode*>((*i)->GetExpr())->CreateFilters(state);
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::SetOutputVariableName
//
//  Purpose:
//      Sets the name of the expression.
//
//  Arguments:
//      name     The name of the derived output variable.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2002
//
// ****************************************************************************

void
avtExprFilterNoNamespaceConflict::SetOutputVariableName(const char *name)
{
    if (outputVariableName != NULL)
        delete [] outputVariableName;

    outputVariableName = new char[strlen(name)+1];
    strcpy(outputVariableName, name);
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::PreExecute
//
//  Purpose:
//      Gins up some default extents so that the cumulative ones will work.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004
//    Change to DataExtents -- they now always have two components.
//
// ****************************************************************************
 
void
avtExprFilterNoNamespaceConflict::PreExecute(void)
{
    avtStreamer::PreExecute();
    double exts[2] = {FLT_MAX, -FLT_MAX }; 
    GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueDataExtents()->Set(exts);
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::PostExecute
//
//  Purpose:
//      Gins up some default extents so that the cumulative ones will work.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer:  ??? <Sean Ahern>
//  Creation:    ~June 7, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 09:35:52 PDT 2003
//    Called avtStreamer's PostExecute, since that is the base class.
//    
// ****************************************************************************

void
avtExprFilterNoNamespaceConflict::PostExecute(void)
{
    // Make our derived variable be the active variable.
    avtStreamer::PostExecute();
    OutputSetActiveVariable(outputVariableName);
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//
//  Returns:      The output dataset.
//
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
//  Modifications:
//
//    Sean Ahern, Tue Jun 11 16:35:48 PDT 2002
//    Modified to work with both point and cell data.  Note, this doesn't
//    yet handle filters that convert from point to cell, or vice versa.
//
//    Akira Haddox, Thu Aug 15 16:41:44 PDT 2002
//    Modified to work with vector expressions.
//
//    Akira Haddox, Mon Aug 19 16:39:07 PDT 2002 
//    Modified to use IsPointVariable to determine variable type.
//
//    Hank Childs, Mon Sep  2 18:53:40 PDT 2002
//    Do not use enhanced connectivity points when calculating ranges.
//
//    Hank Childs, Tue Nov 19 08:50:42 PST 2002
//    Use the dimension of the returned type instead of the virtual function
//    call to decide if the variable is a vector.  Did not remove virtual
//    function to determine this, since it may be used before execution (by the
//    GUI, for example to decide if something is a vector,scalar, etc).
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Use NewInstance instead of MakeObject, new vtk api. 
//
//    Hank Childs, Thu Jan 23 11:19:14 PST 2003
//    Set only the cumulative extents.
//
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004
//    Change to DataExtents -- they now always have two components.
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

vtkDataSet *
avtExprFilterNoNamespaceConflict::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int   i, j;

    //
    // Start off by having the derived type calculate the derived variable.
    //
    vtkDataArray *dat = DeriveVariable(in_ds);
    dat->SetName(outputVariableName);

    int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    if (IsPointVariable())
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        }
        else
        {
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        }
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        }
        else
        {
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        }
    }

    //
    // Make our best attempt at maintaining our extents.
    //
    double exts[2];
    unsigned char *ghosts = NULL;
    if (!IsPointVariable())
    {
        vtkUnsignedCharArray *g = (vtkUnsignedCharArray *)
                                 rv->GetCellData()->GetArray("avtGhostZones");
        if (g != NULL)
        {
            ghosts = g->GetPointer(0);
        }
    }
    int ntuples = dat->GetNumberOfTuples();
    int nvars   = dat->GetNumberOfComponents();
    exts[0] = +FLT_MAX;
    exts[1] = -FLT_MAX;
    for (i = 0 ; i < ntuples ; i++)
    {
        if (ghosts != NULL && ghosts[i] > 0) 
        {
            continue;
        }
        double *val = dat->GetTuple(i);
        for (j = 0 ; j < nvars ; j++)
        {
            if (val[j] < exts[0])
            {
                exts[0] = val[j];
            }
            if (val[j] > exts[1])
            {
                exts[1] = val[j];
            }
        }
    }
    GetOutput()->GetInfo().GetAttributes().
                                   GetCumulativeTrueDataExtents()->Merge(exts);

    //
    // Make sure that we don't have any memory leaks.
    //
    dat->Delete();
    ManageMemory(rv);
    rv->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::RefashionDataObjectInfo
//
//  Purpose:
//      Tells our output that we now have a variable.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//
//      Akira Haddox, Mon Aug 19 16:41:12 PDT 2002
//      Modified to set the centering of the variable to cell or point
//      based on IsPointVariable().
//
//      Hank childs, Mon Feb 23 16:10:19 PST 2004
//      Meet new data attributes interface.
//
// ****************************************************************************
 
void
avtExprFilterNoNamespaceConflict::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.AddVariable(outputVariableName);
    outAtts.SetActiveVariable(outputVariableName);
    outAtts.SetVariableDimension(GetVariableDimension());
    outAtts.SetCentering(IsPointVariable()?AVT_NODECENT:AVT_ZONECENT);
}


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::PerformRestriction
//
//  Purpose:
//      Determines if there is a request for the derived type's derived 
//      variable as a secondary variable.  If so, snag the request, because it
//      will only confuse the database.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************

avtPipelineSpecification_p
avtExprFilterNoNamespaceConflict::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = spec;

    avtDataSpecification_p ds = spec->GetDataSpecification();
    if (ds->HasSecondaryVariable(outputVariableName))
    {
        avtDataSpecification_p newds = new avtDataSpecification(ds);
        newds->RemoveSecondaryVariable(outputVariableName);
        rv = new avtPipelineSpecification(spec, newds);
    }

    return rv;
} 


// ****************************************************************************
//  Method: avtExprFilterNoNamespaceConflict::IsPointVariable
//
//  Purpose:
//      Used to determine if expression is for point or cell data. Defaults
//      to cell if input is cell data, or point data otherwise.
//
//  Returns:    True if the data should be point data, false if cell data.
//
//  Programmer: Akira Haddox 
//  Creation:   August 19, 2002
//
// ****************************************************************************

bool
avtExprFilterNoNamespaceConflict::IsPointVariable()
{
    return (GetInput()->GetInfo().GetAttributes().GetCentering()
            == AVT_NODECENT);
}


// ********************************************************************** //
//                             avtSingleInputEFNoNamespaceConflict.h           //
// ********************************************************************** //

#ifndef AVT_SINGLE_INPUT_EXPRESSION_FILTER_H
#define AVT_SINGLE_INPUT_EXPRESSION_FILTER_H



class     vtkDataArray;

// ****************************************************************************
//  Class: avtSingleInputEFNoNamespaceConflict
//
//  Purpose:
//      A filter that performs a calculation on a single variable.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 12 16:43:09 PDT 2002
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:22:43 PDT 2003
//      Added the NumVariableArguments function, specifying that all
//      subclasses of SingleInputExpressionFilter have one variable input.
//
// ****************************************************************************

class avtSingleInputEFNoNamespaceConflict : public avtExprFilterNoNamespaceConflict
{
  public:
                              avtSingleInputEFNoNamespaceConflict() {;}
    virtual                  ~avtSingleInputEFNoNamespaceConflict() {;}

    virtual const char       *GetType(void) { return "avtSingleInputEFNoNamespaceConflict"; }
    virtual const char       *GetDescription(void) = 0;
    virtual int               NumVariableArguments() { return 1; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *) = 0;
};

#endif


// ************************************************************************* //
//                             avtVerdictNoNamespaceConflict.h                            //
// ************************************************************************* //

// Caveat: Verdict filters currently support triangles, but not triangle strips

#ifndef AVT_VERDICT_FILTER_H
#define AVT_VERDICT_FILTER_H
#include <expression_exports.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtVerdictNoNamespaceConflict
//
//  Purpose:
//    This is a abstract base class for the verdict metric filters.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 19 10:44:29 PDT 2005
//    Add support for sub-types operating directly on the mesh.
//
//
// ****************************************************************************

class avtVerdictNoNamespaceConflict : public avtSingleInputEFNoNamespaceConflict
{
  public:
                              avtVerdictNoNamespaceConflict();
    virtual                  ~avtVerdictNoNamespaceConflict() {;};

    virtual void              PreExecute();

    virtual const char       *GetType(void)   { return "avtVerdictNoNamespaceConflict"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Verdict expression."; };

    virtual bool              OperateDirectlyOnMesh(vtkDataSet *)
                                       { return false; };
    virtual void              MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);

    virtual double            Metric(double coordinates[][3], int type) = 0;

    virtual bool              RequiresSizeCalculation() { return false; }

    virtual bool              IsPointVariable() {  return false; }
};

#endif
// ************************************************************************* //
//                             avtVerdictNoNamespaceConflict.C                            //
// ************************************************************************* //


#include <verdict.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>

#include <avtDataTree.h>
#include <avtDataRepresentation.h>

#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <DebugStream.h>

struct SizeData
{
    double hexSize;
    double tetSize;
    double triSize;
    double quadSize;

    int hexCount;
    int tetCount;
    int triCount;
    int quadCount;

    void Clear()
    {
        hexSize = tetSize = triSize = quadSize = 0;
        hexCount = tetCount = triCount = quadCount = 0;
    }
};

SizeData VerdictSizeData;

void SumSize(avtDataRepresentation &, void *, bool &);

inline void
Swap1(double &a, double &b)
{
    double tmp = a;
    a = b;
    b = tmp;
}

inline void
Swap3(double c[][3], int a, int b)
{
    Swap1(c[a][0], c[b][0]);
    Swap1(c[a][1], c[b][1]);
    Swap1(c[a][2], c[b][2]);
}

// ****************************************************************************
//  Method: avtVerdictNoNamespaceConflict::avtVerdictNoNamespaceConflict
//
//  Purpose:
//      Constructor. Only purpose is to throw an exception if verdict is not
//      installed.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
// ****************************************************************************

avtVerdictNoNamespaceConflict::avtVerdictNoNamespaceConflict()
{
    #ifndef HAVE_VERDICT
    EXCEPTION1(ExpressionException, "Verdict was not distributed with this "
                                    "release of VisIt.");
    #endif
}

// ****************************************************************************
//  Method: avtVerdictNoNamespaceConflict::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset, using a metric call.
//
//  Arguments:
//      in_ds     The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//
//  Modifications:
//
//    Akira Haddox, Wed Jul  2 08:26:30 PDT 2003
//    Added conversion from pixel cell type.
//
// ****************************************************************************

vtkDataArray *
avtVerdictNoNamespaceConflict::DeriveVariable(vtkDataSet *in_ds)
{
    int i, j;

    int nCells = in_ds->GetNumberOfCells();

    //
    // Set up a VTK variable reflecting the results we have calculated.
    //
    vtkFloatArray *dv = vtkFloatArray::New();
    dv->SetNumberOfTuples(nCells);

    //
    // Iterate over each cell in the mesh and if it matches a
    // testData prerequisites, run the corresponding metric
    //
    if (OperateDirectlyOnMesh(in_ds))
    {
        MetricForWholeMesh(in_ds, dv);
    }
    else
    {
        const int MAXPOINTS = 100;
        double coordinates[MAXPOINTS][3];
        for (i = 0; i < nCells; i++)
        {
            vtkCell *cell = in_ds->GetCell(i);

            int numPointsForThisCell = cell->GetNumberOfPoints();
            // Grab a pointer to the cell's points' underlying data array
            vtkDataArray *pointData = cell->GetPoints()->GetData();

            //
            // Since the Verdict functions make their own copy of the data
            // anyway it would be nice to get the coordinate data without
            // copying (to cut down on unneeded copying). However, this might
            // be infeasible since Verdict expects doubles, and vtk
            //(potentially) uses floats.
            //

            if (pointData->GetNumberOfComponents() != 3)
            {
                EXCEPTION0(ImproperUseException);
            }

            // Fortunately, Verdict will convert to a double[3] for us
            for (j = 0; j < numPointsForThisCell; j++)
            {
                coordinates[j][2] = 0; // In case of 2d coordinates
                pointData->GetTuple(j,coordinates[j]);
            }

            int cellType = cell->GetCellType();

            // Convert Voxel format into hexahedron format.
            if (cellType == VTK_VOXEL)
            {
                Swap3(coordinates, 2,3);
                Swap3(coordinates, 6,7);
            }

            // Convert Pixel format into quad format.
            if (cellType == VTK_PIXEL)
            {
                Swap3(coordinates, 2, 3);
                cellType = VTK_QUAD;
            }

            float result = Metric(coordinates, cellType);
            dv->SetTuple1(i, result);
        }
    }

    return dv;
}

// ****************************************************************************
//  Method: avtVerdictFilter::MetricForWholeMesh
//
//  Purpose:
//      Calculates a metric for the whole mesh.  This should be re-defined
//      by derived types that re-define OperateDirectlyOnMesh to return true.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVerdictNoNamespaceConflict::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Method: avtVerdictNoNamespaceConflict::PreExecute
//
//  Purpose:
//      If this metric requires size calculation, then before the tree is
//      executed, this function will calculate and set the verdict size
//      variables.
//
//  Arguments:
//      in_ds     The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

void
avtVerdictNoNamespaceConflict::PreExecute()
{
#ifdef HAVE_VERDICT
    if (!RequiresSizeCalculation())
        return;

    avtExprFilterNoNamespaceConflict::PreExecute();

    VerdictSizeData.Clear();

    avtDataTree_p tree = GetInputDataTree();
    bool suc;
    tree->Traverse(SumSize,NULL,suc);

    if (VerdictSizeData.hexCount)
        v_set_hex_size(VerdictSizeData.hexSize/
                     (double)(VerdictSizeData.hexCount));
    if (VerdictSizeData.tetCount)
        v_set_tet_size(VerdictSizeData.tetSize/
                     (double)(VerdictSizeData.tetCount));
    if (VerdictSizeData.triCount)
        v_set_tri_size(VerdictSizeData.triSize/
                     (double)(VerdictSizeData.triCount));
    if (VerdictSizeData.quadCount)
        v_set_quad_size(VerdictSizeData.quadSize/
                      (double)(VerdictSizeData.quadCount));
#endif
}

// ****************************************************************************
//  Function: SumSize
//
//  Purpose:
//      Used for data tree traversal to collect sums of area/volume && quantity
//      of different cell types. Stores information in VerdictSizeData.
//
//  Arguments:
//      adr       The data.
//      <not used>
//      <not used>
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
//    Akira Haddox, Wed Jul  2 08:26:30 PDT 2003
//    Added conversion from pixel cell type.
//
// ****************************************************************************

void SumSize(avtDataRepresentation &adr, void *, bool &)
{
#ifdef HAVE_VERDICT
    vtkDataSet *in_ds = adr.GetDataVTK();
    int i, j;

    int nCells = in_ds->GetNumberOfCells();

    double *results = new double[nCells];

    const int MAXPOINTS = 100;
    double coordinates[MAXPOINTS][3];
    for (i = 0; i < nCells; i++)    
    {
        vtkCell *cell = in_ds->GetCell(i);
        
        int numPointsForThisCell = cell->GetNumberOfPoints();
        // Grab a pointer to the cell's points' underlying data array
        vtkDataArray *pointData = cell->GetPoints()->GetData();

        if (pointData->GetNumberOfComponents() != 3)
        {
            EXCEPTION0(ImproperUseException);
        }

        // Fortunately, Verdict will convert to a double[3] for us
        for (j = 0; j < numPointsForThisCell; j++)
        {
            coordinates[j][2] = 0; // In case of 2d coordinates
            pointData->GetTuple(j,coordinates[j]);
        }

        // Convert Voxel format into hexahedron format.
        if (cell->GetCellType() == VTK_VOXEL)
        {
                Swap3(coordinates, 2,3);
                Swap3(coordinates, 6,7);
        }

        // Convert Pixel format into quad format.
        if (cell->GetCellType() == VTK_PIXEL)
        {
            Swap3(coordinates, 2, 3);
        }

        switch (cell->GetCellType())
        {
            case VTK_VOXEL:
            case VTK_HEXAHEDRON:
                ++VerdictSizeData.hexCount;
                VerdictSizeData.hexSize+=v_hex_volume(8, coordinates);
                break;
            case VTK_TETRA:
                ++VerdictSizeData.tetCount;
                VerdictSizeData.tetSize+=v_tet_volume(4, coordinates);
                break;
            case VTK_TRIANGLE:
                ++VerdictSizeData.triCount;
                VerdictSizeData.triSize+=v_tri_area(3, coordinates);
                break;
            case VTK_PIXEL:
            case VTK_QUAD:
                ++VerdictSizeData.quadCount;
                VerdictSizeData.quadSize+=v_quad_area(3, coordinates);
            default:
                break;
        }
    }
    delete []results;
#endif
}
// ************************************************************************* //
//                          avtVolumeNoNamespaceConflict.h                               //
// ************************************************************************* //

#ifndef AVT_VMETRIC_VOLUME_H
#define AVT_VMETRIC_VOLUME_H

#include <expression_exports.h>


// ****************************************************************************
//  Class: avtVolumeNoNamespaceConflict
//
//  Purpose:
//    This metric measures volume.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Aug 31 12:25:02 PDT 2002
//    Added ability to only consider absolute values of volumes.
//
//    Hank Childs, Thu May 19 10:55:30 PDT 2005
//    Added support for operating on rectilinear meshes directly.
//
// ****************************************************************************

class avtVolumeNoNamespaceConflict : public avtVerdictNoNamespaceConflict
{
  public:
                       avtVolumeNoNamespaceConflict();

    virtual double     Metric(double coords[][3], int type);

    void               UseOnlyPositiveVolumes(bool val)
                                  { useOnlyPositiveVolumes = val; };
    virtual bool       OperateDirectlyOnMesh(vtkDataSet *);
    virtual void       MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

  protected:
    bool               useOnlyPositiveVolumes;
};


#endif


// ************************************************************************* //
//                              avtVolumeNoNamespaceConflict.C                           //
// ************************************************************************* //


#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>

#include <DebugStream.h>


inline
void Copy3(double coords[][3], double a[], int i)
{
    a[0] = coords[i][0];
    a[1] = coords[i][1];
    a[2] = coords[i][2];
}


// ****************************************************************************
//  Method: avtVolumeNoNamespaceConflict constructor
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2002
//
// ****************************************************************************

avtVolumeNoNamespaceConflict::avtVolumeNoNamespaceConflict()
{
    useOnlyPositiveVolumes = false;
}


// ****************************************************************************
//  Method: avtVolumeNoNamespaceConflict::Metric
//
//  Purpose:
//      Inspect an element and calculate the volume.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The volume of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Aug 31 12:25:02 PDT 2002
//    Added support for taking the absolute volumes of values.
//
//    Hank Childs, Fri Sep  6 08:27:05 PDT 2002
//    Only the volume for a tetrahedron is 100% accurate.  Tetrahedralize the
//    remaining cell types to guarantee that we get 100% accurate results.
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.  Remove previous code to tetrahedralize
//    hexahedrons, since that has now been incorporated into the Verdict
//    library.  Also fixed problem with wedge volumes where the tets we
//    were using were inverted.
//
// ****************************************************************************

double avtVolumeNoNamespaceConflict::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    double rv = 0.;
    switch (type)
    {
      case VTK_VOXEL:   // Note that the verdict filter already swapped the
                        // coordinates to make a voxel be like a hex.
      case VTK_HEXAHEDRON:
        rv = v_hex_volume(8,coords);
        break;
        
      case VTK_TETRA:
        rv = v_tet_volume(4,coords);
        break;

      case VTK_WEDGE:
        {
            int   subdiv[3][4] = { {0,5,4,3}, {0,2,1,4}, {0,4,5,2} };
            double tet_coords[4][3];
            for (int i = 0 ; i < 3 ; i++)
            {
                for (int j = 0 ; j < 4 ; j++)
                   for (int k = 0 ; k < 3 ; k++)
                       tet_coords[j][k] = coords[subdiv[i][j]][k];
                double temp = v_tet_volume(4, tet_coords);
                if (temp < 0. && useOnlyPositiveVolumes)
                {
                    temp *= -1.;
                }
                rv += temp;
            }
        }
        break;
        
      // The verdict metric for pyramid I have yet to figure out how to work.
      // However, it does the same thing that we do here: Divide the pyramid
      // into two tetrahedrons.
      case VTK_PYRAMID:
        double one[4][3];
        double two[4][3];
            
        Copy3(coords,one[0], 0);
        Copy3(coords,one[1], 1);
        Copy3(coords,one[2], 2);
        Copy3(coords,one[3], 4);

        Copy3(coords,two[0], 0);
        Copy3(coords,two[1], 2);
        Copy3(coords,two[2], 3);
        Copy3(coords,two[3], 4);

        rv = v_tet_volume(4,one) + v_tet_volume(4,two);
        break;
    }

    if (rv < 0. && useOnlyPositiveVolumes)
    {
        rv *= -1.;
    }

    return rv;
#else
    return -1.;
#endif
}

// ****************************************************************************
//  Method: avtVMetricVolume::OperateDirectlyOnMesh
//
//  Purpose:
//      Determines if we want to speed up the operation by operating directly
//      on the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

bool
avtVolumeNoNamespaceConflict::OperateDirectlyOnMesh(vtkDataSet *ds)
{
    return (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID);
}


// ****************************************************************************
//  Method: avtVMetricVolume::MetricForWholeMesh
//
//  Purpose:
//      Determines the volume for each cell in the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVolumeNoNamespaceConflict::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    int  i, j, k;

    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    vtkDataArray *Z = rg->GetZCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    float *Xdist = new float[dims[0]-1];
    for (i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    float *Ydist = new float[dims[1]-1];
    for (i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);
    float *Zdist = new float[dims[2]-1];
    for (i = 0 ; i < dims[2]-1 ; i++)
        Zdist[i] = Z->GetTuple1(i+1) - Z->GetTuple1(i);

    for (k = 0 ; k < dims[2]-1 ; k++)
        for (j = 0 ; j < dims[1]-1 ; j++)
            for (i = 0 ; i < dims[0]-1 ; i++)
            {
                int idx = k*(dims[1]-1)*(dims[0]-1) + j*(dims[0]-1) + i;
                float vol = Xdist[i]*Ydist[j]*Zdist[k];
                rv->SetTuple1(idx, vol);
            }

    delete [] Xdist;
    delete [] Ydist;
    delete [] Zdist;
}



// ************************************************************************* //
//                          avtAreaNoNamespaceConflict.h                                 //
// ************************************************************************* //

#ifndef AVT_VMETRIC_AREA_H
#define AVT_VMETRIC_AREA_H
#include <expression_exports.h>

// ****************************************************************************
//  Class: avtAreaNoNamespaceConflict
//
//  Purpose:
//      This metric measures area.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 19 10:55:30 PDT 2005
//    Added support for operating on rectilinear meshes directly.
//
// ****************************************************************************

class avtAreaNoNamespaceConflict : public avtVerdictNoNamespaceConflict
{
    public:
        double Metric(double coords[][3], int type);

    virtual bool       OperateDirectlyOnMesh(vtkDataSet *);
    virtual void       MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

};

#endif
// ************************************************************************* //
//                              avtAreaNoNamespaceConflict.C                             //
// ************************************************************************* //


#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtAreaNoNamespaceConflict::Metric
//
//  Purpose:
//      Inspect an element and calculate the Area.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The Area of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

double avtAreaNoNamespaceConflict::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT
    switch(type)
    {
        case VTK_TRIANGLE:
            return v_tri_area(3, coords);
        
        case VTK_QUAD:
            return v_quad_area(4, coords);
    }
#endif
    return -1;
}

// ****************************************************************************
//  Method: avtVMetricArea::OperateDirectlyOnMesh
//
//  Purpose:
//      Determines if we want to speed up the operation by operating directly
//      on the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

bool
avtAreaNoNamespaceConflict::OperateDirectlyOnMesh(vtkDataSet *ds)
{
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int dims[3];
        ((vtkRectilinearGrid *) ds)->GetDimensions(dims);
        if (dims[0] > 1 && dims[1] > 1 && dims[2] == 1)
            return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtVMetricArea::MetricForWholeMesh
//
//  Purpose:
//      Determines the area for each cell in the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtAreaNoNamespaceConflict::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    int  i, j;

    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    float *Xdist = new float[dims[0]-1];
    for (i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    float *Ydist = new float[dims[1]-1];
    for (i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);

    for (j = 0 ; j < dims[1]-1 ; j++)
        for (i = 0 ; i < dims[0]-1 ; i++)
        {
            int idx = j*(dims[0]-1) + i;
            float area = Xdist[i]*Ydist[j];
            rv->SetTuple1(idx, area);
        }

    delete [] Xdist;
    delete [] Ydist;
}


// ************************************************************************* //
//                          avtRevolvedVolumeNoNamespaceConflict.h                              //
// ************************************************************************* //

#ifndef AVT_REVOLVED_VOLUME_H
#define AVT_REVOLVED_VOLUME_H



class     vtkCell;


// ****************************************************************************
//  Class: avtRevolvedVolumeNoNamespaceConflict
//
//  Purpose:
//      Calculates the volume a 2D polygon would occupy if it were revolved
//      around an axis.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

class avtRevolvedVolumeNoNamespaceConflict : public avtSingleInputEFNoNamespaceConflict
{
  public:
                                avtRevolvedVolumeNoNamespaceConflict();

    virtual const char         *GetType(void) { return "avtRevolvedVolumeNoNamespaceConflict"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating revolved volume"; };
    
  protected:
    bool                        haveIssuedWarning;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual void                PreExecute(void);

    virtual bool                IsPointVariable(void)  { return false; };

    double                      GetZoneVolume(vtkCell *);
    double                      GetTriangleVolume(double [3], double [3]);
    double                      RevolveLineSegment(double [2], double [2],
                                                   double *);
};


#endif


// ************************************************************************* //
//                           avtRevolvedVolumeNoNamespaceConflict.C                             //
// ************************************************************************* //


#include <float.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtRevolvedVolumeNoNamespaceConflict constructor
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

avtRevolvedVolumeNoNamespaceConflict::avtRevolvedVolumeNoNamespaceConflict()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtRevolvedVolumeNoNamespaceConflict::PreExecute
//
//  Purpose:
//      Sets up a data member that ensures that we don't issue multiple
//      warnings.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

void
avtRevolvedVolumeNoNamespaceConflict::PreExecute(void)
{
    avtSingleInputEFNoNamespaceConflict::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Revolved volume",
                                               "2-dimensional");
    }
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtRevolvedVolumeNoNamespaceConflict::DeriveVariable
//
//  Purpose:
//      Determines the volume each cell would take if it were revolved around
//      the axis y=0.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
// ****************************************************************************

vtkDataArray *
avtRevolvedVolumeNoNamespaceConflict::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetZoneVolume(cell);
        arr->SetTuple(i, &vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtRevolvedVolumeNoNamespaceConflict::GetZoneVolume
//
//  Purpose:
//      Revolve the zone around the axis x = 0.  This is done by making
//      two volumes for the top of the zone and the bottom of the zone and
//      subtracting them.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The volume of the revolved zone.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//
// ****************************************************************************
 
double
avtRevolvedVolumeNoNamespaceConflict::GetZoneVolume(vtkCell *cell)
{
    int cellType = cell->GetCellType();
    if (cellType != VTK_TRIANGLE && cellType != VTK_QUAD && 
        cellType != VTK_PIXEL)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "The revolved volume is only support for triangles and"
                        " quadrilaterals.  %d is an invalid cell type.",
                         cellType);
           avtCallback::IssueWarning(msg);
        }
        haveIssuedWarning = true;
        return 0.;
    }

    double rv = 0.;
    vtkPoints *pts = cell->GetPoints();
    if (cellType == VTK_TRIANGLE)
    {
        double p0[3];
        double p1[3];
        double p2[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        double  x[3], y[3];
        x[0] = p0[0];
        x[1] = p1[0];
        x[2] = p2[0];
        y[0] = p0[1];
        y[1] = p1[1];
        y[2] = p2[1];

        rv = GetTriangleVolume(x, y);
    }
    else if (cellType == VTK_QUAD)
    {
        double p0[3];
        double p1[3];
        double p2[3];
        double p3[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        pts->GetPoint(3, p3);

        double  x[3], y[3];
        double  volume1, volume2;
     
        //
        // Split into two triangles (P0, P1, and P2), and (P0, P2, and P3) and
        // find their volumes.
        //
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p1[0];
        y[1] = p1[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume1 = GetTriangleVolume(x, y);
 
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume2 = GetTriangleVolume(x, y);
     
        rv = (volume1 + volume2);
    }
    else if (cellType == VTK_PIXEL)
    {
        double p0[3];
        double p1[3];
        double p2[3];
        double p3[3];
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        pts->GetPoint(3, p3);

        double  x[3], y[3];
        double  volume1, volume2;
     
        //
        // Split into two triangles (P0, P1, and P3), and (P0, P2, and P3) and
        // find their volumes.
        //
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p1[0];
        y[2] = p1[1];
        volume1 = GetTriangleVolume(x, y);
 
        x[0] = p0[0];
        y[0] = p0[1];
        x[1] = p3[0];
        y[1] = p3[1];
        x[2] = p2[0];
        y[2] = p2[1];
        volume2 = GetTriangleVolume(x, y);
     
        rv = (volume1 + volume2);
    }

    return rv;
}


// ****************************************************************************
//  Function: GetTriangleVolume
// 
//  Purpose:
//      Revolve a triangle around the y-axis.
// 
//  Arguments:
//      x       The x-coordinates of the triangle.
//      y       The y-coordinates of the triangle.
// 
//  Returns:    The volume of the revolved triangle.
// 
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//  
//  Modifications:
//      Akira Haddox, Wed Jul  2 12:27:24 PDT 2003
//      Dealt with triangles below or on the y-axis.
// 
// ****************************************************************************
 
double
avtRevolvedVolumeNoNamespaceConflict::GetTriangleVolume(double x[3], double y[3])
{
    int     i, j;
    double   cone01, cone02, cone12;
    double   slope01, slope02, slope12;
    double   volume;
    double   ls_x[2], ls_y[2];

    //
    // Check to see if we straddle the line y = 0, and break it up into
    // triangles which don't. Keep in mind the points are ordered
    // by x-coordinate.
    // 
    if (y[0] * y[1] < 0 || y[1] * y[2] < 0 || y[0] * y[2] < 0)
    {
        double oppositeX, oppositeY;
        double pt1X, pt2X, pt1Y, pt2Y;

        // Find the one point that is opposite the other two.
        if (y[1] * y[2] >= 0)
        {
            // The point is 0
            oppositeX = x[0];
            oppositeY = y[0];
            pt1X = x[1];
            pt1Y = y[1];
            pt2X = x[2];
            pt2Y = y[2];
        }
        else if(y[0] * y[2] >= 0)
        {
            // The point is 1
            oppositeX = x[1];
            oppositeY = y[1];
            pt1X = x[0];
            pt1Y = y[0];
            pt2X = x[2];
            pt2Y = y[2];
        }
        else
        {
            // The point is 2
            oppositeX = x[2];
            oppositeY = y[2];
            pt1X = x[0];
            pt1Y = y[0];
            pt2X = x[1];
            pt2Y = y[1];
        }
        
        //
        // Now take this information and find the two intersections.
        //
        double xInt1, xInt2;

        // Special cases: infinite slopes
        if (oppositeX == pt1X)
            xInt1 = oppositeX; 
        else
        {
            double nslope = (oppositeY - pt1Y) / (oppositeX - pt1X);
            xInt1 = oppositeX + oppositeY / nslope;
        }
            
        if (oppositeX == pt2X)
            xInt2 = oppositeX;
        else
        {
            double nslope = (oppositeY - pt2Y) / (oppositeX - pt2X);
            xInt2 = oppositeX + oppositeY / nslope;
        }   

        double v1, v2, v3;     
        
        //
        // Find the volume of the single triangle
        //
        x[0] = oppositeX;
        x[1] = xInt1;
        x[2] = xInt2;
        y[0] = oppositeY;
        y[1] = 0;
        y[2] = 0;

        v1 = GetTriangleVolume(x, y);
        
        //
        // Find the volume of the quad by splitting it into triangles 
        // (the intercepts to pt1, then {pt1, pt2, Intercept2}).
        //

        x[0] = pt1X;
        y[0] = pt1Y;
        v2 = GetTriangleVolume(x, y);
        
        x[1] = pt2X;
        y[1] = pt2Y;
        v3 = GetTriangleVolume(x, y);

        return v1 + v2 + v3;
    }
    
    //
    // Sort the points so that they are ordered by x-coordinate.  This will
    // make things much easier later.
    // 
    for (i = 0 ; i < 3 ; i++)
    {
        for (j = i ; j < 3 ; j++)
        {
            if (x[j] < x[i])
            {
                double tmp_x, tmp_y;
                tmp_x = x[j];
                tmp_y = y[j];
                x[j]  = x[i];
                y[j]  = y[i];
                x[i]  = tmp_x;
                y[i]  = tmp_y;
            }
        }
    }
 
    //
    // Revolve each of the line segments.
    //
    ls_x[0] = x[0];
    ls_y[0] = y[0];
    ls_x[1] = x[1];
    ls_y[1] = y[1];
    cone01 = RevolveLineSegment(ls_x, ls_y, &slope01);
 
    ls_x[1] = x[2];
    ls_y[1] = y[2];
    cone02 = RevolveLineSegment(ls_x, ls_y, &slope02);
 
    ls_x[0] = x[1];
    ls_y[0] = y[1];
    cone12 = RevolveLineSegment(ls_x, ls_y, &slope12);

    bool aboveY;
    if (y[0] < 0 || y[1] < 0 || y[2] < 0)
        aboveY = false;
    else
        aboveY = true;
    
    //
    // This is a little tricky and best shown by picture, but if slope01 is
    // greater than slope02, then P0P1 and P1P2 make up the top of the volume
    // and P0P2 make up the bottom.  If not, P0P1 and P1P2 make up the bottom
    // and P0P2 makes up the top.  Remember, the points are sorted by x-coords.
    // Also, if the slopes are equal, we have three collinnear points, but the
    // math actually works out fine, since the volume of the cone from P0P1
    // plus P1P2 equals P0P2.
    //
    // Note that if we're below the line y = 0, then this rule is reversed.
    //
    if (aboveY)
    {
        if (slope01 < slope02)
            volume = cone02 - cone01 - cone12;
        else
            volume = cone12 + cone01 - cone02;
    }
    else
    {
        if (slope01 < slope02)
            volume = cone12 + cone01 - cone02;
        else
            volume = cone02 - cone01 - cone12;

    }
 
    return volume;
}


// ****************************************************************************
//  Method: avtRevolvedVolumeNoNamespaceConflict::RevolveLineSegment
// 
//  Purpose:
//      Calculate the volume of the cone created by revolving a line segment
//      around the y-axis.
// 
//  Arguments:
//      x       The x-coordinates of the line segment.
//      y       The y-coordinates of the line segment.
//      slope   The slope of the line segments (Output Arg).
// 
//  Returns:    The volume of the (cropped) cone.
// 
//  Programmer: Hank Childs
//  Creation:   March 29, 2000
//  
// ****************************************************************************
 
double
avtRevolvedVolumeNoNamespaceConflict::RevolveLineSegment(double x[2], double y[2], double *slope)
{
    double   m, b;
    double   x_at_y_axis;
    double   radius, height;
    double   coneAll, coneCropped;
 
    //
    // Sort out degenerate cases, slope = 0, infinity
    //
    if (x[0] == x[1])
    {
        /* Vertical line */
        if (y[0] > y[1])
        {
            *slope = -FLT_MAX;
        }
        else
        {
            *slope = FLT_MAX;
        }
        return 0.;
    }
    if (y[0] == y[1])
    {
#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

        // 0-slope line, return cylinder volume, not cone 
        double  radius = y[0];
        double  height = x[1] - x[0];
        double  volume = M_PI * radius * radius * height;
        *slope = 0.;
        return (volume);
    }

    //
    // Calculate where the line segment will hit the line x = 0.
    // Note we have already taken care of degenerate cases.
    //
    m = (y[1] - y[0]) / (x[1] - x[0]);
    b = y[0] - m*x[0];
    x_at_y_axis = -b / m;
 
    //
    // We are now going to calculate the cone that contains are volume and
    // the tip of the cone that needs to be cropped off to give the volume.
    // Note that we have been very careful to construct right circular cones,
    // which have volume PI*r^2*h/3.
    //
    if (m < 0)
    {
        //
        //         . <= P0
        //
        //                . <= P1
        //                      x_at_y_axis
        //                          ||
        //                          \/
        // y-axis  -----------------.--------------------
        //
        radius = y[0];
        height = x_at_y_axis - x[0];
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[1];
        height = x_at_y_axis - x[1];
        coneCropped = M_PI * radius * radius * height / 3;
    }
    else  // m > 0 
    {
        //
        //                           . <= P1
        //
        //                       . <= P0
        //         x_at_y_axis
        //             ||
        //             \/
        // y-axis  ----.---------------------------------
        //
        radius = y[1];
        height = x[1] - x_at_y_axis;
        coneAll = M_PI * radius * radius * height / 3;
 
        radius = y[0];
        height = x[0] - x_at_y_axis;
        coneCropped = M_PI * radius * radius * height / 3;
    }
 
    *slope = m;
    return (coneAll - coneCropped);
}


// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK
// ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK*ENDHACK

// ****************************************************************************
//  Method: avtHistogramPlot constructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtHistogramPlot::avtHistogramPlot()
{
    HistogramFilter = NULL;
    amountFilter = NULL;

    renderer = avtSurfaceAndWireframeRenderer::New();
    avtCustomRenderer_p ren;
    CopyTo(ren, renderer);
    mapper = new avtUserDefinedMapper(ren);

    property = vtkProperty::New();
    property->SetAmbient(0.0);
    property->SetDiffuse(1.0);
 
    renderer->ScalarVisibilityOff();
    renderer->ResolveTopologyOff();
 
    renderer->EdgeStripsOff();
    renderer->EdgePolysOff();

    renderer->SurfaceVertsOff();
    renderer->SurfaceLinesOn();
    renderer->SurfaceStripsOff();
    renderer->SurfacePolysOn();

    fgColor[0] = 0.;
    fgColor[1] = 0.;
    fgColor[2] = 0.;
}


// ****************************************************************************
//  Method: avtHistogramPlot destructor
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtHistogramPlot::~avtHistogramPlot()
{
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }
    if (HistogramFilter != NULL)
    {
        delete HistogramFilter;
        HistogramFilter = NULL;
    }
    if (amountFilter != NULL)
    {
        delete amountFilter;
        amountFilter = NULL;
    }
    if (property != NULL)
    {
        property->Delete();
        property = NULL;
    }
}


// ****************************************************************************
//  Method:  avtHistogramPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtPlot*
avtHistogramPlot::Create()
{
    return new avtHistogramPlot;
}


// ****************************************************************************
//  Method: avtHistogramPlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtMapper *
avtHistogramPlot::GetMapper(void)
{
    return mapper;
}


// ****************************************************************************
//  Method: avtHistogramPlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Histogram plot.  
//      The output from this method is a query-able object.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Histogram plot has been applied.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtDataObject_p
avtHistogramPlot::ApplyOperators(avtDataObject_p input)
{
    if (HistogramFilter != NULL)
        delete HistogramFilter;
    if (amountFilter != NULL)
        delete amountFilter;

    avtDataAttributes &in_atts = input->GetInfo().GetAttributes();
    avtExprFilterNoNamespaceConflict *af = NULL;
    if (in_atts.GetTopologicalDimension() == 3)
    {
        af = new avtVolumeNoNamespaceConflict;
    }
    else
    {
        if (in_atts.GetSpatialDimension() == 2)
        {
            if (atts.GetTwoDAmount() == HistogramAttributes::Area)
                af = new avtAreaNoNamespaceConflict;
            else
                af = new avtRevolvedVolumeNoNamespaceConflict;
        }
        else
            af = new avtAreaNoNamespaceConflict;
    }
    
    af->SetOutputVariableName("_amounts");
    af->SetInput(input);
    amountFilter = af;

    HistogramFilter = new avtHistogramFilter;
    HistogramFilter->SetAttributes(atts);
    HistogramFilter->SetInput(af->GetOutput());
    return HistogramFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtHistogramPlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a Histogram plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Histogram plot has been applied.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

avtDataObject_p
avtHistogramPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtHistogramPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Histogram plot.  This
//      includes behavior like shifting towards or away from the screen.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 10:25:51 PDT 2003
//    I added code to set the window mode to curve.
//
// ****************************************************************************

void
avtHistogramPlot::CustomizeBehavior(void)
{
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);

    renderer->SetProperty(property); 
}


// ****************************************************************************
//  Method: avtHistogramPlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Histogram plot.
//
//  Arguments:
//      atts    The attributes for this Histogram plot.
//
//  Programmer: childs -- generated by xml2info
//  Creation:   Thu Jun 26 10:33:56 PDT 2003
//
// ****************************************************************************

void
avtHistogramPlot::SetAtts(const AttributeGroup *a)
{
    const HistogramAttributes *newAtts = (const HistogramAttributes *)a;

    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);

    atts = *newAtts;

    if (HistogramFilter != NULL)
        HistogramFilter->SetAttributes(*newAtts);

    double rgba[4];
    atts.GetColor().GetRgba(rgba);
    property->SetColor((float)rgba[0], (float)rgba[1], (float)rgba[2]);
    if (atts.GetOutputType() == HistogramAttributes::Block)
    {
        property->SetEdgeColor(fgColor[0], fgColor[1], fgColor[2]);
        property->EdgeVisibilityOn();
        renderer->EdgeLinesOn();
    }
    else
    {
        property->SetEdgeColor((float)rgba[0], (float)rgba[1], (float)rgba[2]);
        property->EdgeVisibilityOff();
        renderer->EdgeLinesOff();
    }
    property->SetLineWidth(LineWidth2Int(Int2LineWidth(atts.GetLineWidth())));
    property->SetLineStipplePattern(
                 LineStyle2StipplePattern(Int2LineStyle(atts.GetLineStyle())));
}


// ****************************************************************************
//  Method: avtHistogramPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   June 26, 2002
//
// ****************************************************************************
 
void
avtHistogramPlot::ReleaseData(void)
{
    avtLineDataPlot::ReleaseData();
 
    if (HistogramFilter != NULL)
    {
        HistogramFilter->ReleaseData();
    }
    if (amountFilter != NULL)
    {
        amountFilter->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtHistogramPlot::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the plot.
//
//  Returns:    true if the plot should be redrawn after the color is set.
//
//  Programmer: Hank Childs
//  Creation:   June 26, 2003
//
// ****************************************************************************

bool
avtHistogramPlot::SetForegroundColor(const double *fg)
{
    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    if (atts.GetOutputType() == HistogramAttributes::Block)
        property->SetEdgeColor(fgColor[0], fgColor[1], fgColor[2]);

    return (atts.GetOutputType() == HistogramAttributes::Block);
}


