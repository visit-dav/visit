/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

// Modifications:
//
//   Hank Childs, Wed Apr 14 20:28:38 PDT 2004
//   Do not assert.
//

// Scales all values by 1/scalar
inline VerdictVector& VerdictVector::operator/=(const VERDICT_REAL scalar)
{
  //assert (scalar != 0);
  if (scalar != 0)
  {
    xVal /= scalar;
    yVal /= scalar;
    zVal /= scalar;
  }
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


// Modifications:
//
//   Hank Childs, Wed Apr 14 20:28:38 PDT 2004
//   Do not assert.
//

inline VERDICT_REAL interior_angle( VERDICT_REAL vec1[], VERDICT_REAL vec2[] )
{
    VERDICT_REAL len1, len2, cosAngle, angleRad;

    if (  ((len1 = length(vec1)) > 0 ) && ((len2 = length(vec2)) > 0 ) )
    {
        cosAngle = dot_product(vec1, vec2) / (len1 * len2);
    }
    else
    {
      /*
        assert(len1 > 0);
        assert(len2 > 0);
       */
       cosAngle = 0.;
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
        //assert(cosAngle < 1.0001 && cosAngle > -1.0001);
        angleRad = 0.;
    }

    return( (angleRad * 180.) / VERDICT_PI );
}

#endif


#endif  // VERDICT_INC_LIB



