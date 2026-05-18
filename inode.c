#include "inode.h"
#include "block.h"
#include "free.h"

#define BLOCK_SIZE 4096

int ialloc(void)
{
    unsigned char block[BLOCK_SIZE];

    bread(1, block);

    int inode_num = find_free(block);

    if (inode_num == -1) {
        return -1;
    }

    set_free(block, inode_num, 1);

    bwrite(1, block);

    return inode_num;
}