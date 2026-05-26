#include <string.h>
#include "free.h"
#include "inode.h"

#include "image.h"
#include "block.h"
#include "ctest.h"

#define BLOCK_SIZE 4096

void test_block_write_read(void)
{
    unsigned char write_buf[BLOCK_SIZE];
    unsigned char read_buf[BLOCK_SIZE];

    memset(write_buf, 0, BLOCK_SIZE);
    memset(read_buf, 0, BLOCK_SIZE);

    strcpy((char *)write_buf, "hello world");

    image_open("disk.img", 1);

    bwrite(0, write_buf);

    bread(0, read_buf);

    CTEST_ASSERT(
        strcmp((char *)read_buf, "hello world") == 0,
        "block write/read works"
    );

    image_close();
}

void mkfs(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0, BLOCK_SIZE);

    bwrite(0, block);
    bwrite(1, block);
    bwrite(3, block);
    bwrite(4, block);
    bwrite(5, block);
    bwrite(6, block);

    memset(block, 0, BLOCK_SIZE);

    block[0] = 0x7F;

    bwrite(2, block);
}

void test_set_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0, BLOCK_SIZE);

    set_free(block, 0, 1);

    CTEST_ASSERT(
        block[0] == 1,
        "set_free works"
    );
}

void test_find_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0xFF, BLOCK_SIZE);

    block[0] = 0xFE;

    CTEST_ASSERT(
        find_free(block) == 0,
        "find_free works"
    );
}

void test_alloc(void)
{
    image_open("disk.img", 1);

    mkfs();

    CTEST_ASSERT(
        alloc() == 7,
        "alloc works"
    );

    image_close();
}

void test_ialloc(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct inode *in = ialloc();

    CTEST_ASSERT(
        in->inode_num == 0,
        "ialloc works"
    );

    image_close();
}

void test_incore(void)
{
    incore_free_all();

    struct inode *x = incore_find_free();

    x->ref_count = 1;
    x->inode_num = 5;

    CTEST_ASSERT(
        incore_find(5) == x,
        "incore_find works"
    );
}

void test_inode_rw(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct inode in = {0};
    struct inode out = {0};

    in.inode_num = 0;
    in.size = 1234;

    write_inode(&in);

    read_inode(&out, 0);

    CTEST_ASSERT(
        out.size == 1234,
        "inode rw works"
    );

    image_close();
}

int main(void)
{
    CTEST_VERBOSE(1);
    test_block_write_read();
    test_alloc();
    test_ialloc();
    test_set_free();
    test_find_free();
    test_incore();
    test_inode_rw();
    CTEST_RESULTS();
    CTEST_EXIT();
}