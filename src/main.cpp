#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include "os.hpp"
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

string getPath(const vector<string>& args, const OS& os)
{
    string path;
    if(args.size() > 1 && args[1][0] != '-') {
        if(args[1] == ".") {
            char currentPath[FILENAME_MAX];
            getcwd(currentPath, FILENAME_MAX);
            path = currentPath;
            return path;
        } else if(isAbsolutePath(args[1], os)) {
            return args[1];
        } else {
            char currentPath[FILENAME_MAX];
            getcwd(currentPath, FILENAME_MAX);
            path = currentPath;
            string temp = args[1];
            int i = 0;
            if(temp[0] == '.' && temp[1] == '.') {
                int parent_dir = 0;
                int counter = 0;
                while(i < temp.size() && (temp[i] == '.' || temp[i] == '/' || temp[i] == '\\')) {
                    if(temp[i] == '.') {
                        counter++;
                    }
                    if((i+1) % 3 == 0 && temp[i] != '/' && temp[i] != '\\') {
                        break;
                    }
                    i++;
                }
                parent_dir = counter / 2;
                while(parent_dir > 0) {
                    if(os == OS::Windows && (path.back() == '/' || path.back() == '\\')) {
                        parent_dir--;
                    } else if(os == OS::Linux && path.back() == '/') {
                        parent_dir--;
                    } else if(os == OS::Mac_OS && path.back() == ':') {
                        parent_dir--;
                    }
                    if(parent_dir != 0) {
                        path.pop_back();
                    }
                }
            } else {
                if(os == OS::Windows && path.back() != '/' && path.back() != '\\') {
                    path += "/";
                } else if(os == OS::Linux && path.back() != '/') {
                    path += "/";
                } else if(os == OS::Mac_OS && path.back() != ':') {
                    path += ":";
                }
                while(i < temp.size() && (invalidFilenameChar(temp[i]) || temp[i] == '.')) {
                    i++;
                }
                while(i < temp.size()) {
                    path.push_back(temp[i]);
                    i++;
                }
                while(invalidFilenameChar(path.back())) {
                    path.pop_back();
                }
            }
        }
    }
    if(path.empty()) {
        char currentPath[FILENAME_MAX];
        getcwd(currentPath, FILENAME_MAX);
        path = currentPath;
    }
    return path;
}

string getTextFile(const vector<string>& args)
{
    string path;
    for(int i = 0; i < args.size(); i++) {
        if(args[i][0] == '-' && i < args.size()-1 && (args[i] == "-o" || args[i] == "--output" || args[i] == "-md" || args[i] == "--make-directory")) {
            path = args[i+1];
        }
    }
    if(!path.empty() && path.find(".txt") == string::npos) {
        path += ".txt";
    }
    return path;
}

void setOptions(const vector<string>& args, unordered_map<string, bool>& options)
{
    for(int i = 0; i < args.size(); i++) {
        if(args[i][0] == '-' && options.count(args[i])) {
            options[args[i]] = true;
        }
    }
}

// int main(int argc, char** argv) 
// {
//     vector<string> args;
//     unordered_map<string, bool> options = {{"-h", 0}, {"--help", 0}, {"-e", 0}, {"--exclude", 0}, {"-o", 0}, {"--output", 0},
//     {"-md", 0}, {"--make-directory", 0}};
//     string program_name = getProgramName(argv[0]);
//     args.assign(argv, argv+argc);
//     setOptions(args, options);
//     if(options.at("-h") || options.at("--help")) {
//         showHelp(program_name);
//         return 0;
//     }
//     filesystem::path path(getPath(args));
//     string text_file = getTextFile(args);
//     if(options.at("-md") || options.at("--make-directory")) {
//         ifstream file(text_file);
//         if(!file.is_open()) {
//             cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
//         } else {
//             makeDirectory(path, file);
//         }
//         file.close();
//     } else {
//         ofstream file(text_file);
//         if(!text_file.empty() && !file.is_open()) {
//             cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
//         } else {
//             unordered_set<string> patterns = getExcludePattern(args);
//             printDirectoryTree(path, patterns, file);
//         }
//         file.close();
//     }
//     return 0;
// }

// Debug
int main(int argc, char** argv) 
{
    OS os = OS::Unknown;
    #ifdef _WIN32
        os = OS::Windows;
    #elif __linux__
        os = OS::Linux;
    #elif __APPLE__
        os = OS::macOS;
    #else
        cerr << "Unknown or unsupported operating system" << endl;
    #endif

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
    string p = getPath(args, os);
    cout << "path: " << p << endl;
    filesystem::path path(p);
    string text_file = getTextFile(args);
    cout << "text_file: " << text_file << endl;
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