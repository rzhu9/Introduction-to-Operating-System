#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "fs.h"

#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Special device

// xv6 fs img
// similar to vsfs
// superblock | inode tabe | bitmap(data) | data blocks
// (some gaps in here)
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <image-file>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "image not found.\n");
        exit(1);
    }
    struct stat stat;
    fstat(fd, &stat);
    char *data = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    int ret = fsck(data, stat.st_size);
    munmap(data, stat.st_size);
    close(fd);
    return ret;
}

int check_parent(char *data, int inode, struct dinode *dn) {
    int i = 0, j = 0;
    struct dirent *dir;
    for (i = 0; i < NDIRECT + 1; i++) {
        if (dn->addrs[i] == 0) {
            break;
        }
        if (i < NDIRECT) {
            dir = (struct dirent *)(data + 512 * dn->addrs[i]);
            for (j = 0; j < 512 / sizeof(struct dirent); j++) {
                if (dir[j].inum == inode) {
                    return 0;
                }
            }
        }
    }
    if (i == NDIRECT + 1) {
        uint *b = (uint *)(data + 512 * dn->addrs[NDIRECT]);
        for (i = 0; i < NINDIRECT; i++) {
            if (b[i] == 0) {
                break;
            }
            dir = (struct dirent *)(data + 512 * dn->addrs[b[i]]);
            for (j = 0; j < 512 / sizeof(struct dirent); j++) {
                if (dir[j].inum == inode) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int check_direction(char *data, int *inodeused, int *links, int inode, struct dinode *dinode) {
    
    int i = 0, j = 0;
    struct dinode *dinode1;
    struct dirent *dir;
    int flag1 = 0;
    int flag2 = 0;
    for (i = 0; i < NDIRECT + 1; i++) {
        if (dinode->addrs[i] == 0) {
            break;
        }
        if (i < NDIRECT) {
            dir = (struct dirent *)(data + 512 * dinode->addrs[i]);
            for (j = 0; j < 512 / sizeof(struct dirent); j++) {
                if (dir[j].inum == 0) {
                    continue;
                }
                inodeused[dir[j].inum] -= 1;
                dinode1 = (struct dinode *)(data + 512 * 2 + sizeof(struct dinode) * dir[j].inum);
                if (strcmp(dir[j].name, "..") == 0) {
                    flag1 = 1;
                    if (check_parent(data, inode, dinode1)) {
                        fprintf(stderr, "ERROR: parent directory mismatch.\n");
                        return 1;
                    }
                    if (inode == 1) {
                        if (dir[j].inum != 1) {
                            fprintf(stderr, "ERROR: root directory does not exist.\n");
                            return 1;
                        }
                    } else {
                        if (dir[j].inum == inode) {
                            fprintf(stderr, "ERROR: parent directory mismatch.\n");
                            return 1;
                        }
                    }
                    
                } else if (strcmp(dir[j].name, ".") == 0) {
                    flag2 = 1;
                } else {
                    links[dir[j].inum] += 1;
                }
                if (dinode1->type == 0) {
                    fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
                    return 1;
                }
            }
        }
    }
    if (i == NDIRECT + 1) {
        uint *b = (uint *)(data + 512 * dinode->addrs[NDIRECT]);
        for (i = 0; i < NINDIRECT; i++) {
            if (b[i] == 0) {
                break;
            }
            dir = (struct dirent *)(data + 512 * dinode->addrs[b[i]]);
            for (j = 0; j < 512 / sizeof(struct dirent); j++) {
                if (dir[j].inum == 0) {
                    continue;
                }
                inodeused[dir[j].inum] -= 1;
                dinode1 = (struct dinode *)(data + 512 * 2 + sizeof(dinode) * dir[j].inum);
                if (strcmp(dir[j].name, "..") == 0) {
                    flag1 = 1;
                    if (check_parent(data, inode, dinode1)) {
                        fprintf(stderr, "ERROR: parent directory mismatch.\n");
                        return 1;
                    }
                    if (inode == 1 && dir[j].inum != 1) {
                        fprintf(stderr, "ERROR: root directory does not exist.\n");
                        return 1;
                    }
                } else if (strcmp(dir[j].name, ".") == 0) {
                    flag2 = 1;
                } else {
                    links[dir[j].inum] += 1;
                }

                if (dinode1->type == 0) {
                    fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
                    return 1;
                }
            }
        }
    }
    if (flag1 == 0 || flag2 == 0) {
        fprintf(stderr, "ERROR: directory not properly formatted.\n");
        return 1;
    }
    return 0;
}

int fsck(char *data, size_t size) {
    // read superblock
    struct superblock sb;
    memcpy(&sb, data + 512, sizeof(sb));
    struct dinode *dinode;
    int i = 0; 
    int *usedblock = calloc(sb.size, sizeof(int));
    for (i = 0; i < sb.ninodes / IPB + 3 + sb.size/(512*8) + 1; i++) {
        usedblock[i] = 1;
    }
    int *links = calloc(sb.ninodes, sizeof(int));
    links[1] = 1;
    int *inodeused = calloc(sb.ninodes, sizeof(int));
    int inode;
    for (inode = 0; inode < sb.ninodes; inode++) {
        dinode = (struct dinode *)(data + 512 * 2 + sizeof(struct dinode) * inode);
        if (inode == 1 && dinode->type != T_DIR) {
            fprintf(stderr, "ERROR: root directory does not exist.\n");
            return 1;
        }
        if (dinode->type == 0) {
            // unused
            continue;
        }
        inodeused[inode] += 1;
        links[inode] -= dinode->nlink;
        if (dinode->type != T_DIR && dinode->type != T_FILE && dinode->type != T_DEV) {
            fprintf(stderr, "ERROR: bad inode.\n");
            return 1;
        }
        for (i = 0; i < NDIRECT + 1; i++) {
            if (dinode->addrs[i] == 0) {
                break;
            }
            if (dinode->addrs[i] >= sb.size) {
                fprintf(stderr, "ERROR: bad address in inode.\n");
                return 1;
            }
            usedblock[dinode->addrs[i]] += 1;
        }
        if (i == NDIRECT + 1) {
            // indirect
            uint *b = (uint *)(data + 512 * dinode->addrs[NDIRECT]);
            for (i = 0; i < NINDIRECT; i++) {
                if (b[i] == 0) {
                    break;
                }
                if (b[i] >= sb.size) {
                    fprintf(stderr, "ERROR: bad address in inode.\n");
                    return 1;
                }
                usedblock[b[i]] += 1;
            }
        }
        if (dinode->type == T_DIR) {
            if (check_direction(data, inodeused, links, inode, dinode)) {
              return 1;
            }
            if (dinode->nlink != 1) {
                fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
                return 1;
            }
        }
    }
    // check usedblock
    char *c = data + 512 * (sb.ninodes / IPB + 3);
    for (i = 0; i < sb.size; i++) {
        if (usedblock[i] > 1) {
            fprintf(stderr, "ERROR: address used more than once.\n");
            return 1;
        }
        int bitmap = ((c[i/8] >> (i % 8)) & 1);
        if (bitmap == usedblock[i]) {
            continue;
        }
        if (bitmap == 0) {
            fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
        } else {
            fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
        }
        return 1;
    }
    for (i = 0; i < sb.ninodes; i++) {
        if (inodeused[i] > 0) {
            fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
            return 1;
        }
        if (links[i]) {
            dinode = (struct dinode *)(data + 512 * 2 + sizeof(struct dinode) * i);
            if (dinode->type == T_DIR) {
                fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
            } else {
                fprintf(stderr, "ERROR: bad reference count for file.\n");
            }
            return 1;
        }
    }
    return 0;
}
