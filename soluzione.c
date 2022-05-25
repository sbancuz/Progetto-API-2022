#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

bool contains(char * str, size_t len, char needle)
{
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == needle)
            return true;
    }

    return false;
}

size_t count(char * str, size_t len, char needle)
{
    size_t cont = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == needle)
            cont++;
    }

    return cont;
}

size_t countCorr(char * str, char * str2, size_t len, char needle)
{
    size_t cont = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == needle && str[i] == str2[i])
            cont++;
    }

    return cont;
}

size_t countScorr(char * str, char * str2, size_t len, char needle)
{
    size_t cont = 0;

    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == needle && str[i] != str2[i])
        {
            cont++;
        }
    }

    return cont;
}

int main()
{
    char * r = "abcabcabcabcabc";
    char * p = "bbaabccbccbcabc";
    size_t len = strlen(r);
    char * res = malloc(len * sizeof(char));

    for (size_t i = 0; i < len; ++i)
    {
        if (r[i] == p[i])
        {
            res[i] = '+';
        }
        else if (!contains(p, len, r[i]))
            res[i] = '/';
        else
        {
            size_t n = count(r, len, p[i]);
            size_t c = countCorr(r, p, len, p[i]);
            if (countScorr(p, r, i, p[i]) >= (n - c))
                res[i] = '/';
            else
                res[i] = '|';
        }
    }

    printf("%s\n", res);

    return 0;
}
