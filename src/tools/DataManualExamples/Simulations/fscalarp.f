c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
c Produced at the Lawrence Livermore National Laboratory
c LLNL-CODE-442911
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
c Date:       Fri Jan 12 14:12:55 PST 2007
c
c Modifications:
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransimV2interface.inc"
      include "mpif.h"
ccc   local variables
      integer err
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size
      save /PARALLEL/

      call MPI_INIT(err)

c     Determine the rank and size of this MPI task so we can tell
c     VisIt's libsim about it.
      call MPI_COMM_RANK(MPI_COMM_WORLD, par_rank, err)
      call MPI_COMM_SIZE(MPI_COMM_WORLD, par_size, err)
      if(par_size.gt.1) then
          err = visitsetparallel(1)
      endif
      err = visitsetparallelrank(par_rank)

      call simulationarguments()
      err = visitsetupenv()
c     Have the master process write the sim file.
      if(par_rank.eq.0) then
          err = visitinitializesim("fscalarp", 8,
     .     "Demonstrates scalar data access function", 40,
     .     "/no/useful/path", 15,
     .     VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     .     VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     .     VISIT_F77NULLSTRING,VISIT_F77NULLSTRINGLEN)
      endif

      call mainloop()

      call MPI_FINALIZE(err)
      stop
      end

c-----------------------------------------------------------------
c processvisitcommand
c-----------------------------------------------------------------

      integer function processvisitcommand()
      implicit none
      include "mpif.h"
      include "visitfortransimV2interface.inc"
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size
      integer command, e, doloop, success, ret
      integer VISIT_COMMAND_PROCESS
      integer VISIT_COMMAND_SUCCESS
      integer VISIT_COMMAND_FAILURE
      parameter (VISIT_COMMAND_PROCESS = 0)
      parameter (VISIT_COMMAND_SUCCESS = 1)
      parameter (VISIT_COMMAND_FAILURE = 2)

      if(par_rank.eq.0) then
          success = visitprocessenginecommand()

          if(success.gt.0) then
              command = VISIT_COMMAND_SUCCESS
              ret = 1
          else
              command = VISIT_COMMAND_FAILURE
              ret = 0
          endif

          call MPI_BCAST(command,1,MPI_INTEGER,0,MPI_COMM_WORLD,e)
      else
          doloop = 1
2345      call MPI_BCAST(command,1,MPI_INTEGER,0,MPI_COMM_WORLD,e)
          if(command.eq.VISIT_COMMAND_PROCESS) then
              success = visitprocessenginecommand()
          elseif(command.eq.VISIT_COMMAND_SUCCESS) then
              ret = 1
              doloop = 0
          else
              ret = 0
              doloop = 0
          endif

          if(doloop.ne.0) then
              goto 2345
          endif
      endif
      processvisitcommand = ret
      end

c-----------------------------------------------------------------
c mainloop
c-----------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "mpif.h"
      include "visitfortransimV2interface.inc"
ccc   functions
      integer processvisitcommand
ccc   local variables
      integer visitstate, result, blocking, ierr
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag,simcycle,simtime
      save /SIMSTATE/
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0
      do 10
          if(runflag.eq.1) then
              blocking = 0 
          else
              blocking = 1
          endif

c         Detect input from VisIt on processor 0 and then broadcast
c         the results of that input to all processors.
          if(par_rank.eq.0) then
              visitstate = visitdetectinput(blocking, -1)
          endif
          call MPI_BCAST(visitstate,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)

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
              if (processvisitcommand().eq.0) then
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
      common /SIMSTATE/ runflag,simcycle,simtime
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
      save /RECTMESH/
c Rectilinear mesh data
      data rmndims /2/
      data rmdims /4, 5, 1/
      data rmx/0., 1., 2.5, 5./
      data rmy/0., 2., 2.25, 2.55,  5./
      data zonal/1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12./

c Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      if(par_rank.eq.0) then
          write (6,*) 'Simulating time step: cycle=',simcycle, 
     .    ' time=', simtime
      endif
      call sleep(1)
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransimV2interface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, args, largs)
      implicit none
      character*8 cmd, args
      integer     lcmd, largs
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag,simcycle,simtime
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
      include "mpif.h"
      integer value, sender, IERR
      call MPI_BCAST(value,1,MPI_INTEGER,sender,MPI_COMM_WORLD,ierr)
      visitbroadcastintfunction = 0
      end

c---------------------------------------------------------------------------
c visitbroadcaststringfunction
c---------------------------------------------------------------------------
      integer function visitbroadcaststringfunction(str, lstr, sender)
      implicit none
      include "mpif.h"
      character*8 str
      integer     lstr, sender, IERR
      call MPI_BCAST(str,lstr,MPI_CHARACTER,sender,MPI_COMM_WORLD,ierr)
      visitbroadcaststringfunction = 0
      end

c---------------------------------------------------------------------------
c visitslaveprocesscallback
c---------------------------------------------------------------------------
      subroutine visitslaveprocesscallback ()
      implicit none
      include "mpif.h"
      integer c, ierr, VISIT_COMMAND_PROCESS
      parameter (VISIT_COMMAND_PROCESS = 0)
      c = VISIT_COMMAND_PROCESS
      call MPI_BCAST(c,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
      end

c---------------------------------------------------------------------------
c visitactivatetimestep
c---------------------------------------------------------------------------
      integer function visitactivatetimestep()
      implicit none
      include "visitfortransimV2interface.inc"
      visitactivatetimestep = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size
c     Local variables
      integer md, mmd, vmd, cmd, emd, err
      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c     Add a 2D rectilinear mesh
          if(visitmdmeshalloc(mmd).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(mmd, "mesh2d", 6)
              err = visitmdmeshsetmeshtype(mmd, 
     .            VISIT_MESHTYPE_RECTILINEAR)
              err = visitmdmeshsettopologicaldim(mmd, 2)
              err = visitmdmeshsetspatialdim(mmd, 2)
              err = visitmdmeshsetnumdomains(mmd, par_size)
              err = visitmdmeshsetdomaintitle(mmd, "Domains", 7)
              err = visitmdmeshsetdomainpiecename(mmd, "domain", 6)
              err = visitmdmeshsetxunits(mmd, "cm", 2)
              err = visitmdmeshsetyunits(mmd, "cm", 2)
              err = visitmdmeshsetxlabel(mmd, "Width", 5)
              err = visitmdmeshsetylabel(mmd, "Height", 6)
              err = visitmdmeshsetcellorigin(mmd, 1)
              err = visitmdmeshsetnodeorigin(mmd, 1)
              err = visitmdsimaddmesh(md, mmd)
          endif

c     Add a zonal variable on mesh2d.
      if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
          err = visitmdvarsetname(vmd, "zonal", 5)
          err = visitmdvarsetmeshname(vmd, "mesh2d", 6)
          err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_ZONE)
          err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
          err = visitmdsimaddvariable(md, vmd)
      endif

c     Add some expressions
      if(visitmdexpralloc(emd).eq.VISIT_OKAY) then
          err = visitmdexprsetname(emd, "zvec", 4)
          err = visitmdexprsetdefinition(emd, "{zonal, zonal}", 14)
          err = visitmdexprsettype(emd, VISIT_VARTYPE_VECTOR)
          err = visitmdsimaddexpression(md, emd)
      endif

c     Add simulation commands
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "halt", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "step", 4)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
          err = visitmdcmdalloc(cmd)
          if(err.eq.VISIT_OKAY) then
              err = visitmdcmdsetname(cmd, "run", 3)
              err = visitmdsimaddgenericcommand(md, cmd)
          endif
      endif
      visitgetmetadata = md
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc" 
ccc   RECTMESH common block (shared with simulate_one_timestep)
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal

ccc   local variables
      integer h, i, x, y, z, nnodes, err
      real rmx2(NX)

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then
          if(visitrectmeshalloc(h).eq.VISIT_OKAY) then
              err = visitvardataalloc(x)
              err = visitvardataalloc(y)
c Create a rectilinear rmesh here by offsetting the mesh in X based
c on the domain number.
              do 20 i = 1, NX
                  rmx2(i) = rmx(i) + (5. * real(domain))
20            continue
              err = visitvardatasetf(x,VISIT_OWNER_COPY,1,NX,rmx2)
              err = visitvardatasetf(y,VISIT_OWNER_SIM,1,NY,rmy)

              err = visitrectmeshsetcoordsxy(h, x, y)
          endif
      endif
      visitgetmesh = h
      end

c---------------------------------------------------------------------------
c visitgetvariable
c---------------------------------------------------------------------------
      integer function visitgetvariable(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 4)
      parameter (NY = 5)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
ccc   local vars
      integer h, nvals, err

      h = VISIT_INVALID_HANDLE
      if(visitstrcmp(name, lname, "zonal", 5).eq.0) then
c A zonal variable has 1 less value in each dimension as there
c are nodes. Send back REAL data.
          if(visitvardataalloc(h).eq.VISIT_OKAY) then
              nvals = (rmdims(1)-1) * (rmdims(2)-1)
              err = visitvardatasetf(h, VISIT_OWNER_SIM,1,nvals,zonal)
          endif
      endif

      visitgetvariable = h
      end

c---------------------------------------------------------------------------
c visitgetmixedvariable
c---------------------------------------------------------------------------
      integer function visitgetmixedvariable(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmixedvariable = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
      include "visitfortransimV2interface.inc"
      visitgetcurve = VISIT_ERROR
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   PARALLEL state common block
      integer par_rank, par_size
      common /PARALLEL/ par_rank, par_size
ccc   local vars
      integer h, dl, err

c     Tell VisIt that there are as many domains as processors and this
c     processor just has one of them.
      h = VISIT_INVALID_HANDLE
      if(visitdomainlistalloc(h).eq.VISIT_OKAY) then
          if(visitvardataalloc(dl).eq.VISIT_OKAY) then
              err = visitvardataseti(dl, VISIT_OWNER_SIM, 1, 1,
     .                               par_rank)
              err = visitdomainlistsetdomains(h, par_size, dl)
          endif
      endif
      visitgetdomainlist = h
      end

c---------------------------------------------------------------------------
c visitgetdomainbounds
c---------------------------------------------------------------------------
      integer function visitgetdomainbounds(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainbounds = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainnesting
c---------------------------------------------------------------------------
      integer function visitgetdomainnesting(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainnesting = VISIT_INVALID_HANDLE
      end


c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmaterial = VISIT_ERROR
      end





























