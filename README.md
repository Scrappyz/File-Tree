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

#### Regular Expressions
- `$` means any or more characters
  - `$.txt` will match all files that end with `.txt`
  - `$.$` will match all files that have an extension
  - `file$` will match all files that start with `file`
  - `My$Folder` will match files such as `MySubFolder` and `MyfavFolder`
- `/` or `\` means print the directory but not its contents
  - `build/` this expression will print the directory `build` but not its contents

#### Additional Info
- If path is not specified, it will print the current path

#### Example Output
Input:
```
FileTree.exe 
```

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

