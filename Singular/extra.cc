/*****************************************
*  Computer Algebra System SINGULAR      *
*****************************************/
/* $Id: extra.cc,v 1.74 1998-11-12 13:06:11 Singular Exp $ */
/*
* ABSTRACT: general interface to internals of Singular ("system" command)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mod2.h"

#ifndef __MWERKS__
#ifdef TIME_WITH_SYS_TIME
# include <time.h>
# ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
# endif
#else
# ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
# else
#   include <time.h>
# endif
#endif
#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#endif
#include <unistd.h>

#include "tok.h"
#include "ipid.h"
#include "polys.h"
#include "kutil.h"
#include "cntrlc.h"
#include "stairc.h"
#include "ipshell.h"
#include "algmap.h"
#include "modulop.h"
#include "febase.h"
#include "matpol.h"
#include "longalg.h"
#include "ideals.h"
#include "kstd1.h"
#include "syz.h"

// Define to enable many more system commands
#define HAVE_EXTENDED_SYSTEM

#ifdef STDTRACE
//#include "comm.h"
#endif

#ifdef HAVE_FACTORY
#define SI_DONT_HAVE_GLOBAL_VARS
#include "clapsing.h"
#include "clapconv.h"
#include "kstdfac.h"
#endif

#include "silink.h"
#include "mpsr.h"

#ifdef HAVE_DYNAMIC_LOADING
#include <dlfcn.h>
#endif /* HAVE_DYNAMIC_LOADING */

#ifdef HAVE_PCV
#include "pcv.h"
#endif

// see clapsing.cc for a description of the `FACTORY_*' options

#ifdef FACTORY_GCD_STAT
#include "gcd_stat.h"
#endif

#ifdef FACTORY_GCD_TIMING
#define TIMING
#include "timing.h"
TIMING_DEFINE_PRINTPROTO( contentTimer );
TIMING_DEFINE_PRINTPROTO( algContentTimer );
TIMING_DEFINE_PRINTPROTO( algLcmTimer );
#endif

void piShowProcList();
static BOOLEAN jjEXTENDED_SYSTEM(leftv res, leftv h);


//void emStart();
/*2
*  the "system" command
*/
BOOLEAN jjSYSTEM(leftv res, leftv args)
{
  if(args->Typ() == STRING_CMD)
  {
    const char *sys_cmd=(char *)(args->Data());
    leftv h=args->next;
// ONLY documented system calls go here
// Undocumented system calls go down into #ifdef HAVE_EXTENDED_SYSTEM
/*==================== nblocks ==================================*/
    if (strcmp(sys_cmd, "nblocks") == 0)
    {
      ring r;
      if (h == NULL)
      {
        if (currRingHdl != NULL)
        {
          r = IDRING(currRingHdl);
        }
        else
        {
          WerrorS("no ring active");
          return TRUE;
        }
      }
      else
      {
        if (h->Typ() != RING_CMD)
        {
          WerrorS("ring expected");
          return TRUE;
        }
        r = (ring) h->Data();
      }
      res->rtyp = INT_CMD;
      res->data = (void*) (rBlocks(r) - 1);
      return FALSE;
    }
/*==================== version ==================================*/
    if(strcmp(sys_cmd,"version")==0)
    {
      res->rtyp=INT_CMD;
      res->data=(void *)SINGULAR_VERSION;
      return FALSE;
    }
    else
/*==================== gen ==================================*/
    if(strcmp(sys_cmd,"gen")==0)
    {
      res->rtyp=INT_CMD;
      res->data=(void *)npGen;
      return FALSE;
    }
    else
/*==================== sh ==================================*/
    if(strcmp(sys_cmd,"sh")==0)
    {
      res->rtyp=INT_CMD;
      #ifndef __MWERKS__
      if (h==NULL) res->data = (void *)system("sh");
      else if (h->Typ()==STRING_CMD)
        res->data = (void*) system((char*)(h->Data()));
      else
        WerrorS("string expected");
      #else
      res->data=(void *)0;
      #endif
      return FALSE;
    }
    else
/*==================== with ==================================*/
    if(strcmp(sys_cmd,"with")==0)
    {
      if (h==NULL)
      {
        res->rtyp=STRING_CMD;
        res->data=(void *)mstrdup(versionString());
        return FALSE;
      }
      else if (h->Typ()==STRING_CMD)
      {
        #define TEST_FOR(A) if(strcmp(s,A)==0) res->data=(void *)1; else
        char *s=(char *)h->Data();
        res->rtyp=INT_CMD;
        #ifdef DRING
          TEST_FOR("DRING")
        #endif
        #ifdef HAVE_DBM
          TEST_FOR("DBM")
        #endif
        #ifdef HAVE_DLD
          TEST_FOR("DLD")
        #endif
        #ifdef HAVE_FACTORY
          TEST_FOR("factory")
        #endif
        #ifdef HAVE_LIBFAC_P
          TEST_FOR("libfac")
        #endif
        #ifdef HAVE_MPSR
          TEST_FOR("MP")
        #endif
        #ifdef HAVE_READLINE
          TEST_FOR("readline")
        #endif
        #ifdef HAVE_TCL
          TEST_FOR("tcl")
        #endif
        #ifdef SRING
          TEST_FOR("SRING")
        #endif
        #ifdef TEST_MAC_ORDER
          TEST_FOR("MAC_ORDER");
        #endif
        #ifdef HAVE_NAMESPACES
          TEST_FOR("Namespaces");
        #endif
        #ifdef HAVE_DYNAMIC_LOADING
          TEST_FOR("DynamicLoading");
        #endif
          ;
        return FALSE;
        #undef TEST_FOR
      }
      return TRUE;
    }
    else
/*==================== pid ==================================*/
    if (strcmp(sys_cmd,"pid")==0)
    {
      res->rtyp=INT_CMD;
    #ifndef MSDOS
    #ifndef __MWERKS__
      res->data=(void *)getpid();
    #else
      res->data=(void *)1;
    #endif
    #else
      res->data=(void *)1;
    #endif
      return FALSE;
    }
    else
/*==================== getenv ==================================*/
    if (strcmp(sys_cmd,"getenv")==0)
    {
      if ((h!=NULL) && (h->Typ()==STRING_CMD))
      {
        res->rtyp=STRING_CMD;
        char *r=getenv((char *)h->Data());
        if (r==NULL) r="";
        res->data=(void *)mstrdup(r);
        return FALSE;
      }
      else
      {
        WerrorS("string expected");
      }
    }
    else
/*==================== tty ==================================*/
    #ifndef __MWERKS__
    #ifndef MSDOS
    #if defined(HAVE_FEREAD) || defined(HAVE_READLINE)
    if (strcmp(sys_cmd,"tty")==0)
    {
      #if defined(HAVE_READLINE) || defined(HAVE_FEREAD)
      system("stty sane");
      #endif
      if ((h!=NULL)&&(h->Typ()==INT_CMD))
      {
        fe_use_fgets=(int)h->Data();
      }
      return FALSE;
    }
    else
    #endif
    #endif
    #endif
/*==================== Singular ==================================*/
#ifndef __MWERKS__
    if (strcmp(sys_cmd, "Singular") == 0)
    {
      res->rtyp=STRING_CMD;
      char *r=feGetExpandedExecutable();
      if (r != NULL)
        res->data = (void*) mstrdup( r );
      else
        res->data = (void*) mstrdup("");
      return FALSE;
    }
    else
#endif
/*==================== options ==================================*/
    if (strstr(sys_cmd, "--") == sys_cmd)
    {
      BOOLEAN mainGetSingOptionValue(const char* name, char** result);
      char* val;

      if (mainGetSingOptionValue(&(sys_cmd)[2], &val))
      {
        if ((unsigned int) val > 1)
        {
          res->rtyp=STRING_CMD;
          res->data = (void*) mstrdup( val );
        }
        else
        {
          res->rtyp=INT_CMD;
          res->data=(void *)val;
        }
        return FALSE;
      }
      else
      {
        Werror("Unknown option %s\n", sys_cmd);
        return TRUE;
      }
    }
    else
/*==================== HC ==================================*/
    if (strcmp(sys_cmd,"HC")==0)
    {
      res->rtyp=INT_CMD;
      res->data=(void *)HCord;
      return FALSE;
    }
    else
/*==================== random ==================================*/
    if(strcmp(sys_cmd,"random")==0)
    {
      if ((h!=NULL) &&(h->Typ()==INT_CMD))
      {
        siRandomStart=(int)h->Data();
#ifdef buildin_rand
        siSeed=siRandomStart;
#else
        srand((unsigned int)siRandomStart);
#endif
        return FALSE;
      }
      else if (h != NULL)
      {
        WerrorS("int expected");
        return TRUE;
      }
      res->rtyp=INT_CMD;
      res->data=(void*) siRandomStart;
      return FALSE;
    }
/*==================== neworder =============================*/
// should go below
#ifdef HAVE_LIBFAC_P
    if(strcmp(sys_cmd,"neworder")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        res->rtyp=STRING_CMD;
        res->data=(void *)singclap_neworder((ideal)h->Data());
        return FALSE;
      }
      else
        WerrorS("ideal expected");
    }
    else
#endif
/*==================== contributors =============================*/
   if(strcmp(sys_cmd,"contributors") == 0)
   {
     res->rtyp=STRING_CMD;
     res->data=(void *)mstrdup(
       "Olaf Bachmann, Hubert Grassmann, Kai Krueger, Wolfgang Neumann, Thomas Nuessler, Wilfred Pohl, Thomas Siebert, Ruediger Stobbe, Tim Wichmann");
     return FALSE;
   }
   else
   {
/*================= Extended system call ========================*/
#ifdef HAVE_EXTENDED_SYSTEM
     return(jjEXTENDED_SYSTEM(res, args));
#else
     Werror( "system(\"%s\",...) %s", sys_cmd, feNotImplemented );
#endif
   }
  } /* typ==string */
  return TRUE;
}



#ifdef HAVE_EXTENDED_SYSTEM
// You can put your own system calls here
#include "fglmcomb.cc"
#include "fglm.h"
#ifdef HAVE_NEWTON
#include <hc_newton.h>
#endif
#include "mpsr.h"

static BOOLEAN jjEXTENDED_SYSTEM(leftv res, leftv h)
{
  if(h->Typ() == STRING_CMD)
  {
    char *sys_cmd=(char *)(h->Data());
    h=h->next;
/*==================== pcv ==================================*/
#ifdef HAVE_PCV
    if(strcmp(sys_cmd,"pcvConv")==0)
    {
      return iiPcvConv(res, h);
    }
    else if(strcmp(sys_cmd,"pcvDim")==0)
    {
      return iiPcvDim(res, h);
    }
    else if(strcmp(sys_cmd,"pcvBasis")==0)
    {
      return iiPcvBasis(res, h);
    }
    else if(strcmp(sys_cmd,"pcvOrd")==0)
    {
      return iiPcvOrd(res,h);
    }
    else
#endif
/*==================== naIdeal ==================================*/
    if(strcmp(sys_cmd,"naIdeal")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        naSetIdeal((ideal)h->Data());
        return FALSE;
      }
      else
         WerrorS("ideal expected");
    }
    else
/*==================== isSqrFree =============================*/
#ifdef HAVE_FACTORY
    if(strcmp(sys_cmd,"isSqrFree")==0)
    {
      if ((h!=NULL) &&(h->Typ()==POLY_CMD))
      {
        res->rtyp=INT_CMD;
        res->data=(void *)singclap_isSqrFree((poly)h->Data());
        return FALSE;
      }
      else
        WerrorS("poly expected");
    }
    else
#endif
/*==================== alarm ==================================*/
#ifndef __MWERKS__
#ifndef MSDOS
#ifndef atarist
#ifdef unix
    if(strcmp(sys_cmd,"alarm")==0)
    {
      if ((h!=NULL) &&(h->Typ()==INT_CMD))
      {
        // standard variant -> SIGALARM (standard: abort)
        //alarm((unsigned)h->next->Data());
        // process time (user +system): SIGVTALARM
        struct itimerval t,o;
        memset(&t,0,sizeof(t));
        t.it_value.tv_sec     =(unsigned)h->Data();
        setitimer(ITIMER_VIRTUAL,&t,&o);
        return FALSE;
      }
      else
        WerrorS("int expected");
    }
    else
#endif
#endif
#endif
#endif
/*==================== red =============================*/
#if 0
    if(strcmp(sys_cmd,"red")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        res->rtyp=IDEAL_CMD;
        res->data=(void *)kStdred((ideal)h->Data(),NULL,testHomog,NULL);
        setFlag(res,FLAG_STD);
        return FALSE;
      }
      else
        WerrorS("ideal expected");
    }
    else
#endif
/*==================== algfetch =====================*/
    if (strcmp(sys_cmd,"algfetch")==0)
    {
      int k;
      idhdl w;
      ideal i0, i1;
      ring r0=(ring)h->Data();
      leftv v = h->next;
      w = r0->idroot->get(v->Name(),myynest);
      i0 = IDIDEAL(w);
      i1 = idInit(IDELEMS(i0),i0->rank);
      for (k=0; k<IDELEMS(i1); k++)
      {
        i1->m[k] = maAlgpolyFetch(r0, i0->m[k]);
      }
      res->rtyp = IDEAL_CMD;
      res->data = (void*)i1;
      return FALSE;
    }
    else
/*==================== algmap =======================*/
    if (strcmp(sys_cmd,"algmap")==0)
    {
      int k;
      idhdl w;
      ideal i0, i1, i, j;
      ring r0=(ring)h->Data();
      leftv v = h->next;
      w = r0->idroot->get(v->Name(),myynest);
      i0 = IDIDEAL(w);
      v = v->next;
      i = (ideal)v->Data();
      v = v->next;
      j = (ideal)v->Data();
      i1 = idInit(IDELEMS(i0),i0->rank);
      for (k=0; k<IDELEMS(i1); k++)
      {
        i1->m[k] = maAlgpolyMap(r0, i0->m[k], i, j);
      }
      res->rtyp = IDEAL_CMD;
      res->data = (void*)i1;
      return FALSE;
    }
    else
    /*==================== trace =============================*/
#ifdef STDTRACE
    /* Parameter : Ideal, Liste mit Links. */
    if(strcmp(sys_cmd,"stdtrace")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        leftv root  = NULL,
              ptr   = NULL,
              lv    = NULL;
        lists l     = NULL;
        ideal I     = (ideal)(h->Data());
        lists links = (lists)(h->next->Data());
        tHomog hom  = testHomog;
        int rw      = (int)(h->next->next->Data());

        if(I==NULL)
          PrintS("I==NULL\n");
        for(int i=0; i <= links->nr ; i++)
        {
          lv = (leftv)Alloc0(sizeof(sleftv));
          lv->Copy(&(links->m[i]));
          if(root==NULL)
          root=lv;
          if(ptr==NULL)
          {
            ptr=lv;
            ptr->next=NULL;
          }
          else
          {
            ptr->next=lv;
            ptr=lv;
          }
        }
        ptr->next=NULL;
        l=TraceStd(root,rw,I,currQuotient,testHomog,NULL);
        idSkipZeroes(((ideal)l->m[0].Data()));
        res->rtyp=LIST_CMD;
        res->data=(void *) l;
        res->next=NULL;
        root->CleanUp();
        Free(root,sizeof(sleftv));
        return FALSE;
      }
      else
         WerrorS("ideal expected");
    }
    else
#endif
#ifdef HAVE_FACTORY
/*==================== fastcomb =============================*/
    if(strcmp(sys_cmd,"fastcomb")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        int i=0;
        if (h->next!=NULL)
        {
          if (h->next->Typ()!=POLY_CMD)
          {
            Warn("Wrong types for poly= comb(ideal,poly)");
          }
        }
        res->rtyp=POLY_CMD;
        res->data=(void *) fglmLinearCombination(
                           (ideal)h->Data(),(poly)h->next->Data());
        return FALSE;
      }
      else
        WerrorS("ideal expected");
    }
    else
/*==================== comb =============================*/
    if(strcmp(sys_cmd,"comb")==0)
    {
      if ((h!=NULL) &&(h->Typ()==IDEAL_CMD))
      {
        int i=0;
        if (h->next!=NULL)
        {
          if (h->next->Typ()!=POLY_CMD)
          {
              Warn("Wrong types for poly= comb(ideal,poly)");
          }
        }
        res->rtyp=POLY_CMD;
        res->data=(void *)fglmNewLinearCombination(
                            (ideal)h->Data(),(poly)h->next->Data());
        return FALSE;
      }
      else
        WerrorS("ideal expected");
    }
    else
#endif
/*==================== barstep =============================*/
    if(strcmp(sys_cmd,"barstep")==0)
    {
      if ((h!=NULL) &&(h->Typ()==MATRIX_CMD))
      {
        if (h->next!=NULL)
        {
          if (h->next->Typ()!=POLY_CMD)
          {
            Warn("Wrong types for barstep(matrix,poly)");
          }
        }
        int r,c;
        poly div=(poly)h->next->Data();
        res->rtyp=MATRIX_CMD;
        res->data=(void *)mpOneStepBareiss((matrix)h->Data(),
                                           &div,&r,&c);
        PrintS("div: ");pWrite(div);
        Print("rows: %d, cols: %d\n",r,c);
        pDelete(&div);
        return FALSE;
      }
      else
        WerrorS("matrix expected");
    }
    else
#ifdef FACTORY_GCD_TEST
/*=======================gcd Testerei ================================*/
    if ( ! strcmp( sys_cmd, "setgcd" ) ) {
        if ( (h != NULL) && (h->Typ() == INT_CMD) ) {
            CFPrimitiveGcdUtil::setAlgorithm( (int)h->Data() );
            return FALSE;
        } else
            WerrorS("int expected");
    }
    else
#endif

#ifdef FACTORY_GCD_TIMING
    if ( ! strcmp( sys_cmd, "gcdtime" ) ) {
        TIMING_PRINT( contentTimer, "time used for content: " );
        TIMING_PRINT( algContentTimer, "time used for algContent: " );
        TIMING_PRINT( algLcmTimer, "time used for algLcm: " );
        TIMING_RESET( contentTimer );
        TIMING_RESET( algContentTimer );
        TIMING_RESET( algLcmTimer );
        return FALSE;
    }
    else
#endif

#ifdef FACTORY_GCD_STAT
    if ( ! strcmp( sys_cmd, "gcdstat" ) ) {
        printGcdTotal();
        printContTotal();
        resetGcdTotal();
        resetContTotal();
        return FALSE;
    }
    else
#endif
/*==================== lib ==================================*/
    if(strcmp(sys_cmd,"LIB")==0)
    {
#ifdef HAVE_NAMESPACES
      idhdl hh=namespaceroot->get((char*)h->Data(),0);
#else /* HAVE_NAMESPACES */
      idhdl hh=idroot->get((char*)h->Data(),0);
#endif /* HAVE_NAMESPACES */
      if ((hh!=NULL)&&(IDTYP(hh)==PROC_CMD))
      {
        res->rtyp=STRING_CMD;
        char *r=iiGetLibName(IDPROC(hh));
        if (r==NULL) r="";
        res->data=mstrdup(r);
        return FALSE;
      }
      else
        Warn("`%s` not found",(char*)h->Data());
    }
    else
#ifdef HAVE_NAMESPACES
/*==================== lib ==================================*/
    if(strcmp(sys_cmd,"nn")==0)
    {
      if ((h!=NULL) && (h->Typ()==STRING_CMD))
      {
        idhdl pck = NULL, id = NULL;
        iiname2hdl(h->Data(), &pck, &id);
        if(pck != NULL) Print("Pack: '%s'\n", pck->id);
        if(id != NULL)  Print("Rec : '%s'\n", id->id);
        return FALSE;
      }
      else
        Warn("`%s` not found",(char*)h->Data());
    }
    else
/*==================== nspush ===================================*/
    if(strcmp(sys_cmd,"nspush")==0)
    {
      if (h->Typ()==PACKAGE_CMD)
      {
        idhdl hh=(idhdl)h->data;
        namespaceroot = namespaceroot->push(IDPACKAGE(hh), IDID(hh));
        return FALSE;
      }
      else
        Warn("argument 2 is not a package");
    }
    else
/*==================== nspop ====================================*/
    if(strcmp(sys_cmd,"nspop")==0)
    {
      namespaceroot->pop();
      return FALSE;
    }
    else
#endif /* HAVE_NAMESPACES */
/*==================== nsstack ===================================*/
    if(strcmp(sys_cmd,"nsstack")==0)
    {
      namehdl nshdl = namespaceroot;
      for( ; nshdl->isroot != TRUE; nshdl = nshdl->next) {
        Print("NSstack: %s:%d, nesting=%d\n", nshdl->name, nshdl->lev, nshdl->myynest);
      }
      Print("NSstack: %s:%d, nesting=%d\n", nshdl->name, nshdl->lev, nshdl->myynest);
      return FALSE;
    }
    else
/*==================== proclist =================================*/
    if(strcmp(sys_cmd,"proclist")==0)
    {
      piShowProcList();
      return FALSE;
    }
    else
#ifdef HAVE_DYNAMIC_LOADING
/*==================== load ==================================*/
    if(strcmp(sys_cmd,"load")==0)
    {
      if ((h!=NULL) && (h->Typ()==STRING_CMD)) {
        int iiAddCproc(char *libname, char *procname, BOOLEAN pstatic,
                       BOOLEAN(*func)(leftv res, leftv v));
        int (*fktn)(int(*iiAddCproc)(char *libname, char *procname,
                                     BOOLEAN pstatic,
                                     BOOLEAN(*func)(leftv res, leftv v)));
        void *vp;
        res->rtyp=STRING_CMD;

        fprintf(stderr, "Loading %s\n", h->Data());
        res->data=(void *)mstrdup("");
        if((vp=dlopen(h->next->Data(),RTLD_LAZY))==(void *)NULL)
        {
          WerrorS("dlopen failed");
          Werror("%s not found", h->Data());
        }
        else
        {
          fktn = dlsym(vp, "mod_init");
          if( fktn!= NULL) (*fktn)(iiAddCproc);
          else Werror("mod_init: %s\n", dlerror());
          piShowProcList();
        }
        return FALSE;
      }
      else WerrorS("string expected");
    }
    else
#endif /* HAVE_DYNAMIC_LOADING */
/* ==================== newton ================================*/
#ifdef HAVE_NEWTON
    if(strcmp(sys_cmd,"newton")==0)
    {
      if ((h->Typ()!=POLY_CMD)
      || (h->next->Typ()!=INT_CMD)
      || (h->next->next->Typ()!=INT_CMD))
      {
        WerrorS("system(\"newton\",<poly>,<int>,<int>) expected");
        return TRUE;
      }
      poly  p=(poly)(h->Data());
      int l=pLength(p);
      short *points=(short *)Alloc(currRing->N*l*sizeof(short));
      int i,j,k;
      k=0;
      poly pp=p;
      for (i=0;pp!=NULL;i++)
      {
        for(j=1;j<=currRing->N;j++)
        {
          points[k]=pGetExp(pp,j);
          k++;
        }
        pIter(pp);
      }
      hc_ERG r=hc_KOENIG(currRing->N,      // dimension
                l,      // number of points
                (short*) points,   // points: x_1, y_1,z_1, x_2,y_2,z2,...
                currRing->OrdSgn==-1,
                (int) (h->next->Data()),      // 1: Milnor, 0: Newton
                (int) (h->next->next->Data()) // debug
               );
      //----<>---Output-----------------------


//  PrintS("Bin jetzt in extra.cc bei der Auswertung.\n"); // **********


      lists L=(lists)Alloc(sizeof(slists));
      L->Init(6);
      L->m[0].rtyp=STRING_CMD;               // newtonnumber;
      L->m[0].data=(void *)mstrdup(r.nZahl);
      L->m[1].rtyp=INT_CMD;
      L->m[1].data=(void *)r.achse;          // flag for unoccupied axes
      L->m[2].rtyp=INT_CMD;
      L->m[2].data=(void *)r.deg;            // #degenerations
      if ( r.deg != 0)              // only if degenerations exist
      {
        L->m[3].rtyp=INT_CMD;
        L->m[3].data=(void *)r.anz_punkte;     // #points
        //---<>--number of points------
        int anz = r.anz_punkte;    // number of points
        int dim = (currRing->N);     // dimension
        intvec* v = new intvec( anz*dim );
        for (i=0; i<anz*dim; i++)    // copy points
          (*v)[i] = r.pu[i];
        L->m[4].rtyp=INTVEC_CMD;
        L->m[4].data=(void *)v;
        //---<>--degenerations---------
        int deg = r.deg;    // number of points
        intvec* w = new intvec( r.speicher );  // necessary memeory
        i=0;               // start copying
        do
        {
          (*w)[i] = r.deg_tab[i];
          i++;
        }
        while (r.deg_tab[i-1] != -2);   // mark for end of list
        L->m[5].rtyp=INTVEC_CMD;
        L->m[5].data=(void *)w;
      }
      else
      {
        L->m[3].rtyp=INT_CMD; L->m[3].data=(char *)0;
        L->m[4].rtyp=DEF_CMD;
        L->m[5].rtyp=DEF_CMD;
      }

      res->data=(void *)L;
      res->rtyp=LIST_CMD;
      // free all pointer in r:
      delete[] r.nZahl;
      delete[] r.pu;
      delete[] r.deg_tab;      // Ist das ein Problem??

      Free((ADDRESS)points,currRing->N*l*sizeof(short));
      return FALSE;
    }
    else
#endif
/*==================== gp =================*/
     if (strcmp(sys_cmd, "gp") == 0)
    {
      if (h->Typ() != LINK_CMD)
      {
        WerrorS("No Link arg");
        return FALSE;
      }
      si_link l = (si_link) h->Data();
      if (strcmp(l->m->type, "MPfile") != 0)
      {
        WerrorS("No MPfile link");
        return TRUE;
      }
      if( ! SI_LINK_R_OPEN_P(l)) // open r ?
      {
        if (slOpen(l, SI_LINK_READ)) return TRUE;
      }
      
      MP_Link_pt link = (MP_Link_pt) l->data;
      if (MP_InitMsg(link) != MP_Success) 
      {
        WerrorS("Can not Init");
      }
      MPT_Tree_pt tree = NULL;
      if (MPT_GetTree(link, &tree) != MPT_Success)
      {
        WerrorS("Can not get tree");
        return TRUE;
      }
      MPT_DeleteTree(tree);
      return FALSE;
    }
    else
/*==================== print all option values =================*/
#ifndef NDEBUG
    if (strcmp(sys_cmd, "options") == 0)
    {
      void mainOptionValues();
      mainOptionValues();
      return FALSE;
    }
    else
#endif
      Werror( "system(\"%s\",...) %s", sys_cmd, feNotImplemented );
  }
  return TRUE;
}
#endif // HAVE_EXTENDED_SYSTEM
/*============================================================*/
