#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void do_find(const char* dir, const char* filename) {
    char buf[512], *p; // 因为要修改路径 需要在目录后面添加'/'
    int fd;
    struct stat st;
    struct dirent de;
    // printf("dir = %s\n", dir);
    if((fd = open(dir, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    if(st.type == T_DIR) {
        //遍历所有的条目
        strcpy(buf, dir);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)) {
            //读取磁盘块上的数据 每16字节读取一次 1024字节一共读取64次
            //如果某一次读取数据为空(从这以后没有文件名了) 它的de.inum为0
            
            //每次读磁盘块上的数据(就是看当前目录下有哪些文件)时 
            //总会有2个信息 '.' '..' 记录了当前目录的inode号和文件名 上层目录的inode号和文件名
            
            // printf("%s\n", dir);
            // printf("de.inum: %d   de.name: %s\n", de.inum, de.name);
            if(de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, "..") ) continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0) {
                fprintf(2, "find: cannot stat %s\n", buf);
                continue;
            }
            if(st.type == T_FILE) {
                if(strcmp(filename, de.name) == 0) {
                    // printf("%s\n", buf);
                    write(1, buf, strlen(buf));
                    write(1, "\n", 1);
                }
            }
            else if(st.type == T_DIR) {
                do_find(buf, filename);
            }
        }
    }

    close(fd);
}

int main(int argc, char* argv[])
{
    if(argc < 3) {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }
    // printf("%s\n", argv[1]);
    // printf("%s\n", argv[2]);
    do_find(argv[1], argv[2]);

    exit(0);
}