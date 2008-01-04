c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2006, The Regents of the University of California
c Produced at the Lawrence Livermore National Laboratory
c LLNL-CODE-400142
c All rights reserved.
c
c This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
c full copyright notice is contained in the file COPYRIGHT located at the root
c of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
c
c Redistribution  and  use  in  source  and  binary  forms,  with  or  without
c modification, are permitted provided that the following conditions are met:
c
c  - Redistributions of  source code must  retain the above  copyright notice,
c    this list of conditions and the disclaimer below.
c  - Redistributions in binary form must reproduce the above copyright notice,
c    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
c    documentation and/or other materials provided with the distribution.
c  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
c    be used to endorse or promote products derived from this software without
c    specific prior written permission.
c
c THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
c AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
c IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
c ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
c LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
c DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
c DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
c SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
c CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
c LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
c OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
c DAMAGE.
c
c-----------------------------------------------------------------------------

c-----------------------------------------------------------------
c Program: main
c
c Programmer: Brad Whitlock
c Date:       Fri Feb 2 17:47:08 PST 2007
c
c Modifications:
c    Shelly Prevost,Thu Jul 26 16:34:40 PDT 2007
c    Added a absolute filename argument to VisItInitializeSocketAndDumpSimFile.
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer err

      err = visitsetupenv()
      err = visitinitializesim("fmaterial", 9,
     . "Demonstrates material data access function", 44,
     . "/no/useful/path", 15,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING,VISIT_F77NULLSTRINGLEN)
      call mainloop()
      stop
      end

c-----------------------------------------------------------------
c mainloop
c-----------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer visitstate, result, blocking
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
      do 10
          if(runflag.eq.1) then
              blocking = 0 
          else
              blocking = 1
          endif

          visitstate = visitdetectinput(blocking, -1)

          if (visitstate.lt.0) then
              goto 1234
          elseif (visitstate.eq.0) then
              call simulate_one_timestep()
          elseif (visitstate.eq.1) then
              runflag = 0
              result = visitattemptconnection()
              if (result.eq.1) then
                  write (6,*) 'VisIt connected!'
              else
                  write (6,*) 'VisIt did not connect!'
              endif
          elseif (visitstate.eq.2) then
              runflag = 0
              if (visitprocessenginecommand().eq.0) then
                  result = visitdisconnect()
                  runflag = 1
              endif
          endif
10    continue
1234  end

      subroutine simulate_one_timestep()
ccc   SIMSTATE common block
      integer runFlag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, ' time=', simtime
      call sleep(1)
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransiminterface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, intdata, 
     .                                 floatdata, stringdata, 
     .                                 lstringdata)
      implicit none
      character*8 cmd, stringdata
      integer     lcmd, lstringdata, intdata
      real        floatdata
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime

c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
          runflag = 0
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
          call simulate_one_timestep()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          runflag = 1
      endif
      end

c---------------------------------------------------------------------------
c visitbroadcastintfunction
c---------------------------------------------------------------------------
      integer function visitbroadcastintfunction(value, sender)
      implicit none
      integer value, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcastintfunction = 0
      end

c---------------------------------------------------------------------------
c visitbroadcaststringfunction
c---------------------------------------------------------------------------
      integer function visitbroadcaststringfunction(str, lstr, sender)
      implicit none
      character*8 str
      integer     lstr, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcaststringfunction = 0
      end

c---------------------------------------------------------------------------
c visitslaveprocesscallback
c---------------------------------------------------------------------------
      subroutine visitslaveprocesscallback ()
      implicit none
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      end

c---------------------------------------------------------------------------
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      integer err, tdim, sdim, mesh, mt, scalar, curve, mat, e

      err = visitmdsetcycletime(handle, simcycle, simtime)
      if(runflag.eq.1) then
          err = visitmdsetrunning(handle, VISIT_SIMMODE_RUNNING)
      else
          err = visitmdsetrunning(handle, VISIT_SIMMODE_STOPPED)
      endif

c     Add a 2D rectilinear mesh
      mt = VISIT_MESHTYPE_RECTILINEAR
      tdim = 2
      sdim = 2
      mesh = visitmdmeshcreate(handle, "mesh2d", 6, mt, tdim, sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "cm", 2)
          err = visitmdmeshsetlabels(handle, mesh, "Width", 5,
     .    "Height", 6, "Depth", 5)
          err = visitmdmeshsetblocktitle(handle, mesh, "Domains", 7)
          err = visitmdmeshsetblockpiecename(handle, mesh, "domain", 6)
      endif

c     Add a material
      mat = visitmdmaterialcreate(handle, "mat", 3, "mesh2d", 6)
      if(mat.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmaterialadd(handle, mat, "Water", 5)
          err = visitmdmaterialadd(handle, mat, "Membrane", 8)
          err = visitmdmaterialadd(handle, mat, "Air", 3)
      endif

c     Add simulation commands
      err = visitmdaddsimcommand(handle, "halt", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "step", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "run", 3, VISIT_CMDARG_NONE,
     .                           1)

      visitgetmetadata = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc" 
ccc   RECTMESH
      integer NX, NY
      parameter (NX = 5)
      parameter (NY = 4)

      real rmx(NX), rmy(NY)
      integer rmdims(3), rmndims
      data rmndims /2/
      data rmdims /NX, NY, 1/
      data rmx/0., 1., 2., 3., 4./
      data rmy/0., 1., 2., 3./
ccc   local variables
      integer m, baseindex(3), minrealindex(3), maxrealindex(3)
      real rmz

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = rmdims(1)-1
          maxrealindex(2) = rmdims(2)-1
          maxrealindex(3) = rmdims(3)-1
c Create a rectilinear rmesh here
          m = visitmeshrectilinear(handle, baseindex, minrealindex,
     .        maxrealindex, rmdims, rmndims, rmx, rmy, rmz)
      endif
      visitgetmesh = m
      end

c---------------------------------------------------------------------------
c visitgetscalar
c---------------------------------------------------------------------------
      integer function visitgetscalar(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
      visitgetscalar = VISIT_ERROR
      end


c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
      include "visitfortransiminterface.inc"
      visitgetcurve = VISIT_ERROR
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
      visitgetdomainlist = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
ccc   RECTMESH
      integer NX, NY
      parameter (NX = 5)
      parameter (NY = 4)

c The matlist table indicates the material numbers that are found in
c each cell. Every 3 numbers indicates the material numbers in a cell.
c A material number of 0 means that the material entry is not used.
      integer matlist(3, NX-1, NY-1)
      data matlist/3,0,0,2,3,0,1,2,0,1,0,0,
     .             3,0,0,2,3,0,1,2,0,1,0,0,
     .             3,0,0,2,3,0,1,2,3,1,2,0/
c The mat_vf table indicates the material volume fractions that are
c found in a cell.
      real mat_vf(3, NX-1, NY-1)
      data mat_vf/1.,0.,0.,0.75,0.25,0.,0.8125,0.1875, 0.,1.,0.,0.,
     . 1.,0.,0.,0.625,0.375,0.,0.5625,0.4375,0.,1.,0.,0.,
     . 1.,0.,0.,0.3,0.7,0.,0.2,0.4,0.4,0.55,0.45,0./

c Size the material object so it has the right dimensions (equal to
c the number of cells in the mesh. Unstructured grids would have
c /ncells,1,1/ size for the material. That means that the cellid array
c would have one linear index in its first element.
      integer dims(3), err, I, J, cellid(2), nmats, m
      integer matno(3), cellmat(3)
      real cellmatvf(3)
      data dims/NX-1, NY-1,1/
      err = visitmaterialsetdims(handle, dims, 2)
      matno(1) = visitmaterialadd(handle, "Water", 5)
      matno(2) = visitmaterialadd(handle, "Membrane", 8)
      matno(3) = visitmaterialadd(handle, "Air", 3)

      do 2020 J=1,NY-1
          cellid(2) = J
          do 2010 I=1,NX-1
              cellid(1) = I
              nmats = 0
              do 2000 m=1,3
                  if(matlist(m,I,J).gt.0) then
                      nmats = nmats + 1
                      cellmat(nmats) = matno(matlist(m,I,J))
                      cellmatvf(nmats) = mat_vf(m,I,J)
                  endif
2000          continue
              if(nmats.gt.1) then
                  err = visitmaterialaddmixed(handle,
     . cellid, cellmat, cellmatvf, nmats)
              else
                  err = visitmaterialaddclean(handle, 
     . cellid, cellmat)
              endif
2010      continue
2020  continue

      visitgetmaterial = err
      end





























