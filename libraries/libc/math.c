#include "libraries/libc/math.h"

#define PI      3.14159265358979323846
#define LN2     0.69314718055994530942

double sqrt(double x)
{
    asm volatile("fsqrt %d0, %d1" : "=w"(x) : "w"(x));
    return x;
}

double fabs(double x)
{
    return x < 0 ? -x : x;
}

double floor(double x)
{
    asm volatile("frintm %d0, %d1" : "=w"(x) : "w"(x));
    return x;
}

double ceil(double x)
{
    asm volatile("frintp %d0, %d1" : "=w"(x) : "w"(x));
    return x;
}

double fmod(double x, double y)
{
    if(y == 0)
        return 0;
    double q = x / y;
    double t;
    asm volatile("frintz %d0, %d1" : "=w"(t) : "w"(q));
    return x - t * y;
}

double sin(double x)
{
    x = fmod(x, 2 * PI);
    if(x > PI)
        x -= 2 * PI;
    else if(x < -PI)
        x += 2 * PI;

    if(x > PI / 2)
        x = PI - x;
    else if(x < -PI / 2)
        x = -PI - x;

    double x2 = x * x;
    double term = x;
    double sum = x;
    for(int i = 1; i <= 8; i++)
    {
        term *= -x2 / ((2 * i) * (2 * i + 1));
        sum += term;
    }
    return sum;
}

double cos(double x)
{
    return sin(x + PI / 2);
}

static double atan_(double x)
{
    if(x < 0)
        return -atan_(-x);
    if(x > 1)
        return PI / 2 - atan_(1 / x);

    double t = x / (1 + sqrt(1 + x * x));
    t = t / (1 + sqrt(1 + t * t));

    double t2 = t * t;
    double term = t;
    double sum = t;
    for(int i = 1; i <= 8; i++)
    {
        term *= -t2;
        sum += term / (2 * i + 1);
    }
    return 4 * sum;
}

double atan2(double y, double x)
{
    if(x > 0)
        return atan_(y / x);
    if(x < 0)
        return y >= 0 ? atan_(y / x) + PI : atan_(y / x) - PI;
    if(y > 0)
        return PI / 2;
    return y < 0 ? -PI / 2 : 0;
}

double acos(double x)
{
    if(x >= 1)
        return 0;
    if(x <= -1)
        return PI;
    return atan2(sqrt(1 - x * x), x);
}

double exp(double x)
{
    if(x > 709)
        return 1.0 / 0.0;
    if(x < -708)
        return 0;

    int k = (int)floor(x / LN2 + 0.5);
    double r = x - k * LN2;

    double term = 1;
    double sum = 1;
    for(int i = 1; i <= 13; i++)
    {
        term *= r / i;
        sum += term;
    }

    double scale = 1;
    double base = k < 0 ? 0.5 : 2;
    for(int i = k < 0 ? -k : k; i > 0; i--)
        scale *= base;
    return sum * scale;
}

double log(double x)
{
    if(x <= 0)
        return -1.0 / 0.0;

    int e = 0;
    while(x >= 1.41421356237)
    {
        x *= 0.5;
        e++;
    }
    while(x < 0.70710678118)
    {
        x *= 2;
        e--;
    }

    double s = (x - 1) / (x + 1);
    double s2 = s * s;
    double term = s;
    double sum = s;
    for(int i = 1; i <= 10; i++)
    {
        term *= s2;
        sum += term / (2 * i + 1);
    }
    return 2 * sum + e * LN2;
}

double pow(double x, double y)
{
    if(y == 0)
        return 1;
    if(x == 0)
        return 0;
    if(x > 0)
        return exp(y * log(x));

    double r = exp(y * log(-x));
    return fmod(floor(y), 2) != 0 ? -r : r;
}

float sqrtf(float x) { return (float)sqrt(x); }
float fabsf(float x) { return x < 0 ? -x : x; }
float floorf(float x) { return (float)floor(x); }
float ceilf(float x) { return (float)ceil(x); }
float fmodf(float x, float y) { return (float)fmod(x, y); }
float sinf(float x) { return (float)sin(x); }
float cosf(float x) { return (float)cos(x); }
float acosf(float x) { return (float)acos(x); }
float atan2f(float y, float x) { return (float)atan2(y, x); }
float logf(float x) { return (float)log(x); }
float powf(float x, float y) { return (float)pow(x, y); }
