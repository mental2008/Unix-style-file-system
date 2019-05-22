#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include "INode.h"
#include "Superblock.h"
#include "File.h"
#include "Filesystem.h"
#include "Address.h"

using namespace std;

int str2num(string s) {
    int res = 0;
    int len = (int)s.size();
    for(int i = 0; i < len; ++i) {
        if(s[i]>='0' && s[i]<='9') res = res*10+s[i]-'0';
        else return -1;
    }
    return res;
}

int main() {
    Filesystem filesystem;
    filesystem.welcome();
    filesystem.initialize();
    if(!filesystem.flag) {
        return 0;
    }
    while(1) {
        filesystem.tip();
        string input;
        getline(cin, input);
        int len = input.size();
        vector<string> command;
        string temp = "";
        for(int i = 0; i < len; ++i) {
            if(input[i] == ' ') {
                if(temp != "") {
                    command.push_back(temp);
                    temp = "";
                }
                continue;
            }
            temp += input[i];
        }
        if(temp != "") command.push_back(temp);
        int sz = command.size();
        if(sz == 0) continue;
        if(command[0] == "createFile") {
            if(sz > 3) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 3) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else {
                int res = str2num(command[2]);
                if(res>=0) filesystem.giveState(command[0], filesystem.createFile(command[1], res));
                else cout << "Command \'" << command[0] << "\': Not correct parameter for fileSize." << endl;
            }
        }
        else if(command[0] == "deleteFile") {
            if(sz > 2) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 2) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.deleteFile(command[1]));
        }
        else if(command[0] == "createDir") {
            if(sz > 2) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 2) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.createDir(command[1]));
        }
        else if(command[0] == "deleteDir") {
            if(sz > 2) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 2) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.deleteDir(command[1]));
        }
        else if(command[0] == "changeDir") {
            if(sz > 2) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 2) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.changeDir(command[1]));
        }
        else if(command[0] == "dir") {
            if(sz > 1) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else filesystem.dir();
        }
        else if(command[0] == "cp") {
            if(sz > 3) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 3) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.cp(command[1], command[2]));
        }
        else if(command[0] == "sum") {
            if(sz > 1) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else filesystem.sum();
        }
        else if(command[0] == "cat") {
            if(sz > 2) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else if(sz < 2) cout << "Command \'" << command[0] << "\': too few parameters" << endl;
            else filesystem.giveState(command[0], filesystem.cat(command[1]));
        }
        else if(command[0] == "help") {
            if(sz > 1) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else filesystem.help();
        }
        else if(command[0] == "exit") {
            if(sz > 1) cout << "Command \'" << command[0] << "\': too many parameters" << endl;
            else break;
        }
        else {
            cout << command[0] << ": command not found" << endl;
        }
    }
    return 0;
}
