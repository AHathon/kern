#include "libraries/libc/stdio.h"
#include "libraries/libc/stdlib.h"
#include "libraries/libc/string.h"
#include "libraries/libc/math.h"
#include "libraries/services/svc.h"

#define FLAG_LEFT   (1 << 0)
#define FLAG_PLUS   (1 << 1)
#define FLAG_SPACE  (1 << 2)
#define FLAG_ZERO   (1 << 3)
#define FLAG_ALT    (1 << 4)

typedef struct
{
    char *buf;
    size_t cap;
    size_t len;
} Out;

static void Out_Char(Out *o, char c)
{
    if(o->len + 1 < o->cap)
        o->buf[o->len] = c;
    o->len++;
}

static void Out_Pad(Out *o, char c, int n)
{
    while(n-- > 0)
        Out_Char(o, c);
}

static void Out_Field(Out *o, char sign, const char *body, int len, int width, int flags)
{
    int total = len + (sign ? 1 : 0);
    if(!(flags & (FLAG_LEFT | FLAG_ZERO)))
        Out_Pad(o, ' ', width - total);
    if(sign)
        Out_Char(o, sign);
    if(flags & FLAG_ZERO && !(flags & FLAG_LEFT))
        Out_Pad(o, '0', width - total);
    for(int i = 0; i < len; i++)
        Out_Char(o, body[i]);
    if(flags & FLAG_LEFT)
        Out_Pad(o, ' ', width - total);
}

static char Sign(int neg, int flags)
{
    if(neg)
        return '-';
    if(flags & FLAG_PLUS)
        return '+';
    return flags & FLAG_SPACE ? ' ' : 0;
}

static void Out_Int(Out *o, unsigned long long v, int neg, int base, int upper, int width, int prec, int flags)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    int n = 0;
    do
    {
        tmp[n++] = digits[v % base];
        v /= base;
    } while(v);

    char body[64];
    int len = 0;
    for(int i = n; i < prec && len < 40; i++)
        body[len++] = '0';
    while(n)
        body[len++] = tmp[--n];

    if(prec >= 0)
        flags &= ~FLAG_ZERO;
    Out_Field(o, Sign(neg, flags), body, len, width, flags);
}

static int FormatFixed(char *out, int cap, double v, int prec)
{
    double round = 0.5;
    for(int i = 0; i < prec; i++)
        round /= 10;
    v += round;

    double ip = floor(v);
    double frac = v - ip;
    int len = 0;

    if(ip >= 9.0e18)
    {
        out[len++] = 'L';
        out[len++] = 'R';
        out[len++] = 'G';
        return len;
    }

    char tmp[24];
    int n = 0;
    unsigned long long whole = (unsigned long long)ip;
    do
    {
        tmp[n++] = '0' + whole % 10;
        whole /= 10;
    } while(whole);
    while(n)
        out[len++] = tmp[--n];

    if(prec > 0)
    {
        out[len++] = '.';
        for(int i = 0; i < prec && len < cap - 1; i++)
        {
            frac *= 10;
            int d = (int)frac;
            out[len++] = '0' + d;
            frac -= d;
        }
    }
    return len;
}

static int Exponent10(double v)
{
    if(v == 0)
        return 0;
    return (int)floor(log(v) / log(10.0) + 1e-12);
}

static int FormatExp(char *out, int cap, double v, int prec, char e)
{
    int ex = Exponent10(v);
    double m = v / pow(10.0, ex);
    if(m >= 10)
    {
        m /= 10;
        ex++;
    }
    else if(m > 0 && m < 1)
    {
        m *= 10;
        ex--;
    }

    int len = FormatFixed(out, cap - 6, m, prec);
    out[len++] = e;
    out[len++] = ex < 0 ? '-' : '+';
    int a = ex < 0 ? -ex : ex;
    if(a >= 100)
        out[len++] = '0' + a / 100;
    out[len++] = '0' + (a / 10) % 10;
    out[len++] = '0' + a % 10;
    return len;
}

static void Out_Float(Out *o, double v, char conv, int width, int prec, int flags)
{
    char body[96];
    int len;
    int neg = v < 0 || (v == 0 && 1.0 / v < 0);
    if(neg)
        v = -v;

    if(v != v)
    {
        Out_Field(o, 0, "nan", 3, width, flags & ~FLAG_ZERO);
        return;
    }
    if(v > 1.7976931348623157e308)
    {
        Out_Field(o, Sign(neg, flags), "inf", 3, width, flags & ~FLAG_ZERO);
        return;
    }

    if(prec < 0)
        prec = 6;
    if(prec > 40)
        prec = 40;

    if(conv == 'f' || conv == 'F')
        len = FormatFixed(body, sizeof(body), v, prec);
    else if(conv == 'e' || conv == 'E')
        len = FormatExp(body, sizeof(body), v, prec, conv);
    else
    {
        int p = prec ? prec : 1;
        int ex = Exponent10(v);
        if(ex >= -4 && ex < p)
            len = FormatFixed(body, sizeof(body), v, p - 1 - ex);
        else
            len = FormatExp(body, sizeof(body), v, p - 1, conv == 'G' ? 'E' : 'e');

        if(!(flags & FLAG_ALT) && memchr(body, '.', len))
        {
            char *e = memchr(body, conv == 'G' ? 'E' : 'e', len);
            int end = e ? (int)(e - body) : len;
            int cut = end;
            while(body[cut - 1] == '0')
                cut--;
            if(body[cut - 1] == '.')
                cut--;
            memmove(body + cut, body + end, len - end);
            len -= end - cut;
        }
    }

    Out_Field(o, Sign(neg, flags), body, len, width, flags);
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    Out o = { buf, size, 0 };

    for(; *fmt; fmt++)
    {
        if(*fmt != '%')
        {
            Out_Char(&o, *fmt);
            continue;
        }
        fmt++;

        int flags = 0;
        for(;; fmt++)
        {
            if(*fmt == '-') flags |= FLAG_LEFT;
            else if(*fmt == '+') flags |= FLAG_PLUS;
            else if(*fmt == ' ') flags |= FLAG_SPACE;
            else if(*fmt == '0') flags |= FLAG_ZERO;
            else if(*fmt == '#') flags |= FLAG_ALT;
            else break;
        }

        int width = 0;
        if(*fmt == '*')
        {
            width = va_arg(args, int);
            if(width < 0)
            {
                flags |= FLAG_LEFT;
                width = -width;
            }
            fmt++;
        }
        else
            for(; *fmt >= '0' && *fmt <= '9'; fmt++)
                width = width * 10 + (*fmt - '0');

        int prec = -1;
        if(*fmt == '.')
        {
            fmt++;
            prec = 0;
            if(*fmt == '*')
            {
                prec = va_arg(args, int);
                fmt++;
            }
            else
                for(; *fmt >= '0' && *fmt <= '9'; fmt++)
                    prec = prec * 10 + (*fmt - '0');
        }

        int longs = 0;
        for(; *fmt == 'l' || *fmt == 'h' || *fmt == 'z' || *fmt == 't' || *fmt == 'j'; fmt++)
            if(*fmt != 'h')
                longs = 1;

        switch(*fmt)
        {
            case 'd':
            case 'i':
            {
                long long v = longs ? va_arg(args, long long) : va_arg(args, int);
                unsigned long long u = v < 0 ? -(unsigned long long)v : (unsigned long long)v;
                Out_Int(&o, u, v < 0, 10, 0, width, prec, flags);
                break;
            }
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            {
                unsigned long long v = longs ? va_arg(args, unsigned long long) : va_arg(args, unsigned int);
                int base = *fmt == 'u' ? 10 : *fmt == 'o' ? 8 : 16;
                Out_Int(&o, v, 0, base, *fmt == 'X', width, prec, flags);
                break;
            }
            case 'p':
                Out_Int(&o, (uintptr_t)va_arg(args, void*), 0, 16, 0, width, prec, flags);
                break;
            case 'c':
            {
                char c = (char)va_arg(args, int);
                Out_Field(&o, 0, &c, 1, width, flags & ~FLAG_ZERO);
                break;
            }
            case 's':
            {
                const char *s = va_arg(args, const char*);
                if(!s)
                    s = "(null)";
                int len = 0;
                while((prec < 0 || len < prec) && s[len])
                    len++;
                Out_Field(&o, 0, s, len, width, flags & ~FLAG_ZERO);
                break;
            }
            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                Out_Float(&o, va_arg(args, double), *fmt, width, prec, flags);
                break;
            case '%':
                Out_Char(&o, '%');
                break;
            case 0:
                fmt--;
                break;
            default:
                Out_Char(&o, *fmt);
                break;
        }
    }

    if(size)
        buf[o.len < size ? o.len : size - 1] = 0;
    return (int)o.len;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return r;
}

int printf(const char *fmt, ...)
{
    char buf[100];
    va_list args;
    va_start(args, fmt);
    int r = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    svcDebug(buf);
    return r;
}

static const char *ParseDouble(const char *s, double *out)
{
    const char *start = s;
    int neg = *s == '-';
    if(*s == '-' || *s == '+')
        s++;

    double v = 0;
    int digits = 0;
    for(; *s >= '0' && *s <= '9'; s++, digits++)
        v = v * 10 + (*s - '0');

    if(*s == '.')
    {
        double scale = 0.1;
        for(s++; *s >= '0' && *s <= '9'; s++, digits++, scale *= 0.1)
            v += (*s - '0') * scale;
    }

    if(!digits)
        return start;

    if(*s == 'e' || *s == 'E')
    {
        const char *p = s + 1;
        int eneg = *p == '-';
        if(*p == '-' || *p == '+')
            p++;
        if(*p >= '0' && *p <= '9')
        {
            int e = 0;
            for(; *p >= '0' && *p <= '9'; p++)
                e = e * 10 + (*p - '0');
            v *= pow(10.0, eneg ? -e : e);
            s = p;
        }
    }

    *out = neg ? -v : v;
    return s;
}

double atof(const char *s)
{
    double v = 0;
    while(*s == ' ' || *s == '\t')
        s++;
    ParseDouble(s, &v);
    return v;
}

static int IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int DigitValue(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 99;
}

int sscanf(const char *str, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const char *s = str;
    int count = 0;

    for(; *fmt; fmt++)
    {
        if(IsSpace(*fmt))
        {
            while(IsSpace(*s))
                s++;
            continue;
        }
        if(*fmt != '%')
        {
            if(*s != *fmt)
                break;
            s++;
            continue;
        }
        fmt++;

        int skip = *fmt == '*';
        if(skip)
            fmt++;

        int width = 0;
        for(; *fmt >= '0' && *fmt <= '9'; fmt++)
            width = width * 10 + (*fmt - '0');

        int size = 0;
        for(; *fmt == 'l' || *fmt == 'h' || *fmt == 'z'; fmt++)
            size += *fmt == 'h' ? -1 : 1;

        if(*fmt == 'n')
        {
            *va_arg(args, int*) = (int)(s - str);
            continue;
        }
        if(*fmt != 'c')
            while(IsSpace(*s))
                s++;

        if(*fmt == 'd' || *fmt == 'i' || *fmt == 'u' || *fmt == 'x' || *fmt == 'X')
        {
            int base = (*fmt == 'x' || *fmt == 'X') ? 16 : 10;
            const char *p = s;
            int neg = *p == '-';
            if(*p == '-' || *p == '+')
                p++;

            unsigned long long v = 0;
            const char *digits = p;
            while(DigitValue(*p) < base && (!width || p - s < width))
                v = v * base + DigitValue(*p++);
            if(p == digits)
                break;
            s = p;

            if(!skip)
            {
                if(neg)
                    v = -v;
                if(size >= 1) *va_arg(args, unsigned long long*) = v;
                else if(size == -1) *va_arg(args, unsigned short*) = (unsigned short)v;
                else if(size <= -2) *va_arg(args, unsigned char*) = (unsigned char)v;
                else *va_arg(args, unsigned int*) = (unsigned int)v;
                count++;
            }
        }
        else if(*fmt == 'f' || *fmt == 'g' || *fmt == 'e')
        {
            double v;
            const char *p = ParseDouble(s, &v);
            if(p == s)
                break;
            s = p;

            if(!skip)
            {
                if(size >= 1) *va_arg(args, double*) = v;
                else *va_arg(args, float*) = (float)v;
                count++;
            }
        }
        else if(*fmt == 's')
        {
            char *out = skip ? 0 : va_arg(args, char*);
            int n = 0;
            while(*s && !IsSpace(*s) && (!width || n < width))
            {
                if(out)
                    out[n] = *s;
                n++;
                s++;
            }
            if(!n)
                break;
            if(out)
            {
                out[n] = 0;
                count++;
            }
        }
        else if(*fmt == 'c')
        {
            if(!*s)
                break;
            if(!skip)
            {
                *va_arg(args, char*) = *s;
                count++;
            }
            s++;
        }
        else
            break;
    }

    va_end(args);
    return count;
}
