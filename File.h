#ifndef FILE_H
#define FILE_H

#include "time.h"

#define MAX_FILENAME_SIZE (20)

class File {
public:
    int inode_id;
    char filename[MAX_FILENAME_SIZE];

public:
    File();
    ~File();

};

#endif