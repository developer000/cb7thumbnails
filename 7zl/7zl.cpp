//============================================================================
// Name        : 7zl.cpp
// Author      : developer000
// Version     : 1.0.1
// License     : GPL3
// Description : 7zip list archive - only files in archive
//============================================================================

#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>

using namespace std;
const int MAX_BUFFER = 255;
const char* ver="1.0.1";
const char* run(const char* cmd)
{
    string stdout;
    char buffer[MAX_BUFFER];
    FILE *stream = popen(cmd, "r");
    while ( fgets(buffer, MAX_BUFFER, stream) != NULL )
    stdout.append(buffer);
    pclose(stream);
    return stdout.c_str();
}
int main(int argc, char *argv[])
{
    if (1 == argc)
    {
        cout<<"7zl version: "<<ver<<endl;
        cout<<"usage: <7z location> <file>"<<endl;
        return 0;
    }
    string help("-h");
    string param(argv[1]);
    if (help == param)
    {
        cout<<"7zl version: "<<ver<<endl;
        cout<<"usage: <7z location> <file>"<<endl;
        return 0;
    }
    if (3 != argc)
    {
        cout<<argv[1]<<argv[2];
        return -1;
    }
    ostringstream arg;
    arg<<argv[1]<<" l \""<<argv[2]<<"\" -slt | sed -n -e '/Path = /s/Path = //p' | sed -e '1d'";
    cout << run(arg.str().c_str());
    return 0;
}
