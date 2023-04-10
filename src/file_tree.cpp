#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unistd.h>
#include "os.hpp"
#include "file_tree.hpp"
#include "print.hpp"

using namespace std;

bool isPathFormatValid(const string& path) 
{
    try {
        filesystem::path testPath(path);
        return true;
    }
    catch (const exception&) {
        return false;
    }
}

bool pathExists(const string& path)
{
    return filesystem::exists(filesystem::path(path));
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

bool isDirectorySeparator(char ch)
{
    #ifdef _WIN32
        if(ch == '/' || ch == '\\') {
            return true;
        } else {
            return false;
        }
    #elif __linux__
        if(ch == '/') {
            return true;
        } else {
            return false;
        }
    #elif __APPLE__
        if(ch == ':') {
            return true;
        } else {
            return false;
        }
    #else
        std::cerr << "[Error] Unknown Operating System" << std::endl;
        return false;
    #endif
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
                i = filename.size();
                j = ex.size();
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

string getCallPath()
{
    char currentPath[FILENAME_MAX];
    getcwd(currentPath, FILENAME_MAX);
    return string(currentPath);
}

void addDirectorySeparator(string& path, const OS& os)
{
    if(os == OS::Windows && path.back() != '/' && path.back() != '\\') {
        path += "/";
    } else if(os == OS::Linux && path.back() != '/') {
        path += "/";
    } else if(os == OS::Mac_OS && path.back() != ':') {
        path += ":";
    }
}

string joinPath(const string& path, const string& child_path)
{
    if(!isPathFormatValid(path)) {
        throw runtime_error("Invalid path: " + path);
    }
    if(!isPathFormatValid(child_path)) {
        throw runtime_error("Invalid child path: " + child_path);
    }

    filesystem::path p1(path);
    filesystem::path p2(child_path);

    if(p2.is_absolute()) {
        return p2.string();
    }
    string temp;
    for(int i = 0; i < child_path.size(); i++) {
        if(isDirectorySeparator(child_path[i]) || i == child_path.size()-1) {
            if(i == child_path.size()-1 && !isDirectorySeparator(child_path[i])) {
                temp.push_back(child_path[i]);
            }
            if(temp == "..") {
                p1 = p1.parent_path();
            } else {
                p1 /= temp;
            }
            temp.clear();
        } else {
            temp.push_back(child_path[i]);
        }
    }

    return p1.string();
}

void createFile(const string& path)
{
    ofstream file(path);
    file.close();
}

void createFolder(const string& path)
{
    filesystem::path dir(path);
    filesystem::create_directories(dir);
}

void makeDirectory(const filesystem::path& path, ifstream& text_file)
{
    if(filesystem::exists(path)) {
        vector<pair<string, int>> files;
        string str;
        while(getline(text_file, str)) {
            int i = 0;
            int lvl = 0;
            string filename;
            while(str.back() == ' ') {
                str.pop_back();
            }
            while(i < str.size() && (str[i] == '+' || str[i] == '|' || str[i] == '-' || str[i] == ' ')) {
                lvl++;
                i++;
            }
            lvl /= 4;
            while(i < str.size()) {
                filename.push_back(str[i]);
                i++;
            }
            files.push_back(make_pair(filename, lvl));
        }

        str.clear();
        str = filesystem::absolute(path).string();
        if(!files.empty()) {
            str += files[0].first;
            if(str.back() != '/' && str.back() != '\\') {
                str += "/";
            }
            int sub = 0;
            for(int i = 1; i < files.size(); i++) {
                sub = (files[i-1].second - files[i].second) + 1;
                if(sub > 0) {
                    int counter = 0;
                    while(str.back() == '/' || str.back() == '\\') {
                        str.pop_back();
                    }
                    for(int j = str.size()-1; j >= 0 && counter < sub; j--) {
                        if(str[j] == '/' || str[j] == '\\') {
                            counter++;
                        } 
                        if(counter != sub) {
                            str.pop_back();
                        }
                    }
                }
                str += files[i].first;
                bool is_dir = false;
                if(i < files.size()-1 && files[i].second < files[i+1].second) {
                    is_dir = true;
                }
                if(str.back() == '/' || str.back() == '\\' || is_dir) {
                    createFolder(str);
                    if(str.back() != '/' && str.back() != '\\') {
                        str += "/";
                    }
                } else {
                    createFile(str);
                }
            }
        }
    }
}

void printDirectoryTree(const filesystem::path& path, const unordered_set<string>& excludes, ofstream& text_file, int level, const string& append)
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
            cout << path.filename().string() << "/" << endl;
        } else if(level == 0 && text_file.is_open()) {
            text_file << path.filename().string() << "/" << endl;
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
                text_file << filename;
            } else {
                if(level > 0) {
                    cout << append;
                }
                cout << "+-- ";
                cout << filename;
            }
            if(filesystem::is_directory(it.status())) {
                if(text_file.is_open()) {
                    text_file << "/" << endl;
                } else {
                    cout << "/" << endl;
                }
                if(!isExclude(filename + "/", excludes) && !isExclude(filename + "\\", excludes)) {
                    string next_append = append + "    ";
                    if(counter >= file_count-1) {
                        next_append[level*4] = ' ';
                    } else {
                        next_append[level*4] = '|';
                    }
                    printDirectoryTree(it, excludes, text_file, level+1, next_append);
                }
            } else {
                if(text_file.is_open()) {
                    text_file << endl;
                } else {
                    cout << endl;
                }
            }
            counter++;
        }
    }
}