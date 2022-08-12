#include"kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc,char *argv[])
{
    int ffd[2];
    int chfd[2];

    pipe(ffd);
    pipe(chfd);
    char buf[2];
    if(!fork())
    { 
         close(chfd[1]);
        close(ffd[0]);
        read(chfd[0],buf,1);
        write(ffd[1],"a",1);
        printf("%d: received ping\n",getpid());
        exit(0);
    }
    else
    {
       close(chfd[0]);
        close(ffd[1]);
        write(chfd[1],'a',1);
        wait((int*)0);
        read(ffd[0],buf,1);
        printf("%d: received pong\n",getpid());
        exit(0);
    }

        // close(chfd[0]);
        // close(ffd[1]);
        // close(chfd[1]);
        // close(ffd[0]);
    
    exit(0);
}