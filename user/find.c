#include"kernel/types.h"
#include "kernel/stat.h"
#include"user/user.h"
#include"kernel/fs.h"


// char*fmtname(char* path)
// {
//     static char buf[DIRSIZ + 1];
//     char *p;
//     for(p = path+strlen(path);p >= path && *p != '/';p--){

//     }
//     p++;
//     if(strlen(p) >= DIRSIZ){
//         return p;
//     }
//     memmove(buf,p,strlen(p));
//     memset(buf+strlen(p),' ',DIRSIZ-strlen(p));
//     return buf;
// }
void find(char *path,char target[])
{
    char buf[512];
    char *p;
    int fd;
    struct dirent dir;
    struct stat st;

    if((fd = open(path,0)) < 0)
    {
        fprintf(2,"cannot open %s\n",path);
        return;
    }
    if(fstat(fd,&st) < 0)
    {
        fprintf(2,"cannot stat %s\n",path);
        close(fd);
        return;
    }

    if(st.type == T_DIR)
    {
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("path too long\n");
        }
        strcpy(buf,path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &dir, sizeof(dir)) == sizeof(dir))
        {
            if(dir.inum == 0)
                continue;
            memmove(p, dir.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0)
            {
                printf("cannot stat %s\n", buf);
                continue;
            }
            if(strcmp(target,p) == 0){
                printf("%s\n",buf);
            }else if(st.type == T_DIR && strcmp(".",p) != 0 && strcmp("..",p) != 0){
                find(buf,target);
            }
        }
    }
    close(fd);
}


int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("error to input\n");
        exit(0);
    }
    find(argv[1],argv[2]);
    exit(0);
}