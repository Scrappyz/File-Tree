#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include "file_tree.hpp"

using namespace std;

void showHelp(const string& program)
{
    cout << "FORMAT:" << endl;
    cout << program << " <PATH>" << endl;
    cout << program << " <PATH> <EXCLUDES>" << endl;
    cout << program << " <PATH> <EXCLUDES> <OUTPUT_FILE>" << endl;
    cout << "EXAMPLE:" << endl;
    cout << program << " path/to/dir -e '$.txt' 'My$folder' -o output.txt" << endl;
    cout << "OPTIONS:" << endl;
    cout << "-h, --help         Show help text" << endl;
    cout << "-e, --exclude      Exclude files from printing via regular expression" << endl;
    cout << "-o, --output       Generate text file" << endl;
    cout << "REGULAR EXPRESSIONS:" << endl;
    cout << "- '$' means any or more characters" << endl;
    cout << "   - '$.txt' will match all files that end with '.txt'" << endl;
    cout << "   - '$.$' will match all files that have an extension" << endl;
    cout << "   - 'file$' will match all files that start with 'file'" << endl;
    cout << "   - 'My$Folder' will match files such as 'MySubFolder' and 'MyfavFolder'" << endl;
    cout << "- '/' or '\\' means print the directory but not its contents" << endl;
    cout << "   - 'build/' this expression will print the directory 'build' but not its contents" << endl;
    cout << "ADDITIONAL INFO:" << endl;
    cout << "- If path is not specified, it will print the current path" << endl;
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

unordered_set<string> getExcludePattern(const vector<string>& args)
{
    unordered_set<string> excludes;
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
        int i = 0;
        while(i < args[1].size() && (invalidFilenameChar(args[1][i]) || args[1][i] == '.')) {
            i++;
        }
        while(i < args[1].size()) {
            str.push_back(args[1][i]);
            i++;
        }
        while(invalidFilenameChar(str.back())) {
            str.pop_back();
        }
    }
    if(str.empty()) {
        char currentPath[FILENAME_MAX];
        getcwd(currentPath, FILENAME_MAX);
        str = currentPath;
    }
    return str;
}

string getTextFile(const vector<string>& args)
{
    string str;
    for(int i = 0; i < args.size(); i++) {
        if(args[i][0] == '-' && i < args.size()-1 && (args[i] == "-o" || args[i] == "--output" || args[i] == "-md" || args[i] == "--make-directory")) {
            str = args[i+1];
        }
    }
    if(!str.empty() && str.find(".txt") == string::npos) {
        str += ".txt";
    }
    return str;
}

void setOptions(const vector<string>& args, unordered_map<string, bool>& options)
{
    for(int i = 0; i < args.size(); i++) {
        if(args[i][0] == '-' && options.count(args[i])) {
            options[args[i]] = true;
        }
    }
}

int main(int argc, char** argv) 
{
    vector<string> args;
    unordered_map<string, bool> options = {{"-h", 0}, {"--help", 0}, {"-e", 0}, {"--exclude", 0}, {"-o", 0}, {"--output", 0},
    {"-md", 0}, {"--make-directory", 0}};
    string program_name = getProgramName(argv[0]);
    args.assign(argv, argv+argc);
    setOptions(args, options);
    if(options.at("-h") || options.at("--help")) {
        showHelp(program_name);
        return 0;
    }
    //string str = "D:/Documents/Codes/VS Code/C++/Tools/FileTree/bin/Debug";
    filesystem::path path(getPath(args));
    string text_file = getTextFile(args);
    if(options.at("-md") || options.at("--make-directory")) {
        ifstream file(text_file);
        if(!file.is_open()) {
            cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
        } else {
            makeDirectory(path, file);
        }
        file.close();
    } else {
        ofstream file(text_file);
        if(!text_file.empty() && !file.is_open()) {
            cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
        } else {
            unordered_set<string> patterns = getExcludePattern(args);
            printDirectoryTree(path, patterns, file);
        }
        file.close();
    }
    return 0;
}
