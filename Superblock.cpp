#include "Superblock.h"

Superblock::Superblock() {
    systemsize = 0;
    blocksize = 0;
    blocknum = 0;
    address_length = 0;
    max_filename_size = 0;
    superblock_size = 0;
    inode_size = 0;
    inode_bitmap_size = 0;
    block_bitmap_size = 0;
    inode_table_size = 0;
}

Superblock::~Superblock() {
}