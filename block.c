#include <unistd.h>
#include "free.h"

#include "block.h"
#include "image.h"

#define BLOCK_SIZE 4096

unsigned char *bread(int block_num, unsigned char *block)
{
    lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET);

    read(image_fd, block, BLOCK_SIZE);

    return block;
}

void bwrite(int block_num, unsigned char *block)
{
    lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET);

    write(image_fd, block, BLOCK_SIZE);
}

int alloc(void)
{
    unsigned char block[BLOCK_SIZE];

    bread(2, block);

    int block_num = find_free(block);

    if (block_num == -1) {
        return -1;
    }

    set_free(block, block_num, 1);

    bwrite(2, block);

    return block_num;
}