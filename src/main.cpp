#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <unistd.h>

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

vector<string> getExcludes(const vector<string>& args)
{
    unordered_set<string> s;
    vector<string> excludes;
    bool exclude = false;
    for(int i = 0; i < args.size(); i++) {
        if(args[i] == "-e") {
            exclude = true;
        } else if(exclude && args[i].size() > 1 && args[i][0] == '.') {
            if(s.find(args[i]) == s.end()) {
                s.insert(args[i]);
                excludes.push_back(args[i]);
            } 
        } else if(exclude && args[i][0] == '-') {
            return excludes;
        }
        if(exclude && args[i] == ".*") {
            return vector<string>(1, ".*");
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
    return str;
}

void printDirectoryTree(const filesystem::path& path, int level = 0, string append = "")
{
    string current_path = path.filename().string();
    if(filesystem::exists(path)) {
        int file_count = 0;
        for(const auto& i : filesystem::directory_iterator(path)) {
            file_count++;
        }
        if(level == 0) {
            cout << path.filename().string() << endl;
        }
        int counter = 0;
        for(const auto& it : filesystem::directory_iterator(path)) {
            string filename = it.path().filename().string();
            if(level > 0) {
                cout << append;
            }
            cout << "+-- ";
            cout << filename << " [level " << level << "]" << endl;
            if(filesystem::is_directory(it.status())) {
                string next_append = append + "    ";
                if(counter >= file_count-1) {
                    next_append[level*4] = ' ';
                } else {
                    next_append[level*4] = '|';
                }
                printDirectoryTree(it, level+1, next_append);
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
    string output_text = getOutputText(args);
    vector<string> excludes = getExcludes(args);
    printDirectoryTree(path);
    return 0;
}
