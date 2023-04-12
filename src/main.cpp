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

template<typename T>
void print(const std::vector<T>& v, char append = '\0', const std::string& separator = ", ") 
{
    std::cout << "{";
    for (std::size_t i = 0; i < v.size(); i++) {
        if constexpr (std::is_scalar_v<T> || std::is_same_v<T, std::string>) {
            std::cout << v[i];
        } else {
            print(v[i], '\0', separator);
        }

        if (i != v.size() - 1) {
            std::cout << separator;
        }
    }
    std::cout << "}" << append;
}

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
    string path;
    if(args.size() > 1 && args[1][0] != '-') {
        string temp = args[1];
        while(invalidFilenameChar(temp.back())) {
            temp.pop_back();
        }

        filesystem::path p(temp);
        if(p.string() == ".") {
            return getCallPath();
        } else if(p.is_absolute()) {
            return args[1];
        } else {
            path = joinPath(getCallPath(), p.string());
        }
    }
    if(path.empty()) {
        path = getCallPath();
    }
    return path;
}

string getTextFile(const vector<string>& args) 
{
    string path;
    for(int i = 0; i < args.size(); i++) {
        if(i < args.size()-1 && args[i][0] == '-' && (args[i] == "-o" || args[i] == "--output" || args[i] == "-md" || args[i] == "--make-directory")) {
            string temp = args[i+1];
            while(invalidFilenameChar(temp.back())) {
                temp.pop_back();
            }
            path = joinPath(getCallPath(), temp);
            break;
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

    string p = getPath(args);
    if(!pathExists(p)) {
        cout << "[Error] Path \"" << p << "\" does not exists" << endl;
        return 0;
    }

    filesystem::path path(p);
    string text_file = getTextFile(args);
    unordered_set<string> patterns = getExcludePattern(args);

    if(options.at("-md") || options.at("--make-directory")) {
        ifstream file(text_file);
        if(!file.is_open()) {
            cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
        } else {
            makeDirectory(path, file, patterns);
        }
        file.close();
    } else {
        ofstream file(text_file);
        if(!text_file.empty() && !file.is_open()) {
            cout << "[Error] Could not open file \"" << text_file << "\"" << endl;
        } else {
            if(path.has_extension()) {
                cout << "[Info] " << path.filename() << " is not a directory" << endl;
            } else {
                printDirectoryTree(path, patterns, file);
            }
        }
        file.close();
    }
    return 0;
}