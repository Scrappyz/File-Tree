## About The Project
A simple C++ program that will print the directory tree of the specified directory

## How To Use
#### Format
```
FileTree.exe <PATH> <EXCLUDES> <OUTPUT_FILE>
```

#### Example
```
FileTree.exe path/to/dir -e '$.txt' 'My$folder' -o output.txt
```

#### Options
```
-h, --help         Show help text
-e, --exclude      Exclude files from printing
-o, --output       Generate text file
```

#### Additional Info
- If path is not specified, it will print the current path
- '$' means any or more characters
  - Eg: `$.txt` will match all files that end with `.txt` and `file$` will match all files that start with `file`

#### Example Output
```
CurrentDir
+-- Folder1
|   +-- SubFolder1
|   |   +-- file1.txt
|   |   +-- SuberFolder
|   |   |   +-- new.txt
|   |   |   +-- new4.txt
|   |   +-- SuberFolder2
|   +-- SubFolder2
+-- Folder2
    +-- file2.txt
    +-- Sub1
    |   +-- hello.txt
    |   +-- hello2.txt
    |   +-- Sub2
    |   |   +-- gg.txt
    |   |   +-- ggg.txt
    |   +-- Sub3
    +-- Sub2
    |   +-- sub.txt
    |   +-- SubSub1
    +-- Sub3
        +-- new.txt
        +-- x.txt
```

