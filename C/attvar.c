/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		attvar.c						 *
* Last rev:								 *
* mods:									 *
* comments:	YAP support for attributed vars				 *
*									 *
*************************************************************************/
#ifdef SCCS
static char SccsId[]="%W% %G%";
#endif

#include "Yap.h"

#include "Yatom.h"
#include "Heap.h"
#include "heapgc.h"
#include "attvar.h"
#ifndef NULL
#define NULL (void *)0
#endif

#ifdef COROUTINING

static CELL *
AddToQueue(attvar_record *attv)
{
  Term t[2];
  Term WGs, ng;

  t[0] = (CELL)&(attv->Done);
  t[1] = attv->Value;
  /* follow the chain */
  WGs = Yap_ReadTimedVar(WokenGoals);
  ng = Yap_MkApplTerm(FunctorAttGoal, 2, t);

  Yap_UpdateTimedVar(WokenGoals, MkPairTerm(ng, WGs));
  if ((Term)WGs == TermNil) {
    /* from now on, we have to start waking up goals */
    Yap_signal(YAP_WAKEUP_SIGNAL);
  }
  return(RepAppl(ng)+2);
}

static void
AddFailToQueue(void)
{
  Term WGs;

  /* follow the chain */
  WGs = Yap_ReadTimedVar(WokenGoals);

  Yap_UpdateTimedVar(WokenGoals, MkPairTerm(MkAtomTerm(AtomFail),WGs));
  if ((Term)WGs == TermNil) {
    /* from now on, we have to start waking up goals */
    Yap_signal(YAP_WAKEUP_SIGNAL);
  }
}

static int
CopyAttVar(CELL *orig, CELL ***to_visit_ptr, CELL *res)
{
  register attvar_record *attv = (attvar_record *)orig;
  register attvar_record *newv;
  CELL **to_visit = *to_visit_ptr;
  CELL *vt;

  /* add a new attributed variable */
  newv = DelayTop();
  if (H0 - (CELL *)newv < 1024)
    return FALSE;
  RESET_VARIABLE(&(newv->Value));
  RESET_VARIABLE(&(newv->Done));
  vt = &(attv->Atts);
  to_visit[0] = vt-1;
  to_visit[1] = vt;
  if (IsVarTerm(attv->Atts)) {
    newv->Atts = (CELL)H;
    to_visit[2] = H;
    H++;
  } else {
    to_visit[2] = &(newv->Atts);
  }
  to_visit[3] = (CELL *)vt[-1];
  *to_visit_ptr = to_visit+4;
  *res = (CELL)&(newv->Done);
  SetDelayTop(newv+1);
  return TRUE;
}

static Term
AttVarToTerm(CELL *orig)
{
  attvar_record *attv = (attvar_record *)orig;

  return attv->Atts;
}

static attvar_record *
BuildNewAttVar(void)
{
  attvar_record *attv = DelayTop();
  if (H0 - (CELL *)(attv+1) < 1024) {
    return NULL;
  }
  RESET_VARIABLE(&(attv->Done));
  RESET_VARIABLE(&(attv->Value));
  RESET_VARIABLE(&(attv->Atts));
  SetDelayTop(attv+1);  
  return attv;
}

static int
TermToAttVar(Term attvar, Term to)
{
  attvar_record *attv = BuildNewAttVar();
  if (!attv)
    return FALSE;
  attv->Atts = attvar;
  *VarOfTerm(to) = (CELL)attv;
  return TRUE;
}

static void
WakeAttVar(CELL* pt1, CELL reg2)
{
  
  /* if bound to someone else, follow until we find the last one */
  attvar_record *attv = (attvar_record *)pt1;
  CELL *myH = H;
  CELL *bind_ptr;

  if (IsVarTerm(reg2)) {
    if (pt1 == VarOfTerm(reg2))
      return;
    if (IsAttachedTerm(reg2)) {
      attvar_record *susp2 = (attvar_record *)VarOfTerm(reg2);

      /* binding two suspended variables, be careful */
      if (susp2 >= attv) {
	if (!IsVarTerm(susp2->Value) || !IsUnboundVar(&susp2->Value)) {
	  /* oops, our goal is on the queue to be woken */
	  if (!Yap_unify(susp2->Value, (CELL)pt1)) {
	    AddFailToQueue();
	  }
	}
	Bind_Global(&(susp2->Value), (CELL)pt1);
	AddToQueue(susp2);
	return;
      }
    } else {
      Bind(VarOfTerm(reg2), (CELL)pt1);
      return;
    }
  }
  if (!IsVarTerm(attv->Value) || !IsUnboundVar(&attv->Value)) {
    /* oops, our goal is on the queue to be woken */
    if (!Yap_unify(attv->Value, reg2)) {
      AddFailToQueue();
    }
    return;
  }
  bind_ptr = AddToQueue(attv);
  if (IsNonVarTerm(reg2)) {
    if (IsPairTerm(reg2) && RepPair(reg2) == myH)
      reg2 = AbsPair(H);
    else if (IsApplTerm(reg2) && RepAppl(reg2) == myH)
      reg2 = AbsAppl(H);
  }
  *bind_ptr = reg2;
  Bind_Global(&(attv->Value), reg2);
}

void
Yap_WakeUp(CELL *pt0) {
  CELL d0 = *pt0;
  RESET_VARIABLE(pt0);
  TR--;
  WakeAttVar(pt0, d0);
}


static void
mark_attvar(CELL *orig)
{
  register attvar_record *attv = (attvar_record *)orig;

  Yap_mark_external_reference(&(attv->Value));
  Yap_mark_external_reference(&(attv->Done));
  Yap_mark_external_reference(&(attv->Atts));
}

#if FROZEN_STACKS
static Term
CurrentTime(void) {
  return(MkIntegerTerm(TR-(tr_fr_ptr)Yap_TrailBase));
}
#endif

static Term
BuildAttTerm(Functor mfun, UInt ar)
{
  CELL *h0 = H;
  UInt i;

  if (H+(1024+ar) > ASP) {
    return 0L;
  }
  H[0] = (CELL)mfun;
  RESET_VARIABLE(H+1);
  H += 2;
  for (i = 1; i< ar; i++) {
    *H = TermFoundVar;
    H++;
  }
  return AbsAppl(h0);
}

static Term 
SearchAttsForModule(Term start, Functor mfun)
{
  do {
    if (IsVarTerm(start) ||
	FunctorOfTerm(start) == mfun)
      return start;
    start = ArgOfTerm(1,start);
  } while (TRUE);
}

static Term 
SearchAttsForModuleName(Term start, Atom mname)
{
  do {
    if (IsVarTerm(start) ||
	NameOfFunctor(FunctorOfTerm(start)) == mname)
      return start;
    start = ArgOfTerm(1,start);
  } while (TRUE);
}

static void 
AddNewModule(attvar_record *attv, Term t, int new)
{
  if (IsVarTerm(attv->Atts)) {
    if (new) {
      attv->Atts = t;
    } else {
      Bind(&(attv->Atts),t);
    }
  } else {
    Term *wherep = &attv->Atts;

    do {
      if (IsVarTerm(*wherep)) {
	Bind_Global(wherep,t);      
	return;
      } else {
	wherep = RepAppl(Deref(*wherep))+1;
      }
    } while (TRUE);
  }
}

static void 
ReplaceAtts(attvar_record *attv, Term oatt, Term att)
{
  UInt ar = ArityOfFunctor(FunctorOfTerm(oatt)), i;
  CELL *oldp = RepAppl(oatt)+1;
  CELL *newp = RepAppl(att)+1;

  *newp++ = *oldp++;
  for (i=1; i< ar; i++) {
    if (*newp == TermFoundVar) {
      *newp = *oldp;
    }
    oldp++;
    newp++;
  }
  if (attv->Atts == oatt) {
    if (RepAppl(attv->Atts) >= HB)
      attv->Atts = att;
    else
      MaBind(&(attv->Atts), att);
  } else {
    Term *wherep = &attv->Atts;

    do {
      if (*wherep == oatt) {
	MaBind(wherep, att);
	return;
      } else {
	wherep = RepAppl(Deref(*wherep))+1;
      }
    } while (TRUE);
  }
}

static void 
DelAtts(attvar_record *attv, Term oatt)
{
  if (attv->Atts == oatt) {
    if (RepAppl(attv->Atts) >= HB)
      attv->Atts = ArgOfTerm(1,oatt);
    else
      MaBind(&(attv->Atts), ArgOfTerm(1,oatt));
  } else {
    Term *wherep = &attv->Atts;

    do {
      if (*wherep == oatt) {
	MaBind(wherep, ArgOfTerm(1,oatt));
	return;
      } else {
	wherep = RepAppl(Deref(*wherep))+1;
      }
    } while (TRUE);
  }
}

static void 
PutAtt(Int pos, Term atts, Term att)
{
  if (IsVarTerm(att) && (CELL *)att > H && (CELL *)att < LCL0) {
    /* globalise locals */
    Term tnew = MkVarTerm();
    Bind((CELL *)att, tnew);
    att = tnew;
  }
  MaBind(RepAppl(atts)+pos, att);
}

static Int
BindAttVar(attvar_record *attv) {
  if (IsVarTerm(attv->Done) && IsUnboundVar(&attv->Done)) {
    /* make sure we are not trying to bind a variable against itself */
    if (!IsVarTerm(attv->Value)) {
      Bind_Global(&(attv->Done), attv->Value);
    } else if (IsVarTerm(attv->Value)) {
      Term t = Deref(attv->Value);
      if (IsVarTerm(t)) {
	if (IsAttachedTerm(t)) {
	  attvar_record *attv2 = (attvar_record *)VarOfTerm(t);
	  if (attv2 < attv) {
	    Bind_Global(&(attv->Done), t);
	  } else {
	    Bind_Global(&(attv2->Done), (CELL)attv);
	  }
	} else {
	  Yap_Error(SYSTEM_ERROR,(CELL)&(attv->Done),"attvar was bound when unset");
	  return(FALSE);
	}
      } else {
	Bind_Global(&(attv->Done), t);
      }
    }
    return(TRUE);
  } else {
    Yap_Error(SYSTEM_ERROR,(CELL)&(attv->Done),"attvar was bound when set");
    return(FALSE);
  }
}

static Term
GetAllAtts(attvar_record *attv) {
  /* check if we are already there */
  return attv->Atts;
}

static Term
AllAttVars(attvar_record *attv) {
  CELL *h0 = H;
  attvar_record *max = DelayTop();

  while (attv != max) {

    if (ASP - H < 1024) {
      H = h0;
      return 0L;
    }
    if (IsVarTerm(attv->Done) && IsUnboundVar(&attv->Done)) {
      if (IsIntegerTerm(attv->Atts)) {
	/* skip call residue(s) */
	UInt n = IntegerOfTerm(attv->Atts)-1;
	attv += n;
      } else {
	if (H != h0) {
	  H[-1] = AbsPair(H);
	}
	H[0] = (CELL)attv;
	H += 2;
      }
    }
    attv++;
  }
  if (H != h0) {
    H[-1] = TermNil;
    return AbsPair(h0);
  } else {
    return TermNil;
  }
}

static Int
p_put_att(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    attvar_record *attv;
    Atom modname = AtomOfTerm(Deref(ARG2));
    UInt ar = IntegerOfTerm(Deref(ARG3));
    Functor mfun;
    Term tatts;
    int new = FALSE;

    if (IsAttachedTerm(inp)) {
      attv = (attvar_record *)VarOfTerm(inp);
    } else {
      while (!(attv = BuildNewAttVar())) {
	if (!Yap_growglobal(NULL)) {
	  Yap_Error(OUT_OF_ATTVARS_ERROR, ARG1, Yap_ErrorMessage);
	  return FALSE;
	}
	inp = Deref(ARG1);
      }
      new = TRUE;
    }
    mfun= Yap_MkFunctor(modname,ar);
    if (IsVarTerm(tatts = SearchAttsForModule(attv->Atts,mfun))) {
      while (!(tatts = BuildAttTerm(mfun,ar))) {
	if (!Yap_gc(5, ENV, P)) {
	  Yap_Error(OUT_OF_STACK_ERROR, TermNil, Yap_ErrorMessage);
	  return FALSE;
	}    
      }
      Yap_unify(ARG1, (Term)attv);
      AddNewModule(attv,tatts,new);
    }
    PutAtt(IntegerOfTerm(Deref(ARG4)), tatts, Deref(ARG5));
    return TRUE;
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return(FALSE);
  }
}

static Int
p_put_att_term(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    attvar_record *attv;
    int new = FALSE;

    if (IsAttachedTerm(inp)) {
      attv = (attvar_record *)VarOfTerm(inp);
    } else {
      while (!(attv = BuildNewAttVar())) {
	if (!Yap_growglobal(NULL)) {
	  Yap_Error(OUT_OF_ATTVARS_ERROR, ARG1, Yap_ErrorMessage);
	  return FALSE;
	}
	inp = Deref(ARG1);
      }
      new = TRUE;
    }
    if (new) {
      attv->Atts = Deref(ARG2);
    } else {
      MaBind(&(attv->Atts), Deref(ARG2));
    }
    return TRUE;
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return(FALSE);
  }
}

static Int
p_rm_att(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    attvar_record *attv;
    Atom modname = AtomOfTerm(Deref(ARG2));
    UInt ar = IntegerOfTerm(Deref(ARG3));
    Functor mfun;
    Term tatts;
    int new = FALSE;

    if (IsAttachedTerm(inp)) {
      attv = (attvar_record *)VarOfTerm(inp);
    } else {
      while (!(attv = BuildNewAttVar())) {
	if (!Yap_growglobal(NULL)) {
	  Yap_Error(OUT_OF_ATTVARS_ERROR, ARG1, Yap_ErrorMessage);
	  return FALSE;
	}
	inp = Deref(ARG1);
      }
      new = TRUE;
      Yap_unify(ARG1, (Term)attv);
    }
    mfun= Yap_MkFunctor(modname,ar);
    if (IsVarTerm(tatts = SearchAttsForModule(attv->Atts,mfun))) {
      while (!(tatts = BuildAttTerm(mfun,ar))) {
	if (!Yap_gc(4, ENV, P)) {
	  Yap_Error(OUT_OF_STACK_ERROR, TermNil, Yap_ErrorMessage);
	  return FALSE;
	}    
      }
      AddNewModule(attv,tatts,new);
    } else {
      PutAtt(IntegerOfTerm(Deref(ARG4)), tatts, TermFoundVar);
    }
    return TRUE;
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return(FALSE);
  }
}

static Int
p_put_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  Term otatts;

  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    attvar_record *attv;
    Term tatts = Deref(ARG2);
    Functor mfun = FunctorOfTerm(tatts);
    int new = FALSE;

    if (IsAttachedTerm(inp)) {
      attv = (attvar_record *)VarOfTerm(inp);
    } else {
      while (!(attv = BuildNewAttVar())) {
	if (!Yap_growglobal(NULL)) {
	  Yap_Error(OUT_OF_ATTVARS_ERROR, ARG1, Yap_ErrorMessage);
	  return FALSE;
	}
	tatts = Deref(ARG2);
      }
      new = TRUE;
      Yap_unify(ARG1, (Term)attv);
    }
    if (IsVarTerm(otatts = SearchAttsForModule(attv->Atts,mfun))) {
      AddNewModule(attv,tatts,new);
    } else {
      ReplaceAtts(attv, otatts, tatts);
    }
    return TRUE;
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return FALSE;
  }
}

static Int
p_del_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  Term otatts;

  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    attvar_record *attv;
    Term tatts = Deref(ARG2);
    Functor mfun = FunctorOfTerm(tatts);

    if (IsAttachedTerm(inp)) {
      attv = (attvar_record *)VarOfTerm(inp);
    } else {
      return TRUE;
    }
    if (IsVarTerm(otatts = SearchAttsForModule(attv->Atts,mfun))) {
      return TRUE;
    } else {
      DelAtts(attv, otatts);
    }
    return TRUE;
  } else {
    return TRUE;
  }
}

static Int
p_get_att(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    Atom modname = AtomOfTerm(Deref(ARG2));

    if (IsAttachedTerm(inp)) {
      attvar_record *attv;
      Term tout, tatts;

      attv = (attvar_record *)VarOfTerm(inp);
      if (IsVarTerm(tatts = SearchAttsForModuleName(attv->Atts,modname)))
	return FALSE;
      tout = ArgOfTerm(IntegerOfTerm(Deref(ARG3)),tatts);
      if (tout == TermFoundVar) return FALSE;
      return Yap_unify(tout, ARG4);      
    } else {
      /* Yap_Error(INSTANTIATION_ERROR,inp,"get_att/2"); */
      return FALSE;
    }
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return(FALSE);
  }
}

static Int
p_free_att(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    Atom modname = AtomOfTerm(Deref(ARG2));

    if (IsAttachedTerm(inp)) {
      attvar_record *attv;
      Term tout, tatts;

      attv = (attvar_record *)VarOfTerm(inp);
      if (IsVarTerm(tatts = SearchAttsForModuleName(attv->Atts,modname)))
	return TRUE;
      tout = ArgOfTerm(IntegerOfTerm(Deref(ARG3)),tatts);
      return (tout == TermFoundVar);
    } else {
      /* Yap_Error(INSTANTIATION_ERROR,inp,"get_att/2"); */
      return TRUE;
    }
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"put_attributes/2");
    return(FALSE);
  }
}

static Int
p_get_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv;
      Term tatts;
      Term access = Deref(ARG2);
      Functor mfun = FunctorOfTerm(access);
      UInt ar, i;
      CELL *old, *new;

      attv = (attvar_record *)VarOfTerm(inp);
      if (IsVarTerm(tatts = SearchAttsForModule(attv->Atts,mfun)))
	return FALSE;
      
      ar = ArityOfFunctor(mfun);
      new = RepAppl(access)+2;
      old = RepAppl(tatts)+2;
      for (i = 1; i < ar; i++,new++,old++) {
	if (*new != TermFreeTerm) {
	  if (*old == TermFoundVar && *new != TermFoundVar)
	    return FALSE;
	  if (!Yap_unify(*new,*old)) return FALSE;
	}
      }
      return TRUE;
    } else {
      /* Yap_Error(INSTANTIATION_ERROR,inp,"get_att/2"); */
      return FALSE;
    }
  } else {
    //    Yap_Error(TYPE_ERROR_VARIABLE,inp,"get_attributes/2");
    return(FALSE);
  }
}

static Int
p_has_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv;
      Term tatts;
      Term access = Deref(ARG2);
      Functor mfun = FunctorOfTerm(access);

      attv = (attvar_record *)VarOfTerm(inp);
      return !IsVarTerm(tatts = SearchAttsForModule(attv->Atts,mfun));
    } else {
      /* Yap_Error(INSTANTIATION_ERROR,inp,"get_att/2"); */
      return FALSE;
    }
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"has_attributes/2");
    return(FALSE);
  }
}

static Int
p_bind_attvar(void) {
  /* receive a variable in ARG1 */
  Term  inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv = (attvar_record *)VarOfTerm(inp);
      return(BindAttVar(attv));
    }
    return(TRUE);
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"bind_att/2");
    return(FALSE);
  }
}

static Int
p_get_all_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv = (attvar_record *)VarOfTerm(inp);
      return Yap_unify(ARG2,GetAllAtts(attv));
    }
    return TRUE;
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"get_att/2");
    return FALSE;
  }
}

static int
ActiveAtt(Term tatt, UInt ar)
{
  CELL *cp = RepAppl(tatt);
  UInt i;

  for (i = 1; i < ar; i++) {
    if (cp[i] != TermFoundVar)
      return TRUE;
  }
  return FALSE;
}

static Int
p_modules_with_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv = (attvar_record *)VarOfTerm(inp);
      CELL *h0 = H;
      Term tatt;

      if (IsVarTerm(tatt = attv->Atts))
	  return Yap_unify(ARG2,TermNil);
      while (!IsVarTerm(tatt)) {
	Functor f = FunctorOfTerm(tatt);
	if (H != H0)
	  H[-1] = AbsPair(H);
	if (ActiveAtt(tatt, ArityOfFunctor(f))) {
	  *H = MkAtomTerm(NameOfFunctor(f));
	  H+=2;
	}
	tatt = ArgOfTerm(1,tatt);
      }
      if (h0 != H) {
	H[-1] = TermNil;
	return Yap_unify(ARG2,AbsPair(h0));
      }
    }
    return Yap_unify(ARG2,TermNil);
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"get_att/2");
    return FALSE;
  }
}

static Int
p_swi_all_atts(void) {
  /* receive a variable in ARG1 */
  Term inp = Deref(ARG1);
  Functor attf = Yap_MkFunctor(Yap_LookupAtom("att"),3);

  /* if this is unbound, ok */
  if (IsVarTerm(inp)) {
    if (IsAttachedTerm(inp)) {
      attvar_record *attv = (attvar_record *)VarOfTerm(inp);
      CELL *h0 = H;
      Term tatt;

      if (IsVarTerm(tatt = attv->Atts))
	  return Yap_unify(ARG2,TermNil);
      while (!IsVarTerm(tatt)) {
	Functor f = FunctorOfTerm(tatt);

	if (ArityOfFunctor(f) == 2) {
	  if (H != h0)
	    H[-1] = AbsAppl(H);
	  H[0] = (CELL) attf;
	  H[1] = MkAtomTerm(NameOfFunctor(f));
	  H[2] = ArgOfTerm(2,tatt);
	  H+=4;
	}
	tatt = ArgOfTerm(1,tatt);
      }
      if (h0 != H) {
	H[-1] = TermNil;
	return Yap_unify(ARG2,AbsAppl(h0));
      }
    }
    return Yap_unify(ARG2,TermNil);
  } else {
    Yap_Error(TYPE_ERROR_VARIABLE,inp,"get_att/2");
    return FALSE;
  }
}

static Int
p_all_attvars(void)
{
  do {
    Term out;
    attvar_record *base;

    base = (attvar_record *)Yap_GlobalBase+IntegerOfTerm(Yap_ReadTimedVar(AttsMutableList));
    if (!(out = AllAttVars(base))) {
      if (!Yap_gc(1, ENV, P)) {
	Yap_Error(OUT_OF_STACK_ERROR, TermNil, Yap_ErrorMessage);
	return FALSE;
      }    
    } else {
      return Yap_unify(ARG1,out);
    }
  } while (TRUE);
}

static Int
p_is_attvar(void)
{
  Term t = Deref(ARG1);
  return(IsVarTerm(t) &&
	 IsAttachedTerm(t));
}

/* check if we are not redoing effort */
static Int
p_attvar_bound(void)
{
  Term t = Deref(ARG1);
  return(IsVarTerm(t) &&
	 IsAttachedTerm(t) &&
         !IsUnboundVar(&((attvar_record *)VarOfTerm(t))->Done));
}

#else

static Int
p_all_attvars(void)
{
  return FALSE;
}

static Int
p_is_attvar(void)
{
  return FALSE;
}

static Int
p_attvar_bound(void)
{
  return FALSE;
}

#endif /* COROUTINING */

static Int
p_void_term(void)
{
  return Yap_unify(ARG1,TermFoundVar);
}

static Int
p_free_term(void)
{
  return Yap_unify(ARG1,TermFreeTerm);
}

void Yap_InitAttVarPreds(void)
{
  Term OldCurrentModule = CurrentModule;
  CurrentModule = ATTRIBUTES_MODULE;
#ifdef COROUTINING
  attas[attvars_ext].bind_op = WakeAttVar;
  attas[attvars_ext].copy_term_op = CopyAttVar;
  attas[attvars_ext].to_term_op = AttVarToTerm;
  attas[attvars_ext].term_to_op = TermToAttVar;
  attas[attvars_ext].mark_op = mark_attvar;
  Yap_InitCPred("get_att", 4, p_get_att, SafePredFlag);
  Yap_InitCPred("get_module_atts", 2, p_get_atts, SafePredFlag);
  Yap_InitCPred("has_module_atts", 2, p_has_atts, SafePredFlag);
  Yap_InitCPred("get_all_atts", 2, p_get_all_atts, SafePredFlag);
  Yap_InitCPred("get_all_swi_atts", 2, p_swi_all_atts, SafePredFlag);
  Yap_InitCPred("free_att", 3, p_free_att, SafePredFlag);
  Yap_InitCPred("put_att", 5, p_put_att, 0);
  Yap_InitCPred("put_att_term", 5, p_put_att_term, 0);
  Yap_InitCPred("put_module_atts", 2, p_put_atts, 0);
  Yap_InitCPred("del_all_module_atts", 2, p_del_atts, 0);
  Yap_InitCPred("rm_att", 4, p_rm_att, 0);
  Yap_InitCPred("bind_attvar", 1, p_bind_attvar, SafePredFlag);
  Yap_InitCPred("void_term", 1, p_void_term, SafePredFlag);
  Yap_InitCPred("free_term", 1, p_free_term, SafePredFlag);
  Yap_InitCPred("modules_with_attributes", 2, p_modules_with_atts, SafePredFlag);
#endif /* COROUTINING */
  Yap_InitCPred("all_attvars", 1, p_all_attvars, 0);
  CurrentModule = OldCurrentModule;
  Yap_InitCPred("attvar", 1, p_is_attvar, SafePredFlag|TestPredFlag);
  Yap_InitCPred("$att_bound", 1, p_attvar_bound, SafePredFlag|TestPredFlag|HiddenPredFlag);
}



