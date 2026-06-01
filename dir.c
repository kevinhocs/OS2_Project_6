#include "dir.h"
#include "block.h"
#include "pack.h"

#include <stdlib.h>
#include <string.h>
#define DIR_ENTRY_SIZE 32

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

    unsigned char block[4096];

    int data_block_index =
        dir->offset / 4096;

    int data_block_num =
        dir->inode->block_ptr[data_block_index];

    bread(data_block_num, block);

    int offset_in_block =
        dir->offset % 4096;

    ent->inode_num =
        read_u16(block + offset_in_block);

    strcpy(
        ent->name,
        (char *)(block + offset_in_block + 2)
    );

    dir->offset += DIR_ENTRY_SIZE;
    return 0;
}

void directory_close(struct directory *dir)
{
    iput(dir->inode);
    free(dir);
}