!cccccccccccccccccccccccccccccccccccccccccccccccccccccccc -*- fortran -*-
!
! VisIt Fortran Simulation Interface V2 include file
!
! Programmer: Brad Whitlock
! Date: Fri Feb 19 15:44:42 PST 2010
!
! Notes:
!  1. All functions, unless noted, return either VISIT_OKAY or 
!     VISIT_ERROR.
!  2. Data functions take "visit_handle" as an argument, which is an
!     integer created by calling an alloc function.
!  3. Any place where "string" is mentioned as a function argument, it
!     means array of character. All string arguments are followed by a
!     length argument that specifies the length of the character array.
!  4. If you alloc an object and put it into another object, the 
!     container object will own the alloc'd object and you won't have
!     to free it. If you don't put the object into another object then
!     you will need to free it unless you are returning the object to
!     VisIt's data callback functions.
!
! Modifications:
!   Brad Whitlock, Fri Jul 22 09:47:01 PDT 2011
!   Expose visitdetectinputwithtimeout
!
!   Brad Whitlock, Wed Jul 27 10:46:13 PDT 2011
!   Add visitgetsockets.
!
!   Brad Whitlock, Thu Aug 11 12:03:43 PDT 2011
!   Added functions to set/get ghost zones on meshes.
!
!   Brad Whitlock, Thu Dec 15 15:09:25 PST 2011
!   I added hidefromgui and numcomponents attributes for variable 
!   metadata.
!
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

!... Commonly useful declarations
      external visitstrcmp
      integer  visitstrcmp

!... Null string values
      character*12 VISIT_F77NULLSTRING
      parameter   (VISIT_F77NULLSTRING         = "NULLSTRING")
      integer      VISIT_F77NULLSTRINGLEN
      parameter   (VISIT_F77NULLSTRINGLEN      = 10)

!... Function return values
      integer      VISIT_ERROR
      parameter   (VISIT_ERROR                 = 0)
      integer      VISIT_OKAY
      parameter   (VISIT_OKAY                  = 1)

!... Handle values
      integer      VISIT_INVALID_HANDLE
      parameter   (VISIT_INVALID_HANDLE        = -1)

!... Mesh types
      integer      VISIT_MESHTYPE_UNKNOWN
      parameter   (VISIT_MESHTYPE_UNKNOWN      = 0)
      integer      VISIT_MESHTYPE_RECTILINEAR
      parameter   (VISIT_MESHTYPE_RECTILINEAR  = 1)
      integer      VISIT_MESHTYPE_CURVILINEAR
      parameter   (VISIT_MESHTYPE_CURVILINEAR  = 2)
      integer      VISIT_MESHTYPE_UNSTRUCTURED
      parameter   (VISIT_MESHTYPE_UNSTRUCTURED = 3)
      integer      VISIT_MESHTYPE_POINT
      parameter   (VISIT_MESHTYPE_POINT        = 4)
      integer      VISIT_MESHTYPE_CSG
      parameter   (VISIT_MESHTYPE_CSG          = 5)
      integer      VISIT_MESHTYPE_AMR
      parameter   (VISIT_MESHTYPE_AMR          = 6)

!... Centering
      integer      VISIT_VARCENTERING_NODE
      parameter   (VISIT_VARCENTERING_NODE     = 0)
      integer      VISIT_VARCENTERING_ZONE
      parameter   (VISIT_VARCENTERING_ZONE     = 1)

!... Variable Type
      integer      VISIT_VARTYPE_UNKNOWN             
      parameter   (VISIT_VARTYPE_UNKNOWN             = 0)
      integer      VISIT_VARTYPE_SCALAR              
      parameter   (VISIT_VARTYPE_SCALAR              = 1)
      integer      VISIT_VARTYPE_VECTOR              
      parameter   (VISIT_VARTYPE_VECTOR              = 2)
      integer      VISIT_VARTYPE_TENSOR              
      parameter   (VISIT_VARTYPE_TENSOR              = 3)
      integer      VISIT_VARTYPE_SYMMETRIC_TENSOR    
      parameter   (VISIT_VARTYPE_SYMMETRIC_TENSOR    = 4)
      integer      VISIT_VARTYPE_MATERIAL            
      parameter   (VISIT_VARTYPE_MATERIAL            = 5)
      integer      VISIT_VARTYPE_MATSPECIES          
      parameter   (VISIT_VARTYPE_MATSPECIES          = 6)
      integer      VISIT_VARTYPE_LABEL          
      parameter   (VISIT_VARTYPE_LABEL               = 7)
      integer      VISIT_VARTYPE_ARRAY          
      parameter   (VISIT_VARTYPE_ARRAY               = 8)
      integer      VISIT_VARTYPE_MESH          
      parameter   (VISIT_VARTYPE_MESH                = 100)
      integer      VISIT_VARTYPE_CURVE          
      parameter   (VISIT_VARTYPE_CURVE               = 101)

!... Command Argument Type
      integer      VISIT_CMDARG_NONE                 
      parameter   (VISIT_CMDARG_NONE                 = 0)
      integer      VISIT_CMDARG_INT                  
      parameter   (VISIT_CMDARG_INT                  = 1)
      integer      VISIT_CMDARG_FLOAT                
      parameter   (VISIT_CMDARG_FLOAT                = 2)
      integer      VISIT_CMDARG_STRING               
      parameter   (VISIT_CMDARG_STRING               = 3)

!... Simulation Mode
      integer      VISIT_SIMMODE_UNKNOWN             
      parameter   (VISIT_SIMMODE_UNKNOWN             = 0)
      integer      VISIT_SIMMODE_RUNNING             
      parameter   (VISIT_SIMMODE_RUNNING             = 1)
      integer      VISIT_SIMMODE_STOPPED             
      parameter   (VISIT_SIMMODE_STOPPED             = 2)

!... Data Type
      integer      VISIT_DATATYPE_CHAR               
      parameter   (VISIT_DATATYPE_CHAR               = 0)
      integer      VISIT_DATATYPE_INT                
      parameter   (VISIT_DATATYPE_INT                = 1)
      integer      VISIT_DATATYPE_FLOAT              
      parameter   (VISIT_DATATYPE_FLOAT              = 2)
      integer      VISIT_DATATYPE_DOUBLE             
      parameter   (VISIT_DATATYPE_DOUBLE             = 3)

!... Array Owner
      integer      VISIT_OWNER_SIM                   
      parameter   (VISIT_OWNER_SIM                   = 0)
      integer      VISIT_OWNER_VISIT                 
      parameter   (VISIT_OWNER_VISIT                 = 1)
      integer      VISIT_OWNER_COPY                 
      parameter   (VISIT_OWNER_COPY                  = 2)

!... Cell Types
      integer      VISIT_CELL_BEAM
      parameter   (VISIT_CELL_BEAM                   = 0)
      integer      VISIT_CELL_TRI
      parameter   (VISIT_CELL_TRI                    = 1)
      integer      VISIT_CELL_QUAD
      parameter   (VISIT_CELL_QUAD                   = 2)
      integer      VISIT_CELL_TET
      parameter   (VISIT_CELL_TET                    = 3)
      integer      VISIT_CELL_PYR
      parameter   (VISIT_CELL_PYR                    = 4)
      integer      VISIT_CELL_WEDGE
      parameter   (VISIT_CELL_WEDGE                  = 5)
      integer      VISIT_CELL_HEX
      parameter   (VISIT_CELL_HEX                    = 6)
      integer      VISIT_CELL_POINT
      parameter   (VISIT_CELL_POINT                  = 7)
      integer      VISIT_CELL_POLYHEDRON
      parameter   (VISIT_CELL_POLYHEDRON             = 8)

!... Coordinate modes 
      integer      VISIT_COORD_MODE_SEPARATE
      parameter   (VISIT_COORD_MODE_SEPARATE         = 0)
      integer      VISIT_COORD_MODE_INTERLEAVED
      parameter   (VISIT_COORD_MODE_INTERLEAVED      = 1)

!... Ghost cell types
      integer       VISIT_GHOSTCELL_REAL                
      parameter    (VISIT_GHOSTCELL_REAL              = 0)
      integer       VISIT_GHOSTCELL_INTERIOR_BOUNDARY     
      parameter    (VISIT_GHOSTCELL_INTERIOR_BOUNDARY = 1)
      integer       VISIT_GHOSTCELL_EXTERIOR_BOUNDARY     
      parameter    (VISIT_GHOSTCELL_EXTERIOR_BOUNDARY = 2)
      integer       VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY 
      parameter    (VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY = 3)
      integer       VISIT_GHOSTCELL_REDUCED_CONNECTIVITY  
      parameter    (VISIT_GHOSTCELL_REDUCED_CONNECTIVITY = 4)
      integer       VISIT_GHOSTCELL_BLANK                 
      parameter    (VISIT_GHOSTCELL_BLANK             = 5)

!... Image formats 
      integer      VISIT_IMAGEFORMAT_BMP
      parameter   (VISIT_IMAGEFORMAT_BMP             = 0)
      integer      VISIT_IMAGEFORMAT_JPEG
      parameter   (VISIT_IMAGEFORMAT_JPEG            = 1)
      integer      VISIT_IMAGEFORMAT_PNG
      parameter   (VISIT_IMAGEFORMAT_PNG             = 2)
      integer      VISIT_IMAGEFORMAT_POVRAY 
      parameter   (VISIT_IMAGEFORMAT_POVRAY          = 3)
      integer      VISIT_IMAGEFORMAT_PPM
      parameter   (VISIT_IMAGEFORMAT_PPM             = 4)
      integer      VISIT_IMAGEFORMAT_RGB
      parameter   (VISIT_IMAGEFORMAT_RGB             = 5)
      integer      VISIT_IMAGEFORMAT_TIFF
      parameter   (VISIT_IMAGEFORMAT_TIFF            = 6)

! Definitions for CSG boundary types 
!   Designed so low-order 16 bits are unused
!
!   The last few characters of the symbol are intended
!   to indicate the representational form of the surface type
!
!   G = generalized form  (n values, depends on surface type)
!   P = point (3 values, x,y,z in 3D, 2 values in 2D x,y)
!   N = normal (3 values, Nx,Ny,Nz in 3D, 2 values in 2D Nx,Ny)
!   R = radius (1 value)
!   A = angle (1 value in degrees)
!   L = length (1 value)
!   X = x-intercept (1 value)
!   Y = y-intercept (1 value)
!   Z = z-intercept (1 value)
!   K = arbitrary integer
!   F = planar face defined by point-normal pair (6 values)
      integer    VISIT_CSG_QUADRIC_G     
      parameter (VISIT_CSG_QUADRIC_G           = 16777216       )
      integer    VISIT_CSG_SPHERE_PR     
      parameter (VISIT_CSG_SPHERE_PR           = 33619968       )
      integer    VISIT_CSG_ELLIPSOID_PRRR
      parameter (VISIT_CSG_ELLIPSOID_PRRR      = 33685504       )
      integer    VISIT_CSG_PLANE_G       
      parameter (VISIT_CSG_PLANE_G             = 50331648       )
      integer    VISIT_CSG_PLANE_X       
      parameter (VISIT_CSG_PLANE_X             = 50397184       )
      integer    VISIT_CSG_PLANE_Y       
      parameter (VISIT_CSG_PLANE_Y             = 50462720       )
      integer    VISIT_CSG_PLANE_Z       
      parameter (VISIT_CSG_PLANE_Z             = 50528256       )
      integer    VISIT_CSG_PLANE_PN      
      parameter (VISIT_CSG_PLANE_PN            = 50593792       )
      integer    VISIT_CSG_PLANE_PPP     
      parameter (VISIT_CSG_PLANE_PPP           = 50659328       )
      integer    VISIT_CSG_CYLINDER_PNLR 
      parameter (VISIT_CSG_CYLINDER_PNLR       = 67108864       )
      integer    VISIT_CSG_CYLINDER_PPR  
      parameter (VISIT_CSG_CYLINDER_PPR        = 67174400       )
      integer    VISIT_CSG_BOX_XYZXYZ    
      parameter (VISIT_CSG_BOX_XYZXYZ          = 83886080       )
      integer    VISIT_CSG_CONE_PNLA     
      parameter (VISIT_CSG_CONE_PNLA           = 100663296      )
      integer    VISIT_CSG_CONE_PPA      
      parameter (VISIT_CSG_CONE_PPA            = 100728832      )
      integer    VISIT_CSG_POLYHEDRON_KF 
      parameter (VISIT_CSG_POLYHEDRON_KF       = 117440512      )
      integer    VISIT_CSG_HEX_6F        
      parameter (VISIT_CSG_HEX_6F              = 117506048      )
      integer    VISIT_CSG_TET_4F        
      parameter (VISIT_CSG_TET_4F              = 117571584      )
      integer    VISIT_CSG_PYRAMID_5F    
      parameter (VISIT_CSG_PYRAMID_5F          = 117637120      )
      integer    VISIT_CSG_PRISM_5F      
      parameter (VISIT_CSG_PRISM_5F            = 117702656      )

!... Definitions for 2D CSG boundary types
      integer    VISIT_CSG_QUADRATIC_G   
      parameter (VISIT_CSG_QUADRATIC_G         = 134217728      )
      integer    VISIT_CSG_CIRCLE_PR     
      parameter (VISIT_CSG_CIRCLE_PR           = 150994944      )
      integer    VISIT_CSG_ELLIPSE_PRR   
      parameter (VISIT_CSG_ELLIPSE_PRR         = 151060480      )
      integer    VISIT_CSG_LINE_G        
      parameter (VISIT_CSG_LINE_G              = 167772160      )
      integer    VISIT_CSG_LINE_X        
      parameter (VISIT_CSG_LINE_X              = 167837696      )
      integer    VISIT_CSG_LINE_Y        
      parameter (VISIT_CSG_LINE_Y              = 167903232      )
      integer    VISIT_CSG_LINE_PN       
      parameter (VISIT_CSG_LINE_PN             = 167968768      )
      integer    VISIT_CSG_LINE_PP       
      parameter (VISIT_CSG_LINE_PP             = 168034304      )
      integer    VISIT_CSG_BOX_XYXY      
      parameter (VISIT_CSG_BOX_XYXY            = 184549376      )
      integer    VISIT_CSG_ANGLE_PNLA    
      parameter (VISIT_CSG_ANGLE_PNLA          = 201326592      )
      integer    VISIT_CSG_ANGLE_PPA     
      parameter (VISIT_CSG_ANGLE_PPA           = 201392128      )
      integer    VISIT_CSG_POLYGON_KP    
      parameter (VISIT_CSG_POLYGON_KP          = 218103808      )
      integer    VISIT_CSG_TRI_3P        
      parameter (VISIT_CSG_TRI_3P              = 218169344      )
      integer    VISIT_CSG_QUAD_4P       
      parameter (VISIT_CSG_QUAD_4P             = 218234880      )

!... Definitions for CSG Region operators
      integer    VISIT_CSG_INNER         
      parameter (VISIT_CSG_INNER               = 2130706432     )
      integer    VISIT_CSG_OUTER         
      parameter (VISIT_CSG_OUTER               = 2130771968     )
      integer    VISIT_CSG_ON            
      parameter (VISIT_CSG_ON                  = 2130837504     )
      integer    VISIT_CSG_UNION         
      parameter (VISIT_CSG_UNION               = 2130903040     )
      integer    VISIT_CSG_INTERSECT     
      parameter (VISIT_CSG_INTERSECT           = 2130968576     )
      integer    VISIT_CSG_DIFF          
      parameter (VISIT_CSG_DIFF                = 2131034112     )
      integer    VISIT_CSG_COMPLIMENT    
      parameter (VISIT_CSG_COMPLIMENT          = 2131099648     )
      integer    VISIT_CSG_XFORM         
      parameter (VISIT_CSG_XFORM               = 2131165184     )
      integer    VISIT_CSG_SWEEP         
      parameter (VISIT_CSG_SWEEP               = 2131230720     )

!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
! Control interface functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitattemptconnection()
      external visitattemptconnection
      integer  visitattemptconnection

! visitclosetracefile()
      external visitclosetracefile
      integer  visitclosetracefile

! visitdebug1(string str, int lstr)
      external visitdebug1
      integer  visitdebug1

! visitdebug2(string str, int lstr)
      external visitdebug2
      integer  visitdebug2

! visitdebug3(string str, int lstr)
      external visitdebug3
      integer  visitdebug3

! visitdebug4(string str, int lstr)
      external visitdebug4
      integer  visitdebug4

! visitdebug5(string str, int lstr)
      external visitdebug5
      integer  visitdebug5

! visitdetectinput(int blocking, int consoledesc)
!     returns int codes -5 through 3
      external visitdetectinput
      integer  visitdetectinput

! visitdetectinputwithtimeout(int blocking, int micsec, int consoledesc)
!     returns int codes -5 through 3
      external visitdetectinputwithtimeout
      integer  visitdetectinputwithtimeout

! visitdisconnect()
      external visitdisconnect
      integer  visitdisconnect

! visitenablesynchronize(int val)
      external visitenablesynchronize
      integer  visitenablesynchronize

! visitexecutecommand(string command, int lcommand)
      external visitexecutecommand
      integer  visitexecutecommand

! visitgetlasterror(string dest, int ldest)
      external visitgetlasterror
      integer  visitgetlasterror

! visitgetsockets(int lsocket, int csocket)
      external visitgetsockets
      integer  visitgetsockets

! visitinitializesim(
!    string name, int lname,
!    string comment, int lcomment,
!    string path, int lpath,
!    string inputfile, int linputfile,
!    string uifile, int luifile,
!    string absoluteFilename, int labsoluteFilename)
      external visitinitializesim
      integer  visitinitializesim

! visitisconnected()
      external visitisconnected
      integer  visitisconnected

! visitopentracefile(string name, int lname)
      external visitopentracefile
      integer  visitopentracefile

! visitprocessenginecommand()
      external visitprocessenginecommand
      integer  visitprocessenginecommand

! visitsavewindow(string filename, int lfilename, int width, int height, int format)
      external visitsavewindow
      integer  visitsavewindow

! visitsetdirectory(string dir, int ldir)
      external visitsetdirectory
      integer  visitsetdirectory

! visitsetoptions(string opt, int lopt)
      external visitsetoptions
      integer  visitsetoptions

! visitsetparallel(int val)
      external visitsetparallel
      integer  visitsetparallel

! visitsetparallelrank(int rank)
      external visitsetparallelrank
      integer  visitsetparallelrank

! visitsetupenv()
      external visitsetupenv
      integer  visitsetupenv

! visitsynchronize()
      external visitsynchronize
      integer  visitsynchronize

! visittimestepchanged()
      external visittimestepchanged
      integer  visittimestepchanged

! visitupdateplots()
      external visitupdateplots
      integer  visitupdateplots


!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
! Metadata functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! VisIt_CommandMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdcmdalloc(visit_handle h)
      external visitmdcmdalloc
      integer  visitmdcmdalloc

! visitmdcmdfree(visit_handle h)
      external visitmdcmdfree
      integer  visitmdcmdfree

! visitmdcmdsetname(visit_handle h, string val, int lval)
      external visitmdcmdsetname
      integer  visitmdcmdsetname

! visitmdcmdgetname(visit_handle h, string val, int lval)
      external visitmdcmdgetname
      integer  visitmdcmdgetname

! VisIt_CurveMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdcurvealloc(visit_handle h)
      external visitmdcurvealloc
      integer  visitmdcurvealloc

! visitmdcurvefree(visit_handle h)
      external visitmdcurvefree
      integer  visitmdcurvefree

! visitmdcurvesetname(visit_handle h, string val, int lval)
      external visitmdcurvesetname
      integer  visitmdcurvesetname

! visitmdcurvegetname(visit_handle h, string val, int lval)
      external visitmdcurvegetname
      integer  visitmdcurvegetname

! visitmdcurvesetxunits(visit_handle h, string val, int lval)
      external visitmdcurvesetxunits
      integer  visitmdcurvesetxunits

! visitmdcurvegetxunits(visit_handle h, string val, int lval)
      external visitmdcurvegetxunits
      integer  visitmdcurvegetxunits

! visitmdcurvesetyunits(visit_handle h, string val, int lval)
      external visitmdcurvesetyunits
      integer  visitmdcurvesetyunits

! visitmdcurvegetyunits(visit_handle h, string val, int lval)
      external visitmdcurvegetyunits
      integer  visitmdcurvegetyunits

! visitmdcurvesetxlabel(visit_handle h, string val, int lval)
      external visitmdcurvesetxlabel
      integer  visitmdcurvesetxlabel

! visitmdcurvegetxlabel(visit_handle h, string val, int lval)
      external visitmdcurvegetxlabel
      integer  visitmdcurvegetxlabel

! visitmdcurvesetylabel(visit_handle h, string val, int lval)
      external visitmdcurvesetylabel
      integer  visitmdcurvesetylabel

! visitmdcurvegetylabel(visit_handle h, string val, int lval)
      external visitmdcurvegetylabel
      integer  visitmdcurvegetylabel

! VisIt_ExpressionMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdexpralloc(visit_handle h)
      external visitmdexpralloc
      integer  visitmdexpralloc

! visitmdexprfree(visit_handle h)
      external visitmdexprfree
      integer  visitmdexprfree

! visitmdexprsetname(visit_handle h, string val, int lval)
      external visitmdexprsetname
      integer  visitmdexprsetname

! visitmdexprgetname(visit_handle h, string val, int lval)
      external visitmdexprgetname
      integer  visitmdexprgetname

! visitmdexprsetdefinition(visit_handle h, string val, int lval)
      external visitmdexprsetdefinition
      integer  visitmdexprsetdefinition

! visitmdexprgetdefinition(visit_handle h, string val, int lval)
      external visitmdexprgetdefinition
      integer  visitmdexprgetdefinition

! visitmdexprsettype(visit_handle h, int val)
      external visitmdexprsettype
      integer  visitmdexprsettype

! visitmdexprgettype(visit_handle h, int val)
      external visitmdexprgettype
      integer  visitmdexprgettype


! VisIt_MaterialMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdmatalloc(visit_handle h)
      external visitmdmatalloc
      integer  visitmdmatalloc

! visitmdmatfree(visit_handle h)
      external visitmdmatfree
      integer  visitmdmatfree

! visitmdmatsetname(visit_handle h, string val, int lval)
      external visitmdmatsetname
      integer  visitmdmatsetname

! visitmdmatgetname(visit_handle h, string val, int lval)
      external visitmdmatgetname
      integer  visitmdmatgetname

! visitmdmatsetmeshname(visit_handle h, string val, int lval)
      external visitmdmatsetmeshname
      integer  visitmdmatsetmeshname

! visitmdmatgetmeshname(visit_handle h, string val, int lval)
      external visitmdmatgetmeshname
      integer  visitmdmatgetmeshname

! visitmdmataddmaterialname(visit_handle h, string val, int lval)
      external visitmdmataddmaterialname
      integer  visitmdmataddmaterialname

! visitmdmatgetnummaterialname(visit_handle h, int val)
      external visitmdmatgetnummaterialname
      integer  visitmdmatgetnummaterialname

! visitmdmatgetmaterialname(visit_handle h, int i, string val, int lval)
      external visitmdmatgetmaterialname
      integer  visitmdmatgetmaterialname


! VisIt_MeshMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdmeshalloc(visit_handle h)
      external visitmdmeshalloc
      integer  visitmdmeshalloc

! visitmdmeshfree(visit_handle h)
      external visitmdmeshfree
      integer  visitmdmeshfree

! visitmdmeshsetname(visit_handle h, string val, int lval)
      external visitmdmeshsetname
      integer  visitmdmeshsetname

! visitmdmeshgetname(visit_handle h, string val, int lval)
      external visitmdmeshgetname
      integer  visitmdmeshgetname

! visitmdmeshsetmeshtype(visit_handle h, int val)
      external visitmdmeshsetmeshtype
      integer  visitmdmeshsetmeshtype

! visitmdmeshgetmeshtype(visit_handle h, int val)
      external visitmdmeshgetmeshtype
      integer  visitmdmeshgetmeshtype

! visitmdmeshsettopologicaldim(visit_handle h, int val)
      external visitmdmeshsettopologicaldim
      integer  visitmdmeshsettopologicaldim

! visitmdmeshgettopologicaldim(visit_handle h, int val)
      external visitmdmeshgettopologicaldim
      integer  visitmdmeshgettopologicaldim

! visitmdmeshsetspatialdim(visit_handle h, int val)
      external visitmdmeshsetspatialdim
      integer  visitmdmeshsetspatialdim

! visitmdmeshgetspatialdim(visit_handle h, int val)
      external visitmdmeshgetspatialdim
      integer  visitmdmeshgetspatialdim

! visitmdmeshsetnumdomains(visit_handle h, int val)
      external visitmdmeshsetnumdomains
      integer  visitmdmeshsetnumdomains

! visitmdmeshgetnumdomains(visit_handle h, int val)
      external visitmdmeshgetnumdomains
      integer  visitmdmeshgetnumdomains

! visitmdmeshsetdomaintitle(visit_handle h, string val, int lval)
      external visitmdmeshsetdomaintitle
      integer  visitmdmeshsetdomaintitle

! visitmdmeshgetdomaintitle(visit_handle h, string val, int lval)
      external visitmdmeshgetdomaintitle
      integer  visitmdmeshgetdomaintitle

! visitmdmeshsetdomainpiecename(visit_handle h, string val, int lval)
      external visitmdmeshsetdomainpiecename
      integer  visitmdmeshsetdomainpiecename

! visitmdmeshgetdomainpiecename(visit_handle h, string val, int lval)
      external visitmdmeshgetdomainpiecename
      integer  visitmdmeshgetdomainpiecename

! visitmdmeshadddomainname(visit_handle h, string val, int lval)
      external visitmdmeshadddomainname
      integer  visitmdmeshadddomainname

! visitmdmeshgetnumdomainname(visit_handle h, int val)
      external visitmdmeshgetnumdomainname
      integer  visitmdmeshgetnumdomainname

! visitmdmeshgetdomainname(visit_handle h, int i, string val, int lval)
      external visitmdmeshgetdomainname
      integer  visitmdmeshgetdomainname

! visitmdmeshsetnumgroups(visit_handle h, int val)
      external visitmdmeshsetnumgroups
      integer  visitmdmeshsetnumgroups

! visitmdmeshgetnumgroups(visit_handle h, int val)
      external visitmdmeshgetnumgroups
      integer  visitmdmeshgetnumgroups

! visitmdmeshsetgrouptitle(visit_handle h, string val, int lval)
      external visitmdmeshsetgrouptitle
      integer  visitmdmeshsetgrouptitle

! visitmdmeshgetgrouptitle(visit_handle h, string val, int lval)
      external visitmdmeshgetgrouptitle
      integer  visitmdmeshgetgrouptitle

! visitmdmeshsetgrouppiecename(visit_handle h, string val, int lval)
      external visitmdmeshsetgrouppiecename
      integer  visitmdmeshsetgrouppiecename

! visitmdmeshgetgrouppiecename(visit_handle h, string val, int lval)
      external visitmdmeshgetgrouppiecename
      integer  visitmdmeshgetgrouppiecename

! visitmdmeshsetgroupids(visit_handle h, int val)
      external visitmdmeshsetgroupids
      integer  visitmdmeshsetgroupids

! visitmdmeshgetnumgroupids(visit_handle h, int n)
      external visitmdmeshgetnumgroupids
      integer  visitmdmeshgetnumgroupids

! visitmdmeshgetgroupids(visit_handle h, int i, int val)
      external visitmdmeshgetgroupids
      integer  visitmdmeshgetgroupids

! visitmdmeshsetxunits(visit_handle h, string val, int lval)
      external visitmdmeshsetxunits
      integer  visitmdmeshsetxunits

! visitmdmeshgetxunits(visit_handle h, string val, int lval)
      external visitmdmeshgetxunits
      integer  visitmdmeshgetxunits

! visitmdmeshsetyunits(visit_handle h, string val, int lval)
      external visitmdmeshsetyunits
      integer  visitmdmeshsetyunits

! visitmdmeshgetyunits(visit_handle h, string val, int lval)
      external visitmdmeshgetyunits
      integer  visitmdmeshgetyunits

! visitmdmeshsetzunits(visit_handle h, string val, int lval)
      external visitmdmeshsetzunits
      integer  visitmdmeshsetzunits

! visitmdmeshgetzunits(visit_handle h, string val, int lval)
      external visitmdmeshgetzunits
      integer  visitmdmeshgetzunits

! visitmdmeshsetxlabel(visit_handle h, string val, int lval)
      external visitmdmeshsetxlabel
      integer  visitmdmeshsetxlabel

! visitmdmeshgetxlabel(visit_handle h, string val, int lval)
      external visitmdmeshgetxlabel
      integer  visitmdmeshgetxlabel

! visitmdmeshsetylabel(visit_handle h, string val, int lval)
      external visitmdmeshsetylabel
      integer  visitmdmeshsetylabel

! visitmdmeshgetylabel(visit_handle h, string val, int lval)
      external visitmdmeshgetylabel
      integer  visitmdmeshgetylabel

! visitmdmeshsetzlabel(visit_handle h, string val, int lval)
      external visitmdmeshsetzlabel
      integer  visitmdmeshsetzlabel

! visitmdmeshgetzlabel(visit_handle h, string val, int lval)
      external visitmdmeshgetzlabel
      integer  visitmdmeshgetzlabel

! visitmdmeshsetcellorigin(visit_handle h, int origin)
      external visitmdmeshsetcellorigin
      integer visitmdmeshsetcellorigin

! visitmdmeshgetcellorigin(visit_handle h, int origin)
      external visitmdmeshgetcellorigin
      integer visitmdmeshgetcellorigin

! visitmdmeshsetnodeorigin(visit_handle h, int origin)
      external visitmdmeshsetnodeorigin
      integer visitmdmeshsetnodeorigin

! visitmdmeshgetnodeorigin(visit_handle h, int origin)
      external visitmdmeshgetnodeorigin
      integer visitmdmeshgetnodeorigin

! visitmdmeshsethasspatialextents(visit_handle h, int hasSE)
      external visitmdmeshsethasspatialextents
      integer visitmdmeshsethasspatialextents

! visitmdmeshgethasspatialextents(visit_handle h, int hasSE)
      external visitmdmeshgethasspatialextents
      integer visitmdmeshgethasspatialextents

! visitmdmeshsetspatialextents(visit_handle h, double extents[6])
      external visitmdmeshsetspatialextents
      integer visitmdmeshsetspatialextents

! visitmdmeshgetspatialextents(visit_handle h, double extents[6])
      external visitmdmeshgetspatialextents
      integer visitmdmeshgetspatialextents


! VisIt_NameList functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitnamelistalloc(visit_handle h)
      external visitnamelistalloc
      integer  visitnamelistalloc

! visitnamelistfree(visit_handle h)
      external visitnamelistfree
      integer  visitnamelistfree

! visitnamelistaddname(visit_handle h, string val, int lval)
      external visitnamelistaddname
      integer  visitnamelistaddname

! visitnamelistgetnumname(visit_handle h, int val)
      external visitnamelistgetnumname
      integer  visitnamelistgetnumname

! visitnamelistgetname(visit_handle h, int i, string val, int lval)
      external visitnamelistgetname
      integer  visitnamelistgetname


! VisIt_SimulationMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdsimalloc(visit_handle h)
      external visitmdsimalloc
      integer  visitmdsimalloc

! visitmdsimfree(visit_handle h)
      external visitmdsimfree
      integer  visitmdsimfree

! visitmdsimsetmode(visit_handle h, int mode)
      external visitmdsimsetmode
      integer  visitmdsimsetmode

! visitmdsimsetcycletime(visit_handle h, int cycle, double time)
      external visitmdsimsetcycletime
      integer  visitmdsimsetcycletime

! visitmdsimaddmesh(visit_handle h, visit_handle md)
      external visitmdsimaddmesh
      integer  visitmdsimaddmesh

! visitmdsimaddvariable(visit_handle h, visit_handle md)
      external visitmdsimaddvariable
      integer  visitmdsimaddvariable

! visitmdsimaddmaterial(visit_handle h, visit_handle md)
      external visitmdsimaddmaterial
      integer  visitmdsimaddmaterial

! visitmdsimaddcurve(visit_handle h, visit_handle md)
      external visitmdsimaddcurve
      integer  visitmdsimaddcurve

! visitmdsimaddexpression(visit_handle h, visit_handle md)
      external visitmdsimaddexpression
      integer  visitmdsimaddexpression

! visitmdsimaddspecies(visit_handle h, visit_handle md)
      external visitmdsimaddspecies
      integer  visitmdsimaddspecies

! visitmdsimaddgenericcommand(visit_handle h, visit_handle md)
      external visitmdsimaddgenericcommand
      integer  visitmdsimaddgenericcommand

! visitmdsimaddcustomcommand(visit_handle h, visit_handle md)
      external visitmdsimaddcustomcommand
      integer  visitmdsimaddcustomcommand


! VisIt_SpeciesMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdspeciesalloc(visit_handle h)
      external visitmdspeciesalloc
      integer  visitmdspeciesalloc

! visitmdspeciesfree(visit_handle h)
      external visitmdspeciesfree
      integer  visitmdspeciesfree

! visitmdspeciessetname(visit_handle h, string val, int lval)
      external visitmdspeciessetname
      integer  visitmdspeciessetname

! visitmdspeciesgetname(visit_handle h, string val, int lval)
      external visitmdspeciesgetname
      integer  visitmdspeciesgetname

! visitmdspeciessetmeshname(visit_handle h, string val, int lval)
      external visitmdspeciessetmeshname
      integer  visitmdspeciessetmeshname

! visitmdspeciesgetmeshname(visit_handle h, string val, int lval)
      external visitmdspeciesgetmeshname
      integer  visitmdspeciesgetmeshname

! visitmdspeciessetmaterialname(visit_handle h, string val, int lval)
      external visitmdspeciessetmaterialname
      integer  visitmdspeciessetmaterialname

! visitmdspeciesgetmaterialname(visit_handle h, string val, int lval)
      external visitmdspeciesgetmaterialname
      integer  visitmdspeciesgetmaterialname

! visitmdspeciesaddspeciesname(visit_handle h, visit_handle val)
      external visitmdspeciesaddspeciesname
      integer  visitmdspeciesaddspeciesname

! visitmdspeciesgetnumspeciesname(visit_handle h, int val)
      external visitmdspeciesgetnumspeciesname
      integer  visitmdspeciesgetnumspeciesname

! visitmdspeciesgetspeciesname(visit_handle h, int i, visit_handle val)
      external visitmdspeciesgetspeciesname
      integer  visitmdspeciesgetspeciesname


! VisIt_VariableMetaData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmdvaralloc(visit_handle h)
      external visitmdvaralloc
      integer  visitmdvaralloc

! visitmdvarfree(visit_handle h)
      external visitmdvarfree
      integer  visitmdvarfree

! visitmdvarsetname(visit_handle h, string val, int lval)
      external visitmdvarsetname
      integer  visitmdvarsetname

! visitmdvargetname(visit_handle h, string val, int lval)
      external visitmdvargetname
      integer  visitmdvargetname

! visitmdvarsetmeshname(visit_handle h, string val, int lval)
      external visitmdvarsetmeshname
      integer  visitmdvarsetmeshname

! visitmdvargetmeshname(visit_handle h, string val, int lval)
      external visitmdvargetmeshname
      integer  visitmdvargetmeshname

! visitmdvarsetunits(visit_handle h, string val, int lval)
      external visitmdvarsetunits
      integer  visitmdvarsetunits

! visitmdvargetunits(visit_handle h, string val, int lval)
      external visitmdvargetunits
      integer  visitmdvargetunits

! visitmdvarsetcentering(visit_handle h, int val)
      external visitmdvarsetcentering
      integer  visitmdvarsetcentering

! visitmdvargetcentering(visit_handle h, int val)
      external visitmdvargetcentering
      integer  visitmdvargetcentering

! visitmdvarsettype(visit_handle h, int val)
      external visitmdvarsettype
      integer  visitmdvarsettype

! visitmdvargettype(visit_handle h, int val)
      external visitmdvargettype
      integer  visitmdvargettype

! visitmdvarsettreatasascii(visit_handle h, int val)
      external visitmdvarsettreatasascii
      integer  visitmdvarsettreatasascii

! visitmdvargettreatasascii(visit_handle h, int val)
      external visitmdvargettreatasascii
      integer  visitmdvargettreatasascii

! visitmdvarsethidefromgui(visit_handle h, int val)
      external visitmdvarsethidefromgui
      integer  visitmdvarsethidefromgui

! visitmdvargethidefromgui(visit_handle h, int val)
      external visitmdvargethidefromgui
      integer  visitmdvargethidefromgui

! visitmdvarsetnumcomponents(visit_handle h, int val)
      external visitmdvarsetnumcomponents
      integer  visitmdvarsetnumcomponents

! visitmdvargetnumcomponents(visit_handle h, int val)
      external visitmdvargetnumcomponents
      integer  visitmdvargetnumcomponents

!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
! Data functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! VisIt_CSGMesh functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
! visitcsgmeshalloc(visit_handle obj)
      external visitcsgmeshalloc
      integer  visitcsgmeshalloc

! visitcsgmeshfree(visit_handle obj)
      external visitcsgmeshfree
      integer  visitcsgmeshfree

! visitcsgmeshsetregions(visit_handle obj, visit_handle boolops, visit_handle leftids, visit_handle rightids)
      external visitcsgmeshsetregions
      integer  visitcsgmeshsetregions

! visitcsgmeshsetzonelist(visit_handle obj, visit_handle zl)
      external visitcsgmeshsetzonelist
      integer  visitcsgmeshsetzonelist

! visitcsgmeshsetboundarytypes(visit_handle obj, visit_handle boundarytypes)
      external visitcsgmeshsetboundarytypes
      integer  visitcsgmeshsetboundarytypes

! visitcsgmeshsetboundarycoeffs(visit_handle obj, visit_handle boundarycoeffs)
      external visitcsgmeshsetboundarycoeffs
      integer  visitcsgmeshsetboundarycoeffs

! visitcsgmeshsetextents(visit_handle obj, double mins, double maxs)
      external visitcsgmeshsetextents
      integer  visitcsgmeshsetextents

! visitcsgmeshgetregions(visit_handle obj, visit_handle boolops, 
!                        visit_handle leftids, visit_handle rightids)
      external visitcsgmeshgetregions
      integer  visitcsgmeshgetregions

! visitcsgmeshgetzonelist(visit_handle obj, visit_handle zl)
      external visitcsgmeshgetzonelist
      integer  visitcsgmeshgetzonelist

! visitcsgmeshgetboundarytypes(visit_handle obj, visit_handle boundarytypes)
      external visitcsgmeshgetboundarytypes
      integer  visitcsgmeshgetboundarytypes

! visitcsgmeshgetboundarycoeffs(visit_handle obj, visit_handle boundarycoeffs)
      external visitcsgmeshgetboundarycoeffs
      integer  visitcsgmeshgetboundarycoeffs

! visitcsgmeshgetextents(visit_handle obj, double mins, double maxs)
      external visitcsgmeshgetextents
      integer  visitcsgmeshgetextents


! VisIt_CurveData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitcurvedataalloc(visit_handle obj)
      external visitcurvedataalloc
      integer  visitcurvedataalloc

! visitcurvedatafree(visit_handle obj)
      external visitcurvedatafree
      integer  visitcurvedatafree

! visitcurvedatasetcoordsxy(visit_handle obj, visit_handle x, visit_handle y)
      external visitcurvedatasetcoordsxy
      integer  visitcurvedatasetcoordsxy


! VisIt_CurvilinearMesh functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitcurvmeshalloc(visit_handle obj)
      external visitcurvmeshalloc
      integer  visitcurvmeshalloc

! visitcurvmeshfree(visit_handle obj)
      external visitcurvmeshfree
      integer  visitcurvmeshfree

! visitcurvmeshsetcoordsxy(visit_handle obj, int dims, visit_handle x, visit_handle y)
      external visitcurvmeshsetcoordsxy
      integer  visitcurvmeshsetcoordsxy

! visitcurvmeshsetcoordsxyz(visit_handle obj, int dims, visit_handle x, visit_handle y, visit_handle z)
      external visitcurvmeshsetcoordsxyz
      integer  visitcurvmeshsetcoordsxyz

! visitcurvmeshsetcoords2(visit_handle obj, int dims, visit_handle c)
      external visitcurvmeshsetcoords2
      integer  visitcurvmeshsetcoords2

! visitcurvmeshsetcoords3(visit_handle obj, int dims, visit_handle c)
      external visitcurvmeshsetcoords3
      integer  visitcurvmeshsetcoords3

! visitcurvmeshsetbaseindex(visit_handle obj, int base_index)
      external visitcurvmeshsetbaseindex
      integer  visitcurvmeshsetbaseindex

! visitcurvmeshsetrealindices(visit_handle obj, int mins, int maxs)
      external visitcurvmeshsetrealindices
      integer  visitcurvmeshsetrealindices

! visitcurvmeshsetghostcells(visit_handle obj, visit_handle gz)
      external visitcurvmeshsetghostcells
      integer  visitcurvmeshsetghostcells

! visitcurvmeshgetcoords(visit_handle obj, int ndims, int dims, int coordMode,
!                        visit_handle x, visit_handle y, visit_handle z, 
!                        visit_handle c)
      external visitcurvmeshgetcoords
      integer  visitcurvmeshgetcoords

! visitcurvmeshgetbaseindex(visit_handle obj, int base_index)
      external visitcurvmeshgetbaseindex
      integer  visitcurvmeshgetbaseindex

! visitcurvmeshgetrealindices(visit_handle obj, int mins, int maxs)
      external visitcurvmeshgetrealindices
      integer  visitcurvmeshgetrealindices

! visitcurvmeshgetghostcells(visit_handle obj, visit_handle gz)
      external visitcurvmeshgetghostcells
      integer  visitcurvmeshgetghostcells


! VisIt_DomainBoundaries functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitdboundsalloc(visit_handle obj)
      external visitdboundsalloc
      integer  visitdboundsalloc

! visitdboundsfree(visit_handle obj)
      external visitdboundsfree
      integer  visitdboundsfree

! visitdboundssettype(visit_handle obj, int type)
      external visitdboundssettype
      integer  visitdboundssettype

! visitdboundssetnumdomains(visit_handle obj, int numdomains)
      external visitdboundssetnumdomains
      integer  visitdboundssetnumdomains

! visitdboundssetamrindices(visit_handle obj, int patch, int level, const int extents)
      external visitdboundssetamrindices
      integer  visitdboundssetamrindices

! visitdboundssetrectindices(visit_handle obj, int domain, const int extents)
      external visitdboundssetrectindices
      integer  visitdboundssetrectindices


! VisIt_DomainList functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitdomainlistalloc(visit_handle obj)
      external visitdomainlistalloc
      integer  visitdomainlistalloc

! visitdomainlistfree(visit_handle obj)
      external visitdomainlistfree
      integer  visitdomainlistfree

! visitdomainlistsetdomains(visit_handle obj, int alldoms, visit_handle mydoms)
      external visitdomainlistsetdomains
      integer  visitdomainlistsetdomains


! VisIt_DomainNesting functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitdnestingalloc(visit_handle obj)
      external visitdnestingalloc
      integer  visitdnestingalloc

! visitdnestingfree(visit_handle obj)
      external visitdnestingfree
      integer  visitdnestingfree

! visitdnestingsetdimensions(visit_handle obj, int npatches, int nlevels, int ndimensions)
      external visitdnestingsetdimensions
      integer  visitdnestingsetdimensions

! visitdnestingsetlevelrefinement(visit_handle obj, int level, const int ratios)
      external visitdnestingsetlevelrefinement
      integer  visitdnestingsetlevelrefinement

! visitdnestingsetnestingforpatch(visit_handle obj, int patch, int level, 
      external visitdnestingsetnestingforpatch
      integer  visitdnestingsetnestingforpatch


! VisIt_MaterialData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitmatdataalloc(visit_handle h)
      external visitmatdataalloc
      integer  visitmatdataalloc

! visitmatdatafree(visit_handle h)
      external visitmatdatafree
      integer  visitmatdatafree

! visitmatdataappendcells(visit_handle h, int ncells)
      external visitmatdataappendcells
      integer  visitmatdataappendcells

! visitmatdataaddmat(visit_handle obj, visit_f77string matname, int lmatname, int matno)
      external visitmatdataaddmat
      integer  visitmatdataaddmat

! visitmatdataaddcleancell(visit_handle h, int cell, int matno)
      external visitmatdataaddcleancell
      integer  visitmatdataaddcleancell

! visitmatdataaddmixedcell(visit_handle h, int cell, int matnos, float matvf, int nmats)
      external visitmatdataaddmixedcell
      integer  visitmatdataaddmixedcell

! visitmatdatasetmats(visit_handle obj, visit_handle matlist)
      external visitmatdatasetmats
      integer  visitmatdatasetmats

! visitmatdatasetmixedmats(visit_handle obj, visit_handle mix_mat, 
      external visitmatdatasetmixedmats
      integer  visitmatdatasetmixedmats


! VisIt_PointMesh functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitpointmeshalloc(visit_handle obj)
      external visitpointmeshalloc
      integer  visitpointmeshalloc

! visitpointmeshfree(visit_handle obj)
      external visitpointmeshfree
      integer  visitpointmeshfree

! visitpointmeshsetcoordsxy(visit_handle obj, visit_handle x, visit_handle y)
      external visitpointmeshsetcoordsxy
      integer  visitpointmeshsetcoordsxy

! visitpointmeshsetcoordsxyz(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
      external visitpointmeshsetcoordsxyz
      integer  visitpointmeshsetcoordsxyz

! visitpointmeshsetcoords(visit_handle obj, visit_handle c)
      external visitpointmeshsetcoords
      integer  visitpointmeshsetcoords

! visitpointmeshgetcoords(visit_handle obj, int ndims, int coordmode,
!    visit_handle x, visit_handle y, visit_handle z, visit_handle c)
      external visitpointmeshgetcoords
      integer  visitpointmeshgetcoords


! VisIt_RectilinearMesh functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitrectmeshalloc(visit_handle obj)
      external visitrectmeshalloc
      integer  visitrectmeshalloc

! visitrectmeshfree(visit_handle obj)
      external visitrectmeshfree
      integer  visitrectmeshfree

! visitrectmeshsetcoordsxy(visit_handle obj, visit_handle x, visit_handle y)
      external visitrectmeshsetcoordsxy
      integer  visitrectmeshsetcoordsxy

! visitrectmeshsetcoordsxyz(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
      external visitrectmeshsetcoordsxyz
      integer  visitrectmeshsetcoordsxyz

! visitrectmeshsetbaseindex(visit_handle obj, int base_index)
      external visitrectmeshsetbaseindex
      integer  visitrectmeshsetbaseindex

! visitrectmeshsetrealindices(visit_handle obj, int mins, int maxs)
      external visitrectmeshsetrealindices
      integer  visitrectmeshsetrealindices

! visitrectmeshsetghostcells(visit_handle obj, visit_handle gz)
      external visitrectmeshsetghostcells
      integer  visitrectmeshsetghostcells

! visitrectmeshgetcoords(visit_handle obj, int ndims, visit_handle x, 
!                        visit_handle y, visit_handle z)
      external visitrectmeshgetcoords
      integer  visitrectmeshgetcoords

! visitrectmeshgetbaseindex(visit_handle obj, int base_index)
      external visitrectmeshgetbaseindex
      integer  visitrectmeshgetbaseindex

! visitrectmeshgetrealindices(visit_handle obj, int mins, int maxs)
      external visitrectmeshgetrealindices
      integer  visitrectmeshgetrealindices

! visitrectmeshgetghostcells(visit_handle obj, visit_handle gz)
      external visitrectmeshgetghostcells
      integer  visitrectmeshgetghostcells

! VisIt_SpeciesData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitspecdataalloc(visit_handle h)
      external visitspecdataalloc
      integer  visitspecdataalloc

! visitspecdatafree(visit_handle h)
      external visitspecdatafree
      integer  visitspecdatafree

! visitspecdataaddspeciesname(visit_handle h, visit_handle val)
      external visitspecdataaddspeciesname
      integer  visitspecdataaddspeciesname

! visitspecdatasetspecies(visit_handle h, visit_handle val)
      external visitspecdatasetspecies
      integer  visitspecdatasetspecies

! visitspecdatasetspeciesmf(visit_handle h, visit_handle val)
      external visitspecdatasetspeciesmf
      integer  visitspecdatasetspeciesmf

! visitspecdatasetmixedspecies(visit_handle h, visit_handle val)
      external visitspecdatasetmixedspecies
      integer  visitspecdatasetmixedspecies


! VisIt_UnstructuredMesh functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitucdmeshalloc(visit_handle obj)
      external visitucdmeshalloc
      integer  visitucdmeshalloc

! visitucdmeshfree(visit_handle obj)
      external visitucdmeshfree
      integer  visitucdmeshfree

! visitucdmeshsetcoordsxy(visit_handle obj, visit_handle x, visit_handle y)
      external visitucdmeshsetcoordsxy
      integer  visitucdmeshsetcoordsxy

! visitucdmeshsetcoordsxyz(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
      external visitucdmeshsetcoordsxyz
      integer  visitucdmeshsetcoordsxyz

! visitucdmeshsetcoords(visit_handle obj, visit_handle c)
      external visitucdmeshsetcoords
      integer  visitucdmeshsetcoords

! visitucdmeshsetconnectivity(visit_handle obj, int nzones, visit_handle c)
      external visitucdmeshsetconnectivity
      integer  visitucdmeshsetconnectivity

! visitucdmeshsetrealindices(visit_handle obj, int min, int max)
      external visitucdmeshsetrealindices
      integer  visitucdmeshsetrealindices

! visitucdmeshsetghostcells(visit_handle obj, visit_handle gz)
      external visitucdmeshsetghostcells
      integer  visitucdmeshsetghostcells

! visitucdmeshgetcoords(visit_handle obj, int ndims, int coordmode,  
!     visit_handle x, visit_handle y, visit_handle z, visit_handle c)
      external visitucdmeshgetcoords
      integer  visitucdmeshgetcoords

! visitucdmeshgetconnectivity(visit_handle obj, int nzones, visit_handle c)
      external visitucdmeshgetconnectivity
      integer  visitucdmeshgetconnectivity

! visitucdmeshgetrealindices(visit_handle obj, int min, int max)
      external visitucdmeshgetrealindices
      integer  visitucdmeshgetrealindices

! visitucdmeshgetghostcells(visit_handle obj, visit_handle gz)
      external visitucdmeshgetghostcells
      integer  visitucdmeshgetghostcells

! VisIt_VariableData functions
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! visitvardataalloc(visit_handle obj)
      external visitvardataalloc
      integer  visitvardataalloc

! visitvardatafree(visit_handle obj)
      external visitvardatafree
      integer  visitvardatafree

! visitvardatasetc(visit_handle obj, int owner, int ncomps, int ntuples, char ptr)
      external visitvardatasetc
      integer  visitvardatasetc

! visitvardataseti(visit_handle obj, int owner, int ncomps, int ntuples, int ptr)
      external visitvardataseti
      integer  visitvardataseti

! visitvardatasetf(visit_handle obj, int owner, int ncomps, int ntuples, float ptr)
      external visitvardatasetf
      integer  visitvardatasetf

! visitvardatasetd(visit_handle obj, int owner, int ncomps, int ntuples, double ptr)
      external visitvardatasetd
      integer  visitvardatasetd

! visitvardatagettype(visit_handle obj, int datatype)
      external visitvardatagettype
      integer  visitvardatagettype

! visitvardatagetc(visit_handle obj, int owner, int ncomps, int ntuples,
!                  char dest, int ldest)
      external visitvardatagetc
      integer  visitvardatagetc

! visitvardatageti(visit_handle obj, int owner, int ncomps, int ntuples,
!                  int dest, int ldest)
      external visitvardatageti
      integer  visitvardatageti

! visitvardatagetf(visit_handle obj, int owner, int ncomps, int ntuples,
!                  float dest, int ldest)
      external visitvardatagetf
      integer  visitvardatagetf

! visitvardatagetd(visit_handle obj, int owner, int ncomps, int ntuples,
!                  double dest, int ldest)
      external visitvardatagetd
      integer  visitvardatagetd

