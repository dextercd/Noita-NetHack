/* NetHack 3.7	tclib.c	$NHDT-Date: 1596498287 2020/08/03 23:44:47 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.10 $ */
/* Copyright (c) Robert Patrick Rankin, 1995                      */
/* NetHack may be freely redistributed.  See license for details. */

/* termcap library implementation */

#include "config.h"

#ifndef TERMCAP /* name of default termcap file */
#define TERMCAP "/etc/termcap"
#endif
#ifndef TCBUFSIZ /* size of tgetent buffer; Unix man page says 1024 */
#define TCBUFSIZ 1024
#endif
#define ESC '\033' /* termcap's '\E' */
#define BEL '\007' /* ANSI C's '\a' (we assume ASCII here...) */

/* exported variables, as per man page */
char PC;
char *BC, *UP;
short ospeed;

/* exported routines */
int tgetent(char *, const char *);
int tgetflag(const char *);
int tgetnum(const char *);
char *tgetstr(const char *, char **);
char *tgoto(const char *, int, int);
char *tparam(const char *, char *, int, int, int, int, int);
void tputs(const char *, int, int (*)(int));

/* local support data */
static char tc_entry[] =
":am:bs:km:mi:ms:xn:"
":co#90:it#8:li#30:"
":AL=\\E[%dL:DC=\\E[%dP:DL=\\E[%dM:DO=\\E[%dB:IC=\\E[%d@:"
":K1=\\EOw:K2=\\EOy:K3=\\EOu:K4=\\EOq:K5=\\EOs:LE=\\E[%dD:"
":RI=\\E[%dC:UP=\\E[%dA:ae=^O:al=\\E[L:as=^N:bl=^G:bt=\\E[Z:"
":cd=\\E[J:ce=\\E[K:cl=\\E[H\\E[2J:cm=\\E[%i%d;%dH:cr=^M:"
":cs=\\E[%i%d;%dr:ct=\\E[3g:dc=\\E[P:dl=\\E[M:do=^J:ec=\\E[%dX:"
":ei=\\E[4l:ho=\\E[H:ic=\\E[@:im=\\E[4h:"
":is=\\E7\\E[r\\E[m\\E[?7h\\E[?1;3;4;6l\\E[4l\\E8\\E>:"
":k1=\\E[11~:k2=\\E[12~:k3=\\E[13~:k4=\\E[14~:k5=\\E[15~:"
":k6=\\E[17~:k7=\\E[18~:k8=\\E[19~:k9=\\E[20~:kD=\\177:kI=\\E[2~:"
":kN=\\E[6~:kP=\\E[5~:kb=^H:kd=\\EOB:ke=\\E[?1l\\E>:kh=\\EOH:"
":kl=\\EOD:kr=\\EOC:ks=\\E[?1h\\E=:ku=\\EOA:le=^H:md=\\E[1m:"
":me=\\E[m\\017:mr=\\E[7m:nd=\\E[C:rc=\\E8:sc=\\E7:se=\\E[27m:"
":sf=^J:so=\\E[7m:sr=\\EM:st=\\EH:ta=^I:te=\\E[2J\\E[?47l\\E8:"
":ti=\\E7\\E[?47h:ue=\\E[24m:up=\\E[A:us=\\E[4m:"
":vb=\\E[?5h\\E[?5l:ve=\\E[?25h:vi=\\E[?25l:vs=\\E[?25h:"
":k1=\\EOP:k2=\\EOQ:k3=\\EOR:k4=\\EOS:"
":5i:"
":*6@:@0@:@7=\\E[4~:ei=:ic@:im=:is=\\E[\\041p\\E[?3;4l\\E[4l\\E>:"
":kD=\\E[3~:kh=\\E[1~:mb=\\E[5m:mk=\\E[8m:pf=\\E[4i:po=\\E[5i:"
":ps=\\E[i:r1=\\Ec:r2=\\E[\\041p\\E[?3;4l\\E[4l\\E>:"
":..sa=\\E[0%?%p1%p6%|%t;1%;%?%p2%t;4%;%?%p1%p3%|%t;7%;%?%p4%t;5%;%?%p7%t;8%;m%?%p9%t\\016%e\\017%;:"
":te=\\E[?1047l\\E[?1048l:ti=\\E[?1048h\\E[?1047h:"
":@7=\\EOF:K1=\\EOH:K2=\\EOE:K3=\\E[5~:K4=\\EOF:K5=\\E[6~:kD=\\177:"
":kh=\\EOH:te=\\E[?1049l:ti=\\E[?1049h:"
":Co#16:NC#32:pa#256:"
":AB=\\E[4%p1%dm:AF=\\E[3%p1%dm:"
":..Sb=%p1%{8}%/%{6}%*%{4}%+\\E[%d%p1%{8}%m%Pa%?%ga%{1}%=%t4%e%ga%{3}%=%t6%e%ga%{4}%=%t1%e%ga%{6}%=%t3%e%ga%d%;m:"
":..Sf=%p1%{8}%/%{6}%*%{3}%+\\E[%d%p1%{8}%m%Pa%?%ga%{1}%=%t4%e%ga%{3}%=%t6%e%ga%{4}%=%t1%e%ga%{6}%=%t3%e%ga%d%;m:";

static char bc_up_buf[24];
#ifndef NO_DELAY_PADDING
/* `ospeed' to baud rate conversion table, adapted from GNU termcap-1.2 */
static short baud_rates[] = {
    0,    50,   75,    110,  135,  150,
#ifdef VMS
    300,  600,  1200,  1800, 2000, 2400, 3600, 4800, 7200,
#else /* assume Unix */
    200,  300,  600,  1200, 1800, 2400, 4800,
#endif
    9600, -192, -384, /* negative is used as `100 * abs(entry)' */
#ifdef VMS
    -576, -768, -1152,
#endif
};
#endif /* !NO_DELAY_PADDING */

/* local support code */
static int tc_store(const char *, const char *);
static char *tc_find(FILE *, const char *, char *, int);
static char *tc_name(const char *, char *);
static const char *tc_field(const char *, const char **);

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

/* search for an entry in the termcap file */
static char *
tc_find(FILE *fp, const char *term, char *buffer, int bufsiz)
{
    int in, len, first, skip;
    char *ip, *op, *tc_fetch, tcbuf[TCBUFSIZ];

    buffer[0] = '\0';
    do {
        ip = tcbuf, in = min(bufsiz, TCBUFSIZ);
        first = 1, skip = 0;
        /* load entire next entry, including any continuations */
        do {
            if (!fgets(ip, min(in, BUFSIZ), fp))
                break;
            if (first)
                skip = (*ip == '#'), first = 0;
            len = (int) strlen(ip);
            if (!skip && len > 1 && *(ip + len - 1) == '\n'
                && *(ip + len - 2) == '\\')
                len -= 2;
            ip += len, in -= len;
        } while (*(ip - 1) != '\n' && in > 0);
        if (ferror(fp) || ip == buffer || *(ip - 1) != '\n')
            return (char *) 0;
        *--ip = '\0'; /* strip newline */
        if (!skip)
            ip = tc_name(term, tcbuf);
    } while (skip || !ip);

    /* we have the desired entry; strip cruft and look for :tc=other: */
    tc_fetch = 0;
    for (op = buffer; *ip; ip++) {
        if (op == buffer || *(op - 1) != ':'
            || (*ip != ' ' && *ip != '\t' && *ip != ':'))
            *op++ = *ip, bufsiz -= 1;
        if (ip[0] == ':' && ip[1] == 't' && ip[2] == 'c' && ip[3] == '=') {
            tc_fetch = &ip[4];
            if ((ip = strchr(tc_fetch, ':')) != 0)
                *ip = '\0';
            break;
        }
    }
    *op = '\0';

    if (tc_fetch) {
        rewind(fp);
        tc_fetch = tc_find(fp, tc_fetch, tcbuf, min(bufsiz, TCBUFSIZ));
        if (!tc_fetch)
            return (char *) 0;
        if (op > buffer && *(op - 1) == ':' && *tc_fetch == ':')
            ++tc_fetch;
        strcpy(op, tc_fetch);
    }
    return buffer;
}

/* check whether `ent' contains `nam'; return start of field entries */
static char *
tc_name(const char *nam, char *ent)
{
    char *nxt, *lst, *p = ent;
    size_t n = strlen(nam);

    if ((lst = strchr(p, ':')) == 0)
        lst = p + strlen(p);

    while (p < lst) {
        if ((nxt = strchr(p, '|')) == 0 || nxt > lst)
            nxt = lst;
        if ((long) (nxt - p) == (long) n && strncmp(p, nam, n) == 0)
            return lst;
        p = nxt + 1;
    }
    return (char *) 0;
}

/* look up a numeric entry */
int
tgetnum(const char *which)
{
    const char *q, *p = tc_field(which, &q);
    char numbuf[32];
    size_t n;

    if (!p || p[2] != '#')
        return -1;
    p += 3;
    if ((n = (size_t)(q - p)) >= sizeof numbuf)
        return -1;
    (void) strncpy(numbuf, p, n);
    numbuf[n] = '\0';
    return atoi(numbuf);
}

/* look up a boolean entry */
int
tgetflag(const char *which)
{
    const char *p = tc_field(which, (const char **) 0);

    return (!p || p[2] != ':') ? 0 : 1;
}

static char dummy_out[100];

/* look up a string entry; update `*outptr' */
char *
tgetstr(const char *which, char **outptr)
{
    int n;
    char c, *r, *result;
    const char *q, *p = tc_field(which, &q);

    if (outptr == NULL || *outptr == NULL) {
        char* pdummy_out = dummy_out;
        outptr = &pdummy_out;
    }

    if (!p || p[2] != '=')
        return (char *) 0;
    p += 3;
    if ((q = strchr(p, ':')) == 0)
        q = p + strlen(p);
    r = result = *outptr;
    while (p < q) {
        switch ((*r = *p++)) {
        case '\\':
            switch ((c = *p++)) {
            case 'E':
                *r = ESC;
                break;
            case 'a':
                *r = BEL;
                break;
            case 'b':
                *r = '\b';
                break;
            case 'f':
                *r = '\f';
                break;
            case 'n':
                *r = '\n';
                break;
            case 'r':
                *r = '\r';
                break;
            case 't':
                *r = '\t';
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                n = c - '0';
                if (*p >= '0' && *p <= '7')
                    n = 8 * n + (*p++ - '0');
                if (*p >= '0' && *p <= '7')
                    n = 8 * n + (*p++ - '0');
                *r = (char) n;
                break;
            /* case '^': case '\\': */
            default:
                *r = c;
                break;
            }
            break;
        case '^':
            *r = (*p++ & 037);
            if (!*r)
                *r = (char) '\200';
            break;
        default:
            break;
        }
        ++r;
    }
    *r++ = '\0';
    *outptr = r;
    return result;
}

/* look for a particular field name */
static const char *
tc_field(const char *field, const char **tc_end)
{
    const char *end, *q, *p = tc_entry;

    end = p + strlen(p);
    while (p < end) {
        if ((p = strchr(p, ':')) == 0)
            break;
        ++p;
        if (p[0] == field[0] && p[1] == field[1]
            && (p[2] == ':' || p[2] == '=' || p[2] == '#' || p[2] == '@'))
            break;
    }
    if (tc_end) {
        if (p) {
            if ((q = strchr(p + 2, ':')) == 0)
                q = end;
        } else
            q = 0;
        *tc_end = q;
    }
    return p;
}

static char cmbuf[64];

/* produce a string which will position the cursor at <row,col> if output */
char *
tgoto(const char *cm, int col, int row)
{
    return tparam(cm, cmbuf, (int) (sizeof cmbuf), row, col, 0, 0);
}

/* format a parameterized string, ala sprintf */
char *
tparam(const char *ctl, /* parameter control string */
       char *buf,       /* output buffer */
       int buflen,      /* ought to have been `size_t'... */
       int row, int col, int row2, int col2)
{
    int atmp, ac, av[5];
    char c, *r, *z, *bufend, numbuf[32];
    const char *fmt;
#ifndef NO_SPECIAL_CHARS_FIXUP
    int bc = 0, up = 0;
#endif

    av[0] = row, av[1] = col, av[2] = row2, av[3] = col2, av[4] = 0;
    ac = 0;
    r = buf, bufend = r + buflen - 1;
    while (*ctl) {
        if ((*r = *ctl++) == '%') {
            if (ac > 4)
                ac = 4;
            fmt = 0;
            switch ((c = *ctl++)) {
            case '%':
                break; /* '%' already copied */
            case 'd':
                fmt = "%d";
                break;
            case '2':
                fmt = "%02d";
                break;
            case '3':
                fmt = "%03d";
                break;
            case '+': /*FALLTHRU*/
            case '.':
                *r = (char) av[ac++];
                if (c == '+')
                    *r += *ctl++;
                if (!*r) {
                    *r = (char) '\200';
                } else {
#ifndef NO_SPECIAL_CHARS_FIXUP
                    /* avoid terminal driver intervention for
                       various control characters, to prevent
                       LF from becoming CR+LF, for instance; only
                       makes sense if this is a cursor positioning
                       sequence, but we have no way to check that */
                    while (strchr("\004\t\n\013\f\r", *r)) {
                        if (ac & 1) { /* row */
                            if (!UP || !*UP)
                                break; /* can't fix */
                            ++up;      /* incr row now, later move up */
                        } else {       /* column */
                            if (!BC || !*BC)
                                break; /* can't fix */
                            ++bc;      /* incr column, later backspace */
                        }
                        (*r)++;
                    }
#endif /* !NO_SPECIAL_CHARS_FIXUP */
                }
                break;
            case '>':
                if (av[ac] > (*ctl++ & 0377))
                    av[ac] += *ctl;
                ++ctl;
                break;
            case 'r':
                atmp = av[0];
                av[0] = av[1];
                av[1] = atmp;
                atmp = av[2];
                av[2] = av[3];
                av[3] = atmp;
                --r;
                break;
            case 'i':
                ++av[0];
                ++av[1];
                ++av[2];
                ++av[3];
                --r;
                break;
            case 'n':
                av[0] ^= 0140;
                av[1] ^= 0140;
                av[2] ^= 0140;
                av[3] ^= 0140;
                --r;
                break;
            case 'B':
                av[0] = ((av[0] / 10) << 4) + (av[0] % 10);
                av[1] = ((av[1] / 10) << 4) + (av[1] % 10);
                av[2] = ((av[2] / 10) << 4) + (av[2] % 10);
                av[3] = ((av[3] / 10) << 4) + (av[3] % 10);
                --r;
                break;
            case 'D':
                av[0] -= (av[0] & 15) << 1;
                av[1] -= (av[1] & 15) << 1;
                av[2] -= (av[2] & 15) << 1;
                av[3] -= (av[3] & 15) << 1;
                --r;
                break;
            default:
                *++r = c;
                break; /* erroneous entry... */
            }
            if (fmt) {
                (void) sprintf(numbuf, fmt, av[ac++]);
                for (z = numbuf; *z && r <= bufend; z++)
                    *r++ = *z;
                --r; /* will be re-incremented below */
            }
        }
        if (++r > bufend)
            return (char *) 0;
    }
#ifndef NO_SPECIAL_CHARS_FIXUP
    if (bc || up) {
        while (--bc >= 0)
            for (z = BC; *z && r <= bufend; z++)
                *r++ = *z;
        while (--up >= 0)
            for (z = UP; *z && r <= bufend; z++)
                *r++ = *z;
        if (r > bufend)
            return (char *) 0;
    }
#endif /* !NO_SPECIAL_CHARS_FIXUP */
    *r = '\0';
    return buf;
}

/* send a string to the terminal, possibly padded with trailing NULs */
void
tputs(const char *string, /* characters to output */
      int range,          /* number of lines affected, used for `*' delays */
      int (*output_func)(int)) /* actual output routine;
                                  * return value ignored */
{
    register int c, num = 0;
    register const char *p = string;

    if (!p || !*p)
        return;

    /* pick out padding prefix, if any */
    if (*p >= '0' && *p <= '9') {
        do { /* note: scale `num' by 10 to accommodate fraction */
            num += (*p++ - '0'), num *= 10;
        } while (*p >= '0' && *p <= '9');
        if (*p == '.')
            ++p, num += (*p >= '0' && *p <= '9') ? (*p++ - '0') : 0;
        if (*p == '*')
            ++p, num *= range;
    }

    /* output the string */
    while ((c = *p++) != '\0') {
        if (c == '\200')
            c = '\0'; /* undo tgetstr's encoding */
        (void) (*output_func)(c);
    }

#ifndef NO_DELAY_PADDING
    /* perform padding */
    if (num) {
        long pad;

        /* figure out how many chars needed to produce desired elapsed time */
        pad = (long) baud_rates[ospeed];
        if (pad < 0)
            pad *= -100L;
        pad *= (long) num;
        /* 100000 == 10 bits/char * (1000 millisec/sec scaled by 10) */
        num = (int) (pad / 100000L); /* number of characters */

        c = PC; /* assume output_func isn't allowed to change PC */
        while (--num >= 0)
            (void) (*output_func)(c);
    }
#endif /* !NO_DELAY_PADDING */

    return;
}

/*tclib.c*/
