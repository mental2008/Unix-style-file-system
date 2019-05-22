#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define SYSTEM_SIZE (16*1024*1024)
#define BLOCK_SIZE (1024)
#define BLOCK_NUM (SYSTEM_SIZE/BLOCK_SIZE)
#define ADDRESS_LENGTH (24)
#define BLOCK_BIT (14)
#define OFFSET (10)
#define MAX_PATH (1000)
#define INODE_SIZE (128)
#define SUPER_BLOCK_SIZE (1024)
#define INODE_BITMAP_SIZE (2*1024)
#define BLOCK_BITMAP_SIZE (2*1024)
#define INODE_TABLE_SIZE (2*1024*1024)
#define INODE_BITMAP_START SUPER_BLOCK_SIZE
#define BLOCK_BITMAP_START (INODE_BITMAP_START+INODE_BITMAP_SIZE)
#define INODE_TABLE_START (BLOCK_BITMAP_START+BLOCK_BITMAP_SIZE)
#define MAX_FILE_SIZE (351)
#define FILE_MODE 0
#define DENTRY_MODE 1
#define HOME "unix.os"
#define root_dir "~"

#include <fstream>
#include <ctime>
#include <iomanip>
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include "Superblock.h"
#include "File.h"
#include "INode.h"
#include "Address.h"

enum State {
    DIR_NOT_EXIST = 0,
    SUCCESS = 1,
    NO_FILENAME = 2,
    FILE_EXISTS = 3,
    LENGTH_EXCEED = 4,
    DIRECTORY_EXCEED = 5,
    NO_ENOUGH_SPACE = 6,
    NO_SUCH_FILE = 7,
    NO_DIRNAME = 8,
    NO_SUCH_DIR = 9,
    DIR_NOT_EMPTY = 10,
    CAN_NOT_DELETE_TEMP_DIR = 11,
    DIR_EXISTS = 12
};

class Filesystem {

public:
    char curpath[MAX_PATH];
    std::FILE *fp;
    bool flag;
    Superblock superblock;
    INode root_Inode;
    INode cur_Inode;

public:
    Filesystem();
    ~Filesystem();
    void initialize();
    void welcome();
    State createFile(std::string fileName, int fileSize);
    State deleteFile(std::string fileName);
    State createDir(std::string dirName);
    State deleteDir(std::string dirName);
    State changeDir(std::string path);
    void dir();
    State cp(std::string file1, std::string file2);
    void sum();
    State cat(std::string fileName);
    void tip();
    void help();
    int numberOfAvailableBlock();
    int findAvailableInode();
    int findAvailableBlock();
    void modifyInodeBitmap(int pos);
    void modifyBlockBitmap(int pos);
    void writeInode(int pos, INode inode);
    void writeFileToDentry(File file, INode inode);
    INode readInode(int pos);
    INode findNxtInode(INode inode, std::string fileName, bool &canFind);
    void writeRandomStringToBlock(int blockid);
    void writeAddressToBlock(Address address, int blockid, int offset);
    void deleteFileFromDentry(INode inode, std::string fileName);
    void giveState(std::string command, State st);

};

#endif