#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void rec_find(char* filename, char* path) {
    char buf[512], *p;
    int fd;
    struct dirent de;  // directory
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) {
        fprintf(2, "usage: find <DIRECTORY> <filename>\n");
        return;
    }

    // search file in the curent dir path
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("ls: path too long\n");
    }
    strcpy(buf, path);
    p = buf + strlen(buf);  // p starting at the end of buf
    *p++ = '/';             // add a / to the end of cur dir
    // search in the cur dir
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0) {
            printf("ls: cannot stat %s\n", buf);
            continue;
        }

        if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
            rec_find(filename, buf);
        } else if (strcmp(filename, p) == 0) {
            printf("%s\n", buf);
        }
    }

    // search the sub dirs recursively (!.)
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(2, "Correct usage: find <directory> <filename>\n");
        exit(1);
    }
    char* path = argv[1];
    char* filename = argv[2];

    rec_find(filename, path);
    exit(0);
}
