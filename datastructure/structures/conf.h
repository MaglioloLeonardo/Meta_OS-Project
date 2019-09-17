#ifndef CONF
#define CONF

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

#define PERMISSION 0775
//_________________________Struttura_conf_____________
struct conf{
    unsigned int typeOfGroups[4];
    unsigned int nof_invites;
    unsigned int max_reject; 
};
//_________________________________________________________


//___________________Segmento_Dichiarazioni_________________________
struct conf *getConf(key_t key); //funzione restituisce puntatore a nuovo conf allocato dinamicamente
struct conf *attachConf(key_t key);
void confInizialize(struct conf *elem, FILE *fPointer); 
//__________________________________________________________________

//_________________________Metodi_Conf____________
struct conf *getConf(key_t key){
        int shmid;
        void *toCast;
        struct conf *confPointer;
        shmid = shmget(key, sizeof(struct conf), IPC_CREAT | PERMISSION);
        if(shmid == -1){
           printf("\nAttempt to create list with key: %p wide bytes: %d\n", key, sizeof(struct conf));
           printf("\n shmid error: probabilente esiste già una ipc generata con quella key ma con size byte diversa.\n Per eliminarla si vedano i comandi linux: 'ipcs shows' e 'ipcrm'");
           system("ipcs shows");
           exit(1);
        }
        toCast = shmat(shmid, NULL, 0);
        confPointer = (struct conf *) toCast;
        return confPointer;
}

struct conf *attachConf(key_t key){
        int shmid;
        void *toCast;
        struct conf *confPointer;
        shmid = shmget(key, sizeof(struct conf), PERMISSION);
        if(shmid == -1){
           printf("\nAttempt to attach list with key: %p wide bytes: %d\n", key, sizeof(struct conf));
           printf("\n shmid error: probabilente esiste già una ipc generata con quella key ma con size byte diversa.\n Per eliminarla si vedano i comandi linux: 'ipcs shows' e 'ipcrm'");
           system("ipcs shows");
           exit(1);
        }
        toCast = shmat(shmid, NULL, 0);
        confPointer = (struct conf *) toCast;
        return confPointer;
}


void confInizialize(struct  conf *thisConf, FILE *fPointer){
	int i, j,  bufflen = 50;
	char singleLine[bufflen];
	
	for(i = 0; i<=5; i++){
		int val_len = 0, val_bufflen = 10;
		char value[val_bufflen];
		
		for(j = 0; j<=val_bufflen; j++){
			value[j] = j;
		}
		j = 0;

		fgets(singleLine, bufflen, fPointer);
		while(singleLine[j] != '=' && j<bufflen){
			j++;
		}

		j++;

		while(singleLine[j] == ' '&& j<bufflen){
			j++;
		}

		while(singleLine[j] >= '0' &&  singleLine[j]<='9'&& j<bufflen && val_len<val_bufflen){
			value[val_len] = singleLine[j];
			j++;
			val_len++;
		}

		if(atoi(value) < 0){
			printf("ERROR confInizialize: out of bufferlen\n");
			exit(-1);
		}
		
		if(i<4){
			thisConf->typeOfGroups[i] = atoi(value);
			//printf("%s\n", value);
		}else if(i == 4){
			thisConf->nof_invites = atoi(value);
		}else if (i == 5){
			thisConf->max_reject = atoi(value);
		}

	}
	
	fclose(fPointer);
}
//__________________________________________________________________
#endif