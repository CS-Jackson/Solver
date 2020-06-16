#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "mysqlClient.h"
using namespace std;

int main(int argc, const char* argv[])
{
    //ofstream outFile;

    bool done = false;
    ifstream inFile("train.csv", ios::in);
    string lineStr;
    vector<vector<string>> strArray;
    MyDB newguy;
    
    while(getline(inFile, lineStr)) {
        //print the hole line
        cout << lineStr << endl;
        //save to 2D struct;
        stringstream ss(lineStr);
        string str;
        vector<string> lineArray;

        while(getline(ss, str, ',')) {
            cout << "Per str: " << str << endl;
            lineArray.push_back(str);
        }

        strArray.push_back(lineArray);

        cout << "Get Next line?(Y/N): " ;
        char c = getchar();
        switch(c) {
            case 'n': done = true; break;
            case 'y': 
            default : break;
        }
        if(done) break;
        //break;
    }
    // getchar();
    return 0;
}