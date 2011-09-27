! A parallel Jacobi solver for the Laplacian equation in 2D
! Written by Jean M. Favre, Swiss National Supercomputing Center
! Sun Oct 31 2010
! Code inspired from an older example by Kadin Tseng, Boston University, November 1999
! The compile flag -D_VISIT_ enables compilation with VisIt. Otherwise, the pogram runs
! in stand-alone mode

program Jacobi
use jacobi_module
implicit none
#if ( defined _VISIT_ )
  include "visitfortransimV2interface.inc"
  external processvisitcommand
  integer processvisitcommand
#endif

  call InitMPI()

  m = 20 ! mesh size = (m+2)x(m+2) including the bc grid lines
! We make no attempt to check that the number of grid points divides evenly
! with the number of MPI tasks.
! rank 0 will display the bottom (southern) boundary wall
! rank (size-1) will display the top (northern) boundary wall
! if run with m=20 and 4 MPI tasks, we will have 5 grid lines per rank
! and VisIt will display a 22x22 grid
#if ( defined _VISIT_ )
  if(par_size.gt.1) then
    ierr = VISITSETPARALLEL(1)
  end if
  ierr = VISITSETPARALLELRANK(par_rank)

  call SIMULATIONARGUMENTS()

  ierr = VISITSETUPENV()
!     Have the master process write the sim file.
  if(par_rank.eq.0) then
  ierr = VISITINITIALIZESIM("pjacobi", 7, &
            "Interface to pjacobi", 20, &
            "/no/useful/path", 15, &
            VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN, &
            VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN, &
            VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN)
  end if

#endif

  call MPI_Bcast(m, 1, MPI_INTEGER, 0, MPI_COMM_WORLD, ierr)

! no check is done to make sure the two numbers below divide evenly
  mp = m/par_size                            ! columns for each proc

  print *, 'proc ', par_rank, ' out of ', par_size, ' m=', m, ' mp=', mp

  allocate ( Temp(0:m+1,0:mp+1), oldTemp(0:m+1,0:mp+1) )  ! mem for Temp, v

  call set_initial_bc()             ! set up boundary values

#if ( defined _VISIT_ )
  visRunFlag = .FALSE.
#endif

  do while (gdel > TOL)               ! iterate until error below threshold

  if(iter > MAXSTEPS) then
    write(*,*)'Iteration terminated (exceeds ', MAXSTEPS, ')'
    exit                            ! nonconvergent solution
  end if

#if ( defined _VISIT_ )
  if(visRunFlag) then
    blocking = 0 
  else
    blocking = 1
  endif

  if( par_rank == 0 ) then
    visitstate = visitdetectinput(blocking, -1)
  end if

  call MPI_BCAST(visitstate,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)

  if (visitstate < 0) then
    print *, 'visitState < 0 in time stepping loop. Ignoring and continuing'
  elseif (visitstate == 0) then
    call simulate_one_timestep()
  elseif (visitstate == 1) then
    visRunFlag = .TRUE.
    visResult = visitattemptconnection()
    if (visResult == 1) then
       write (6,*) 'VisIt connected!'
    else
       write (6,*) 'VisIt did not connect!'
    endif

  elseif (visitstate == 2) then
    visRunFlag = .FALSE.
    if (processvisitcommand() == 0) then
      visResult = visitdisconnect()
      visRunFlag = .TRUE.
    endif
  endif
#else
    call simulate_one_timestep()
#endif
  end do
  call MPIIOWriteData('/tmp/Jacobi.bin')
  deallocate (Temp, oldTemp)

  call MPI_Finalize(ierr)

end program Jacobi
 
