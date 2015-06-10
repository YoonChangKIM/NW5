//ID: 20113267
//Name: YoonChang KIM

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define MAX_STRING 100
#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

int num = 0;
int test = 0;
int wqlength = 0; //Waiting queue의 카운터.

typedef union _semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
} semun;

int initsem (key_t semkey, int n)
{
    int status = 0, semid;
    if ((semid = semget (semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
    {
        if (errno == EEXIST)
            semid = semget (semkey, 1, 0);
    }
    else
    {
        semun arg;
        arg.val = n;
        status = semctl(semid, 0, SETVAL, arg);
    }
    if (semid == -1 || status == -1)
    {
        perror("initsem failed");
        return (-1);
    }
    return (semid);
}

int p (int semid)
{
    struct sembuf p_buf;
    p_buf.sem_num = 0;
    p_buf.sem_op = -1;
    p_buf.sem_flg = SEM_UNDO;
    if (semop(semid, &p_buf, 1) == -1)
    {
        perror ("p(semid) failed");
        exit(1);
    }
    return (0);
}

int v (int semid)
{
    struct sembuf v_buf;
    v_buf.sem_num = 0;
    v_buf.sem_op = 1;
    v_buf.sem_flg = SEM_UNDO;
    if (semop(semid, &v_buf, 1) == -1)
    {
        perror ("v(semid) failed");
        exit(1);
    }
    return (0);
}

void _wait(int c1, int semid){
    wqlength++;
    v(semid);
    p(c1);
    p(semid);
}

void _signal(int c1){
    if(wqlength > 0){
        v(c1);
        wqlength--;
    }
}

void add1()
{
    FILE * fp = fopen("/Users/mac/Documents/input.txt", "r");
    
    if ( fp == NULL ) {
        printf("파일을 열 수 없습니다!\n");
    }
    char line[MAX_STRING];
    int length;
    int h;
    int result = 0;
    
    while(!feof(fp)){
        fgets(line, MAX_STRING, fp);
    }
    
    length = strlen(line);
    
    for(h = 0; h < length - 1; h++)
        result = result * 10 + (line[h] - '0');
    fclose(fp);
    
    fp = fopen("/Users/mac/Documents/input.txt", "a");
    fprintf(fp, "%d\n", result+1);
    fclose(fp);
}

void AddToQ(int a, int semid, int c1){
    p(semid);
    num += a;
    _signal(c1);
    v(semid);
}

void RemoveFQ(int a, int semid, int c1){
    p(semid);
    while(num == 0){
        _wait(c1, semid);
    }
    num -= a;
    v(semid);
}

int main()
{
    key_t semkey = 0x200; // 서버에서 작업할 때는 자기 학번 등을 이용하여 다른 사람의 키와 중복되지 않게 해야 함
    key_t semkey2 = 0x201;
    int semid;              //semid 는 semid을 뜻한다.
    int c1;
    pid_t pid;
    int queue[100];
    int i;
    
    pid = getpid();
    if ((semid = initsem(semkey,1)) < 0)    // 세마포를 연결한다.(없으면 초기값을 1로 주면서 새로 만들어서 연결한다.)
        exit(1);
    if ((c1 = initsem(semkey2,0)) < 0)     // c1 세마포 연결한다.
        exit(1);
    // prinff("\nprocess %d before critical section\n", pid);
    /*
    for(i = 0; i < 1000; i++){
        p(semid);   // semid Acquire()
        add1();
        printf("Length : %d\n", length);
        // printf("process %d leaving critical section\n", pid);
        v(semid);   // semid Release()
    }*/
    for(i = 1; i < 10; i++){
        p(semid);
        test += i;
        _signal(c1);
        v(semid);
        
        p(semid);
        while(test == 0){
            _wait(c1, semid);
        }
        test -= i;

        v(semid);

    }
    
    printf("process %d exiting\n",pid);
    exit(0);
    
    return 0;
}