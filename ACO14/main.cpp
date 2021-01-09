#include "ACO.h"
#include "supervisor.h"

using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        cout << "No filename parameter." << endl;
        cout << "Usage: " << argv[0] << " " << "filename" << endl;
        return 0;
    }
    if(!loadData(argv[1]))
    {
        cout << "Data file error." << endl;
        return 0;
    }
    beginSupervisor("./outputData/stat");
    writeProblemData();

    init();
    writeInitData();


    cleanup();

    endSupervisor();
    /* cleans parser memory space */
    freeData();
    return 0;
}







