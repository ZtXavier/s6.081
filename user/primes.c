#include"kernel/types.h"
#include "kernel/stat.h"
#include"user/user.h"

void prime(int*fd)
{
    int pnum;
    // 首先第一个读到的一定是素数
    int ret = read(fd[0],&pnum,sizeof(pnum));
    int fd_next[2];
    // 递归结束条件
    if(ret == 0)
    {
        close(fd[0]);
        close(fd[1]);
        exit(1);
    }
    else
    {
        printf("prime %d\n",pnum);
    }

    pipe(fd_next);
    int num;

    int pid = fork();
    
    if(pid > 0)
    {
        close(fd_next[0]);
        while(read(fd[0],&num,sizeof(num)))
        {
           if(num % pnum != 0)
           {
               write(fd_next[1],&num,sizeof(num));
           }
        }
        close(fd_next[1]);
        wait((int*)0);
        exit(0);
    }
    else if(pid == 0)
    {  
        close(fd[0]);
        close(fd_next[1]);
        prime(fd_next);
        close(fd_next[0]);
        exit(0);
    }
}


int main()
{
    int fd[2];
    pipe(fd);
    int i;
    for(i = 2;i <= 35;i++)
    {
        write(fd[1],&i,sizeof(i));
    }

    int pid = fork();
    if(pid == 0)
    {
        close(fd[1]);
        prime(fd);
        exit(0);
    }
    else if(pid > 0)
    {
        close(fd[1]);
        close(fd[0]);
        wait((int*)0);
        exit(0);
    }
    printf("OK\n");
    exit(0);
}