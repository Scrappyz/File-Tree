#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include "file_tree.hpp"

using namespace std;

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
        if(!files.empty()) {
            str = files[0].first;
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
                    filesystem::path dir(str);
                    filesystem::create_directories(dir);
                    if(str.back() != '/' && str.back() != '\\') {
                        str += "/";
                    }
                } else {
                    ofstream fs(str);
                    fs.close();
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