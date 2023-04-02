#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include "print.hpp"

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

bool invalidFilenameChar(char ch)
{
    switch(ch) {
        case '#':
        case '%':
        case '&':
        case '{':
        case '}':
        case '\\':
        case '/':
        case '<':
        case '>':
        case '*':
        case '?':
        case '~':
        case '!':
        case '\'':
        case '\"':
        case ':':
        case '@':
        case '+':
        case '`':
        case '|':
        case '=':
            return true;
        default:
            return false;
    }
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

string getOutputText(const vector<string>& args)
{
    string str;
    for(int i = 0; i < args.size(); i++) {
        if((args[i] == "-o" || args[i] == "--output") && i < args.size()-1) {
            str = args[i+1];
        }
    }
    if(!str.empty() && str.find(".txt") == string::npos) {
        str += ".txt";
    }
    return str;
}

bool isExclude(const string& filename, const unordered_set<string>& excludes)
{
    for(const auto& exc : excludes) {
        string ex = exc;
        if(ex == "$" || ex == filename) {
            return true;
        }
        int i = 0;
        int j = 0;
        while(i < filename.size() && j < ex.size()) {
            if(filename[i] == ex[j]) {
                i++;
                j++;
            } else if(j < ex.size()-1 && ex[j] == '$') {
                j++;
                while(i < filename.size() && filename[i] != ex[j]) {
                    i++;
                }
            } else if(j >= ex.size()-1 && ex.back() == '$') {
                i++;
                if(i >= filename.size()) {
                    j++;
                }
            } else {
                break;
            }
        }
        if(i >= filename.size() && j >= ex.size()) {
            return true;
        }
    }
    return false;
}

void printDirectoryTree(const filesystem::path& path, const unordered_set<string>& excludes, ofstream& text_file, int level = 0, const string& append = "")
{
    if(filesystem::exists(path)) {
        string filename = path.filename().string();
        if(level == 0 && isExclude(filename, excludes)) {
            return;
        }
        int file_count = 0;
        unordered_set<string> excluded_files;
        for(const auto& i : filesystem::directory_iterator(path)) {
            filename = i.path().filename().string();
            if(isExclude(filename, excludes)) {
                excluded_files.insert(filename);
            } else {
                file_count++;
            }
        }
        if(level == 0 && !text_file.is_open()) {
            cout << path.filename().string() << endl;
        } else if(level == 0 && text_file.is_open()) {
            text_file << path.filename().string() << endl;
        }
        int counter = 0;
        for(const auto& it : filesystem::directory_iterator(path)) {
            filename = it.path().filename().string();
            if(excluded_files.find(filename) != excluded_files.end()) {
                continue;
            }
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
            if(filesystem::is_directory(it.status()) && (!isExclude(filename + "/", excludes) && !isExclude(filename + "\\", excludes))) {
                string next_append = append + "    ";
                if(counter >= file_count-1) {
                    next_append[level*4] = ' ';
                } else {
                    next_append[level*4] = '|';
                }
                printDirectoryTree(it, excludes, text_file, level+1, next_append);
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
    if(args.size() > 1 && (args[1] == "-h" || args[1] == "--help")) {
        showHelp(program_name);
        return 0;
    }
    filesystem::path path(getPath(args));
    ofstream output_text(getOutputText(args));
    unordered_set<string> patterns = getExcludePattern(args);
    printDirectoryTree(path, patterns, output_text);
    return 0;
}
