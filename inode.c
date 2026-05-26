#include "inode.h"
#include "block.h"
#include "free.h"
#include "pack.h"

#define BLOCK_SIZE 4096
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

struct inode *incore_find_free(void)
{
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (incore[i].ref_count == 0) {
            return &incore[i];
        }
    }

    return 0;
}

struct inode *incore_find(unsigned int inode_num)
{
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        if (
            incore[i].ref_count > 0 &&
            incore[i].inode_num == inode_num
        ) {
            return &incore[i];
        }
    }

    return 0;
}

void incore_free_all(void)
{
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
        incore[i].ref_count = 0;
    }
}

void read_inode(struct inode *in, int inode_num)
{
    unsigned char block[BLOCK_SIZE];

    int block_num =
        inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;

    int block_offset =
        inode_num % INODES_PER_BLOCK;

    int offset =
        block_offset * INODE_SIZE;

    bread(block_num, block);

    in->size = read_u32(block + offset);
    in->owner_id = read_u16(block + offset + 4);
    in->permissions = read_u8(block + offset + 6);
    in->flags = read_u8(block + offset + 7);
    in->link_count = read_u8(block + offset + 8);

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] =
            read_u16(block + offset + 9 + i * 2);
    }
}

void write_inode(struct inode *in)
{
    unsigned char block[BLOCK_SIZE];

    int block_num =
        in->inode_num / INODES_PER_BLOCK +
        INODE_FIRST_BLOCK;

    int block_offset =
        in->inode_num % INODES_PER_BLOCK;

    int offset =
        block_offset * INODE_SIZE;

    bread(block_num, block);

    write_u32(block + offset, in->size);
    write_u16(block + offset + 4, in->owner_id);
    write_u8(block + offset + 6, in->permissions);
    write_u8(block + offset + 7, in->flags);
    write_u8(block + offset + 8, in->link_count);

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(
            block + offset + 9 + i * 2,
            in->block_ptr[i]
        );
    }

    bwrite(block_num, block);
}

struct inode *iget(int inode_num)
{
    struct inode *in;

    in = incore_find(inode_num);

    if (in) {
        in->ref_count++;

        return in;
    }

    in = incore_find_free();

    if (!in) {
        return 0;
    }

    read_inode(in, inode_num);

    in->ref_count = 1;
    in->inode_num = inode_num;

    return in;
}

void iput(struct inode *in)
{
    if (in->ref_count == 0) {
        return;
    }

    in->ref_count--;

    if (in->ref_count == 0) {
        write_inode(in);
    }
}

struct inode *ialloc(void)
{
    unsigned char block[BLOCK_SIZE];

    bread(1, block);

    int inode_num = find_free(block);

    if (inode_num == -1) {
        return 0;
    }

    struct inode *in = iget(inode_num);

    if (!in) {
        return 0;
    }

    set_free(block, inode_num, 1);

    bwrite(1, block);

    in->size = 0;
    in->owner_id = 0;
    in->permissions = 0;
    in->flags = 0;
    in->link_count = 0;

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = 0;
    }

    in->inode_num = inode_num;

    write_inode(in);

    return in;
}

