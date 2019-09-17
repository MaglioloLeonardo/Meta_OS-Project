#ifndef SEMAPHORE 
#define SEMAPHORE  

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>
#define PERMISSION 0775

int getSemaphore(int key);
int setSemaphore(int semid);
int Wait_Sem (int id_sem);
int Signal_Sem (int id_sem);


int getSemaphore(int key){
    int ctl = semget(key, 1, IPC_CREAT | PERMISSION); // l'indice dei semafori parte da 0
    if(ctl == -1){
        perror("\nERROR getSemaphore: Can't get semid by semget. Try to use 'ipcs shows' and 'ipcs ipcrm'.\n");
        exit(1);
    }
    return ctl;
}

int setSemaphore(int semid){
    int ctl = semctl(semid, 0, SETVAL, 1);
    if(ctl == -1){
        perror("\nERROR setSemaphore: Can't set value by semctl. Try to use 'ipcs shows' and 'ipcs ipcrm'.\n");
        exit(1);
    }
    return ctl;
}

int setSemaphoreTo0(int semid){
    int ctl = semctl(semid, 0, SETVAL, 0);
    if(ctl == -1){
        perror("\nERROR setSemaphore: Can't set value by semctl. Try to use 'ipcs shows' and 'ipcs ipcrm'.\n");
        exit(1);
    }
    return ctl;
}

int Wait_Sem (int id_sem) {
    int numsem = 0;
    struct sembuf sem_buf;
    sem_buf.sem_num=numsem;
    sem_buf.sem_flg=0;
    sem_buf.sem_op=-1;
    return semop(id_sem,&sem_buf,1); //semaforo rosso
}
int Signal_Sem (int id_sem) {
    int numsem = 0;
    struct sembuf sem_buf;
    sem_buf.sem_num=numsem;
    sem_buf.sem_flg=0;
    sem_buf.sem_op=1;
    return semop(id_sem,&sem_buf,1); //semaforo verde
}

#endif