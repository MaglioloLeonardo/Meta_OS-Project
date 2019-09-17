#ifndef LISTASTUDENTI
#define LISTASTUDENTI

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PERMISSION 0775

//_________________________Struttura_studente_____________
struct studente{
    unsigned int pid;
    unsigned int matricola;
    unsigned int voto_AdE;
    unsigned int nof_elems;
    unsigned int grupOwner; //Se appartiene a un gruppo indica chi è il proprietario di esso.
    unsigned int voto;
};
//_________________________________________________________


//___________________Segmento_Dichiarazioni_________________________
struct studente *listaStudenti(key_t key, int n); //funzione restituisce un puntatore a lista con n studenti
struct studente *attachStudenti(key_t, int n);//funzione annette una struttura dati studente con n e key medesimi, ritorna 0 se la struttura non esiste

struct studente *studentGetPos(struct studente *lista, int n);//funzione restituisce un puntatore all'n-esimo studente della lista
void studentsPrint(struct studente *lista, int size);
void studentAdd(struct studente *lista, struct studente *elem, int n);
struct studente *studentFindPid(struct studente *lista, int pid, int n);

void studentInizialize(struct studente *lista, int n);//funzione inizializza ogni membro della lista studenti lunga n
struct studente *studentGetLast(struct studente *lista, int n);

//__________________________________________________________________

//_________________________Metodo_Inizializzazione_Lista____________

struct studente *listaStudenti(key_t key, int n){
    if(n > 0){
        int shmid;
        void *toCast;
        struct studente *stdpointer;
        shmid = shmget(key, sizeof(struct studente) * n, IPC_CREAT | PERMISSION);
        if(shmid == -1){
           printf("\nAttempt to create list with key: %p wide bytes: %d\n", key, sizeof(struct studente)*n);
           printf("\n shmid error: probabilente esiste già una ipc generata con quella key ma con size byte diversa.\n Per eliminarla si vedano i comandi linux: 'ipcs shows' e 'ipcrm'");
           system("ipcs shows");
           exit(1);
        }
        toCast = shmat(shmid, NULL, 0);
        stdpointer = (struct studente *) toCast;
        studentInizialize(stdpointer, n);
        return stdpointer;
    }else{
        printf("listaStudenti: N Must be > 0");
        return 0;
    }
    
}


struct studente *attachStudenti(key_t key, int n){
    if(n > 0){
        int shmid;
        void *toCast;
        struct studente *stdpointer;
        shmid = shmget(key, sizeof(struct studente) * n, PERMISSION);
        if(shmid == -1){
            printf("\nAttempt to attach list with key: %p wide bytes: %d\n", key, sizeof(struct studente)*n);
           printf("\n shmid error: probabilente esiste già una ipc generata con quella key ma con size byte diversa.\n Per eliminarla si vedano i comandi linux: 'ipcs shows' e 'ipcrm'");
           system("ipcs shows");
           exit(1);
           return 0;
        }
        toCast = shmat(shmid, NULL, 0);
        stdpointer = (struct studente *) toCast;
        return stdpointer;
    }else{
        printf("attachStudenti: N Must be > 0");
        return 0;
    }
    
}
//____________________________________________________________________


//________________________Metodi_Op._Su_Lista__________________________
struct studente *studentGetPos(struct studente *lista, int n){
    int i;
    for(i=0; i<n; i++){
        lista++;
    }
    return lista;
}

void studentAdd(struct studente *lista, struct studente *elem, int n){
    struct studente *last = studentGetLast(lista, n);
    last->pid = elem->pid;
    last->matricola = elem->matricola;
    last->voto_AdE = elem->voto_AdE;
    last->nof_elems = elem->nof_elems;
    last->voto = elem->voto;
}

struct studente *studentFindPid(struct studente *lista, int pid, int n){
    int i;
    for(i=0; i<n; i++){
        if(lista[i].pid == pid){
            return lista+i;
        }
    }
    return -1;
}


void studentsPrint(struct studente *lista, int size){
    int i=0;
    if(lista == 0){
        printf("signPrint: Empty list");
    }else{                                                                      
        while(i<size && lista[i].pid != 0){
            printf("\n|Elem n: %d pid: %d matricola: %d voto_AdE: \n%d nof_elems: %d grupOwner: %d voto: %d  \n\n|", i, lista[i].pid, lista[i].matricola, lista[i].voto_AdE, lista[i].nof_elems, lista[i].grupOwner,lista[i].voto);
            i++;
        }
   }
}


//_______________________________________________________________________

//______________________Metodi_implementazione___________________________


struct studente *studentGetLast(struct studente *lista, int n){
    int i;
    for(i=0; i<n; i++){
        if(lista[i].pid == 0){
            return lista+i;
        }
    }
    printf("studentGetLast: List is full. Returning lista+(n-1)");
    return lista+(n-1);
}


void studentInizialize(struct studente *lista, int n){
    int i;
    for(i=0; i<n; i++){
        lista[i].pid = 0;
        lista[i].matricola = 0;
        lista[i].voto_AdE = 0;
        lista[i].nof_elems = 0;
        lista[i].voto = 0;
    }
}
//_______________________________________________________________________
#endif