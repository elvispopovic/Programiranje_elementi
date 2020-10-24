#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

class ITestFunkcije
{
public:
    virtual int  __stdcall Test(const char *)=0;
    virtual void __stdcall Click(const char *)=0;
};

typedef void(*vFuncITF)(ITestFunkcije *);

#endif // TEST_H_INCLUDED
