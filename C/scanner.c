/*************************************************************************
 *                   *
 *   YAP Prolog                *
 *                   *
 *  Yap Prolog was developed at NCCUP - Universidade do Porto   *
 *                   *
 * Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-2003   *
 *                   *
 **************************************************************************
 *                   *
 * File:    %W% %G%                     *
 * Last rev:  22-1-03               *
 * mods:                   *
 * comments:  Prolog's scanner           *
 *                   *
 *************************************************************************/

/*
 * Description:
 *
 * This module produces a list of tokens for use by the parser. The calling
 * program should supply a routine int nextch(charpos) int *charpos; which,
 * when called should produce the next char or -1 if none availlable. The
 * scanner will stop producing tokens when it either finds an end of file
 * (-1) or a token consisting of just '.' followed by a blank or control
 * char. Scanner errors will be signalled by the scanner exiting with a non-
 * zero  ErrorMsg and ErrorPos. Note that, even in this case, the scanner
 * will try to find the end of the term. A function char
 * *AllocScannerMemory(nbytes) should be supplied for allocating (temporary)
 * space for strings and for the table of prolog variables occurring in the
 * term.
 *
 */

/**

@defgroup Formal_Syntax Syntax of Terms
@ingroup YAPSyntax
@{


Prolog tokens are grouped into the following categories:

+ Numbers

Numbers can be further subdivided into integer and floating-point numbers.

  + Integers

Integer numbers
are described by the following regular expression:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<integer> := {<digit>+<single-quote>|0{xXo}}<alpha_numeric_char>+

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

where {...} stands for optionality, \a + optional repetition (one or
more times), \a \\\<digit\\\> denotes one of the characters 0 ... 9, \a |
denotes or, and \a \\\<single-quote\\\> denotes the character "'". The digits
before the \a \\\<single-quote\\\> character, when present, form the number
basis, that can go from 0, 1 and up to 36. Letters from `A` to
`Z` are used when the basis is larger than 10.

Note that if no basis is specified then base 10 is assumed. Note also
that the last digit of an integer token can not be immediately followed
by one of the characters 'e', 'E', or '.'.

Following the ISO standard, YAP also accepts directives of the
form `0x` to represent numbers in hexadecimal base and of the form
`0o` to represent numbers in octal base. For usefulness,
YAP also accepts directives of the form `0X` to represent
numbers in hexadecimal base.

Example:
the following tokens all denote the same integer

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
10  2'1010  3'101  8'12  16'a  36'a  0xa  0o12
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Numbers of the form `0'a` are used to represent character
constants. So, the following tokens denote the same integer:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
0'd  100
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

YAP (version 6.3.4) supports integers that can fit
the word size of the machine. This is 32 bits in most current machines,
but 64 in some others, such as the Alpha running Linux or Digital
Unix. The scanner will read larger or smaller integers erroneously.

  + Floats

Floating-point numbers are described by:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   <float> := <digit>+{<dot><digit>+}
               <exponent-marker>{<sign>}<digit>+
            |<digit>+<dot><digit>+
               {<exponent-marker>{<sign>}<digit>+}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

where \a \\\<dot\\\> denotes the decimal-point character '.',
\a \\\<exponent-marker\\\> denotes one of 'e' or 'E', and \a \\\<sign\\\>
denotes
one of '+' or '-'.

Examples:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
10.0   10e3   10e-3   3.1415e+3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Floating-point numbers are represented as a double in the target
machine. This is usually a 64-bit number.

+ Strings Character Strings

Strings are described by the following rules:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  string --> '"' string_quoted_characters '"'
  string --> '`' string_quoted_characters '`'

  string_quoted_characters --> '"' '"' string_quoted_characters
  string_quoted_characters --> '`' '`' string_quoted_characters
string_quoted_characters --> '\'
                          escape_sequence string_quoted_characters
  string_quoted_characters -->
                          string_character string_quoted_characters

  escape_sequence --> 'a' | 'b' | 'r' | 'f' | 't' | 'n' | 'v'
  escape_sequence --> '\' | '"' | ''' | '`'
  escape_sequence --> at_most_3_octal_digit_seq_char '\'
  escape_sequence --> 'x' at_most_2_hexa_digit_seq_char '\'
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
where `string_character` in any character except the double quote
and escape characters.

Examples:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
""   "a string"   "a double-quote:"""
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first string is an empty string, the last string shows the use of
double-quoting. The implementation of YAP represents strings as
lists of integers. Since YAP 4.3.0 there is no static limit on string
size.

Escape sequences can be used anf include the non-printable characters
`a` (alert), `b` (backspace), `r` (carriage return),
`f` (form feed), `t` (horizontal tabulation), `n` (new
line), and `v` (vertical tabulation). Escape sequences also be
include the meta-characters `\\`, `"`, `'`, and
```. Last, one can use escape sequences to include the characters
either as an octal or hexadecimal number.

The next examples demonstrates the use of escape sequences in YAP:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"\x0c\" "\01\" "\f" "\\"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first three examples return a list including only character 12 (form
feed). The last example escapes the escape character.

Escape sequences were not available in C-Prolog and in original
versions of YAP up to 4.2.0. Escape sequences can be disable by using:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:- yap_flag(character_escapes,false).
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since 6.3.4 YAP supports compact strings, that are not represented as
lists of codes, but instead as a sequence of UTF-8 encoded characters
in the execution stack. These strings do not require allocating a
symbol, as atoms do, but are much more compact than using lists of
codes.




+ Atoms Atoms

Atoms are defined by one of the following rules:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   atom --> solo-character
   atom --> lower-case-letter name-character*
   atom --> symbol-character+
   atom --> single-quote  single-quote
   atom --> ''' atom_quoted_characters '''

  atom_quoted_characters --> ''' ''' atom_quoted_characters
  atom_quoted_characters --> '\' atom_sequence string_quoted_characters
  atom_quoted_characters --> character string_quoted_characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
where:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   <solo-character>     denotes one of:    ! ;
   <symbol-character>   denotes one of:    # & * + - . / : <
                                           = > ? @ \ ^ ~ `
   <lower-case-letter>  denotes one of:    a...z
   <name-character>     denotes one of:    _ a...z A...Z 0....9
   <single-quote>       denotes:           '
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

and `string_character` denotes any character except the double quote
and escape characters. Note that escape sequences in strings and atoms
follow the same rules.

Examples:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a   a12x   '$a'   !   =>  '1 2'
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Version `4.2.0` of YAP removed the previous limit of 256
characters on an atom. Size of an atom is now only limited by the space
available in the system.

+ Variables Variables

Variables are described by:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   <variable-starter><variable-character>+
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
where

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  <variable-starter>   denotes one of:    _ A...Z
  <variable-character> denotes one of:    _ a...z A...Z
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a variable is referred only once in a term, it needs not to be named
and one can use the character `_` to represent the variable. These
variables are known as anonymous variables. Note that different
occurrences of `_` on the same term represent <em>different</em>
anonymous variables.

+ Punctuation Tokens

Punctuation tokens consist of one of the following characters:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
( ) , [ ] { } |
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These characters are used to group terms.

@subsection Layout Layout
Any characters with ASCII code less than or equal to 32 appearing before
a token are ignored.

All the text appearing in a line after the character \a % is taken to
be a comment and ignored (including \a %).  Comments can also be
inserted by using the sequence `/\*` to start the comment and
`\*` followed by `/` to finish it. In the presence of any sequence of comments
or
layout characters, the YAP parser behaves as if it had found a
single blank character. The end of a file also counts as a blank
character for this purpose.

+ Encoding Wide Character Support

YAP now implements a SWI-Prolog compatible interface to wide
characters and the Universal Character Set (UCS). The following text
was adapted from the SWI-Prolog manual.

YAP now  supports wide characters, characters with character
codes above 255 that cannot be represented in a single byte.
<em>Universal Character Set</em> (UCS) is the ISO/IEC 10646 standard
that specifies a unique 31-bits unsigned integer for any character in
any language.  It is a superset of 16-bit Unicode, which in turn is
a superset of ISO 8859-1 (ISO Latin-1), a superset of US-ASCII.  UCS
can handle strings holding characters from multiple languages and
character classification (uppercase, lowercase, digit, etc.) and
operations such as case-conversion are unambiguously defined.

For this reason YAP, following SWI-Prolog, has two representations for
atoms. If the text fits in ISO Latin-1, it is represented as an array
of 8-bit characters.  Otherwise the text is represented as an array of
wide chars, which may take 16 or 32 bits.  This representational issue
is completely transparent to the Prolog user.  Users of the foreign
language interface sometimes need to be aware of these issues though.

Character coding comes into view when characters of strings need to be
read from or written to file or when they have to be communicated to
other software components using the foreign language interface. In this
section we only deal with I/O through streams, which includes file I/O
as well as I/O through network sockets.

  + Stream_Encoding Wide character encodings on streams

Although characters are uniquely coded using the UCS standard
internally, streams and files are byte (8-bit) oriented and there are a
variety of ways to represent the larger UCS codes in an 8-bit octet
stream. The most popular one, especially in the context of the web, is
UTF-8. Bytes 0...127 represent simply the corresponding US-ASCII
character, while bytes 128...255 are used for multi-byte
encoding of characters placed higher in the UCS space. Especially on
MS-Windows the 16-bit Unicode standard, represented by pairs of bytes is
also popular.

Prolog I/O streams have a property called <em>encoding</em> which
specifies the used encoding that influence `get_code/2` and
`put_code/2` as well as all the other text I/O predicates.

The default encoding for files is derived from the Prolog flag
`encoding`, which is initialized from the environment.  If the
environment variable `LANG` ends in "UTF-8", this encoding is
assumed. Otherwise the default is `text` and the translation is
left to the wide-character functions of the C-library (note that the
Prolog native UTF-8 mode is considerably faster than the generic
`mbrtowc()` one).  The encoding can be specified explicitly in
load_files/2 for loading Prolog source with an alternative
encoding, `open/4` when opening files or using `set_stream/2` on
any open stream (not yet implemented). For Prolog source files we also
provide the `encoding/1` directive that can be used to switch
between encodings that are compatible to US-ASCII (`ascii`,
`iso_latin_1`, `utf8` and many locales).



For
additional information and Unicode resources, please visit
<http://www.unicode.org/>.

YAP currently defines and supports the following encodings:

  + octet
Default encoding for <em>binary</em> streams.  This causes
the stream to be read and written fully untranslated.

  + ascii
7-bit encoding in 8-bit bytes.  Equivalent to `iso_latin_1`,
but generates errors and warnings on encountering values above
127.

  + iso_latin_1
8-bit encoding supporting many western languages.  This causes
the stream to be read and written fully untranslated.

  + text
C-library default locale encoding for text files.  Files are read and
written using the C-library functions `mbrtowc()` and
`wcrtomb()`.  This may be the same as one of the other locales,
notably it may be the same as `iso_latin_1` for western
languages and `utf8` in a UTF-8 context.

  + utf8
Multi-byte encoding of full UCS, compatible to `ascii`.
See above.

  + unicode_be
Unicode Big Endian.  Reads input in pairs of bytes, most
significant byte first.  Can only represent 16-bit characters.

  + unicode_le
Unicode Little Endian.  Reads input in pairs of bytes, least
significant byte first.  Can only represent 16-bit characters.


Note that not all encodings can represent all characters. This implies
that writing text to a stream may cause errors because the stream
cannot represent these characters. The behaviour of a stream on these
errors can be controlled using `open/4` or `set_stream/2` (not
implemented). Initially the terminal stream write the characters using
Prolog escape sequences while other streams generate an I/O exception.

    + BOM BOM: Byte Order Mark

From Stream Encoding, you may have got the impression that
text-files are complicated. This section deals with a related topic,
making live often easier for the user, but providing another worry to
the programmer.   *BOM* or <em>Byte Order Marker</em> is a technique
for identifying Unicode text-files as well as the encoding they
use. Such files start with the Unicode character `0xFEFF`, a
non-breaking, zero-width space character. This is a pretty unique
sequence that is not likely to be the start of a non-Unicode file and
uniquely distinguishes the various Unicode file formats. As it is a
zero-width blank, it even doesn't produce any output. This solves all
problems, or ...

Some formats start of as US-ASCII and may contain some encoding mark to
switch to UTF-8, such as the `encoding="UTF-8"` in an XML header.
Such formats often explicitly forbid the the use of a UTF-8 BOM. In
other cases there is additional information telling the encoding making
the use of a BOM redundant or even illegal.

The BOM is handled by the `open/4` predicate. By default, text-files are
probed for the BOM when opened for reading. If a BOM is found, the
encoding is set accordingly and the property `bom(true)` is
available through stream_property/2. When opening a file for
writing, writing a BOM can be requested using the option
`bom(true)` with `open/4`.

                               */

#include "Yap.h"
#include "YapEval.h"
#include "YapHeap.h"
#include "Yatom.h"
#include "alloc.h"
#include "yapio.h"
/* stuff we want to use in standard YAP code */
#include "YapText.h"
#if _MSC_VER || defined(__MINGW32__)
#if HAVE_FINITE == 1
#undef HAVE_FINITE
#endif
#include <windows.h>
#endif
#include "iopreds.h"
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_WCTYPE_H
#include <wctype.h>
#endif
#if O_LOCALE
#include "locale.h"
#endif

/* You just can't trust some machines */
#define my_isxdigit(C, SU, SL)                                                 \
  (chtype(C) == NU || (C >= 'A' && C <= (SU)) || (C >= 'a' && C <= (SL)))
#define my_isupper(C) (C >= 'A' && C <= 'Z')
#define my_islower(C) (C >= 'a' && C <= 'z')

static Term float_send(char *, int);
static Term get_num(int *, int *, struct stream_desc *, int, char **, size_t *);

static void Yap_setCurrentSourceLocation(struct stream_desc *s) {
  CACHE_REGS
#if HAVE_SOCKET
  if (s->status & Socket_Stream_f)
    LOCAL_SourceFileName = AtomSocket;
  else
#endif
      if (s->status & Pipe_Stream_f)
    LOCAL_SourceFileName = AtomPipe;
  else if (s->status & InMemory_Stream_f)
    LOCAL_SourceFileName = s->name;
  else
    LOCAL_SourceFileName = s->name;
  LOCAL_SourceFileLineno = s->linecount;
}

/* token table with some help from Richard O'Keefe's PD scanner */
char_kind_t Yap_chtype0[NUMBER_OF_CHARS + 1] = {
    EF,
    /* nul soh stx etx eot enq ack bel  bs  ht  nl  vt  np  cr  so  si

           */
    BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS,

    /* dle dc1 dc2 dc3 dc4 nak syn etb can  em sub esc  fs  gs  rs  us
     */
    BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS,

    /* sp   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
    BS, SL, DC, SY, SY, CC, SY, QT, BK, BK, SY, SY, BK, SY, SY, SY,

    /* 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
    NU, NU, NU, NU, NU, NU, NU, NU, NU, NU, SY, SL, SY, SY, SY, SY,

    /* @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
    SY, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC,

    /* P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
    UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, BK, SY, BK, SY, UL,

    /* `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
    SY, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC,

    /* p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ del */
    LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, BK, BK, BK, SY, BS,

    /* 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143
     */
    BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS,

    /* 144 145    147 148 149 150 151 152 153 154 155 156 157 158 159
     */
    BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS, BS,

    /*     ¡   ¢   £   ¤   ¥   ¦   §   ¨   ©   ª   «   ¬   ­   ®   ¯ */
    BS, SY, SY, SY, SY, SY, SY, SY, SY, SY, LC, SY, SY, SY, SY, SY,

    /* °   ±   ²   ³   ´   µ   ¶   ·   ¸   ¹   º   »   ¼   ½   ¾   ¿ */
    SY, SY, LC, LC, SY, SY, SY, SY, SY, LC, LC, SY, SY, SY, SY, SY,

    /* À   Á   Â   Ã   Ä   Å   Æ   Ç   È   É   Ê   Ë   Ì   Í   Î   Ï */
    UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC,

/* Ð   Ñ   Ò   Ó   Ô   Õ   Ö   ×   Ø   Ù   Ú   Û   Ü   Ý   Þ   ß    */
#ifdef vms
    UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, UC, LC,
#else
    UC, UC, UC, UC, UC, UC, UC, SY, UC, UC, UC, UC, UC, UC, UC, LC,
#endif
    /* à   á   â   ã   ä   å   æ   ç   è   é   ê   ë   ì   í   î   ï */
    LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC,

/* ð   ñ   ò   ó   ô   õ   ö   ÷   ø   ù   ú   û   ü   cannot write the last
 * three because of lcc    */
#ifdef vms
    LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC, LC
#else
    LC, LC, LC, LC, LC, LC, LC, SY, LC, LC, LC, LC, LC, LC, LC, LC
#endif
};

typedef struct scanner_internals {
  StreamDesc *t;
  TokEntry *ctok;
  char *_ScannerStack; // = (char *)TR;
  char *ScannerExtraBlocks;
  CELL *CommentsTail;
  CELL *Comments;
  CELL *CommentsNextChar;
  wchar_t *CommentsBuff;
  size_t CommentsBuffLim;
} scanner_internals;

// standard get char, uses conversion table
// and converts to wide
static inline int getchr(struct stream_desc *inp) {
  /* if (inp != inp0) { fprintf(stderr,"\n %s
   * **********************************\n", AtomName(inp->name)); */
  /*   inp0 = inp; */
  /* } */
  int sno = inp - GLOBAL_Stream;
  int ch = inp->stream_wgetc_for_read(sno);
  // fprintf(stderr,"%c", ch);
  return ch;
}
// get char for quoted data, eg, quoted atoms and so on
// converts to wide
static inline int getchrq(struct stream_desc *inp) {
  int ch = inp->stream_wgetc(inp - GLOBAL_Stream);
  return ch;
}

/* in case there is an overflow */
typedef struct scanner_extra_alloc {
  struct scanner_extra_alloc *next;
  void *filler;
} ScannerExtraBlock;

#define CodeSpaceError(t, p, l) CodeSpaceError__(t, p, l PASS_REGS)
static TokEntry *CodeSpaceError__(TokEntry *t, TokEntry *p,
                                  TokEntry *l USES_REGS) {
  LOCAL_Error_TYPE = RESOURCE_ERROR_HEAP;
  LOCAL_ErrorMessage = "Code Space Overflow";
  if (t) {
    t->Tok = eot_tok;
    t->TokInfo = TermOutOfHeapError;
  }
  /* serious error now */
  return l;
}

#define TrailSpaceError(t, l) TrailSpaceError__(t, l PASS_REGS)
static TokEntry *TrailSpaceError__(TokEntry *t, TokEntry *l USES_REGS) {
  LOCAL_ErrorMessage = "Trail Overflow";
  LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
  if (t) {
    t->Tok = eot_tok;
    t->TokInfo = TermOutOfTrailError;
  }
  return l;
}

static void *InitScannerMemory(void) {
  CACHE_REGS
  LOCAL_ErrorMessage = NULL;
  LOCAL_Error_Size = 0;
  LOCAL_ScannerExtraBlocks = NULL;
  LOCAL_ScannerStack = (char *)TR;
  return (char *)TR;
}

static char *AllocScannerMemory(unsigned int size) {
  CACHE_REGS
  char *AuxSpScan;

  AuxSpScan = LOCAL_ScannerStack;
  size = AdjustSize(size);
  if (LOCAL_ScannerExtraBlocks) {
    struct scanner_extra_alloc *ptr;

    if (!(ptr = (struct scanner_extra_alloc *)Malloc(
              size + sizeof(ScannerExtraBlock)))) {
      return NULL;
    }
    ptr->next = LOCAL_ScannerExtraBlocks;
    LOCAL_ScannerExtraBlocks = ptr;
    return (char *)(ptr + 1);
  } else if (LOCAL_TrailTop <= AuxSpScan + size) {
    UInt alloc_size = sizeof(CELL) * K16;

    if (size > alloc_size)
      alloc_size = size;
    if (!Yap_growtrail(alloc_size, TRUE)) {
      struct scanner_extra_alloc *ptr;

      if (!(ptr = (struct scanner_extra_alloc *)Malloc(
                size + sizeof(ScannerExtraBlock)))) {
        return NULL;
      }
      ptr->next = LOCAL_ScannerExtraBlocks;
      LOCAL_ScannerExtraBlocks = ptr;
      return (char *)(ptr + 1);
    }
  }
  LOCAL_ScannerStack = AuxSpScan + size;
  return AuxSpScan;
}

char *Yap_AllocScannerMemory(unsigned int size) {
  /* I assume memory has been initialized */
  return AllocScannerMemory(size);
}

extern double atof(const char *);

static Term float_send(char *s, int sign) {
  Float f = (Float)(sign * atof(s));
#if HAVE_ISFINITE || defined(isfinite)
  if (trueGlobalPrologFlag(ISO_FLAG)) { /* iso */
    if (!isfinite(f)) {
      CACHE_REGS
      LOCAL_ErrorMessage = "Float overflow while scanning";
      return (MkEvalFl(f));
    }
  }
#elif HAVE_FINITE
  if (trueGlobalPrologFlag(ISO_FLAG)) { /* iso */
    if (!finite(f)) {
      LOCAL_ErrorMessage = "Float overflow while scanning";
      return (MkEvalFl(f));
    }
  }
#endif
  {
    CACHE_REGS
    return MkFloatTerm(f);
  }
}

/* we have an overflow at s */
static Term read_int_overflow(const char *s, Int base, Int val, int sign) {
#ifdef USE_GMP
  /* try to scan it as a bignum */
  mpz_t new;
  Term t;

  mpz_init_set_str(new, s, base);
  if (sign < 0)
    mpz_neg(new, new);
  t = Yap_MkBigIntTerm(new);
  mpz_clear(new);
  return t;
#else
  CACHE_REGS
  /* try to scan it as a float */
  return MkIntegerTerm(val);
#endif
}

static int send_error_message(char s[]) {
  CACHE_REGS
  LOCAL_ErrorMessage = s;
  return 0;
}

static wchar_t read_quoted_char(int *scan_nextp, struct stream_desc *st) {
  int ch;

/* escape sequence */
do_switch:
  ch = getchrq(st);
  switch (ch) {
  case 10:
    return 0;
  case '\\':
    return '\\';
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 'c':
    while (chtype((ch = getchrq(st))) == BS)
      ;
    {
      if (ch == '\\') {
        goto do_switch;
      }
      return ch;
    }
  case 'd':
    return 127;
  case 'e':
    return '\x1B'; /* <ESC>, a.k.a. \e */
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 's': /* space */
    return ' ';
  case 't':
    return '\t';
  case 'u': {
    int i;
    wchar_t wc = '\0';

    for (i = 0; i < 4; i++) {
      ch = getchrq(st);
      if (ch >= '0' && ch <= '9') {
        wc += (ch - '0') << ((3 - i) * 4);
      } else if (ch >= 'a' && ch <= 'f') {
        wc += ((ch - 'a') + 10) << ((3 - i) * 4);
      } else if (ch >= 'A' && ch <= 'F') {
        wc += ((ch - 'A') + 10) << ((3 - i) * 4);
      } else {
        return send_error_message("invalid escape sequence");
      }
    }
    return wc;
  }
  case 'U': {
    int i;
    wchar_t wc = '\0';

    for (i = 0; i < 8; i++) {
      ch = getchrq(st);
      if (ch >= '0' && ch <= '9') {
        wc += (ch - '0') << ((7 - i) * 4);
      } else if (ch >= 'a' && ch <= 'f') {
        wc += ((ch - 'a') + 10) << ((7 - i) * 4);
      } else if (ch >= 'A' && ch <= 'F') {
        wc += ((ch - 'A') + 10) << ((7 - i) * 4);
      } else {
        return send_error_message("invalid escape sequence");
      }
    }
    return wc;
  }
  case 'v':
    return '\v';
  case 'z': /* Prolog end-of-file */
    return send_error_message("invalid escape sequence \\z");
  case '\'':
    return '\'';
  case '"':
    return '"';
  case '`':
    return '`';
  case '^':
    if (trueGlobalPrologFlag(ISO_FLAG)) {
      return send_error_message("invalid escape sequence");
    } else {
      ch = getchrq(st);
      if (ch == '?') { /* delete character */
        return 127;
      } else if (ch >= 'a' && ch < 'z') { /* hexa */
        return ch - 'a';
      } else if (ch >= 'A' && ch < 'Z') { /* hexa */
        return ch - 'A';
      } else {
        return '^';
      }
    }
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
    /* character in octal: maximum of 3 digits, terminates with \ */
    /* follow ISO */
    {
      unsigned char so_far = ch - '0';
      ch = getchrq(st);
      if (ch >= '0' && ch < '8') { /* octal */
        so_far = so_far * 8 + (ch - '0');
        ch = getchrq(st);
        if (ch >= '0' && ch < '8') { /* octal */
          so_far = so_far * 8 + (ch - '0');
          ch = getchrq(st);
          if (ch != '\\') {
            return send_error_message("invalid octal escape sequence");
          }
          return so_far;
        } else if (ch == '\\') {
          return so_far;
        } else {
          return send_error_message("invalid octal escape sequence");
        }
      } else if (ch == '\\') {
        return so_far;
      } else {
        return send_error_message("invalid octal escape sequence");
      }
    }
  case 'x':
    /* hexadecimal character (YAP allows empty hexadecimal  */
    {
      unsigned char so_far = 0;
      ch = getchrq(st);
      if (my_isxdigit(ch, 'f', 'F')) { /* hexa */
        so_far =
            so_far * 16 + (chtype(ch) == NU
                               ? ch - '0'
                               : (my_isupper(ch) ? ch - 'A' : ch - 'a') + 10);
        ch = getchrq(st);
        if (my_isxdigit(ch, 'f', 'F')) { /* hexa */
          so_far =
              so_far * 16 + (chtype(ch) == NU
                                 ? ch - '0'
                                 : (my_isupper(ch) ? ch - 'A' : ch - 'a') + 10);
          ch = getchrq(st);
          if (ch == '\\') {
            return so_far;
          } else {
            return send_error_message("invalid hexadecimal escape sequence");
          }
        } else if (ch == '\\') {
          return so_far;
        } else {
          return send_error_message("invalid hexadecimal escape sequence");
        }
      } else if (ch == '\\') {
        return so_far;
      } else {
        return send_error_message("invalid hexadecimal escape sequence");
      }
    }
  default:
    /* accept sequence. Note that the ISO standard does not
     consider this sequence legal, whereas SICStus would
     eat up the escape sequence. */
    return send_error_message("invalid escape sequence");
  }
}

static int num_send_error_message(char s[]) {
  CACHE_REGS
  LOCAL_ErrorMessage = s;
  return MkStringTerm(s);
}

#define number_overflow()                                                      \
  {                                                                            \
    imgsz = Yap_Min(imgsz * 2, imgsz);                              \
    char *nbuf;                                                                \
      nbuf = Realloc(buf, imgsz);                                                left = imgsz - max_size;                                                   \
      max_size = imgsz;                                                          \
      buf = nbuf;                                                              \
  }

/* reads a number, either integer or float */

static Term get_num(int *chp, int *chbuffp, StreamDesc *st, int sign, char **bufp, size_t *szp) {
  int ch = *chp;
  Int val = 0L, base = ch - '0';
  int might_be_float = TRUE, has_overflow = FALSE;
  const unsigned char *decimalpoint;
  char *buf0 = *bufp, *sp = buf0, *buf = buf0;
  size_t imgsz = *szp, max_size = imgsz, left = max_size-2;

  *sp++ = ch;
  ch = getchr(st);
  /*
   * because of things like 00'2, 03'2 and even better 12'2, I need to
   * do this (have mercy)
   */
  if (chtype(ch) == NU) {
    *sp++ = ch;
    if (--left == 0)
      number_overflow();
    base = 10 * base + ch - '0';
    ch = getchr(st);
  }
  if (ch == '\'') {
    if (base > 36) {
        Yap_ThrowError(SYNTAX_ERROR, MkIntegerTerm(base), "Admissible bases are 0..36");
    }
    might_be_float = FALSE;
    if (--left == 0)
      number_overflow();
    *sp++ = ch;
    ch = getchr(st);
    if (base == 0) {
      CACHE_REGS
      wchar_t ascii = ch;
      int scan_extra = TRUE;

      if (ch == '\\' &&
          Yap_GetModuleEntry(CurrentModule)->flags & M_CHARESCAPE) {
        ascii = read_quoted_char(&scan_extra, st);
      }
      /* a quick way to represent ASCII */
      if (scan_extra)
        *chp = getchr(st);
      if (sign == -1) {
        return MkIntegerTerm(-ascii);
      }
      return MkIntegerTerm(ascii);
    } else if (base >= 10 && base <= 36) {
      int upper_case = 'A' - 11 + base;
      int lower_case = 'a' - 11 + base;

      while (my_isxdigit(ch, upper_case, lower_case)) {
        Int oval = val;
        int chval =
            (chtype(ch) == NU ? ch - '0'
                              : (my_isupper(ch) ? ch - 'A' : ch - 'a') + 10);
        if (--left == 0)
          number_overflow();
        *sp++ = ch;
        val = oval * base + chval;
        if (oval != (val - chval) / base) /* overflow */
          has_overflow = (has_overflow || TRUE);
        ch = getchr(st);
      }
    }
  } else if (ch == 'x' && base == 0) {
    might_be_float = FALSE;
    if (--left == 0)
      number_overflow();
    *sp++ = ch;
    ch = getchr(st);
    if (!my_isxdigit(ch, 'F', 'f'))  {
        Term t = ( Yap_local.ActiveError->errorRawTerm ?  Yap_local.ActiveError->errorRawTerm : MkIntegerTerm(ch) );
        Yap_local.ActiveError->errorRawTerm = 0;
      Yap_ThrowError(SYNTAX_ERROR, t, "invalid hexadecimal digit 0x%C",ch)   ;
      return 0;
    }
    while (my_isxdigit(ch, 'F', 'f')) {
      Int oval = val;
      int chval =
          (chtype(ch) == NU ? ch - '0'
                            : (my_isupper(ch) ? ch - 'A' : ch - 'a') + 10);
      if (--left == 0)
        number_overflow();
      *sp++ = ch;
      val = val * 16 + chval;
      if (oval != (val - chval) / 16) /* overflow */
        has_overflow = TRUE;
      ch = getchr(st);

    }
    *chp = ch;
  } else if (ch == 'o' && base == 0) {
    might_be_float = false;
    base = 8;
    ch = getchr(st);
      if (ch < '0' || ch > '7') {
          Term t = ( Yap_local.ActiveError->errorRawTerm ?  Yap_local.ActiveError->errorRawTerm : MkIntegerTerm(ch) );
          Yap_local.ActiveError->errorRawTerm = 0;
          Yap_ThrowError(SYNTAX_ERROR, t, "invalid octal digit 0x%C",ch)   ;
        return 0;
      }
  } else if (ch == 'b' && base == 0) {
    might_be_float = false;
    base = 2;
    ch = getchr(st);
    if (ch < '0' || ch > '1') {
        Term t = ( Yap_local.ActiveError->errorRawTerm ?  Yap_local.ActiveError->errorRawTerm : MkIntegerTerm(ch) );
        Yap_local.ActiveError->errorRawTerm = 0;
        Yap_ThrowError(SYNTAX_ERROR, t, "invalid binary digit 0x%C",ch)   ;
      return 0;
    }


  } else {
    val = base;
    base = 10;
  }
  while (chtype(ch) == NU) {
    Int oval = val;
    if (!(val == 0 && ch == '0') || has_overflow) {
      if (--left == 0)
        number_overflow();
      *sp++ = ch;
    }
    if (ch - '0' >= base) {
      CACHE_REGS
      if (sign == -1)
        return MkIntegerTerm(-val);
      return MkIntegerTerm(val);
    }
    val = val * base + ch - '0';
    if (val / base != oval || val - oval * base != ch - '0') /* overflow */
      has_overflow = true;
    ch = getchr(st);
  }
  if (might_be_float && (ch == '.' || ch == 'e' || ch == 'E')) {
    int has_dot = (ch == '.');
    if (has_dot) {
      unsigned char *dp;
      int dc;
      if (chtype(ch = getchr(st)) != NU) {
        if (ch == 'e' || ch == 'E') {
          if (trueGlobalPrologFlag(ISO_FLAG))
            return num_send_error_message(
                "Float format not allowed in ISO mode");
        } else { /* followed by a letter, end of term? */
          CACHE_REGS
          sp[0] = '\0';
          *chbuffp = '.';
          *chp = ch;
          if (has_overflow)
            return read_int_overflow(buf, base, val, sign);
          if (sign == -1)
            return MkIntegerTerm(-val);
          return MkIntegerTerm(val);
        }
      }
#if O_LOCALE
      if ((decimalpoint = (unsigned char *)(localeconv()->decimal_point)) ==
          NULL)
#endif
        decimalpoint = (const unsigned char *)".";
      dp = (unsigned char *)decimalpoint;
      /* translate . to current locale */
      while ((dc = *dp++) != '\0') {
        *sp++ = dc;
        if (--left == 0)
          number_overflow();
      }
      /* numbers after . */
      if (chtype(ch) == NU) {
        do {
          if (--left == 0)
            number_overflow();
          *sp++ = ch;
        } while (chtype(ch = getchr(st)) == NU);
      }
    }
    if (ch == 'e' || ch == 'E') {
      if (--left == 0)
        number_overflow();
      *sp++ = ch;
      ch = getchr(st);
      if (ch == '-') {
        if (--left == 0)
          number_overflow();
        *sp++ = '-';
        ch = getchr(st);
      } else if (ch == '+') {
        ch = getchr(st);
      }
      if (chtype(ch) != NU) {
        CACHE_REGS
        if (has_dot)
          return float_send(buf, sign);
        return MkIntegerTerm(sign * val);
      }
      do {
        if (--left == 0)
          number_overflow();
        *sp++ = ch;
      } while (chtype(ch = getchr(st)) == NU);
    }
    *sp = '\0';
    *chp = ch;
    return float_send(buf, sign);
  } else if (has_overflow) {
    *sp = '\0';
    /* skip base */
    *chp = ch;
    if (buf[0] == '0' && buf[1] == 'x')
      return read_int_overflow(buf + 2, 16, val, sign);
    else if (buf[0] == '0' && buf[1] == 'o')
      return read_int_overflow(buf + 2, 8, val, sign);
    else if (buf[0] == '0' && buf[1] == 'b')
      return read_int_overflow(buf + 2, 2, val, sign);
    if (buf[1] == '\'')
      return read_int_overflow(buf + 2, base, val, sign);
    if (buf[2] == '\'')
      return read_int_overflow(buf + 3, base, val, sign);
    return read_int_overflow(buf, base, val, sign);
  } else {
    CACHE_REGS
    *chp = ch;
    return MkIntegerTerm(val * sign);
  }
}

/* given a function getchr scan until we  either find the number
   or end of file */
Term Yap_scan_num(StreamDesc *inp, bool error_on) {
  CACHE_REGS
  Term out;
  int sign = 1;
  int ch, cherr = 0;
  char *ptr;
  void *old_tr = TR;

  InitScannerMemory();
  LOCAL_VarTable = LOCAL_AnonVarTable = NULL;
  if (!(ptr = AllocScannerMemory(4096))) {
    LOCAL_ErrorMessage = "Trail Overflow";
    LOCAL_Error_TYPE = RESOURCE_ERROR_TRAIL;
    return 0;
  }
#if HAVE_ISWSPACE
  while (iswspace(ch = getchr(inp)))
    ;
#else
  while (isspace(ch = getchr(inp)))
    ;
#endif
  TokEntry *tokptr = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
  tokptr->TokLine = GetCurInpLine(inp);
  tokptr->TokPos = GetCurInpPos(inp);
  if (ch == '-') {
    sign = -1;
    ch = getchr(inp);
  } else if (ch == '+') {
    ch = getchr(inp);
  }
  if (chtype(ch) == NU) {
    cherr = '\0';
    if (ASP - HR < 1024) {
      Yap_clean_tokenizer(old_tr, NULL, NULL);
      LOCAL_ErrorMessage = "Stack Overflow";
      LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;
      return 0;
    }
    size_t sz = 1024;
    char *buf = Malloc(sz);
    int lvl = push_text_stack();
    out = get_num(&ch, &cherr, inp, sign, &buf, &sz); /*  */
    pop_text_stack(lvl);
  } else {
    out = 0;
  }
#if HAVE_ISWSPACE
  while (iswspace(ch = getchr(inp)))
    ;
#else
  while (isspace(ch = getchr(inp)))
    ;
#endif
  if (ch == EOF)
  return out;
  return 0;
}

#define CHECK_SPACE()                                                          \
  if (ASP - HR < 1024) {                                                       \
    LOCAL_ErrorMessage = "Stack Overflow";                                     \
    LOCAL_Error_TYPE = RESOURCE_ERROR_STACK;                                   \
    LOCAL_Error_Size = 0L;                                                     \
    if (p) {                                                                   \
      p->Tok = Ord(kind = eot_tok);                                            \
      p->TokInfo = TermOutOfStackError;                                        \
    }                                                                          \
    /* serious error now */                                                    \
    return l;                                                                  \
  }

Term Yap_tokRep(void *tokptre) {
  CACHE_REGS
  TokEntry *tokptr = tokptre;
  Term info = tokptr->TokInfo;

  switch (tokptr->Tok) {
  case Name_tok:
    if (!info) {
      info = TermNil;
    } else {
      info = MkAtomTerm((Atom)info);
    }
    return Yap_MkApplTerm(Yap_MkFunctor(AtomAtom, 1), 1, &info);
  case QuasiQuotes_tok:
    info = MkAtomTerm(Yap_LookupAtom("<QQ>"));
    return Yap_MkApplTerm(Yap_MkFunctor(AtomAtom, 1), 1, &info);
  case Number_tok:
    return Yap_MkApplTerm(Yap_MkFunctor(AtomNumber, 1), 1, &info);
    break;
  case Var_tok: {
    Term t[2];
    VarEntry *varinfo = (VarEntry *)info;
    if ((t[0] = varinfo->VarAdr) == TermNil) {
      t[0] = varinfo->VarAdr = MkVarTerm();
    }
    t[1] = MkAtomTerm((Atom)(varinfo->VarRep));
    return Yap_MkApplTerm(Yap_MkFunctor(AtomGVar, 2), 2, t);
  }
  case String_tok:
    return Yap_MkApplTerm(Yap_MkFunctor(AtomString, 1), 1, &info);
  case BQString_tok:
    return Yap_MkApplTerm(Yap_MkFunctor(AtomString, 1), 1, &info);
  case Error_tok:
    return MkAtomTerm(AtomError);
  case eot_tok:
    return MkAtomTerm(Yap_LookupAtom("EOT"));
  case Ponctuation_tok:
    if (info == Terml)
      return TermBeginBracket;
    else
      return info;
  }
  return TermDot;
}

const char *Yap_tokText(void *tokptre) {
  CACHE_REGS
  TokEntry *tokptr = tokptre;
  Term info = tokptr->TokInfo;

  switch (tokptr->Tok) {
  case eot_tok:
    return "EOT";
  case Ponctuation_tok:
    if (info == Terml)
      return "(";
  case Error_tok:
  case BQString_tok:
  case String_tok:
  case Name_tok:
    return AtomOfTerm(info)->StrOfAE;
  case QuasiQuotes_tok:
    return "<QQ>";
  case Number_tok:
    if (IsIntegerTerm(info)) {
      char *s = Malloc(36);
      snprintf(s, 35, Int_FORMAT, IntegerOfTerm(info));
      return s;
    } else if (IsFloatTerm(info)) {
      char *s = Malloc(64);
      snprintf(s, 63, "%6g", FloatOfTerm(info));
      return s;
    } else {
      size_t len = Yap_gmp_to_size(info, 10);
      char *s = Malloc(len + 2);
      return Yap_gmp_to_string(info, s, len + 1, 10);
    }
    break;
  case Var_tok:
    if (info == 0)
      return "[]";
    return ((Atom)info)->StrOfAE;
  }
  return ".";
}

static void open_comment(int ch, StreamDesc *st USES_REGS) {
  CELL *h0 = HR;
  HR += 5;
  h0[0] = AbsAppl(h0 + 2);
  h0[1] = TermNil;
  if (!LOCAL_CommentsTail) {
    /* first comment */
    LOCAL_Comments = AbsPair(h0);
  } else {
    /* extra comment */
    *LOCAL_CommentsTail = AbsPair(h0);
  }
  LOCAL_CommentsTail = h0 + 1;
  h0 += 2;
  h0[0] = (CELL)FunctorMinus;
  h0[1] = Yap_StreamPosition(st - GLOBAL_Stream);
  h0[2] = TermNil;
  LOCAL_CommentsNextChar = h0 + 2;
  LOCAL_CommentsBuff = (wchar_t *)Malloc(1024 * sizeof(wchar_t));
  LOCAL_CommentsBuffLim = 1024;
  LOCAL_CommentsBuff[0] = ch;
  LOCAL_CommentsBuffPos = 1;
}

static void extend_comment(int ch USES_REGS) {
  LOCAL_CommentsBuff[LOCAL_CommentsBuffPos] = ch;
  LOCAL_CommentsBuffPos++;
  if (LOCAL_CommentsBuffPos == LOCAL_CommentsBuffLim - 1) {
    LOCAL_CommentsBuff = (wchar_t *)realloc(
        LOCAL_CommentsBuff, sizeof(wchar_t) * (LOCAL_CommentsBuffLim + 4096));
    LOCAL_CommentsBuffLim += 4096;
  }
}

static void close_comment(USES_REGS1) {
  LOCAL_CommentsBuff[LOCAL_CommentsBuffPos] = '\0';
  *LOCAL_CommentsNextChar = Yap_WCharsToString(LOCAL_CommentsBuff PASS_REGS);
  Free(LOCAL_CommentsBuff);
  LOCAL_CommentsBuff = NULL;
  LOCAL_CommentsBuffLim = 0;
}

// mark that we reached EOF,
// next  token will be end_of_file)
static void mark_eof(struct stream_desc *st) {
  st->status |= Push_Eof_Stream_f;
}

#define add_ch_to_buff(ch)                                                     \
  { charp += put_utf8(charp, ch); }

TokEntry *Yap_tokenizer(struct stream_desc *st,
                        scanner_params *params) {

  CACHE_REGS
  TokEntry *t, *l, *p;
  enum TokenKinds kind;
  int solo_flag = TRUE;
  int lvl = push_text_stack();
  int32_t ch, och = ' ';
  struct qq_struct_t *cur_qq = NULL;
  int sign = 1;
  size_t imgsz = 1024;
  char *TokImage = Malloc(imgsz PASS_REGS);
  bool store_comments = params->store_comments;

  InitScannerMemory();
  LOCAL_VarTable = NULL;
  LOCAL_AnonVarTable = NULL;
  l = NULL;
  p = NULL; /* Just to make lint happy */
  ch = getchr(st);
  while (chtype(ch) == BS) {
    och=ch;
    ch = getchr(st);
  }
  params->tposOUTPUT = Yap_StreamPosition(st - GLOBAL_Stream);
  Yap_setCurrentSourceLocation(st);
  LOCAL_StartLineCount = st->linecount;
  LOCAL_StartLinePos = st->linepos;
  do {
    int quote, isvar;
    unsigned char *charp, *mp;
    size_t len;

    t = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
    t->TokNext = NULL;
    if (t == NULL) {
        pop_text_stack(lvl);
      return TrailSpaceError(p, l);
    }
    if (!l)
      l = t;
    else
      p->TokNext = t;
    p = t;
  restart:
    while (chtype(ch) == BS) {
      ch = getchr(st);
    }
    t->TokPos = GetCurInpPos(st);
    t->TokLine = GetCurInpLine(st);

    switch (chtype(ch)) {

    case CC:
      if (store_comments) {
        open_comment(ch, st PASS_REGS);
      continue_comment:
        while ((ch = getchr(st)) != 10 && chtype(ch) != EF) {
          extend_comment(ch PASS_REGS);
        }
        extend_comment(ch PASS_REGS);
        if (chtype(ch) != EF) {
          ch = getchr(st);
          if (chtype(ch) == CC) {
            extend_comment(ch PASS_REGS);
            goto continue_comment;
          }
        }
        close_comment(PASS_REGS1);
      } else {
        while ((ch = getchr(st)) != 10 && chtype(ch) != EF)
          ;
      }
      if (chtype(ch) != EF) {
        /* blank space */
        if (t == l) {
          /* we found a comment before reading characters */
          while (chtype(ch) == BS) {
            ch = getchr(st);
          }
          params->tposOUTPUT = Yap_StreamPosition(st - GLOBAL_Stream);
          Yap_setCurrentSourceLocation(st);
        }
        goto restart;
      } else {
        t->Tok = Ord(kind = eot_tok);
        mark_eof(st);
        t->TokInfo = TermEof;
      }
      break;

    case UC:
    case UL:
    case LC:
      och = ch;
      ch = getchr(st);
    scan_name:
        {
      charp = (unsigned char *)TokImage;
      isvar = (chtype(och) != LC);
      add_ch_to_buff(och);
      for (; chtype(ch) <= NU; ch = getchr(st)) {
        if (charp == (unsigned char *)TokImage + (imgsz - 1)) {
          unsigned char *p0 = (unsigned char *)TokImage;
          imgsz = Yap_Min(imgsz * 2, imgsz + MBYTE);
          TokImage = Realloc(p0, imgsz);
          if (TokImage == NULL) {
              pop_text_stack(lvl);
              return CodeSpaceError(t, p, l);
          }
          charp =(unsigned char *) TokImage + (charp - p0);
        }
        add_ch_to_buff(ch);
      }
      while (ch == '\'' && isvar &&
             params->ce) {
        if (charp == (unsigned char *)AuxSp - 1024) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        add_ch_to_buff(ch);
        ch = getchr(st);
      }
      add_ch_to_buff('\0');
      if (!isvar || (ch == '(' && params->vn_asfl) ||
	  (TokImage[0] != '_' && params->vprefix)) {
        Atom ae;
        /* don't do this in iso */
        ae = Yap_LookupAtom(TokImage);
        if (ae == NIL) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->TokInfo = MkAtomTerm(ae);
        if (ch == '(')
          solo_flag = FALSE;
        t->Tok = Ord(kind = Name_tok);
      } else {
        VarEntry *ve = Yap_LookupVar((const char *)TokImage);
        t->TokInfo = Unsigned(ve);
        if (cur_qq) {
          ve->refs++;
        }
        t->Tok = Ord(kind = Var_tok);
      }

    } break;

    case NU: {
      int cherr;
      int cha;
      sign = 1;

    scan_number:
      cha = ch;
      cherr = 0;
      CHECK_SPACE();
      if ((t->TokInfo = get_num(&cha, &cherr, st, sign,&TokImage,&imgsz)) == 0L) {
        if (t->TokInfo == 0) {
          p->Tok = eot_tok;
          t->TokInfo = TermError;
        }
        /* serious error now */
          pop_text_stack(lvl);
          return l;
      }
      ch = cha;
      if (cherr) {
        TokEntry *e;
        t->Tok = Number_tok;
        t->TokPos = GetCurInpPos(st);
        t->TokLine = GetCurInpLine(st);
        e = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
        if (e == NULL) {
            pop_text_stack(lvl);
            return TrailSpaceError(p, l);

        } else {
          e->TokNext = NULL;
        }
        t->TokNext = e;
        t = e;
        p = e;
        switch (cherr) {
        case 'e':
        case 'E':
          och = cherr;
           goto scan_name;
          break;
        case '=':
        case '_':
          /* handle error while parsing a float */
          {
            TokEntry *e2;

            t->Tok = Ord(Var_tok);
            t->TokInfo = (Term)Yap_LookupVar("E");
            t->TokPos = GetCurInpPos(st);
            t->TokLine = GetCurInpLine(st);
            e2 = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
            if (e2 == NULL) {
                pop_text_stack(lvl);
                return TrailSpaceError(p, l);
            } else {
              e2->TokNext = NULL;
            }
            t->TokNext = e2;
            t = e2;
            p = e2;
            if (cherr == '=')
              och = '+';
            else
              och = '-';
          }
          goto enter_symbol;
        case '+':
        case '-':
          /* handle error while parsing a float */
          {
            TokEntry *e2;

            if (chtype(ch) == NU) {
              if (och == '-')
                sign = -1;
              else
                sign = 1;
              goto scan_number;
            }
            t->Tok = Name_tok;
            if (ch == '(')
              solo_flag = FALSE;
            t->TokInfo = MkAtomTerm(AtomE);
            t->TokLine = GetCurInpLine(st);
            t->TokPos = GetCurInpPos(st);
            e2 = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
            if (e2 == NULL) {
                pop_text_stack(lvl);
                return TrailSpaceError(p, l);
            } else {
              e2->TokNext = NULL;
            }
            t->TokNext = e2;
            t = e2;
            p = e2;
          }
        default:
          och = cherr;
          goto enter_symbol;
        }
      } else {
        t->Tok = Ord(kind = Number_tok);
      }
    } break;

    case QT:
    case DC:
    quoted_string:
      charp =(unsigned char *) TokImage;
      quote = ch;
      len = 0;
      ch = getchrq(st);

      while (TRUE) {
        if (charp > (unsigned char *)TokImage + (imgsz - 1)) {
	  size_t sz = charp-(unsigned char *)TokImage;
          TokImage = Realloc(TokImage, (imgsz = Yap_Min(imgsz * 2, imgsz + MBYTE)));
          if (TokImage == NULL) {
              pop_text_stack(lvl);
              return CodeSpaceError(t, p, l);
          }
	  charp = (unsigned char *)TokImage+sz;
          break;
        }
        if (ch == 10 && (trueGlobalPrologFlag(ISO_FLAG) ||
			 trueLocalPrologFlag(MULTILINE_QUOTED_TEXT_FLAG))) {
          /* in ISO a new line terminates a string */
          LOCAL_ErrorMessage = "layout character \n inside quotes";
          break;
        }
        if (ch == EOFCHAR) {
          break;
        }
        if (ch == quote) {
          ch = getchrq(st);
          if (ch != quote)
            break;
          add_ch_to_buff(ch);
          ch = getchrq(st);
        } else if (ch == '\\' &&
                   Yap_GetModuleEntry(CurrentModule)->flags & M_CHARESCAPE) {
          int scan_next = TRUE;
          if ((ch = read_quoted_char(&scan_next, st))) {
            add_ch_to_buff(ch);
          }
          if (scan_next) {
            ch = getchrq(st);
          }
        } else {
          add_ch_to_buff(ch);
          ch = getchrq(st);
        }
        ++len;
      }
      *charp = '\0';
      if (quote == '"') {
        t->TokInfo = Yap_CharsToTDQ((char *)TokImage, CurrentModule,
                                    LOCAL_encoding PASS_REGS);
        if (!(t->TokInfo)) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->Tok = Ord(kind = String_tok);
      } else if (quote == '`') {
        t->TokInfo = Yap_CharsToTBQ((char *)TokImage, CurrentModule,
                                    LOCAL_encoding PASS_REGS);
        if (!(t->TokInfo)) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->Tok = Ord(kind = String_tok);
      } else {
        t->TokInfo = MkAtomTerm(Yap_LookupAtom(TokImage));
        if (!(t->TokInfo)) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->Tok = Ord(kind = Name_tok);
        if (ch == '(')
          solo_flag = false;
      }
      break;

    case BS:
      if (ch == '\0') {
        int pch;
        t->Tok = Ord(kind = eot_tok);
        pch = Yap_peek(st - GLOBAL_Stream);
        if (chtype(pch) == EF) {
          mark_eof(st);
        } else {
	  if (params->get_eot_blank)
         getchr(st);
        }
        t->TokInfo = TermEof;
          pop_text_stack(lvl);
          return l;
      } else
        ch = getchr(st);
      break;
    case SY: {
      int pch;
      if (ch == '.' && (pch = Yap_peek(st - GLOBAL_Stream)) &&
          (chtype(pch) == BS || chtype(pch) == EF || pch == '%')) {
            if (chtype(ch) != EF)
              getchr(st);
        t->Tok = Ord(kind = eot_tok);
        // consume...
        if (pch == '%') {
          t->TokInfo = TermNewLine;
            pop_text_stack(lvl);
            return l;
        }
          pop_text_stack(lvl);
          return l;
      }
      if (ch == '`')
        goto quoted_string;
      och = ch;
      ch = getchr(st);
      if (och == '.') {
        if (chtype(ch) == BS || chtype(ch) == EF || ch == '%') {
          t->Tok = Ord(kind = eot_tok);
          if (ch == '%') {
            t->TokInfo = TermNewLine;
              pop_text_stack(lvl);
              return l;
          }
          if (chtype(ch) == EF) {
            mark_eof(st);
            t->TokInfo = TermEof;
          } else {
            t->TokInfo = TermNewLine;
          }
            pop_text_stack(lvl);
            return l;
        }
      }
      if (och == '/' && ch == '*') {
        if (store_comments) {
          CHECK_SPACE();
          open_comment('/', st PASS_REGS);
          while ((och != '*' || ch != '/') && chtype(ch) != EF) {
            och = ch;
            CHECK_SPACE();
            extend_comment(ch PASS_REGS);
            ch = getchr(st);
          }
          if (chtype(ch) != EF) {
            CHECK_SPACE();
            extend_comment(ch PASS_REGS);
          }
          close_comment(PASS_REGS1);
        } else {
          while ((och != '*' || ch != '/') && chtype(ch) != EF) {
            och = ch;
            ch = getchr(st);
          }
        }
        if (chtype(ch) == EF) {
          t->Tok = Ord(kind = eot_tok);
          t->TokInfo = TermEof;
          break;
        } else {
          /* leave comments */
          ch = getchr(st);
          if (t == l) {
            /* we found a comment before reading characters */
            while (chtype(ch) == BS) {
              ch = getchr(st);
            }
            CHECK_SPACE();
            params->tposOUTPUT = Yap_StreamPosition(st - GLOBAL_Stream);
            Yap_setCurrentSourceLocation(st);
          }
        }
        goto restart;
      }
    }
    enter_symbol:
      if (och == '.' && (chtype(ch) == BS || chtype(ch) == EF || ch == '%')) {
        t->Tok = Ord(kind = eot_tok);
        if (ch == '%') {
          t->TokInfo = TermNewLine;
            pop_text_stack(lvl);
            return l;
        }
        if (chtype(ch) == EF) {
          mark_eof(st);
          t->TokInfo = TermEof;
        } else {
          t->TokInfo = TermNl;
        }
          pop_text_stack(lvl);
          return l;
      } else {
        Atom ae;
        charp = (unsigned char *)TokImage;
        add_ch_to_buff(och);
        for (; chtype(ch) == SY; ch = getchr(st)) {
          if (charp >= (unsigned char *)TokImage + (imgsz - 10)) {
	    size_t sz = charp - (unsigned char *)TokImage;
            imgsz = Yap_Min(imgsz * 2, imgsz + MBYTE);
            TokImage = Realloc(TokImage, imgsz);
            if (!TokImage) {
                pop_text_stack(lvl);
                return CodeSpaceError(t, p, l);
            }
	    charp = (unsigned char *)TokImage+sz;
          }
          add_ch_to_buff(ch);
        }
        add_ch_to_buff('\0');
        ae = Yap_LookupAtom(TokImage);
        if (ae == NIL) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->TokInfo = MkAtomTerm(ae);
        if (t->TokInfo == (CELL)NIL) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        t->Tok = Ord(kind = Name_tok);
        if (ch == '(')
          solo_flag = false;
        else
          solo_flag = true;
      }
      break;

    case SL: {
      unsigned char chs[2];
      chs[0] = ch;
      chs[1] = '\0';
      ch = getchr(st);
      t->TokInfo = MkAtomTerm(Yap_ULookupAtom(chs));
      t->Tok = Ord(kind = Name_tok);
      if (ch == '(')
        solo_flag = FALSE;
    } break;

    case BK:
      och = ch;
      ch = getchr(st);
      {
	unsigned char *chs;
        charp = chs = (unsigned char *)TokImage;
        add_ch_to_buff(och);
        charp[0] = '\0';
        t->TokInfo = MkAtomTerm(Yap_ULookupAtom(chs));
      }
      if (och == '(') {
        while (chtype(ch) == BS) {
          ch = getchr(st);
        }
        if (ch == ')') {
          t->TokInfo = TermEmptyBrackets;
          t->Tok = Ord(kind = Name_tok);
          ch = getchr(st);
          solo_flag = FALSE;
          break;
        } else if (!solo_flag) {
          t->TokInfo = Terml;
          solo_flag = TRUE;
        }
      } else if (och == '[') {
        while (chtype(ch) == BS) {
          ch = getchr(st);
        };
        if (ch == ']') {
          t->TokInfo = TermNil;
          t->Tok = Ord(kind = Name_tok);
          ch = getchr(st);
          solo_flag = FALSE;
          break;
        }
      } else if (och == '{') {
        if (ch == '|') {
          qq_t *qq = (qq_t *)calloc(sizeof(qq_t), 1);
          if (!qq) {
            LOCAL_ErrorMessage = "not enough heap space to read in quasi quote";
            t->Tok = Ord(kind = eot_tok);
            t->TokInfo = TermOutOfHeapError;
              pop_text_stack(lvl);
              return l;
          }
          if (cur_qq) {
            LOCAL_ErrorMessage = "quasi quote in quasi quote";
            Yap_ReleasePreAllocCodeSpace((CODEADDR)TokImage);
            t->Tok = Ord(kind = eot_tok);
            t->TokInfo = TermOutOfHeapError;
              pop_text_stack(lvl);
              return l;
          } else {
            cur_qq = qq;
          }
          t->TokInfo = (CELL)qq;
          if (st->status & Seekable_Stream_f) {
            qq->start.byteno = fseek(st->file, 0, 0);
          } else {
            qq->start.byteno = st->charcount - 1;
          }
          qq->start.lineno = st->linecount;
          qq->start.linepos = st->linepos - 1;
          qq->start.charno = st->charcount - 1;
          t->Tok = Ord(kind = QuasiQuotes_tok);
          ch = getchr(st);
          solo_flag = FALSE;
          break;
        }
        while (chtype(ch) == BS) {
          ch = getchr(st);
        };
        if (ch == '}') {
          t->TokInfo = TermBraces;
          t->Tok = Ord(kind = Name_tok);
          ch = getchr(st);
          solo_flag = FALSE;
          break;
        }
      } else if (och == '|' && ch == '|') {
        qq_t *qq = cur_qq;
        if (!qq) {
          LOCAL_ErrorMessage = "quasi quoted's || without {|";
          Yap_ReleasePreAllocCodeSpace((CODEADDR)TokImage);
          Free(cur_qq);
          cur_qq = NULL;
          t->Tok = Ord(kind = eot_tok);
          t->TokInfo = TermError;
            pop_text_stack(lvl);
            return l;
        }
        cur_qq = NULL;
        t->TokInfo = (CELL)qq;
        if (st->status & Seekable_Stream_f) {
          qq->mid.byteno = fseek(st->file, 0, 0);
        } else {
          qq->mid.byteno = st->charcount - 1;
        }
        qq->mid.lineno = st->linecount;
        qq->mid.linepos = st->linepos - 1;
        qq->mid.charno = st->charcount - 1;
        t->Tok = Ord(kind = QuasiQuotes_tok);
        ch = getchr(st);
        charp = (unsigned char *)TokImage;
        quote = ch;
        len = 0;
        ch = getchrq(st);

        while (TRUE) {
          if (ch == '|') {
            ch = getchrq(st);
            if (ch != '}') {
            } else {
              charp += put_utf8((unsigned char *)charp, och);
              charp += put_utf8((unsigned char *)charp, ch);
              /* we're done */
              break;
            }
          } else if (chtype(ch) == EF) {
            mark_eof(st);
            t->Tok = Ord(kind = eot_tok);
            t->TokInfo = TermOutOfHeapError;
            break;
          } else {
            charp += put_utf8(charp, ch);
            ch = getchrq(st);
          }
        }
        len = charp - (unsigned char *)TokImage;
        mp = malloc(len + 1);
        if (mp == NULL) {
          LOCAL_ErrorMessage = "not enough heap space to read in quasi quote";
          t->Tok = Ord(kind = eot_tok);
          t->TokInfo = TermOutOfHeapError;
            pop_text_stack(lvl);
            return l;
        }
        strncpy((char *)mp, (const char *)TokImage, len + 1);
        qq->text = (unsigned char *)mp;
        if (st->status & Seekable_Stream_f) {
          qq->end.byteno = fseek(st->file, 0, 0);
        } else {
          qq->end.byteno = st->charcount - 1;
        }
        qq->end.lineno = st->linecount;
        qq->end.linepos = st->linepos - 1;
        qq->end.charno = st->charcount - 1;
        if (!(t->TokInfo)) {
            pop_text_stack(lvl);
            return CodeSpaceError(t, p, l);
        }
        Yap_ReleasePreAllocCodeSpace((CODEADDR)TokImage);
        solo_flag = FALSE;
        ch = getchr(st);
        break;
      }
      t->Tok = Ord(kind = Ponctuation_tok);
      break;
    case EF:
      mark_eof(st);
      t->Tok = Ord(kind = eot_tok);
      t->TokInfo = TermEof;
            pop_text_stack(lvl);
            return l;

    default: {
      kind = Error_tok;
      char err[1024];
      snprintf(err, 1023, "\n++++ token: unrecognised char %c (%d), type %c\n",
               ch, ch, chtype(ch));
    }
      t->Tok = Ord(kind = eot_tok);
      t->TokInfo = TermEof;
    }
    if (LOCAL_ErrorMessage) {
      /* insert an error token to inform the system of what happened */
      TokEntry *e = (TokEntry *)AllocScannerMemory(sizeof(TokEntry));
      if (e == NULL) {
          pop_text_stack(lvl);
          return TrailSpaceError(p, l);
      }
      p->TokNext = e;
      e->Tok = Error_tok;
      e->TokInfo = MkAtomTerm(Yap_LookupAtom(LOCAL_ErrorMessage));
      e->TokPos = GetCurInpPos(st);
      e->TokLine = GetCurInpLine(st);
      e->TokNext = NULL;
      LOCAL_ErrorMessage = NULL;
      p = e;
    }
  } while (kind != eot_tok);
    pop_text_stack(lvl);

  return (l);
}

void Yap_clean_tokenizer(TokEntry *tokstart, VarEntry *vartable,
                         VarEntry *anonvartable) {
  CACHE_REGS
  struct scanner_extra_alloc *ptr = LOCAL_ScannerExtraBlocks;
  while (ptr) {
    struct scanner_extra_alloc *next = ptr->next;
    Free(ptr);
    ptr = next;
  }
  TR = (tr_fr_ptr)tokstart;
  LOCAL_Comments = TermNil;
  LOCAL_CommentsNextChar = LOCAL_CommentsTail = NULL;
  if (LOCAL_CommentsBuff) {
    Free(LOCAL_CommentsBuff);
    LOCAL_CommentsBuff = NULL;
  }
  LOCAL_ScannerStack = NULL;
  LOCAL_CommentsBuffLim = 0;
}

/// @}
