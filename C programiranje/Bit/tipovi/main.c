#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeCharBits(char** ptBuff, char c);
char* endian();
char* getCharBits(char x);
char* getShortBits(short x);
char* getIntBits(int x);
char* getLongLongBits(long long x);
char* getFloatBits(float x);
char* getDoubleBits(double x);

int main()
{
    char c;
    short s;
    int a;
    long long l;
    double x;
    float y;
    printf("Endian: %s\n",endian());

    c=-64;
    printf("char, %d: %s\n",c,getCharBits(c));

    s=-32768;
    printf("short, %d: %s\n",s,getShortBits(s));

    a=-2000000000;
    printf("int, %d: %s\n",a,getIntBits(a));

    l=16000000004;
    #ifdef _WIN32
        printf("long long, %I64d: %s\n",l,getLongLongBits(l));
    #elif linux
        printf("long long, %lld: %s\n",l,getLongLongBits(l));
    #endif // linux

    x=-1.0E-10;
    printf("double, %E: %s\n",x, getDoubleBits(x));

    y=-2.5E-1;
    printf("float, %E: %s\n",y, getFloatBits(y));
    return 0;
}

void writeCharBits(char** ptBuff, char c)
{
    for(int i=7; i>=0; i--)
        if(c&(1<<i))
            *(*ptBuff)++='1';
        else
            *(*ptBuff)++='0';
    *(*ptBuff)++=' ';
}

char* endian()
{
    static char res[15];
    int x = 1;
    if(*((char *)(&x))==1)
        strncpy(res,"little endian",14);
    else
        strncpy(res,"big endian",11);
    return res;
}

char* getCharBits(char x)
{
    static char res[10];
    char* ptRes = res;
    writeCharBits(&ptRes, x);
    (*ptRes++)='\0';
    return res;
}

char* getShortBits(short x)
{
    static char res[20];
    char* ptRes = res;
    int i, o = 1;
    size_t l = sizeof(x);
    if(*((char *)(&o))==0) //big endian
        for(i=0; i<l; i++)
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    else
        for(i=l-1; i>=0; i--) //little endian
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    (*ptRes++)='\0';
    return res;
}
char* getIntBits(int x)
{
    static char res[40];
    char* ptRes = res;
    int i, o = 1;
    size_t l = sizeof(x);
    if(*((char *)(&o))==0) //big endian
        for(i=0; i<l; i++)
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    else
        for(i=l-1; i>=0; i--) //little endian
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    (*ptRes++)='\0';
    return res;
}

char* getLongLongBits(long long x)
{
    static char res[80];
    char* ptRes = res;
    int i, o = 1;
    size_t l = sizeof(x);
    if(*((char *)(&o))==0) //big endian
        for(i=0; i<l; i++)
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    else
        for(i=l-1; i>=0; i--) //little endian
            writeCharBits(&ptRes, ((char*)(&x))[i]);
    (*ptRes++)='\0';
    return res;
}

char* getFloatBits(float x)
{
    static char res[40];
    char* ptRes = res;
    int i;
    size_t l = sizeof(x);
    for(i=l-1; i>=0; i--)
        writeCharBits(&ptRes, ((char*)(&x))[i]);
    (*ptRes++)='\0';
    return res;
}

char* getDoubleBits(double x)
{
    static char res[80];
    char* ptRes = res;
    int i;
    size_t l = sizeof(x);
    for(i=l-1; i>=0; i--)
        writeCharBits(&ptRes, ((char*)(&x))[i]);
    (*ptRes++)='\0';
    return res;
}
