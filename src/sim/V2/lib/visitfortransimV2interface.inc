!cccccccccccccccccccccccccccccccccccccccccccccccccccccccc -*- fortran -*-
!
! VisIt Fortran Simulation Interface V2 include file
!
! Programmer: Brad Whitlock
! Date: Fri Feb 19 15:44:42 PST 2010
!
! Modifications:
!
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

!... Commonly useful declarations
      external visitstrcmp
      integer  visitstrcmp

      character*12 VISIT_F77NULLSTRING
      parameter   (VISIT_F77NULLSTRING         = "NULLSTRING")
      integer      VISIT_F77NULLSTRINGLEN
      parameter   (VISIT_F77NULLSTRINGLEN      = 10)
      integer      VISIT_INVALID_HANDLE
      parameter   (VISIT_INVALID_HANDLE        = -1)

      integer      VISIT_OKAY
      parameter   (VISIT_OKAY                  = 0)
      integer      VISIT_ERROR
      parameter   (VISIT_ERROR                 = 1)

!... Functions for the control interface.
      external visitsetdirectory,visitsetoptions,visitsetupenv
      external visitinitializesim,visitsetparallel,visitsetparallelrank
      external visitdetectinput,visitattemptconnection
      external visitprocessenginecommand,visittimestepchanged
      external visitupdateplots,visitexecutecommand
      external visitdisconnect,visitgetlasterror,visitopentracefile
      external visitclosetracefile

      integer  visitsetdirectory,visitsetoptions,visitsetupenv
      integer  visitinitializesim,visitsetparallel,visitsetparallelrank
      integer  visitdetectinput,visitattemptconnection
      integer  visitprocessenginecommand,visittimestepchanged
      integer  visitupdateplots,visitexecutecommand
      integer  visitdisconnect,visitgetlasterror,visitopentracefile
      integer  visitclosetracefile
 
!... Functions for the data interface
      external visitmdsetcycletime
      integer  visitmdsetcycletime
      external visitmdsetrunning
      integer  visitmdsetrunning
      external visitmdmeshcreate
      integer  visitmdmeshcreate
      external visitmdmeshsetunits
      integer  visitmdmeshsetunits
      external visitmdmeshsetlabels
      integer  visitmdmeshsetlabels
      external visitmdmeshsetblocktitle
      integer  visitmdmeshsetblocktitle
      external visitmdmeshsetblockpiecename
      integer  visitmdmeshsetblockpiecename
      external visitmdmeshsetgrouptitle
      integer  visitmdmeshsetgrouptitle
      external visitmdmeshsetgrouppiecename
      integer  visitmdmeshsetgrouppiecename
      external visitmdscalarcreate
      integer  visitmdscalarcreate
      external visitmdscalarsetunits
      integer  visitmdscalarsetunits
      external visitmdaddsimcommand
      integer  visitmdaddsimcommand
      external visitmdcurvecreate
      integer  visitmdcurvecreate
      external visitmdcurvesetunits
      integer  visitmdcurvesetunits
      external visitmdcurvesetlabels
      integer  visitmdcurvesetlabels
      external visitmdmaterialcreate
      integer  visitmdmaterialcreate
      external visitmdmaterialadd
      integer  visitmdmaterialadd
      external visitmdexpressioncreate
      integer  visitmdexpressioncreate

      external visitmeshrectilinear
      integer  visitmeshrectilinear
      external visitmeshrectilinear2
      integer  visitmeshrectilinear2
      external visitmeshcurvilinear
      integer  visitmeshcurvilinear
      external visitmeshcurvilinear2
      integer  visitmeshcurvilinear2
      external visitmeshpoint
      integer  visitmeshpoint
      external visitmeshpoint2
      integer  visitmeshpoint2
      external visitmeshunstructured
      integer  visitmeshunstructured
      external visitmeshunstructured2
      integer  visitmeshunstructured2

      external visitscalarsetdata
      integer  visitscalarsetdata
      external visitscalarsetdatac
      integer  visitscalarsetdatac
      external visitscalarsetdatai
      integer  visitscalarsetdatai
      external visitscalarsetdataf
      integer  visitscalarsetdataf
      external visitscalarsetdatad
      integer  visitscalarsetdatad

      external visitsetdomainlist
      integer  visitsetdomainlist

      external visitcurvesetdataf
      integer  visitcurvesetdataf
      external visitcurvesetdatad 
      integer  visitcurvesetdatad

      external visitmaterialsetdims
      integer  visitmaterialsetdims
      external visitmaterialadd
      integer  visitmaterialadd
      external visitmaterialaddclean
      integer  visitmaterialaddclean
      external visitmaterialaddmixed
      integer  visitmaterialaddmixed

!... Mesh types
      integer      VISIT_MESHTYPE_RECTILINEAR
      parameter   (VISIT_MESHTYPE_RECTILINEAR  = 0)
      integer      VISIT_MESHTYPE_CURVILINEAR
      parameter   (VISIT_MESHTYPE_CURVILINEAR  = 1)
      integer      VISIT_MESHTYPE_UNSTRUCTURED
      parameter   (VISIT_MESHTYPE_UNSTRUCTURED = 2)
      integer      VISIT_MESHTYPE_POINT
      parameter   (VISIT_MESHTYPE_POINT        = 3)
      integer      VISIT_MESHTYPE_SURFACE
      parameter   (VISIT_MESHTYPE_SURFACE      = 4)
      integer      VISIT_MESHTYPE_UNKNOWN
      parameter   (VISIT_MESHTYPE_UNKNOWN      = 5)

!... Centering
      integer      VISIT_VARCENTERING_NODE
      parameter   (VISIT_VARCENTERING_NODE     = 0)
      integer      VISIT_VARCENTERING_ZONE
      parameter   (VISIT_VARCENTERING_ZONE     = 1)

!... Variable Type
      integer      VISIT_VARTYPE_MESH                
      parameter   (VISIT_VARTYPE_MESH                = 0)
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
      integer      VISIT_VARTYPE_CURVE               
      parameter   (VISIT_VARTYPE_CURVE               = 7)
      integer      VISIT_VARTYPE_UNKNOWN             
      parameter   (VISIT_VARTYPE_UNKNOWN             = 8)

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

! End.
