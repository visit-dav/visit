#include <stdio.h>
#include <string>
using std::string;

string
i2s(int i)
{
    char tmp[1000];
    sprintf(tmp, "%d", i);
    return string(tmp);
}

char
toLower(char c)
{
    if (c>='A' && c<='Z')
        return c + (int('a')-int('A'));
    else
        return c;
}

char
toUpper(char c)
{
    if (c>='a' && c<='z')
        return c + (int('A')-int('a'));
    else
        return c;
}

string
toUpper(string s)
{
    for (int i=0; i<s.length(); i++)
        s[i] = toUpper(s[i]);
    return s;
}

string
findWord(const string &s)
{
    int i=0;
    string foo("");
    while (s.length() > i && (s[i] == ' ' || s[i] == 0x08 || s[i] == '\t' || s[i] == '\n'))
        i++;
    while (s.length() > i && (s[i] != ' ' && s[i] != 0x08 && s[i] != '\t' && s[i] != '\n'))
    {
        foo += s[i];
        i++;
    }
    return foo;
}

string
stripWord(string &s)
{
    int i = 0;
    string foo("");
    while (s.length() > i && (s[i] == ' ' || s[i] == 0x08 || s[i] == '\t' || s[i] == '\n'))
        i++;
    int len=0;
    while (s.length() > i && (s[i] != ' ' && s[i] != 0x08 && s[i] != '\t' && s[i] != '\n'))
    {
        foo += s[i];
        i++;
        len++;
    }
    if (len && s.length() > i)
        s = s.substr(i, s.length() - i);
    return foo;
}

