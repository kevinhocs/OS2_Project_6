#include <string.h>

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

int main(void)
{
    CTEST_VERBOSE(1);

    test_block_write_read();

    CTEST_RESULTS();

    CTEST_EXIT();
}