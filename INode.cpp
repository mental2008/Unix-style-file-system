#include "INode.h"
#include <cstring>

INode::INode() {
    this->id = -1;
    this->fsize = 0;
    this->fmode = 0;
    this->ctime = 0;
    this->count = 0;
    this->mcount = 0;
    memset(dir_address, 0, sizeof(dir_address));
    memset(indir_address, 0, sizeof(indir_address));
}

INode::~INode() {
}

void INode::clear() {
    this->id = -1;
    this->fsize = 0;
    this->fmode = 0;
    this->ctime = 0;
    this->count = 0;
    this->mcount = 0;
    memset(dir_address, 0, sizeof(dir_address));
    memset(indir_address, 0, sizeof(indir_address));
}
