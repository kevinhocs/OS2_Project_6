#include <string.h>

#include "free.h"
#include "inode.h"
#include "image.h"
#include "block.h"
#include "ctest.h"
#include "fs_layout.h"
#include "pack.h"
#include "dir.h"
#include "namei.h"

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

    CTEST_ASSERT(strcmp((char *)read_buf, "hello world") == 0,
        "block write/read works");

    image_close();
}

void mkfs(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0, BLOCK_SIZE);

    bwrite(SUPERBLOCK_NUM, block);
    bwrite(INODE_BITMAP_BLOCK_NUM, block);

    for (int block_num = INODE_TABLE_FIRST_BLOCK;
         block_num < INODE_TABLE_FIRST_BLOCK + INODE_TABLE_BLOCK_COUNT;
         block_num++) {
        bwrite(block_num, block);
    }

    memset(block, 0, BLOCK_SIZE);

    block[0] = INITIAL_RESERVED_BLOCKS_MASK;

    bwrite(FREE_BITMAP_BLOCK_NUM, block);

    struct inode *root = ialloc();

    int root_block = alloc();

    root->flags = DIRECTORY_FLAGS;
    root->size = DIRECTORY_INITIAL_SIZE;
    root->block_ptr[0] = root_block;

    unsigned char dirblock[BLOCK_SIZE];

    memset(dirblock, 0, BLOCK_SIZE);

    write_u16(dirblock + DIRECTORY_ENTRY_DOT_OFFSET, root->inode_num);

    strcpy((char *)(dirblock + DIRECTORY_ENTRY_NAME_OFFSET), ".");

    write_u16(dirblock + DIRECTORY_ENTRY_DOTDOT_OFFSET, root->inode_num);

    strcpy((char *)(dirblock + DIRECTORY_ENTRY_DOTDOT_NAME_OFFSET), "..");

    bwrite(root_block, dirblock);

    write_inode(root);

    iput(root);
}

void test_set_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0, BLOCK_SIZE);

    set_free(block, 0, 1);

    CTEST_ASSERT(block[0] == 1, "set_free works");
}

void test_find_free(void)
{
    unsigned char block[BLOCK_SIZE];

    memset(block, 0xFF, BLOCK_SIZE);

    block[0] = 0xFE;

    CTEST_ASSERT(find_free(block) == 0, "find_free works");
}

void test_alloc(void)
{
    image_open("disk.img", 1);

    mkfs();

    CTEST_ASSERT(alloc() == 8, "alloc works");

    image_close();
}

void test_ialloc(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct inode *in = ialloc();

    CTEST_ASSERT(in->inode_num == 1, "ialloc works");

    image_close();
}

void test_incore(void)
{
    incore_free_all();

    struct inode *x = incore_find_free();

    x->ref_count = 1;
    x->inode_num = 5;

    CTEST_ASSERT(incore_find(5) == x, "incore_find works");
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

    CTEST_ASSERT(out.size == 1234, "inode rw works");

    image_close();
}

void test_iget(void)
{
    incore_free_all();

    struct inode *a = iget(3);
    struct inode *b = iget(3);

    CTEST_ASSERT(a == b, "iget returns same inode");

    CTEST_ASSERT(a->ref_count == 2, "iget increments ref_count");
}

void test_iput(void)
{
    incore_free_all();

    struct inode *in = iget(1);

    iput(in);

    CTEST_ASSERT(in->ref_count == 0, "iput decrements ref_count");
}

void test_directory(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct directory *dir = directory_open(0);
    struct directory_entry ent;

    directory_get(dir, &ent);

    CTEST_ASSERT(strcmp(ent.name, ".") == 0, "first directory entry is .");

    directory_get(dir, &ent);

    CTEST_ASSERT(strcmp(ent.name, "..") == 0, "second directory entry is ..");

    directory_close(dir);

    image_close();
}

void test_namei_root(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct inode *in = namei("/");

    CTEST_ASSERT(in != 0, "namei returns root inode");

    CTEST_ASSERT(in->inode_num == 0, "namei finds root");

    iput(in);

    image_close();
}

void test_directory_make(void)
{
    image_open("disk.img", 1);

    mkfs();

    directory_make("/foo");

    struct directory *dir = directory_open(0);
    struct directory_entry ent;

    directory_get(dir, &ent); // .
    directory_get(dir, &ent); // ..

    directory_get(dir, &ent); // foo

    CTEST_ASSERT(strcmp(ent.name, "foo") == 0, "directory foo created");

    directory_close(dir);

    image_close();
}

void test_namei_foo(void)
{
    image_open("disk.img", 1);

    mkfs();

    directory_make("/foo");

    struct inode *in = namei("/foo");

    CTEST_ASSERT(in != 0, "namei finds foo");

    iput(in);

    image_close();
}

void test_new_directory_contents(void)
{
    image_open("disk.img", 1);

    mkfs();

    directory_make("/foo");

    struct inode *in = namei("/foo");

    struct directory *dir = directory_open(in->inode_num);

    struct directory_entry ent;

    directory_get(dir, &ent);

    CTEST_ASSERT(strcmp(ent.name, ".") == 0, "foo contains .");

    directory_get(dir, &ent);

    CTEST_ASSERT(strcmp(ent.name, "..") == 0, "foo contains ..");

    directory_close(dir);
    iput(in);

    image_close();
}

void test_directory_make_bad_path(void)
{
    image_open("disk.img", 1);

    mkfs();

    CTEST_ASSERT(directory_make("foo") == -1,
        "directory_make rejects bad path");

    image_close();
}

void test_namei_bad(void)
{
    image_open("disk.img", 1);

    mkfs();

    struct inode *in = namei("/doesnotexist");

    CTEST_ASSERT(in == 0, "namei rejects bad path");

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
    test_iget();
    test_iput();
    test_directory();
    test_namei_root();
    test_directory_make();
    test_namei_foo();
    test_new_directory_contents();
    test_directory_make_bad_path();
    test_namei_bad();
    CTEST_RESULTS();
    CTEST_EXIT();
}