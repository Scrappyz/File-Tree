#pragma once

bool isAbsolutePath(const std::string& path, const OS& os);
bool invalidFilenameChar(char ch);
bool isDirectorySeparator(char ch, const OS& os);
bool isExclude(const std::string& filename, const std::unordered_set<std::string>& excludes);
std::string getCallPath();
void addDirectorySeparator(std::string& path, const OS& os);
void joinPath(std::string& path, const std::string& child_path, const OS& os = OS::Unknown);
void createFile(const std::string& path);
void createFolder(const std::string& path);
void makeDirectory(const std::filesystem::path& path, std::ifstream& text_file);
void printDirectoryTree(const std::filesystem::path& path, const std::unordered_set<std::string>& excludes, std::ofstream& text_file, int level = 0, const std::string& append = "");