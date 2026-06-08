#include "dir.h"
#include "block.h"
#include "fs_layout.h"
#include "pack.h"
#include "namei.h"

#include <stdlib.h>
#include <string.h>

struct directory *directory_open(int inode_num)
{
    struct inode *in = iget(inode_num);

    if (!in) {
        return 0;
    }

    struct directory *dir = malloc(sizeof(struct directory));

    if (!dir) {
        iput(in);
        return 0;
    }

    dir->inode = in;
    dir->offset = 0;

    return dir;
}

int directory_get(
    struct directory *dir,
    struct directory_entry *ent
)
{
    if (dir->offset >= dir->inode->size) {
        return -1;
    }

    unsigned char block[BLOCK_SIZE];

    int data_block_index = dir->offset / BLOCK_SIZE;

    int data_block_num = dir->inode->block_ptr[data_block_index];

    bread(data_block_num, block);

    int offset_in_block = dir->offset % BLOCK_SIZE;

    ent->inode_num = read_u16(block + offset_in_block);

    strcpy(ent->name,
        (char *)(block + offset_in_block + DIRECTORY_ENTRY_NAME_OFFSET));

    dir->offset += DIRECTORY_ENTRY_SIZE;
    return 0;
}

void directory_close(struct directory *dir)
{
    iput(dir->inode);
    free(dir);
}

int directory_make(char *path)
{
    if (path[0] != '/') {
        return -1;
    }

    char *name = path + 1;

    struct inode *parent;
    struct inode *newdir;
    int block_num;

    parent = namei("/");

    if (!parent) {
        return -1;
    }

    newdir = ialloc();

    if (!newdir) {
        iput(parent);
        return -1;
    }

    block_num = alloc();

    if (block_num < 0) {
        iput(newdir);
        iput(parent);
        return -1;
    }

    unsigned char dirblock[BLOCK_SIZE];

    memset(dirblock, 0, BLOCK_SIZE);

    write_u16(dirblock + DIRECTORY_ENTRY_DOT_OFFSET, newdir->inode_num);

    strcpy((char *)(dirblock + DIRECTORY_ENTRY_NAME_OFFSET), ".");

    write_u16(dirblock + DIRECTORY_ENTRY_DOTDOT_OFFSET, parent->inode_num);

    strcpy((char *)(dirblock + DIRECTORY_ENTRY_DOTDOT_NAME_OFFSET), "..");

    bwrite(block_num, dirblock);

    newdir->flags = DIRECTORY_FLAGS;
    newdir->size = DIRECTORY_INITIAL_SIZE;
    newdir->block_ptr[0] = block_num;

    write_inode(newdir);

    unsigned char parent_block[BLOCK_SIZE];

    bread(parent->block_ptr[0], parent_block);

    write_u16(parent_block + parent->size, newdir->inode_num);

    strcpy((char *)(parent_block + parent->size + DIRECTORY_ENTRY_NAME_OFFSET),
        name);

    bwrite(parent->block_ptr[0], parent_block);

    parent->size += DIRECTORY_ENTRY_SIZE;

    write_inode(parent);

    iput(newdir);
    iput(parent);

    return 0;
}

