#include "namei.h"
#include "dir.h"
#include "fs_layout.h"

#include <string.h>

struct inode *namei(char *path)
{
    if (path[0] != '/') {
        return 0;
    }

    if (strcmp(path, "/") == 0) {
        return iget(ROOT_INODE_NUM);
    }

    struct directory *dir = directory_open(ROOT_INODE_NUM);

    struct directory_entry ent;

    while (directory_get(dir, &ent) == 0) {
        if (strcmp(ent.name, path + 1) == 0) {
            directory_close(dir);
            return iget(ent.inode_num);
        }
    }

    directory_close(dir);

    return 0;
}