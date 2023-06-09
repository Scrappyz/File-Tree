#pragma once

bool isPathFormatValid(const std::string& path);
bool pathExists(const std::string& path);
bool invalidFilenameChar(char ch);
bool isDirectorySeparator(char ch);
bool isExclude(const std::string& filename, const std::unordered_set<std::string>& excludes);
std::string getCallPath();
std::string joinPath(const std::string& path, const std::string& child_path);
void createFile(const std::string& path);
void createFolder(const std::string& path, bool delete_contents = false);
void makeDirectory(const std::filesystem::path& path, std::ifstream& text_file, const std::unordered_set<std::string>& pattern = {});
void printDirectoryTree(const std::filesystem::path& path, const std::unordered_set<std::string>& excludes, std::ofstream& text_file, int level = 0, const std::string& append = "");