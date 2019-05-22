#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

class Superblock {
public:
    int systemsize;
    int blocksize;
    int blocknum;
    int address_length;
    int max_filename_size;
    int superblock_size;
    int inode_size;
    int inode_bitmap_size;
    int block_bitmap_size;
    int inode_table_size;

public:
    Superblock();
    ~Superblock();

};

#endif