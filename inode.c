#include "inode.h"
#include "block.h"
#include "fs_layout.h"
#include "free.h"
#include "pack.h"

#define INODE_SIZE 64
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

#define INODE_OWNER_ID_OFFSET 4
#define INODE_PERMISSIONS_OFFSET 6
#define INODE_FLAGS_OFFSET 7
#define INODE_LINK_COUNT_OFFSET 8
#define INODE_BLOCK_PTR_OFFSET 9
#define INODE_BLOCK_PTR_SIZE 2

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
        if (incore[i].ref_count > 0 &&
            incore[i].inode_num == inode_num) {
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

    int block_num = inode_num / INODES_PER_BLOCK + INODE_TABLE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int offset = block_offset * INODE_SIZE;

    bread(block_num, block);

    in->size = read_u32(block + offset);
    in->owner_id = read_u16(block + offset + INODE_OWNER_ID_OFFSET);
    in->permissions = read_u8(block + offset + INODE_PERMISSIONS_OFFSET);
    in->flags = read_u8(block + offset + INODE_FLAGS_OFFSET);
    in->link_count = read_u8(block + offset + INODE_LINK_COUNT_OFFSET);

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(block + offset + INODE_BLOCK_PTR_OFFSET +
            i * INODE_BLOCK_PTR_SIZE);
    }
}

void write_inode(struct inode *in)
{
    unsigned char block[BLOCK_SIZE];

    int block_num = in->inode_num / INODES_PER_BLOCK + INODE_TABLE_FIRST_BLOCK;
    int block_offset = in->inode_num % INODES_PER_BLOCK;
    int offset = block_offset * INODE_SIZE;

    bread(block_num, block);

    write_u32(block + offset, in->size);
    write_u16(block + offset + INODE_OWNER_ID_OFFSET, in->owner_id);
    write_u8(block + offset + INODE_PERMISSIONS_OFFSET, in->permissions);
    write_u8(block + offset + INODE_FLAGS_OFFSET, in->flags);
    write_u8(block + offset + INODE_LINK_COUNT_OFFSET, in->link_count);

    for (int i = 0; i < INODE_PTR_COUNT; i++) {
        write_u16(block + offset + INODE_BLOCK_PTR_OFFSET +
            i * INODE_BLOCK_PTR_SIZE, in->block_ptr[i]);
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

    bread(INODE_BITMAP_BLOCK_NUM, block);

    int inode_num = find_free(block);

    if (inode_num == -1) {
        return 0;
    }

    struct inode *in = iget(inode_num);

    if (!in) {
        return 0;
    }

    set_free(block, inode_num, 1);

    bwrite(INODE_BITMAP_BLOCK_NUM, block);

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

