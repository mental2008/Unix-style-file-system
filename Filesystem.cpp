#include "Filesystem.h"

char buffer[SYSTEM_SIZE];

Filesystem::Filesystem() {
    strcpy(curpath, root_dir);
    flag = true;
}

Filesystem::~Filesystem() {
    fclose(fp);
}

void Filesystem::welcome() {
    std::cout << std::endl;

    std::cout<<"Welcome to our Operating System! XD" << std::endl;
    std::cout<<"The information of our group is as followed: " << std::endl;

    std::cout<<"\tQihua Li\t\t\tLingyun Yang" << std::endl;
    std::cout<<"\t201630588276\t\t\t201630588436" << std::endl;

    std::cout << std::endl;

    help();

}

void Filesystem::initialize() {
    std::fstream file;
    file.open(HOME, std::ios::in);
    if(!file) {
        std::cout << "The Operating System is initializing..." << std::endl;
        fp = fopen(HOME, "wb+");
        if(fp == NULL) {
            std::cout << "Error when creating the Operating System..." << std::endl;
            flag = false;
            return;
        }
        fwrite(buffer, SYSTEM_SIZE, 1, fp); // create a 16MB space for OS

        fseek(fp, 0, SEEK_SET);
        superblock.systemsize = SYSTEM_SIZE;
        superblock.blocksize = BLOCK_SIZE;
        superblock.blocknum = BLOCK_NUM;
        superblock.address_length = ADDRESS_LENGTH;
        superblock.max_filename_size = MAX_FILENAME_SIZE;
        superblock.superblock_size = SUPER_BLOCK_SIZE;
        superblock.inode_size = INODE_SIZE;
        superblock.inode_bitmap_size = INODE_BITMAP_SIZE;
        superblock.block_bitmap_size = BLOCK_BITMAP_SIZE;
        superblock.inode_table_size = INODE_TABLE_SIZE;
        fwrite(&superblock, sizeof(Superblock), 1, fp); // write superblock

        fseek(fp, INODE_BITMAP_START, SEEK_SET);
        for(int i = 0; i < INODE_BITMAP_SIZE; ++i) {
            unsigned char byte = 0;
            fwrite(&byte, sizeof(unsigned char), 1, fp);
        } // initialize inode bitmap

        fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
        for(int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
            unsigned char byte = 0;
            fwrite(&byte, sizeof(unsigned char), 1, fp);
        } // initialize block bitmap

        modifyBlockBitmap(0); // set block of superblock busy
        
        for(int i = 0; i < INODE_BITMAP_SIZE/1024; ++i) {
            modifyBlockBitmap(i+1);
        } // set block of inode bitmap busy

        for(int i = 0; i < BLOCK_BITMAP_SIZE/1024; ++i) {
            modifyBlockBitmap(i+INODE_BITMAP_SIZE/1024+1);
        } // set block of block bitmap busy

        for(int i = 0; i < INODE_TABLE_SIZE/1024; ++i) {
            modifyBlockBitmap(i+INODE_BITMAP_SIZE/1024+BLOCK_BITMAP_SIZE/1024+1);
        } // set block of inode table busy

        root_Inode.clear();
        root_Inode.count = 0;
        root_Inode.mcount = 0;
        root_Inode.ctime = time(NULL);
        // root_Inode.dir_address[0] = findAvailableBlock();
        root_Inode.fmode = DENTRY_MODE;
        root_Inode.id = 0;
        modifyInodeBitmap(0);
        // modifyBlockBitmap(root_Inode.dir_address[0]);
        writeInode(0, root_Inode);
        cur_Inode = root_Inode;

        // std::cout << findAvailableInode() << "\n";

        sum();
    }
    else {
        std::cout << "The Operating System is loading..." << std::endl;
        fp = fopen(HOME, "rb+");
        if(fp == NULL) {
            std::cout << "Error when loading the Operating System..." << std::endl;
            flag = false;
            return;
        }

        fseek(fp, 0, SEEK_SET);
        fread(&superblock, sizeof(Superblock), 1, fp);

        cur_Inode = root_Inode = readInode(0);
        // std::cout << cur_Inode.fmode << " " << cur_Inode.dir_address[0] << " " << findAvailableInode() << std::endl;
        // std::cout << findAvailableInode() << "\n";

        sum();

        // std::cout << "Next inode can be used: " << findAvailableInode() << std::endl;

    }
}

void Filesystem::tip() {
    std::cout << "[ root@os ]: " << curpath << "# ";
}

void Filesystem::help() {
    std::cout << "Following commands are supported in our Operating System:" << std::endl;

    std::cout << "createFile <fileName> <fileSize>" << std::endl;
    std::cout << "\tCreate a file with a fixed size(in KB)." << std::endl;
    std::cout << "\te.g. createFile /dir/myFile 10" << std::endl;

    std::cout << "deleteFile <fileName>" << std::endl;
    std::cout << "\tDelete a file if it exists." << std::endl;
    std::cout << "\te.g. deleteFile /dir/myFile" << std::endl;

    std::cout << "createDir <dirPath>" << std::endl;
    std::cout << "\tCreate a directory." << std::endl;
    std::cout << "\te.g. createDir /dir/sub" << std::endl;

    std::cout << "deleteDir <dirPath>" << std::endl;
    std::cout << "\tDelete a directory if it exists." << std::endl;
    std::cout << "\te.g. deleteDir /dir/sub" << std::endl;

    std::cout << "changeDir <dirPath>" << std::endl;
    std::cout << "\tChange the current working directory." << std::endl;
    std::cout << "\te.g. changeDir /dir" << std::endl;

    std::cout << "dir" << std::endl;
    std::cout << "\tList all the files and sub-directories under current working directory." << std::endl;

    std::cout << "cp <fileName1> <fileName2>" << std::endl;
    std::cout << "\tCopy a file(file1) to another file(file2)." << std::endl;
    std::cout << "\te.g. cp file1 file2" << std::endl;

    std::cout << "sum" << std::endl;
    std::cout << "\tDisplay the usage of storage space." << std::endl;

    std::cout << "cat <fileName>" << std::endl;
    std::cout << "\tPrint out the contents of the file on the terminal." << std::endl;
    std::cout << "\te.g. cat /dir/myFile" << std::endl;

    std::cout << "help" << std::endl;
    std::cout << "\tDisplay the commands in our Operating System." << std::endl;

    std::cout << "exit" << std::endl;
    std::cout << "\tQuit the program." << std::endl;
}

int Filesystem::findAvailableInode() {
    fseek(fp, INODE_BITMAP_START, SEEK_SET);
    int pos = -1;
    for(int i = 0; i < INODE_BITMAP_SIZE; ++i) {
        unsigned char byte;
        fread(&byte, sizeof(unsigned char), 1, fp);
        for(int j = 0; j < 8; ++j) {
            if(((byte>>j)&1) == 0) {
                pos = i*8+j;
                break;
            }
        }
        if(pos != -1) break;
    }
    return pos;
}

int Filesystem::findAvailableBlock() {
    fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
    int pos = -1;
    for(int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
        unsigned char byte;
        fread(&byte, sizeof(unsigned char), 1, fp);
        for(int j = 0; j < 8; ++j) {
            if(((byte>>j)&1) == 0) {
                // std::cout << i << " " << j << std::endl;
                pos = i*8+j;
                break;
            }
        }
        if(pos != -1) break;
    }
    return pos;
}

int Filesystem::numberOfAvailableBlock() {
    int unused = 0;
    fseek(fp, BLOCK_BITMAP_START, SEEK_SET);
    for(int i = 0; i < BLOCK_BITMAP_SIZE; ++i) {
        unsigned char byte;
        fread(&byte, sizeof(unsigned char), 1, fp);
        for(int j = 0; j < 8; ++j) {
            if(((byte>>j)&1) == 0) unused++;
        }
    }
    return unused;
}

void Filesystem::sum() {
    int unused = numberOfAvailableBlock();
    int used = BLOCK_NUM - unused;

    std::cout << "System Size: " << superblock.systemsize << " Bytes" << std::endl;
    std::cout << "Block Size: " << superblock.blocksize << " Bytes" << std::endl;
    std::cout << "INode Bitmap Size: " << superblock.inode_bitmap_size << " Bytes" << std::endl;
    std::cout << "Block Bitmap Size: " << superblock.inode_bitmap_size << " Bytes" << std::endl;
    std::cout << "INode Table Size: " << superblock.inode_table_size << " Bytes" << std::endl;
    std::cout << "Number of Blocks: " << superblock.blocknum << std::endl;
    std::cout << "Number of Blocks that have been used: " << used << std::endl;
    std::cout << "Number of Blocks that are available: " << unused << std::endl;

    // std::cout << findAvailableInode() << "\n";

}

void Filesystem::modifyInodeBitmap(int pos) {

    // std::cout << "Modify: " << pos << "\n";

    int origin = pos/8;
    int offset = pos%8;
    unsigned char byte;
    fseek(fp, INODE_BITMAP_START+origin, SEEK_SET);
    fread(&byte, sizeof(unsigned char), 1, fp);

    // std::cout << origin << " " << offset << " " << (int)byte << "\n";

    byte = (byte ^ (1<<offset));
    // std::cout << "After: " << (int)byte << "\n";

    fseek(fp, INODE_BITMAP_START+origin, SEEK_SET);
    fwrite(&byte, sizeof(unsigned char), 1, fp);

    // fseek(fp, INODE_BITMAP_START+origin, SEEK_SET);
    // fread(&byte, sizeof(unsigned char), 1, fp);
    // std::cout << (int)byte << std::endl;
    /*
    test
    */
    // std::cout << "Nxt: ";
    // fseek(fp, INODE_BITMAP_START, SEEK_SET);
    // bool fg = false;
    // for(int i = 0; i < INODE_BITMAP_SIZE; ++i) {
    //     unsigned char byte;
    //     fread(&byte, sizeof(unsigned char), 1, fp);
    //     for(int j = 0; j < 8; ++j) {
    //         if((byte>>j)^1) {
    //             fg = true;
    //             std::cout << pos << "\n";
    //             break;
    //         }
    //     }
    //     if(fg) break;
    // }


}

void Filesystem::modifyBlockBitmap(int pos) {

    // std::cout << "Modify: " << pos << "\n";

    int origin = pos/8;
    int offset = pos%8;
    unsigned char byte;
    fseek(fp, BLOCK_BITMAP_START+origin, SEEK_SET);
    fread(&byte, sizeof(unsigned char), 1, fp);
    byte = (byte ^ (1<<offset));
    fseek(fp, BLOCK_BITMAP_START+origin, SEEK_SET);
    fwrite(&byte, sizeof(unsigned char), 1, fp);
}

void Filesystem::writeInode(int pos, INode inode) {
    fseek(fp, INODE_TABLE_START+INODE_SIZE*pos, SEEK_SET);
    fwrite(&inode, sizeof(INode), 1, fp);
}

INode Filesystem::readInode(int pos) {
    INode inode;
    fseek(fp, INODE_TABLE_START+INODE_SIZE*pos, SEEK_SET);
    fread(&inode, sizeof(INode), 1, fp);
    return inode;
}

void Filesystem::dir() {
    cur_Inode = readInode(cur_Inode.id);
    // std::cout << "Temp inode id: " << cur_Inode.id << "\n";
    int cnt = cur_Inode.mcount;
    int FILE_PER_BLOCK = superblock.blocksize/sizeof(File);
    
    std::cout << std::left << std::setw(7) << "Mode";
    std::cout << std::right << std::setw(25) << "Created Time";
    std::cout << std::right << std::setw(17) << "Length(Bytes)";
    std::cout << " ";
    std::cout << std::left << std::setw(25) << "Name";
    std::cout << std::endl;
    std::cout << std::left << std::setw(7) << "----";
    std::cout << std::right << std::setw(25) << "-----------";
    std::cout << std::right << std::setw(17) << "-------------";
    std::cout << " ";
    std::cout << std::left << std::setw(25) << "----";
    std::cout << std::endl;

    // std::cout << cnt << "\n";

    for(int i = 0; i < cur_Inode.NUM_DIRECT_ADDRESS; ++i) {
        if(cnt == 0) break;
        // fseek(fp, BLOCK_SIZE*cur_Inode.dir_address[i], SEEK_SET);
        // std::cout << cur_Inode.dir_address[i] << std::endl;
        for(int j = 0; j < FILE_PER_BLOCK; ++j) {
            if(cnt == 0) break;
            cnt--;
            File file;
            fseek(fp, BLOCK_SIZE*cur_Inode.dir_address[i]+sizeof(File)*j, SEEK_SET);
            fread(&file, sizeof(File), 1, fp);
            if(file.inode_id == -1) continue;
            INode inode = readInode(file.inode_id);
            // std::cout << inode.id << " " << inode.fsize << "\n";
            if(inode.fmode == DENTRY_MODE) std::cout << std::left << std::setw(7) << "Dentry";
            else std::cout << std::left << std::setw(7) << "File";
            char buffer[50];
            strftime(buffer, 40, "%a %b %d %T %G", localtime(&inode.ctime));
            std::cout << std::right << std::setw(25) << buffer;
            std::cout << std::right << std::setw(17) << inode.fsize*1024;
            std::cout << " ";
            std::cout << std::left << std::setw(25) << file.filename;
            std::cout << std::endl;
        }
    }
}

State Filesystem::createFile(std::string fileName, int fileSize) {
    int len = (int)fileName.size();
    INode inode;
    // std::cout << fileName << " " << fileSize << std::endl;
    if(fileName[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(fileName[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += fileName[i];
    }
    if(temp == "") return NO_FILENAME;
    if((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;
    // std::cout << temp << std::endl;
    v.push_back(temp);
    
    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    bool ok = true;
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(ok) return FILE_EXISTS;

    // std::cout << "3" << std::endl;

    int SUM_OF_DIRECTORY = superblock.blocksize/sizeof(File)*INode::NUM_DIRECT_ADDRESS;
    if(inode.count >= SUM_OF_DIRECTORY) return DIRECTORY_EXCEED;

    int unused = numberOfAvailableBlock();
    if(unused < fileSize) return NO_ENOUGH_SPACE;
    if(fileSize > MAX_FILE_SIZE) return NO_ENOUGH_SPACE;

    // std::cout << "6" << std::endl;

    File file;
    file.inode_id = findAvailableInode();
    // std::cout << file.inode_id << "\n";
    modifyInodeBitmap(file.inode_id);
    // std::cout << v[cnt-1].c_str() << std::endl;
    strcpy(file.filename, v[cnt-1].c_str());

    // std::cout << findAvailableInode() << std::endl;

    INode newInode;
    newInode.clear();
    newInode.fsize = fileSize;
    newInode.ctime = time(NULL);
    newInode.fmode = FILE_MODE;
    newInode.id = file.inode_id;

    writeFileToDentry(file, inode);

    int temp_fileSize = fileSize;

    for(int i = 0; i < INode::NUM_DIRECT_ADDRESS; ++i) {
        if(temp_fileSize == 0) {
            break;
        }
        temp_fileSize--;
        newInode.dir_address[i] = findAvailableBlock();
        modifyBlockBitmap(newInode.dir_address[i]);
        writeRandomStringToBlock(newInode.dir_address[i]);
    }
    if(temp_fileSize>0) {
        newInode.indir_address[0] = findAvailableBlock();
        modifyBlockBitmap(newInode.indir_address[0]); // indirect address
        int cnt = 0;
        while(temp_fileSize>0) {
            temp_fileSize--;
            int blockid = findAvailableBlock();
            modifyBlockBitmap(blockid);
            Address address;
            address.setblockID(blockid);
            address.setOffset(0);
            writeRandomStringToBlock(blockid);
            writeAddressToBlock(address, newInode.indir_address[0], cnt);
            cnt++;
        }
    }

    writeInode(newInode.id, newInode);
    return SUCCESS;

}

void Filesystem::writeRandomStringToBlock(int blockid) {
    srand(time(0));
    fseek(fp, blockid*superblock.blocksize, SEEK_SET);
    for(int i = 0; i < superblock.blocksize; ++i) {
        char randomChar = (rand() % 26) + 'a';
        // std::cout << randomChar << std::endl;
        fwrite(&randomChar, sizeof(char), 1, fp);
    }
}

void Filesystem::writeAddressToBlock(Address address, int blockid, int offset) {
    fseek(fp, blockid*superblock.blocksize+offset*sizeof(Address), SEEK_SET);
    fwrite(&address, sizeof(Address), 1, fp);
}

void Filesystem::writeFileToDentry(File file, INode inode) {
    int cnt = inode.count;
    int FILE_PER_BLOCK = superblock.blocksize/sizeof(File);

    if(inode.mcount == cnt) {
        if(cnt % FILE_PER_BLOCK == 0) {
            inode.dir_address[cnt/FILE_PER_BLOCK] = findAvailableBlock();
            modifyBlockBitmap(inode.dir_address[cnt/FILE_PER_BLOCK]);
            fseek(fp, BLOCK_SIZE*inode.dir_address[cnt/FILE_PER_BLOCK], SEEK_SET);
            // std::cout << BLOCK_SIZE*inode.dir_address[cnt/FILE_PER_BLOCK] << " " << (int)ftell(fp) << std::endl;
        }
        else {
            fseek(fp, BLOCK_SIZE*inode.dir_address[cnt/FILE_PER_BLOCK]+sizeof(File)*(cnt%FILE_PER_BLOCK), SEEK_SET);
        }
        // std::cout << (int)ftell(fp) << std::endl;
        fwrite(&file, sizeof(File), 1, fp);

        inode.count++;
        inode.mcount++;
        writeInode(inode.id, inode);
    }
    else {
        // std::cout << inode.count << " " << inode.mcount << "\n";
        bool ok = false;
        int temp = inode.mcount;
        for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
            if(temp == 0) break;
            for(int j = 0; j < FILE_PER_BLOCK; ++j) {
                if(temp == 0) break;
                temp--;
                File tempfile;
                fseek(fp, BLOCK_SIZE*inode.dir_address[i]+sizeof(File)*j, SEEK_SET);
                fread(&tempfile, sizeof(File), 1, fp);
                if(tempfile.inode_id == -1) {
                    // std::cout << i << " " << j << "\n";
                    ok = true;
                    fseek(fp, BLOCK_SIZE*inode.dir_address[i]+sizeof(File)*j, SEEK_SET);
                    fwrite(&file, sizeof(File), 1, fp);
                }
                if(ok) break;
            }
            if(ok) break;
        }
        
        inode.count++;
        writeInode(inode.id, inode);
    }

    // std::cout << "Update Dentry" << std::endl;
    // std::cout << file.inode_id << " " << file.filename << "\n";
    // std::cout << inode.id << " " << inode.count << "\n";

    if(inode.id == cur_Inode.id) cur_Inode = readInode(cur_Inode.id);
    if(inode.id == root_Inode.id) root_Inode = readInode(root_Inode.id);
}

INode Filesystem::findNxtInode(INode inode, std::string fileName, bool &canFind) {
    int cnt = inode.mcount;
    int FILE_PER_BLOCK = superblock.blocksize/sizeof(File);
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(cnt == 0) break;
        fseek(fp, BLOCK_SIZE*inode.dir_address[i], SEEK_SET);
        for(int j = 0; j < FILE_PER_BLOCK; ++j) {
            if(cnt == 0) break;
            cnt--;
            File file;
            fread(&file, sizeof(File), 1, fp);
            if(file.inode_id == -1) continue;
            if(strcmp(file.filename, fileName.c_str()) == 0) {
                return readInode(file.inode_id);
            }
        }
    }
    canFind = false;
    return inode;
}

void Filesystem::deleteFileFromDentry(INode inode, std::string fileName) {
    int cnt = inode.mcount;
    int FILE_PER_BLOCK = superblock.blocksize/sizeof(File);
    bool ok = false;
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(cnt == 0) break;
        fseek(fp, BLOCK_SIZE*inode.dir_address[i], SEEK_SET);
        for(int j = 0; j < FILE_PER_BLOCK; ++j) {
            if(cnt == 0) break;
            cnt--;
            File file;
            fread(&file, sizeof(File), 1, fp);
            if(file.inode_id == -1) continue;
            if(strcmp(file.filename, fileName.c_str()) == 0) {
                // std::cout << fileName << std::endl;
                fseek(fp, -sizeof(File), SEEK_CUR);
                file.inode_id = -1;
                fwrite(&file, sizeof(File), 1, fp);
                ok = true;
            }
            if(ok) break;
        }
        if(ok) break;
    }
    inode.count--;
    writeInode(inode.id, inode);
}

State Filesystem::deleteFile(std::string fileName) {
    int len = (int)fileName.size();
    INode inode;
    if(fileName[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(fileName[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += fileName[i];
    }
    if(temp == "") return NO_SUCH_FILE;
    v.push_back(temp);

    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return NO_SUCH_FILE;
        inode = nxtInode;
    }

    bool ok = true;
    // std::cout << inode.id << "\n";
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode == DENTRY_MODE) ok = false;
    if(!ok) return NO_SUCH_FILE;

    deleteFileFromDentry(inode, v[cnt-1]);
    // std::cout << nxtInode.id << "\n";

    inode = nxtInode;
    modifyInodeBitmap(inode.id);
    int filesize = inode.fsize;
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(filesize == 0) {
            break;
        }
        filesize--;
        modifyBlockBitmap(inode.dir_address[i]);
    }
    if(filesize>0) {
        modifyBlockBitmap(inode.indir_address[0]);
        int offset = 0;
        while(filesize>0) {
            filesize--;
            Address address;
            fseek(fp, inode.indir_address[0]*superblock.blocksize+offset*sizeof(Address), SEEK_SET);
            fread(&address, sizeof(Address), 1, fp);
            // std::cout << address.getblockID() << std::endl;
            modifyBlockBitmap(address.getblockID());
            offset++;
        }
    }

    return SUCCESS;
}

State Filesystem::createDir(std::string dirName) {
    int len = (int)dirName.size();
    INode inode;
    // std::cout << fileName << " " << fileSize << std::endl;
    if(dirName[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(dirName[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += dirName[i];
    }
    if(temp == "") return NO_DIRNAME;
    if((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;
    // std::cout << temp << std::endl;
    v.push_back(temp);
    
    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    bool ok = true;
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(ok) return DIR_EXISTS;

    // std::cout << "3" << std::endl;

    int SUM_OF_DIRECTORY = superblock.blocksize/sizeof(File)*INode::NUM_DIRECT_ADDRESS;
    if(inode.count >= SUM_OF_DIRECTORY) return DIRECTORY_EXCEED;

    // std::cout << "6" << std::endl;

    File file;
    file.inode_id = findAvailableInode();
    // std::cout << file.inode_id << "\n";
    modifyInodeBitmap(file.inode_id);
    // std::cout << v[cnt-1].c_str() << std::endl;
    strcpy(file.filename, v[cnt-1].c_str());

    // std::cout << findAvailableInode() << std::endl;

    INode newInode;
    newInode.clear();
    newInode.fsize = 0;
    newInode.ctime = time(NULL);
    newInode.fmode = DENTRY_MODE;
    newInode.id = file.inode_id;

    writeFileToDentry(file, inode);

    writeInode(newInode.id, newInode);
    return SUCCESS;
}

State Filesystem::deleteDir(std::string dirName) {
    int len = (int)dirName.size();
    INode inode;
    if(dirName[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(dirName[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += dirName[i];
    }
    if(temp == "") return NO_SUCH_DIR;
    v.push_back(temp);

    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return NO_SUCH_DIR;
        inode = nxtInode;
    }

    bool ok = true;
    // std::cout << inode.id << "\n";
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode == FILE_MODE) ok = false;
    if(!ok) return NO_SUCH_DIR;

    if(dirName[0] == '/') {
        std::vector<std::string> cur;
        std::string tempdir = "";
        int sz = strlen(curpath);
        for(int i = 1; i < sz; ++i) {
            if(curpath[i] == '/') {
                if(tempdir != "") {
                    cur.push_back(tempdir);
                    tempdir = "";
                }
                continue;
            }
            tempdir += curpath[i];
        }
        if(tempdir != "") cur.push_back(tempdir);

        int curlen = (int)cur.size();
        // std::cout << sz << " " << curlen << std::endl;
        if(cnt <= curlen) {
            bool ok = true;
            for(int i = 0; i < cnt; ++i) {
                if(v[i] != cur[i]) {
                    // std::cout << v[i] << " " << cur[i] << std::endl;
                    ok = false;
                    break;
                }
            }
            if(ok) return CAN_NOT_DELETE_TEMP_DIR;
        }
    }

    // std::cout << nxtInode.id << " " << nxtInode.count << std::endl;

    if(nxtInode.count > 0) return DIR_NOT_EMPTY;

    deleteFileFromDentry(inode, v[cnt-1]);
    // std::cout << nxtInode.id << "\n";

    inode = nxtInode;
    modifyInodeBitmap(inode.id);
    int count = inode.mcount;
    int FILE_PER_BLOCK = superblock.blocksize/sizeof(File);
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(count <= 0) {
            break;
        }
        count -= FILE_PER_BLOCK;
        modifyBlockBitmap(inode.dir_address[i]);
    }

    return SUCCESS;
}

State Filesystem::changeDir(std::string path) {
    int len = (int)path.size();

    if(path == "/") {
        cur_Inode = root_Inode;
        strcpy(curpath, root_dir);
        return SUCCESS;
    }

    INode inode;
    if(path[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(path[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += path[i];
    }
    if(temp == "") return NO_DIRNAME;
    if((int)temp.size() >= MAX_FILENAME_SIZE) return LENGTH_EXCEED;

    v.push_back(temp);

    int cnt = (int)v.size();

   for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    bool ok = true;
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode != DENTRY_MODE) ok = false;
    if(!ok) return DIR_NOT_EXIST;
    
    cur_Inode = nxtInode;

    if(path[0] == '/') {
        strcpy(curpath, root_dir);
        for(int i = 0; i < cnt; ++i) {
            strcat(curpath, "/");
            strcat(curpath, v[i].c_str());
        }
    }
    else {
        for(int i = 0; i < cnt; ++i) {
            strcat(curpath, "/");
            strcat(curpath, v[i].c_str());
        }
    }
    return SUCCESS;
}

State Filesystem::cat(std::string fileName) {
    int len = (int)fileName.size();
    INode inode;

    if(fileName[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(fileName[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += fileName[i];
    }
    if(temp == "") return NO_FILENAME;

    v.push_back(temp);
    
    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    bool ok = true;
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode == DENTRY_MODE) ok = false;
    if(!ok) return NO_SUCH_FILE;
    
    inode = nxtInode;

    int fileSize = inode.fsize;

    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(fileSize == 0) {
            break;
        }
        fileSize--;
        int blockid = inode.dir_address[i];
        fseek(fp, blockid*superblock.blocksize, SEEK_SET);
        for(int j = 0; j < BLOCK_SIZE; ++j) {
            char buffer;
            fread(&buffer, sizeof(char), 1, fp);
            std::cout << buffer;
        }
    }
    if(fileSize > 0) {
        int offset = 0;
        while(fileSize > 0) {
            fileSize--;
            Address address;
            fseek(fp, inode.indir_address[0]*superblock.blocksize+offset*sizeof(Address), SEEK_SET);
            fread(&address, sizeof(Address), 1, fp);
            int blockid = address.getblockID();
            fseek(fp, blockid*superblock.blocksize, SEEK_SET);
            for(int j = 0; j < BLOCK_SIZE; ++j) {
                char buffer;
                fread(&buffer, sizeof(char), 1, fp);
                std::cout << buffer;
            }
            offset++;
        }
    }
    std::cout << std::endl;

    return SUCCESS;
}

State Filesystem::cp(std::string file1, std::string file2) {
    // read content from file1
    int len = (int)file1.size();
    INode inode;

    if(file1[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    std::vector<std::string> v;
    std::string temp = "";
    for(int i = 0; i < len; ++i) {
        if(file1[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += file1[i];
    }
    if(temp == "") return NO_FILENAME;

    v.push_back(temp);
    
    int cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    bool ok = true;
    INode nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode == DENTRY_MODE) ok = false;
    if(!ok) return NO_SUCH_FILE;
    
    inode = nxtInode;

    std::vector<char> content;
    int fileSize = inode.fsize;
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(fileSize == 0) {
            break;
        }
        fileSize--;
        int blockid = inode.dir_address[i];
        fseek(fp, blockid*superblock.blocksize, SEEK_SET);
        for(int j = 0; j < BLOCK_SIZE; ++j) {
            char buffer;
            fread(&buffer, sizeof(char), 1, fp);
            content.push_back(buffer);
        }
    }
    if(fileSize > 0) {
        int offset = 0;
        while(fileSize > 0) {
            fileSize--;
            Address address;
            fseek(fp, inode.indir_address[0]*superblock.blocksize+offset*sizeof(Address), SEEK_SET);
            fread(&address, sizeof(Address), 1, fp);
            int blockid = address.getblockID();
            fseek(fp, blockid*superblock.blocksize, SEEK_SET);
            for(int j = 0; j < BLOCK_SIZE; ++j) {
                char buffer;
                fread(&buffer, sizeof(char), 1, fp);
                content.push_back(buffer);
            }
            offset++;
        }
    }

    // deleteFile(file2);
    State state = createFile(file2, inode.fsize);
    if(state != SUCCESS) return state;

    // write content to file2

    len = (int)file2.size();

    if(file2[0] == '/') {
        inode = root_Inode;
    }
    else {
        inode = cur_Inode;
    }
    v.clear();
    temp = "";
    for(int i = 0; i < len; ++i) {
        if(file2[i] == '/') {
            if(temp != "") {
                v.push_back(temp);
                temp = "";
            }
            continue;
        }
        temp += file2[i];
    }
    if(temp == "") return NO_FILENAME;

    v.push_back(temp);
    
    cnt = (int)v.size();

    for(int i = 0; i < cnt-1; ++i) {
        bool ok = true;
        INode nxtInode = findNxtInode(inode, v[i], ok);
        if(nxtInode.fmode != DENTRY_MODE) ok = false;
        if(!ok) return DIR_NOT_EXIST;
        inode = nxtInode;
    }
    ok = true;
    nxtInode = findNxtInode(inode, v[cnt-1], ok);
    if(nxtInode.fmode == DENTRY_MODE) ok = false;
    if(!ok) return NO_SUCH_FILE;

    inode = nxtInode;

    fileSize = inode.fsize;
    int index = 0;
    for(int i = 0; i < inode.NUM_DIRECT_ADDRESS; ++i) {
        if(fileSize == 0) {
            break;
        }
        fileSize--;
        int blockid = inode.dir_address[i];
        fseek(fp, blockid*superblock.blocksize, SEEK_SET);
        for(int j = 0; j < BLOCK_SIZE; ++j) {
            char buffer = content[index++];
            fwrite(&buffer, sizeof(char), 1, fp);
        }
    }
    if(fileSize > 0) {
        int offset = 0;
        while(fileSize > 0) {
            fileSize--;
            Address address;
            fseek(fp, inode.indir_address[0]*superblock.blocksize+offset*sizeof(Address), SEEK_SET);
            fread(&address, sizeof(Address), 1, fp);
            int blockid = address.getblockID();
            fseek(fp, blockid*superblock.blocksize, SEEK_SET);
            for(int j = 0; j < BLOCK_SIZE; ++j) {
                char buffer = content[index++];
                fwrite(&buffer, sizeof(char), 1, fp);
            }
            offset++;
        }
    }

    return SUCCESS;
}

void Filesystem::giveState(std::string command, State st){
    if(st != SUCCESS)
        std::cout << "Command \'" << command << "\': ";
	if(st == DIR_NOT_EXIST){
		std::cout << "The directory does not exist!" << std::endl;
	}
    else if(st == NO_FILENAME){
		std::cout << "No file name provided!" << std::endl;
	}
    else if(st == FILE_EXISTS){
		std::cout << "The file has existed!" << std::endl;
	}
    else if(st == LENGTH_EXCEED){
		std::cout << "The length of the file name is exceed!" << std::endl;
	}
    else if(st == DIRECTORY_EXCEED){
		std::cout << "Too many files in the directory!" << std::endl;
	}
    else if(st == NO_ENOUGH_SPACE){
		std::cout << "The system has no enough space for the file!" << std::endl;
	}
    else if(st == NO_SUCH_FILE){
		std::cout << "No such file!" << std::endl;
	}
    else if(st == NO_DIRNAME){
		std::cout << "No directory name provided!" << std::endl;
	}
    else if(st == NO_SUCH_DIR){
		std::cout << "No such directory!" << std::endl;
	}
    else if(st == DIR_NOT_EMPTY){
		std::cout << "The directory is not empty!" << std::endl;
	}
    else if(st == CAN_NOT_DELETE_TEMP_DIR) {
		std::cout << "You cannot delete the current directory!" << std::endl;
	}
    else if(st == DIR_EXISTS) {
		std::cout << "The directory has existed!" << std::endl;
	}
}