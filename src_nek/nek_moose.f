C=======================================================================
      subroutine nek_init_step()
      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      real*4 papi_mflops
      integer*8 papi_flops
      integer icall, kstep, i, pstep
      common /cht_coupler/ pstep
      save icall
 
      if (icall.eq.0) then
      call nekgsync()
      if (instep.eq.0) then
        if(nid.eq.0) write(6,'(/,A,/,A,/)')
     &     ' nsteps=0 -> skip time loop',
     &     ' running solver in post processing mode'
      else
        if(nio.eq.0) write(6,'(/,A,/)') 'Starting time loop ...'
      endif
      isyc  = 0
      itime = 0
      if(ifsync) isyc=1
      itime = 1
      call nek_comm_settings(isyc,itime)
      call nek_comm_startstat()
      istep  = 0
      icall  = 1
      endif

      istep=istep+1

      if (lastep .eq. 1) then
        pstep=2
      else
        call nek_advance_moose
        pstep=2
      endif

      return
      end
C=======================================================================
      subroutine nek_step()

      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      common /cht_coupler/ pstep
      integer pstep

      pstep=pstep+1
      call heat_mod(pstep)

      return
      end
C=======================================================================
      subroutine nek_finalize_step()
      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      common /cht_coupler/ pstep
      integer pstep 
      logical ifdoin
      character*3 prefix 
      real*4 papi_mflops
      integer*8 papi_flops
      integer icall, kstep, knstep, i

      pstep=2
      if (param(103).gt.0)   call q_filter      (param(103))
      call setup_convect (pstep)  ! Save convective velocity _after_ filter

      call userchk

      prefix='his'
      ifdoin=.false. 
      call prepost (ifdoin,prefix)
      call in_situ_check()

      if (mod(istep,nsteps).eq.0) lastep=1
      call nek_comm_settings(isyc,0)
      call comment

      return
      end
C=======================================================================
      subroutine nek_pointscloud()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE'
      integer e,f
      integer tots_np(lp), iwk(lp)

      nxyz=lx1*ly1*lz1

      nw_bd1 = 0
      do e=1,nelt
      eg = lglel(e)
      do f=1,6
        if (cbc(f,e,2).eq.'f  ') then
           nw_bd1=nw_bd1+1
           plmask(e) =1
           pflmask(e)=f 
           pplist(e)=nw_bd1
        endif
      enddo
      enddo
            
      tots_np(nid+1)=nw_bd1
      call igop(tots_np,iwk,'+  ', np)

      offset=0
      if (nid.gt.0) then
        do i=1,nid
        offset=offset+tots_np(i)
        enddo
      endif
 
      do i=1,nelt
         pplist(e)=pplist(e)+offset 
      enddo

      nw_bdt=0
      do i=1,np
        nw_bdt = nw_bdt + tots_np(i)
      enddo
 
      call assign_p(pc_x,xm1)
      call assign_p(pc_y,ym1)
      call assign_p(pc_z,zm1)

      do i=1,nw_bdt
      do j=1,4
         pc_flag((i-1)*4+j)=dble(i)
      enddo 
      enddo 

      return
      end
c-----------------------------------------------------------------------
      subroutine assign_p (vpf,fu)
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE' 
      integer e,eg
      real*8 vpf(lsurf_m*4)
      real*8 fu(lx1,lx1,lx1,lelt)

      integer i_us, ip_stp, iv_istp

      real*8 wtmp(lsurf_m*4)

      do i=1,4*lsurf_m
      vpf(i)=0.0
      enddo

      do e=1,nelt

      if (plmask(e).eq.1)  then

      i_us=(pplist(e)-1)*4
 
      if (pflmask(e).eq.1) then
      vpf(i_us)=fu(1,1,1,e)
      vpf(i_us+1)=fu(lx1,1,1,e)
      vpf(i_us+3)=fu(1,1,lx1,e)
      vpf(i_us+2)=fu(lx1,1,lx1,e)
      endif

      if (pflmask(e).eq.2) then
      vpf(i_us)=fu(lx1,1,1,e)
      vpf(i_us+1)=fu(lx1,lx1,1,e)
      vpf(i_us+3)=fu(lx1,1,lx1,e)
      vpf(i_us+2)=fu(lx1,lx1,lx1,e)
      endif

      if (pflmask(e).eq.3)  then
      vpf(i_us)=fu(1,lx1,1,e)
      vpf(i_us+1)=fu(lx1,lx1,1,e)
      vpf(i_us+3)=fu(1,lx1,lx1,e)
      vpf(i_us+2)=fu(lx1,lx1,lx1,e)
      endif

      if (pflmask(e).eq.4) then
      vpf(i_us)=fu(1,1,1,e)
      vpf(i_us+1)=fu(1,lx1,1,e)
      vpf(i_us+3)=fu(1,1,lx1,e)
      vpf(i_us+2)=fu(1,lx1,lx1,e)
      endif

      if (pflmask(e).eq.5) then
      vpf(i_us)=fu(1,1,1,e)
      vpf(i_us+1)=fu(lx1,1,1,e)
      vpf(i_us+3)=fu(1,lx1,1,e)
      vpf(i_us+2)=fu(lx1,lx1,1,e)
      endif

      if (pflmask(e).eq.6) then
      vpf(i_us)=fu(1,1,lx1,e)
      vpf(i_us+1)=fu(lx1,1,lx1,e)
      vpf(i_us+3)=fu(1,lx1,lx1,e)
      vpf(i_us+2)=fu(lx1,lx1,lx1,e)
      endif

      endif
      enddo

      call  gop(vpf,wtmp,'+  ',4*nw_bdt)

      return
      end
c-----------------------------------------------------------------------
      subroutine rassign (fu,vpf,nn)
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE' 
      integer e,eg

      real*8 vpf(lsurf_m*4)
      real*8 fu(3,3,3,lelt)

c     Placeholder

      return
      end
C=======================================================================
      subroutine nek_interpolation()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE' 
      integer e,f

      call assign_p(pc_t,t)
 
      return
      end
C=======================================================================
      subroutine flux_reconstruction()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE'  
      integer e,f
      ntot=nx1*ny1*nz1*nelt

!     Call to rassign and spectral interpolation
      do e=1,nelt 
        do f=1,6
          call surface_int(sint,sarea,flux_recon,e,f)
          if (cbc(f,e,1).eq.'W  ') then
           sint1=sint1+sint
           sarea1=sarea1+sarea
          endif
         enddo
      enddo

      call  gop(sint1,wtmp,'+  ',1)
      call  gop(sarea1,wtmp,'+  ',1)

      flux_moose=sint1/sarea1

      return
      end
c-----------------------------------------------------------------------
      subroutine cdscal_mod (igeom)
C
C     Solve the convection-diffusion equation for passive scalar IPSCAL
C
      include 'SIZE'
      include 'INPUT'
      include 'GEOM'
      include 'MVGEOM'
      include 'SOLN'
      include 'MASS'
      include 'TSTEP'
      COMMON  /CPRINT/ IFPRINT
      LOGICAL          IFPRINT
      LOGICAL          IFCONV
C
      COMMON /SCRNS/ TA(LX1,LY1,LZ1,LELT)
     $              ,TB(LX1,LY1,LZ1,LELT)
      COMMON /SCRVH/ H1(LX1,LY1,LZ1,LELT)
     $              ,H2(LX1,LY1,LZ1,LELT)

      include 'ORTHOT'

      if (ifdg) then
         call cdscal_dg(igeom)
         return
      endif


c      napprox(1) = laxt  ! Fix this... pff 10/10/15

      nel    = nelfld(ifield)
      n   = nx1*ny1*nz1*nel

      if (igeom.eq.1) then   ! geometry at t^{n-1}

         call makeq
         call lagscal

      else                   ! geometry at t^n

         IF (IFPRINT) THEN
         IF (IFMODEL .AND. IFKEPS) THEN
            NFLDT = NFIELD - 1
            IF (IFIELD.EQ.NFLDT.AND.NID.EQ.0) THEN
               WRITE (6,*) ' Turbulence Model - k/epsilon solution'
            ENDIF
         ELSE
            IF (IFIELD.EQ.2.AND.NID.EQ.0) THEN
               WRITE (6,*) ' Temperature/Passive scalar solution'
            ENDIF
         ENDIF
         ENDIF
         if1=ifield-1
         write(name4,1) if1-1
    1    format('PS',i2)
         if(ifield.eq.2) write(name4,'(A4)') 'TEMP'

C
C        New geometry
C
         isd = 1
         if (ifaxis.and.ifaziv.and.ifield.eq.2) isd = 2
c        if (ifaxis.and.ifmhd) isd = 2 !This is a problem if T is to be T!

         do 1000 iter=1,nmxnl ! iterate for nonlin. prob. (e.g. radiation b.c.)

         INTYPE = 0
         IF (IFTRAN) INTYPE = -1
         CALL SETHLM  (H1,H2,INTYPE)
         CALL BCNEUSC (TA,-1)
         CALL ADD2    (H2,TA,N)
         CALL BCDIRSC (T(1,1,1,1,IFIELD-1))
         CALL AXHELM  (TA,T(1,1,1,1,IFIELD-1),H1,H2,IMESH,isd)
         CALL SUB3    (TB,BQ(1,1,1,1,IFIELD-1),TA,N)
         CALL BCNEUSC (TA,1)
         CALL ADD2    (TB,TA,N)

c        CALL HMHOLTZ (name4,TA,TB,H1,H2
c    $                 ,TMASK(1,1,1,1,IFIELD-1)
c    $                 ,TMULT(1,1,1,1,IFIELD-1)
c    $                 ,IMESH,TOLHT(IFIELD),NMXH,isd)

         if(iftmsh(ifield)) then
           call hsolve  (name4,TA,TB,H1,H2 
     $                   ,tmask(1,1,1,1,ifield-1)
     $                   ,tmult(1,1,1,1,ifield-1)
     $                   ,imesh,tolht(ifield),nmxh,1
     $                   ,approx,napprox,bintm1)
         else
           call hsolve  (name4,TA,TB,H1,H2 
     $                   ,tmask(1,1,1,1,ifield-1)
     $                   ,tmult(1,1,1,1,ifield-1)
     $                   ,imesh,tolht(ifield),nmxh,1
     $                   ,approx,napprox,binvm1)
         endif 

         call add2    (t(1,1,1,1,ifield-1),ta,n)

         call cvgnlps (ifconv) ! Check convergence for nonlinear problem 
         if (ifconv) goto 2000

C        Radiation case, smooth convergence, avoid flip-flop (ER).
         CALL CMULT (TA,0.5,N)
         CALL SUB2  (T(1,1,1,1,IFIELD-1),TA,N)

 1000    CONTINUE
 2000    CONTINUE
         CALL BCNEUSC (TA,1)

      endif

      return
      end
c-----------------------------------------------------------------------
      subroutine heat_mod (igeom)
C
C     Driver for temperature or passive scalar.
C
C     Current version:
C     (1) Varaiable properties.
C     (2) Implicit time stepping.
C     (3) User specified tolerance for the Helmholtz solver
C         (not based on eigenvalues).
C     (4) A passive scalar can be defined on either the 
C         temperatur or the velocity mesh.
C     (5) A passive scalar has its own multiplicity (B.C.).  
C
      include 'SIZE'
      include 'INPUT'
      include 'TSTEP'
      include 'DEALIAS'

      real*8 ts, dnekclock

      ts = dnekclock()

      if (nio.eq.0 .and. igeom.eq.2) 
     &    write(*,'(13x,a)') 'Solving for Hmholtz scalars'

      do ifield = 2,nfield
         if (idpss(ifield-1).eq.0) then      ! helmholtz
            intype        = -1
            if (.not.iftmsh(ifield)) imesh = 1
            if (     iftmsh(ifield)) imesh = 2
            call unorm
            call settolt
            call cdscal_mod(igeom)
         endif
      enddo

      if (nio.eq.0 .and. igeom.eq.2)
     &   write(*,'(4x,i7,a,1p2e12.4)') 
     &   istep,'  Scalars done',time,dnekclock()-ts

      return
      end
c-----------------------------------------------------------------------
      subroutine nek_advance_moose

      include 'SIZE'
      include 'TOTAL'
      include 'CTIMER'

      common /cgeom/ igeom

      call nekgsync
      if (iftran) call settime
      if (ifmhd ) call cfl_check
      call setsolv
      call comment

c      if (ifcmt) then
c         if (nio.eq.0.and.istep.le.1) write(6,*) 'CMT branch active'
c         call cmt_nek_advance
c         return
c      endif

      if (ifsplit) then   ! PN/PN formulation

         do igeom=1,ngeom

         ! within cvode we use the lagged wx for 
         ! extrapolation, that's why we have to call it before gengeom 
         if (ifheat .and. ifcvode) call heat_cvode (igeom)   

         if (ifgeom) then
               call gengeom (igeom)
               call geneig  (igeom)
         endif

         if (ifheat)               call heat_mod (igeom)

         if (igeom.eq.2) then  
                                   call setprop
            if (iflomach)          call qthermal(.true.,.false.,dummy)
         endif

         if (ifflow)               call fluid         (igeom)
         if (ifmvbd)               call meshv         (igeom)
         if (param(103).gt.0)      call q_filter      (param(103))
                                   call setup_convect (igeom)     ! Save convective velocity _after_ filter 
         enddo

      else                ! PN-2/PN-2 formulation

         call setprop
         do igeom=1,ngeom

            if (igeom.gt.2) call userchk_set_xfer

            if (ifgeom) then
               call gengeom (igeom)
               call geneig  (igeom)
            endif

            if (ifneknekm.and.igeom.eq.2) call multimesh_create

            if (ifmhd) then
               if (ifheat)      call heat     (igeom)
                                call induct   (igeom)
            elseif (ifpert) then
               if (ifbase.and.ifheat)  call heat          (igeom)
               if (ifbase.and.ifflow)  call fluid         (igeom)
               if (ifflow)             call fluidp        (igeom)
               if (ifheat)             call heatp         (igeom)
            else  ! std. nek case
               if (ifheat)             call heat_mod      (igeom)
               if (ifflow)             call fluid         (igeom)
               if (ifmvbd)             call meshv         (igeom)
            endif

            if (igeom.eq.ngeom.and.param(103).gt.0) 
     $          call q_filter(param(103))

            call setup_convect (igeom) ! Save convective velocity _after_ filter

         enddo
      endif

      return
      end

c-----------------------------------------------------------------------
