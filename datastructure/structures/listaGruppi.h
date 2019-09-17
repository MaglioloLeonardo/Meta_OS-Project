#ifndef LISTAGRUPPI
#define LISTAGRUPPI

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PERMISSION 0775

//_________________________Struttura_gruppo_____________
struct gruppo{
    unsigned int isUsed;//if isUsed == 0 allora il gruppo non è utilizzato
    unsigned int chiuso; 
    unsigned int pid[4];
};
//Possono utilizzare la struttura gruppo o listagruppi (leggere o scrivere) solo un processo per volta
//_________________________________________________________


//___________________Segmento_Dichiarazioni_________________________
struct gruppo *listaGruppi(key_t, int n); //funzione restituisce puntatore a nuova lista gruppi lunga n allocata dinamicamente
struct gruppo *attachGruppi(key_t, int n);//funzione annette una struttura dati gruppo con n e key medesimi, ritorna 0 se la struttura non esiste

void grupAdd(struct gruppo *lista, struct gruppo *elem, int n);//funzione inserisce l'elemento elem nell'ultima posizione della lista lunga n
                                                               //E' vietato aggiungere manualmente elementi alla lista tramite operazioni su puntatori, utilizzare tassativamente questa funzione

struct gruppo *grupFindPid(struct gruppo *lista, int pid, int n);//funzione restituisce un puntatore al gruppo (in lista lunga n) che contiene il pid cercato altrimenti -1

int grupGetIndex(struct gruppo *lista, struct gruppo *elem);//funzione restituisce l'index dell'elemento gruppo immesso dentro la lista list, ritorna -1 se lista o gruppo sono null
                                                            //se elem non è elemento della lista allora il risultato è incongruente, deve esserlo tassativamente.

struct gruppo *grupGetLast(struct gruppo *lista, int n);//funzione restituisce il gruppo in ultima posizione di una lista lunga n;
void gruppoInizialize(struct gruppo *lista, int n);//funzione inizializza ogni membro della lista gruppi lunga n
void grupInsertPid(struct gruppo *elem, int toInsert);
int grupPidLen(struct gruppo *elem);
void printGrup(struct gruppo *lista, int n); 
//__________________________________________________________________

//_________________________Metodo_Inizializzazione_Lista____________
struct gruppo *listaGruppi(key_t key, int n){
    if(n > 0){
        int shmid;
        void *toCast;
        struct gruppo *grppointer;
        shmid = shmget(key, sizeof(struct gruppo) * n, IPC_CREAT | PERMISSION);
        if(shmid == -1){
           printf("\nAttempt to create list with key: %p wide bytes: %d\n", key, sizeof(struct gruppo)*n);
           printf("\n shmid error: probabilente esiste già una ipc generata con quella key ma con size byte diversa.\n Per eliminarla si vedano i comandi linux: 'ipcs shows' e 'ipcrm'");
           system("ipcs shows");
           exit(1);
        }
        toCast = shmat(shmid, NULL, 0);
        grppointer = (struct gruppo *) toCast;
        gruppoInizialize(grppointer, n);
        return grppointer;
    }else{
        printf("listaGruppi: N Must be > 0");
        return 0;
    }
    
}


struct gruppo *attachGruppi(key_t key, int n){
    if(n > 0){
        int shmid;
        void *toCast;
        struct gruppo *grppointer;
        shmid = shmget(key, sizeof(struct gruppo) * n, PERMISSION);
        if(shmid == -1){
           return 0;
        }
        toCast = shmat(shmid, NULL, 0);
        grppointer = (struct gruppo *) toCast;
        return grppointer;
    }else{
        printf("attachGruppi: N Must be > 0");
        return 0;
    }
    
}
//____________________________________________________________________


//________________________Metodi_Op._Su_Lista__________________________

void grupAdd(struct gruppo *lista, struct gruppo *elem, int n){
    struct gruppo *last = grupGetLast(lista, n);
    last->isUsed = 1;
    last->pid[0] = elem->pid[0];
    last->pid[1] = elem->pid[1];
    last->pid[2] = elem->pid[2];
    last->pid[3] = elem->pid[3];
    last->chiuso = elem->chiuso;
}

struct gruppo *grupFindPid(struct gruppo *lista, int pid, int n){
    int i;
    for(i=0; i<n; i++){
               if(lista[i].pid[0] == pid || lista[i].pid[1] == pid || lista[i].pid[2] == pid || lista[i].pid[3] == pid){
            return lista+i;
        }
    }
    return -1;
}

int grupGetIndex(struct gruppo *lista, struct gruppo *elem){
    if(lista == 0 || elem == 0)return -1;
    return ((int)elem-(int)lista) / sizeof(struct gruppo);
}

void grupInsertPid(struct gruppo *elem, int toInsert){
    int i;
    if(elem->isUsed==0)printf("ALLERT grupNumOfPids: isUsed == 0");
    for(i=0; i<4; i++){
        if(elem->pid[i] == 0){
            elem->pid[i] = toInsert;
            break;
        }
    }
    //if(i == 4 && elem->pid[i] != toInsert)perror("grupInsertPid: PID IN FULL");
}

int grupPidLen(struct gruppo *elem){
    int i,j=0;
    for(i=0; i<4; i++){
        if(elem->pid[i] != 0)j++;
    }
    return j;
}
//_______________________________________________________________________

//__________________________Metodi_A_Scopo_Implementazione_______________
void printGrup(struct gruppo *lista, int n){
    struct gruppo *last = grupGetLast(lista, n);
    int i = 0;
    while(lista != last){
        printf("\ni: %d pid0: %d pid1: %d pid2: %d pid3: %d chiuso: %u|\n ", i,lista->pid[0], lista->pid[1], lista->pid[2], lista->pid[3], lista->chiuso);
        i++; lista++;
    }
    printf("\ni: %d pid0: %d pid1: %d pid2: %d pid3: %d chiuso: %u|\n", i,lista->pid[0], lista->pid[1], lista->pid[2], lista->pid[3], lista->chiuso);
}

struct gruppo *grupGetLast(struct gruppo *lista, int n){
    int i;
    for(i=0; i<n; i++){
        if(lista[i].isUsed == 0){
            return lista+i;
        }
    }
    printf("grupGetLast: List is full. Returning lista+(n-1)");
    return lista+(n-1);
}

void gruppoInizialize(struct gruppo *lista, int n){
    int i;
    for(i=0; i<n; i++){
        lista[i].isUsed = 0;
        lista[i].pid[0] = 0;
        lista[i].pid[1] = 0;
        lista[i].pid[2] = 0;
        lista[i].pid[3] = 0;
        lista[i].chiuso = 0;
    }
}
//________________________________________________________________________
#endif