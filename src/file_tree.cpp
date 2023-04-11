#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <unistd.h>
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

void createFolder(const string& path, bool delete_contents)
{
    filesystem::path dir(path);
    if(delete_contents) {
        filesystem::remove_all(dir);
    }
    filesystem::create_directories(dir);
}

void makeDirectory(const filesystem::path& path, ifstream& text_file)
{
    if(filesystem::exists(path)) {
        vector<pair<string, int>> files;
        string temp;
        while(getline(text_file, temp)) {
            int i = 0;
            int lvl = 0;
            string filename;
            while(temp.back() == ' ' || invalidFilenameChar(temp.back())) {
                temp.pop_back();
            }
            while(i < temp.size() && (temp[i] == '+' || temp[i] == '|' || temp[i] == '-' || temp[i] == ' ')) {
                lvl++;
                i++;
            }
            lvl /= 4;
            while(i < temp.size()) {
                filename.push_back(temp[i]);
                i++;
            }
            
            files.push_back(make_pair(filename, lvl));
        }
        print(files, '\n');
        temp.clear();

        filesystem::path p(path);
        bool all = false;
        for(int i = 0; i < files.size(); i++) {
            if(i == 0 || i > 0 && files[i-1].second < files[i].second) {
                p /= filesystem::path(files[i].first);
            } else if(i > 0 && files[i-1].second >= files[i].second) {
                int sub = (files[i-1].second - files[i].second) + 1;
                while(sub > 0) {
                    p = p.parent_path();
                    sub--;
                }
                p /= filesystem::path(files[i].first);
            }

            bool is_dir = false;
            if(!p.has_extension()) {
                is_dir = true;
            }
          
            if(!pathExists(p.string())) {
                if(is_dir) {
                    createFolder(p.string(), 1);
                } else {
                    createFile(p.string());
                }
            } else if(pathExists(p.string())) {
                char ch;
                if(!all) {
                    cout << "[Warning] " << p.filename() << " already exists. Overwrite file?" << endl;
                    cout << "Input [Y] for yes, [A] for yes to all, [N] for no, [X] to cancel: ";
                    cin >> ch;
                } else {
                    ch = 'Y';
                }
                if(toupper(ch) == 'Y' || toupper(ch) == 'A') {
                    if(toupper(ch) == 'A') {
                        all = true;
                    }
                    if(is_dir) {
                        createFolder(p.string(), 1);
                    } else {
                        createFile(p.string());
                    }
                } else if(toupper(ch) == 'X') {
                    return;
                }
            }

            cout << p.string();
            if(is_dir) {
                cout << " is a folder" << endl;
            } else {
                cout << " is a file" << endl;
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