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
* File:		iopreds.c						 *
* Last rev:	5/2/88							 *
* mods:									 *
* comments:	Input/Output C implemented predicates			 *
*									 *
*************************************************************************/
#ifdef SCCS
static char SccsId[] = "%W% %G%";
#endif

/*
 * This file includes the definition of a miscellania of standard predicates
 * for yap refering to: Files and GLOBAL_Streams, Simple Input/Output, 
 *
 */

#include "Yap.h"
#include "Yatom.h"
#include "YapHeap.h"
#include "yapio.h"
#include "eval.h"
#include "YapText.h"
#include <stdlib.h>
#if HAVE_STDARG_H
#include <stdarg.h>
#endif
#if HAVE_CTYPE_H
#include <ctype.h>
#endif
#if HAVE_WCTYPE_H
#include <wctype.h>
#endif
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#if HAVE_SYS_PARAMS_H
#include <sys/params.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_SELECT_H && !_MSC_VER && !defined(__MINGW32__) 
#include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_LIBGEN_H
#include <libgen.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_FCNTL_H
/* for O_BINARY and O_TEXT in WIN32 */
#include <fcntl.h>
#endif
#ifdef _WIN32
#if HAVE_IO_H
 /* Windows */
#include <io.h>
#endif
#endif
#if !HAVE_STRNCAT
#define strncat(X,Y,Z) strcat(X,Y)
#endif
#if !HAVE_STRNCPY
#define strncpy(X,Y,Z) strcpy(X,Y)
#endif
#if _MSC_VER || defined(__MINGW32__) 
#if HAVE_SOCKET
#include <winsock2.h>
#endif
#include <windows.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x)&_S_IFDIR)==_S_IFDIR)
#endif
#endif
#include "iopreds.h"

#if _MSC_VER || defined(__MINGW32__) 
#define SYSTEM_STAT _stat
#else
#define SYSTEM_STAT stat
#endif



static Int
file_name_extension(USES_REGS1)
{
  Term t1 = Deref(ARG1);
  Term t2 = Deref(ARG2);
  Term t3 = Deref(ARG3);
  bool use_string = false;
 loop:
  if (!IsVarTerm((t3))) {
    const char *f;
    if (IsAtomTerm(t3)) {
      f = AtomName( AtomOfTerm( t3 ) );
    } else if (IsStringTerm( t3 )) {
      f = StringOfTerm( t3 );
      use_string = true;
    } else if (IsApplTerm(t3) && FunctorOfTerm(t3) == FunctorSlash) {
      // descend a compound term of the form a/b.
      Term tn1 = MkVarTerm(), tf1;
      Term ts[2];
      ts[0] = ArgOfTerm(1,t3);
      ts[1] = tn1;
      tf1 = Yap_MkApplTerm( FunctorSlash, 2, ts );
      if (!Yap_unify(ARG1, tf1)) {
	return false;
      }
      t3 = ArgOfTerm(2, t3);
      goto loop;		
    } else {
      Yap_Error(TYPE_ERROR_ATOMIC, t3, "file_name_extension/3");
      return false;
    }
    char *pts = strrchr(f, '/');
#if WIN32_ssss
    char *pts1 = strrchr(f, '\\');
    if (pts11 > pts) pts = pts1;
#endif
    char *ss = strrchr(f, '.');
    if (pts > ss) ss = NULL;
    if (use_string) {
      char *tmp;
      if (!ss) {
	return Yap_unify(ARG1, ARG3) &&
	  Yap_unify(ARG2, MkStringTerm( "" ) );
      }
      tmp = malloc((ss - f)+1);
      strncpy(tmp, f, (ss)-f);
      if (!Yap_unify(ARG1, MkStringTerm( tmp )) ) {
	if (tmp != f) 
	  free( tmp );
	return false;
      }
      if (tmp != f)
	free(tmp);
      // without and with dot
      if (!Yap_unify(ARG2, MkStringTerm(ss+1)))
	return Yap_unify(ARG2, MkStringTerm(ss));
      return true;
    } else {
      char *tmp;
      if (!ss) {
	return Yap_unify(ARG1, ARG3) &&
	  Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom( "" ) ));
      }
      tmp = malloc((ss - f)+1);
      strncpy(tmp, f, (ss)-f);
      if (!Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom( tmp ))) ) {
	if (tmp != f) 
	  free( tmp );
	return false;
      }
      if (tmp != f)
	free(tmp);
      // without and with dot
      if (!Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(ss+1))))
	return Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(ss)));
      return true;
    }
  } else {
    char s[MAXPATHLEN+1];
    const char *f1, *f2;
  loop1:
    if (IsVarTerm(t1)) {
     Yap_Error(INSTANTIATION_ERROR, t1, "access");
     return FALSE;         
    } else if (IsAtomTerm(t1)) {
      f1 = AtomName( AtomOfTerm( t1 ) );
    } else if (IsStringTerm( t1 )) {
      f1 = StringOfTerm( t1 );
      use_string = true;
    } else if (IsApplTerm(t1) && FunctorOfTerm(t1) == FunctorSlash) {
      // descend a compound term of the form a/b.
      Term tn1 = MkVarTerm(), tf1;
      Term ts[2];
      
      ts[0] = ArgOfTerm(1,t1);
      ts[1] = tn1;
      tf1 = Yap_MkApplTerm( FunctorSlash, 2, ts );
      if (!Yap_unify(ARG3, tf1)) {
	return false;
      }
      t1 = ArgOfTerm(2, t1);
      goto loop1;		
    } else {
      Yap_Error(TYPE_ERROR_ATOMIC, t1, "file_name_extension/3");
      return false;
    }

    if (IsVarTerm(t2)) {
     Yap_Error(INSTANTIATION_ERROR, t2, "access");
     return FALSE;         
    } else if (IsAtomTerm(t2)) {
      f2 = AtomName( AtomOfTerm( t2 ) );
    } else if (IsStringTerm( t1 )) {
      f2 = StringOfTerm( t2 );
      use_string = true;
    } else {
      Yap_Error(TYPE_ERROR_ATOMIC, t2, "file_name_extension/3");
      return false;
    }
    if (f2[0] == '.') {
      strncpy(s,f1,MAXPATHLEN);
      strncat(s,f2,MAXPATHLEN);
      if (use_string)
	return Yap_unify_constant(ARG3,MkStringTerm(s));
      else
	return Yap_unify_constant(ARG3,MkAtomTerm(Yap_LookupAtom(s)));
    } else {
      strncpy(s,f1,MAXPATHLEN);
      strncat(s,".",MAXPATHLEN);
      strncat(s,f2,MAXPATHLEN);
      if (use_string)
	return Yap_unify_constant(ARG3,MkStringTerm(s));
      else
	return Yap_unify_constant(ARG3,MkAtomTerm(Yap_LookupAtom(s)));
    }
  }
}

static Int
access_path(USES_REGS1)
{
  Term tname = Deref(ARG1);
  char *file_name;

  if (IsVarTerm(tname)) {
    Yap_Error(INSTANTIATION_ERROR, tname, "access");
    return FALSE;
  } else if (!IsAtomTerm (tname)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "access");
    return FALSE;
  } else {
#if HAVE_STAT
    struct SYSTEM_STAT ss;

    file_name = RepAtom(AtomOfTerm(tname))->StrOfAE;
    if (SYSTEM_STAT(file_name, &ss) != 0) {
    /* ignore errors while checking a file */
      return FALSE;
    }
    return TRUE;
#else
    return FALSE;
#endif
  }
}

static Int
exists_file(USES_REGS1)
{
  Term tname = Deref(ARG1);
  char *file_name;

  if (IsVarTerm(tname)) {
    Yap_Error(INSTANTIATION_ERROR, tname, "access");
    return FALSE;
  } else if (!IsAtomTerm (tname)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "access");
    return FALSE;
  } else {
#if HAVE_STAT
    struct SYSTEM_STAT ss;

    file_name = RepAtom(AtomOfTerm(tname))->StrOfAE;
    if (SYSTEM_STAT(file_name, &ss) != 0) {
    /* ignore errors while checking a file */
      return FALSE;
    }
    return (S_ISREG(ss.st_mode));
#else
    return FALSE;
#endif
  }
}


static Int
time_file(USES_REGS1)
{
  Term tname = Deref(ARG1);
 
  if (IsVarTerm(tname)) {
    Yap_Error(INSTANTIATION_ERROR, tname, "access");
    return FALSE;
  } else if (!IsAtomTerm (tname)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "access");
    return FALSE;
  } else {
    const char *n = RepAtom(AtomOfTerm(tname))->StrOfAE;
#if __WIN32
    FILETIME ftWrite;
    if ((hdl = CreateFile( n, 0, 0, NULL, OPEN_EXISTING, NULL)) == 0)
      return false;
    if (GetFileTime(hdl, NULL,NULL,&ftWrite))
      return false;
    // Convert the last-write time to local time.
    // FileTimeToSystemTime(&ftWrite, &stUTC);
    // SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
    CloseHandle( hdl );
    return Yap_unify(ARG2, MkIntegerTerm(ftWrite));
#elif HAVE_STAT
    struct SYSTEM_STAT ss;
 
    if (SYSTEM_STAT(n, &ss) != 0) {
    /* ignore errors while checking a file */
      return FALSE;
    }
    return Yap_unify(ARG2, MkIntegerTerm(ss.st_mtime));
#else
    return FALSE;
#endif
  }
}

static Int
file_size(USES_REGS1)
{
  int rc;
  Int sno = Yap_CheckStream (ARG1, (Input_Stream_f | Output_Stream_f | Socket_Stream_f), "file_size/2");
  if (sno < 0)
    return (FALSE);
  if (GLOBAL_Stream[sno]. status &  Seekable_Stream_f &&
      !(GLOBAL_Stream[sno]. status &  (InMemory_Stream_f|Socket_Stream_f|Pipe_Stream_f))) {
	// there
	struct stat file_stat;
	if ((rc = fstat(fileno(GLOBAL_Stream[sno].file), &file_stat) )< 0) {
        UNLOCK(GLOBAL_Stream[sno].streamlock);
      if (rc == ENOENT)
        PlIOError( EXISTENCE_ERROR_SOURCE_SINK, ARG1, "%s in file_size", strerror(errno));
      else
	  PlIOError( PERMISSION_ERROR_INPUT_STREAM, ARG1, "%s in file_size", strerror(errno));
	  return false;
	}
	// and back again
      UNLOCK(GLOBAL_Stream[sno].streamlock);
	return Yap_unify_constant( ARG2, MkIntegerTerm( file_stat.st_size) );
  }
    UNLOCK(GLOBAL_Stream[sno].streamlock);
  return false;
}

/** @pred access_file( + Name, -

 */
static Int
access_file(USES_REGS1)
{
  Term tname = Deref(ARG1);
  Term tmode = Deref(ARG2);
  char ares[YAP_FILENAME_MAX];
  Atom atmode;

  if (IsVarTerm(tmode)) {
    Yap_Error(INSTANTIATION_ERROR, tmode, "access_file/2");
    return FALSE;
  } else if (!IsAtomTerm (tmode)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "access_file/2");
    return FALSE;
  }
  atmode = AtomOfTerm(tmode);
  if (IsVarTerm(tname)) {
    Yap_Error(INSTANTIATION_ERROR, tname, "access_file/2");
    return FALSE;
  } else if (!IsAtomTerm (tname)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "access_file/2");
    return FALSE;
  } else {
    if (atmode == AtomNone)
      return TRUE;
    if (!Yap_TrueFileName (RepAtom(AtomOfTerm(tname))->StrOfAE, ares, (atmode == AtomCsult)))
      return FALSE;
  }
#if HAVE_ACCESS
  {
    int mode;

    if (atmode == AtomExist)
      mode = F_OK;
    else if (atmode == AtomWrite)
      mode = W_OK;
    else if (atmode == AtomRead)
      mode = R_OK;
    else if (atmode == AtomAppend)
      mode = W_OK;
    else if (atmode == AtomCsult)
      mode = R_OK;
    else if (atmode == AtomExecute)
      mode = X_OK;
    else {
      PlIOError(DOMAIN_ERROR_IO_MODE, tmode, "access_file/2");
      return FALSE;   
    }
    if (access(ares, mode) < 0) {
      /* ignore errors while checking a file */
      return FALSE;
    }
    return TRUE;
  }
#elif HAVE_STAT
  {
    struct SYSTEM_STAT ss;

    if (SYSTEM_STAT(ares, &ss) != 0) {
    /* ignore errors while checking a file */
      return FALSE;
    }
    return TRUE;
  }
#else
  return FALSE;
#endif
}

static Int
exists_directory(USES_REGS1)
{
  Term tname = Deref(ARG1);
  char *file_name;

  if (IsVarTerm(tname)) {
    Yap_Error(INSTANTIATION_ERROR, tname, "exists_directory/1");
    return FALSE;
  } else if (!IsAtomTerm (tname)) {
    Yap_Error(TYPE_ERROR_ATOM, tname, "exists_directory/1");
    return FALSE;
  } else {
#if HAVE_STAT
    struct SYSTEM_STAT ss;

    file_name = RepAtom(AtomOfTerm(tname))->StrOfAE;
    if (SYSTEM_STAT(file_name, &ss) != 0) {
    /* ignore errors while checking a file */
      return FALSE;
    }
    return (S_ISDIR(ss.st_mode));
#else
    return FALSE;
#endif
  }
}


static Int
is_absolute_file_name ( USES_REGS1 )
{				/* file_base_name(Stream,N)                      */
  Term t = Deref(ARG1);
  Atom at;
  if (IsVarTerm(t)) {
    Yap_Error(INSTANTIATION_ERROR, t, "file_base_name/2");
    return FALSE;    
  }
  at = AtomOfTerm(t);
  if (IsWideAtom(at)) {
#if _WIN32
    return PathisRelativeW(RepAtom(at)->WStrOfAE[0]);
#else
    return RepAtom(at)->WStrOfAE[0] == '/';
#endif
  } else {
    return Yap_IsAbsolutePath( RepAtom(at)->StrOfAE );
  }
  return false;
}

static Int
file_base_name ( USES_REGS1 )
{				/* file_base_name(Stream,N)                      */
  Term t = Deref(ARG1);
  Atom at;
  if (IsVarTerm(t)) {
    Yap_Error(INSTANTIATION_ERROR, t, "file_base_name/2");
    return FALSE;    
  }
  at = AtomOfTerm(t);
  if (IsWideAtom(at)) {
      wchar_t *c = RepAtom(at)->WStrOfAE;
      Int i = wcslen(c);
      while (i && !Yap_dir_separator((int)c[--i]));
      return Yap_unify(ARG2, MkAtomTerm(Yap_LookupWideAtom(c+i)));
  } else {
      char *c = RepAtom(at)->StrOfAE;
      char *s;
#if HAVE_BASENAME
      s = basename( c );
#else
      Int i = strlen(c);
      while (i && !Yap_dir_separator((int)c[--i]));
      if (Yap_dir_separator((int)c[i])) {
	i++;
      }
      s = c+i;
#endif
      return Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(s)));
  }
}

static Int
file_directory_name ( USES_REGS1 )
{				/* file_directory_name(Stream,N)                      */
  Term t = Deref(ARG1);
  Atom at;
  if (IsVarTerm(t)) {
    Yap_Error(INSTANTIATION_ERROR, t, "file_directory_name/2");
    return FALSE;    
  }
  at = AtomOfTerm(t);
  if (IsWideAtom(at)) {
      wchar_t s[YAP_FILENAME_MAX+1];
      wchar_t *c = RepAtom(at)->WStrOfAE;
      Int i = wcslen(c);
      while (i && !Yap_dir_separator((int)c[--i]));
      if (Yap_dir_separator((int)c[i])) {
	i++;
      }
      wcsncpy(s, c, i);
      return Yap_unify(ARG2, MkAtomTerm(Yap_LookupWideAtom(s)));
  } else {
      char *c = RepAtom(at)->StrOfAE;
#if HAVE_BASENAME
      char *s;
      s = dirname( c );
#else
      char *s[YAP_FILENAME_MAX+1];
      Int i = strlen(c);
      while (i && !Yap_dir_separator((int)c[--i]));
      if (Yap_dir_separator((int)c[i])) {
	i++;
      }
      strncpy(s, c, i);
#endif
      return Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(s)));
  }
}


static Int
same_file( USES_REGS1 ) {
  char *f1 = RepAtom(AtomOfTerm(Deref(ARG1)))->StrOfAE;
  char *f2 = RepAtom(AtomOfTerm(Deref(ARG2)))->StrOfAE;

  if (strcmp(f1,f2) == 0)
    return TRUE;
#if HAVE_LSTAT 
  {
    int out;
    struct stat *b1, *b2;
    while ((char *)HR+sizeof(struct stat)*2 > (char *)(ASP-1024)) {
      if (!Yap_gcl(2*sizeof(struct stat), 2, ENV,Yap_gcP() ) ) {
	Yap_Error(RESOURCE_ERROR_STACK, TermNil, LOCAL_ErrorMessage);
	return FALSE;
      }
    }
    b1 = (struct stat *)HR;
    b2 = b1+1;
    if (strcmp(f1,"user_input") == 0) {
      if (fstat(fileno(GLOBAL_Stream[0].file), b1) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;
      }
    } else   if (strcmp(f1,"user_output") == 0) {
      if (fstat(fileno(GLOBAL_Stream[1].file), b1) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;    
      }
    } else   if (strcmp(f1,"user_error") == 0) {
      if (fstat(fileno(GLOBAL_Stream[2].file), b1) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;    
      }
    } else if (stat(f1, b1) == -1) {
      /* file does not exist, but was opened? Return -1 */
      return FALSE;
    }
    if (strcmp(f2,"user_input") == 0) {
      if (fstat(fileno(GLOBAL_Stream[0].file), b2) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;
      }
    } else   if (strcmp(f2,"user_output") == 0) {
      if (fstat(fileno(GLOBAL_Stream[1].file), b2) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;    
      }
    } else   if (strcmp(f2,"user_error") == 0) {
      if (fstat(fileno(GLOBAL_Stream[2].file), b2) == -1) {
	/* file does not exist, but was opened? Return -1 */
	return FALSE;    
      }
    } else if (stat(f2, b2) == -1) {
      /* file does not exist, but was opened? Return -1 */
      return FALSE;
    }
    out = (b1->st_ino == b2->st_ino
#ifdef __LCC__
	   && memcmp((const void *)&(b1->st_dev),(const void *)&(b2->st_dev),sizeof(buf1.st_dev)) == 0
#else
	   && b1->st_dev == b2->st_dev
#endif
		  );
    return out;
  }
#else
  return(FALSE);
#endif
}


void
Yap_InitFiles( void )
{
 Yap_InitCPred ("file_base_name", 2, file_base_name, SafePredFlag);
 Yap_InitCPred ("file_directory_name", 2, file_directory_name, SafePredFlag);
 Yap_InitCPred ("is_absolute_file_name", 1, is_absolute_file_name, SafePredFlag);
 Yap_InitCPred ("same_file", 2, same_file, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("access_file", 2, access_file, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("access", 1, access_path, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("exists_directory", 1, exists_directory, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("exists_file", 1, exists_file, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("time_file64", 2, time_file, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("time_file", 2, time_file, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("file_size", 2,file_size, SafePredFlag|SyncPredFlag);
 Yap_InitCPred ("file_name_extension", 3, file_name_extension, SafePredFlag|SyncPredFlag);
}

