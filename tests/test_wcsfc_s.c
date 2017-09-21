/*------------------------------------------------------------------
 * test_wcsfc_s
 * File 'wcsfc_s.c'
 * Lines executed:94.83% of 116
 *
 *------------------------------------------------------------------
 */

#include "test_private.h"
#include "safe_str_lib.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <locale.h>

#define LEN   ( 128 )

/*#define PERL_TEST*/
/* Must have the same Unicode version 9.0, at least 5.26.
   Better 5.27.3 with Unicode 10.
   perl -MUnicode::UCD -e'print Unicode::UCD::UnicodeVersion()'
*/
#ifndef PERL
/*# define PERL "perl"*/
/*# define PERL "cperl5.27.2"*/
# define PERL   "perl5.27.3"
#endif
#define TESTPL  "test-fc.pl"

int main()
{
    errno_t rc;
    int ind;
    wchar_t str[LEN];
    wchar_t str1[LEN];
    size_t len;
    int errs = 0;
    char *lc_cat, *lang;
#ifdef PERL_TEST
    FILE *pl;
    struct stat st;

    pl = fopen(TESTPL, "w");
#endif

/*--------------------------------------------------*/

    rc = wcsfc_s(NULL, LEN, L"test");
    ERR(ESNULLP);

    rc = wcsfc_s(str, LEN, NULL);
    ERR(ESNULLP);

/*--------------------------------------------------*/

    rc = wcsfc_s(str, 0, L"test");
    ERR(ESZEROL)

/*--------------------------------------------------*/

    rc = wcsfc_s(str, 99999, L"test");
    ERR(ESLEMAX)

/*--------------------------------------------------*/

    rc = wcsfc_s(str, LEN, L"Name");
    ERR(EOK);
    WEXPSTR(str, L"name");
    WCHECK_SLACK(&str[4], LEN-4);

/*--------------------------------------------------*/

    rc = wcsfc_s(str, LEN, L"name");
    ERR(EOK)
    WEXPSTR(str, L"name");
    WCHECK_SLACK(&str[4], LEN-4);

/*--------------------------------------------------*/

    rc = wcsfc_s(str, LEN, L"NOWISTHETIM3");
    ERR(EOK)
    WEXPSTR(str, L"nowisthetim3");
    len = wcslen(L"nowisthetim3");
    WCHECK_SLACK(&str[len], LEN-len);

/*--------------------------------------------------*/

    /* towfc_s() */
    rc = wcsfc_s(str, LEN, L"A" L"\x1fb3"); /* casefold ᾳ => 2 */
    ERR(EOK);
    wcscpy(str1, L"a" L"\x3b1" L"\x3b9"); /* but decomp: 1fb3 => 3b1 345 */
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[3], LEN-3);

    rc = wcsfc_s(str, LEN, L"A" L"\x1fb7"); /* casefold ᾷ => 3 */
    ERR(EOK);
    wcscpy(str1, L"a" L"\x3b1" L"\x342" L"\x3b9");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[4], LEN-4);

    rc = wcsfc_s(str, LEN, L"\x100"); /* fc=>101, nfd=>61 304 */
    ERR(EOK);
    wcscpy(str1, L"\x61" L"\x304");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x101"); /* fc=101, nfd only */
    ERR(EOK);
    wcscpy(str1, L"\x61" L"\x304");   /* A w/ MACRON */
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x115"); /* fc=115, nfd=65 306 */
    ERR(EOK);
    wcscpy(str1, L"\x65" L"\x306");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\xdf"); /* fc=73 73, !nfd */
    ERR(EOK);
    wcscpy(str1, L"\x73" L"\x73");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x385"); /* fc=385, nfd=a8 301 */
    ERR(EOK);
    wcscpy(str1, L"\xa8" L"\x301");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x386"); /* fc=3ac, nfd=3b1 301 */
    ERR(EOK);
    wcscpy(str1, L"\x3b1" L"\x301");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x388"); /* c=0, fc=3ad, nfd=3b5 301 */
    ERR(EOK);
    wcscpy(str1, L"\x3b5" L"\x301");  /* 3ad: 249|TBL(2) */
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x1f71"); /* !fc, nfd=3b1 301 */
    ERR(EOK);
    wcscpy(str1, L"\x3b1" L"\x301");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x1f82"); /*  fc=1f02 389 => l=4 */
    ERR(EOK);
    wcscpy(str1, L"\x3b1" L"\x313" L"\x300" L"\x3b9");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[4], LEN-4);

    rc = wcsfc_s(str, LEN, L"\x1feb"); /*  fc,=17eb nfd=3c5 301 */
    ERR(EOK);
    wcscpy(str1, L"\x3c5" L"\x301");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x1fee"); /*  !fc, nfd=a8 301 */
    ERR(EOK);
    wcscpy(str1, L"\xa8" L"\x301");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

/*--------------------------------------------------*/

    /* wcsfc_s() locale-independent special rules */
    rc = wcsfc_s(str, LEN, L"\x3a3" L" "); /* final sigma */
    ERR(EOK);
    wcscpy(str1, L"\x3c2" L" ");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\x3a3" L"s"); /* non-final sigma */
    ERR(EOK);
    wcscpy(str1, L"\x3c3" L"s");
    WEXPSTR(str, str1);
    WCHECK_SLACK(&str[2], LEN-2);

/*--------------------------------------------------*/

    /* wcsfc_s() locale-dependent special rules */
    SETLOCALE_C;
    SETLANG("C");
    CHKLOCALE_C;

    rc = wcsfc_s(str, LEN, L"\x130"); /* I-dot İ */
    ERR(EOK);
    WEXPSTR(str, L"i\x307"); /* COMBINING DOT ABOVE */
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"I\x307");
    ERR(EOK);
    WEXPSTR(str, L"i\x307");
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"I");
    ERR(EOK);
    WEXPSTR(str, L"i");
    WCHECK_SLACK(&str[1], LEN-1);

    rc = wcsfc_s(str, LEN, L"\xc0"); /* accented a. fc => \xe0, nfd => a\x300 */
    ERR(EOK);
    WEXPSTR(str, L"a\x300");
    WCHECK_SLACK(&str[2], LEN-2);

    rc = wcsfc_s(str, LEN, L"\xcc"); /* accented I/J fc => \xec, nfd => i\x300 */
    ERR(EOK);
    WEXPSTR(str, L"i\x300");
    WCHECK_SLACK(&str[2], LEN-2);

    SETLOCALE("tr_TR");
    SETLANG("tr");
    CHKLOCALE("tr")
    {
        /* pair I-Dot with i for Turkish and Azeri. */
        rc = wcsfc_s(str, LEN, L"\x130"); /* I-dot İ */
        ERR(EOK);
        WEXPSTR(str, L"i"); /* i alone */
        WCHECK_SLACK(&str[1], LEN-1);

        /* Remove dot_above in the sequence
           I + dot_above, which will turn into i */
        rc = wcsfc_s(str, LEN, L"I\x307"); /* skip the I-dot */
        ERR(EOK);
        WEXPSTR(str, L"i"); /* ditto */
        WCHECK_SLACK(&str[1], LEN-1);

        rc = wcsfc_s(str, LEN, L"A\x307"); /* Not with A */
        ERR(EOK);
        wcscpy(str1, L"a" L"\x307");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);

        /* Unless an I is before a dot_above, it turns
           into a dotless i. */
        rc = wcsfc_s(str, LEN, L"I");
        ERR(EOK);
        WEXPSTR(str, L"\x131"); /* dotless ı */
        WCHECK_SLACK(&str[1], LEN-1);

        rc = wcsfc_s(str, LEN, L"I\x307" L"I");
        ERR(EOK);
        wcscpy(str1, L"i" L"\x131");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);
    }

    SETLOCALE("lt_LT");
    SETLANG("lt");
    CHKLOCALE("lt")
    {
        rc = wcsfc_s(str, LEN, L"\xcc"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x69" L"\x307" L"\x300");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[3], LEN-3);

        rc = wcsfc_s(str, LEN, L"\xcd"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x69" L"\x307" L"\x301");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[3], LEN-3);

        rc = wcsfc_s(str, LEN, L"\x128"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x69" L"\x307" L"\x303");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[3], LEN-3);

        rc = wcsfc_s(str, LEN, L"\x49" L"\x300"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x69" L"\x307");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);

        rc = wcsfc_s(str, LEN, L"\x4A" L"\x300"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x6a" L"\x307");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);

        rc = wcsfc_s(str, LEN, L"\x49" L"\x302"); /* not accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x69" L"\x302");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);

        rc = wcsfc_s(str, LEN, L"\x12e" L"\x301"); /* accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x12f" L"\x307");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);

        rc = wcsfc_s(str, LEN, L"\x12e" L"\x302"); /* not accented I/J */
        ERR(EOK);
        wcscpy(str1, L"\x12f" L"\x302");
        WEXPSTR(str, str1);
        WCHECK_SLACK(&str[2], LEN-2);
    }

    SETLOCALE_C;

/*--------------------------------------------------*/

    /* see if we can lower-case and decompose all */
#ifdef PERL_TEST
    fprintf_s(pl, "use v5.27.2;\nno warnings;\nuse Unicode::Normalize;\nmy $err;"
              "sub chk {\n"
              "  my ($ch, $got) = @_;\n"
              "  my $fc = fc($ch);\n"
              "  my $nfd = NFD($fc);\n"
              "  if ($nfd ne $got) {\n"
              "    printf \"Error NFD \\\\x{%%X} = %%s; fc = %%s; got: %%s\\n\",\n"
              "         unpack('W*',$ch),\n"
              "         join('',map{sprintf'\\x{%%X}',$_} unpack 'W*',$nfd),\n"
              "         join('',map{sprintf'\\x{%%X}',$_} unpack 'W*',$fc),\n"
              "         join('',map{sprintf'\\x{%%X}',$_} unpack 'W*',$got);\n"
              "    1\n"
              "#  } elsif ($fc ne $nfd) {\n"
              "#    printf \"  NFD \\\\x{%%X} = %%s; fc = %%s;\\n\",\n"
              "#         unpack('W*',$ch),\n"
              "#         join('',map{sprintf'\\x{%%X}',$_} unpack 'W*',$nfd),\n"
              "#         join('',map{sprintf'\\x{%%X}',$_} unpack 'W*',$fc);\n"
              "  }\n"
              "}\n");
#endif
    for (ind=0xc0; ind<0x02fa20; ind++) {
        static wchar_t src[5];
        int c;
        src[0] = ind;
        src[1] = 0;
        c = iswfc(ind);
        /*debug_printf("%s %u  U+%04X\n", __FUNCTION__, __LINE__, (int)ind);*/
        rc = wcsfc_s(str, 5, src);
        if (!c) {
            ERR(EOK);
        } else if (rc) { /* 409 is not found, a iswfc() mismatch */
            debug_printf("%s %u  Error %d U+%4X L\"%ls\" => L\"%ls\"\n",
                         __FUNCTION__, __LINE__, (int)rc, (int)ind, src, str);
            errs++;
        }
#ifdef PERL_TEST
        {
            int len = wcslen(str);
            int i;
            /* cross-check with perl */
            fprintf_s(pl, "$err += chk(\"\\N{U+%04X}\",\"\\N{U+%04X}", ind, str[0]);
            for (i=1;i<len;i++) {
                fprintf_s(pl, "\\N{U+%04X}", str[i]);
            }
            fprintf_s(pl, "\");\n");
        }
#endif
    }

/*--------------------------------------------------*/

#ifdef PERL_TEST
    fprintf_s(pl, "exit $err;\n");
#ifdef BSD_ALL_LIKE
    fstat(pl->_file, &st);
    fclose(pl);
#elif defined __GLIBC__
    fstat(pl->_fileno, &st);
    fclose(pl);
#else
    fclose(pl);
    stat(TESTPL, &st);
#endif
    if (st.st_size) {
        printf("Cross check with " PERL ":\n");
        fflush(stdout);
        if (system(PERL " " TESTPL) < 0) {
            printf("Redo with perl (probably wrong Unicode version):\n");
            fflush(stdout);
            system("perl " TESTPL);
        }
    }
#ifndef DEBUG
    unlink(TESTPL);
#endif

#endif /* PERL_TEST */

/*--------------------------------------------------*/

    return (errs);
}
