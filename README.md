# Unix-style-file-system

Here is an i-node-based Unix-style file system

## Instructions

- Programming language: `c++`
- Use Unix file system design and i-node usage

## Attributes

- Allocate **16MB** space in memory as the storage for our file system
- The space is divided into blocks with block size **1KB**
- Address length is **24-bit**, **14-bit** for block id and **10-bit** for block offset
- The first a few blocks can be used for storing the i-nodes, and the first i-node can be used for the root directory (/)
- Random strings will be used to fill the files we created.

## Functionalities

- createFile <fileName> <fileSize>

Create a file with a fixed size(in KB).

- deleteFile <fileName>

Delete a file if it exists.

- createDir <dirPath>

Create a directory.

- deleteDir <dirPath>

Delete a directory if it exists.

- changeDir <dirPath>

Change the current working directory.

- dir

List all the files and sub-directories under current working directory.

- cp <fileName1> <fileName2>

Copy a file(file1) to another file(file2).

- sum

Display the usage of storage space.

- cat <fileName>

Print out the contents of the file on the terminal.

- help

Display the commands in our Operating System.

- exit

Quit the program.

## How to run the project

- All the compilation instructions are included in *makefile*
- Run the command in the shell

```shell
make && make clean && ./test
```
## License

Copyright (c) 2019 杨凌云 李其桦

Licensed under [Apache License](https://github.com/mental2008/Unix-style-file-system/blob/master/LICENSE).