#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include "print.hpp"

using namespace std;

void showHelp()
{
    cout << "FORMAT:" << endl;
    
}

string getProgramName(char ch[])
{
    string name;
    name.assign(ch);
    for(int i = name.size()-1; i >= 0; i--) {
        if(name[i] == '\\' || name[i] == '/') {
            name.assign(name.begin()+(i+1), name.end());
            break;
        }
    }
    return name;
}

set<string> getExcludes(const vector<string>& args)
{
    set<string> excludes;
    for(int i = 0; i < args.size(); i++) {
        if(args[i] == "-e" || args[i] == "--exclude") {
            i++;
            while(i < args.size() && args[i][0] != '-') {
                excludes.insert(args[i]);
                i++;
            }
        }
    }
    return excludes;
}

string getPath(const vector<string>& args)
{
    string str;
    if(args.size() > 1 && args[1][0] != '-') {
        str = args[1];
    }
    if(str.empty()) {
        char currentPath[FILENAME_MAX];
        getcwd(currentPath, FILENAME_MAX);
        str = currentPath;
    }
    return str;
}

string getOutputText(const vector<string>& args)
{
    string str;
    for(int i = 0; i < args.size(); i++) {
        if(args[i] == "-o" && i < args.size()-1) {
            str = args[i+1];
        }
    }
    if(!str.empty() && str.find(".txt") == string::npos) {
        str += ".txt";
    }
    return str;
}

void printDirectoryTree(const filesystem::path& path, ofstream& text_file, int level = 0, const string& append = "")
{
    string current_path = path.filename().string();
    if(filesystem::exists(path)) {
        int file_count = 0;
        for(const auto& i : filesystem::directory_iterator(path)) {
            file_count++;
        }
        if(level == 0 && !text_file.is_open()) {
            cout << path.filename().string() << endl;
        } else if(level == 0 && text_file.is_open()) {
            text_file << path.filename().string() << endl;
        }
        int counter = 0;
        for(const auto& it : filesystem::directory_iterator(path)) {
            string filename = it.path().filename().string();
            if(text_file.is_open()) {
                if(level > 0) {
                    text_file << append;
                }
                text_file << "+-- ";
                text_file << filename << endl;
            } else {
                if(level > 0) {
                    cout << append;
                }
                cout << "+-- ";
                cout << filename << endl;
            }
            if(filesystem::is_directory(it.status())) {
                string next_append = append + "    ";
                if(counter >= file_count-1) {
                    next_append[level*4] = ' ';
                } else {
                    next_append[level*4] = '|';
                }
                printDirectoryTree(it, text_file, level+1, next_append);
            }
            counter++;
        }
    }
}

int main(int argc, char** argv) 
{
    vector<string> args;
    string program_name = getProgramName(argv[0]);
    args.assign(argv, argv+argc);
    filesystem::path path(getPath(args));
    ofstream output_text(getOutputText(args));
    set<string> excludes = getExcludes(args);
    printDirectoryTree(path, output_text);
    // print(excludes, '\n');
    // cout << output_text << endl;
    return 0;
}
