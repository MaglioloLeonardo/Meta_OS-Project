#ifndef COMUNICAZIONI
#define COMUNICAZIONI

#include <stdio.h>
//_________________________Strutture_____________________
struct segnale{
    unsigned int fromPid;
    unsigned int sig;
    struct segnale *next;
};

struct comunicazioni{
    struct segnale *listPendingAggToSend; //RICEVUTE
    struct segnale *listPendingAggToRec;  //INVIATE
    struct segnale *fifoQueue;
};
//_______________________________________________________

//___________________Segmento_Dichiarazioni_________________________
struct segnale *getSegnale(); //funzione restituisce puntatore a nuovo segnale allocato dinamicamente
struct comunicazioni *getComunicazioni(); //funzione restituisce puntatore a nuovo comunicazioni allocato dinamicamente 
struct segnale *signGetPos(struct segnale *lista, int n);//funzione restituisce un puntatore all'n-esimo segnale della lista
void *signAdd(struct segnale *lista, struct segnale *elem);//funzione inserisce l'elemento segnale nell'ultima posizione
struct segnale *signAddFifo(struct segnale *lista, struct segnale *elem, int buffer_lentgh);//Commenti alla riga della funzione
struct segnale *signRemovePos(struct segnale *lista, int n);//funzione rimuove il segnale in n-esima posizione restituendo la nuova lista
int signFind(struct segnale *lista, int pid, int sig);//Commenti alla riga della funzione

struct segnale *signGetLast(struct segnale *lista);//funzione restituisce il segnale in ultima posizione
int signGetLength(struct segnale *lista);//funzione restituisce la lunghezza della lista
void signPrint(struct segnale *lista);//funzione stampa la lista di segnali impostata per parametro
//__________________________________________________________________

//_________________________Metodi_Inizializzazione__________________
struct segnale *getSegnale(){
    struct segnale *temp = (struct segnale *)malloc(sizeof(struct segnale));
    temp->sig = 0; temp->fromPid = 0; temp->next = 0;
    return temp;

}

struct comunicazioni *getComunicazioni(){
    struct comunicazioni *temp =  (struct comunicazioni *)malloc(sizeof(struct comunicazioni));
    temp->listPendingAggToSend = getSegnale();
    temp->listPendingAggToRec = getSegnale();
    temp->fifoQueue = getSegnale();
    return temp;
}
//___________________________________________________________________

//___________________Metodi_Su_Liste_Aggregazione____________________
struct segnale *signGetPos(struct segnale *lista, int n){
    int i;
    for(i=0; i<n; i++){
        if(lista->next == 0){
            perror("-signGetPos-N out of bounds on listaSegnali");
        }
        lista = lista->next;
    }
    return lista;
}

void *signAdd(struct segnale *lista, struct segnale *elem){
    struct segnale *last = signGetLast(lista);
    last->next = elem;    
}
 
//funzione inserisce l'elemento segnale in posizione 0, rimuovendo l'ultimo segnale se : buffer_lentgh == della lunghezza della lista.
//buffer_lentgh DEVE essere >= lunghezza della lista.
struct segnale *signAddFifo(struct segnale *lista, struct segnale *elem, int buffer_lentgh){
    int len = signGetLength(lista);
    if(len < buffer_lentgh){
        if(lista == 0){
            perror("Use getSegnale insted of sinAddFifo if list's length is equal to zero ");
        }else{
            elem->next = lista;
            return elem;
        }
    }else if(buffer_lentgh == len){
        if(len == 0){
            perror("AddFifo Cannot add elem to empty list");
        }else if(len == 1){
            return elem;
        }else if(len == 2){
            elem->next = lista;
            return elem;
        }else{
            elem->next = lista;
            lista = elem;
            signGetPos(lista, len-1)->next = 0; //il penultimo elemento diventa l'ultimo
            return lista;
        }
    }else{
        perror("singAddFifo: N must be between >= length of the list");
    }    
}

struct segnale *signRemovePos(struct segnale *lista, int n){
    int len= signGetLength(lista)-1;
    if(n==0){
        struct segnale *temp = signGetPos(lista, 1);
        //free(signGetPos(lista, 0));
        return temp;
    }else if(n>0 && n<len){
        struct segnale *temp = signGetPos(lista, n-1);
        struct segnale *temp1 = signGetPos(lista, n+1);
        //free(temp->next);
        temp->next = temp1;
        return lista;

    }else if(n==len){
        struct segnale *temp = signGetPos(lista, len-1);
        //free(temp->next);
        temp->next = 0;
        return lista;
    }else{
        perror("-signRemovePos-N out of bounds on listaSegnali");
    }
}

//signFind() restitusce un intero compreso tra: 0 e la 'lunghezza della lista passata'-1 se 
//esiste un segnale sign emesso da pid all'interno di lista, che rappresenta la sua posizione all'interno della lista.
//Restitusce la posizione del primo elemento che trova partendo da 0.
//Nel caso in cui il segnale non venga trovato restitusce invece -1.
int signFind(struct segnale *lista, int pid, int sig){
    if(lista == 0){
        perror("lista > 0");
        return -1;
    }else{
        int counter = 0;
        while(lista->next != 0){
            if(lista->fromPid == pid && lista->sig == sig ){
                return counter;
            }
            lista = lista->next;
            counter++;
        }
        if(lista->fromPid == pid && lista->sig == sig ){
            return counter;
        }else{
            return -1;
        }
    }
}

//___________________________________________________________________



//___________________Metodi_per_implementazione_______________________
void signPrint(struct segnale *lista){
    struct segnale *temp = lista;
    int i=0;
    if(lista == 0){
        perror("signPrint: Empty list");
    }else{
        while(temp->next != 0){
            printf("\n|Elem n: %d fromPid: %d sign: %d next: %p", i, temp->fromPid, temp->sig, temp->next );
            i++;
            temp = temp->next;
        }
         printf("\n|Elem n: %d fromPid: %d sign: %d next: %p", i, temp->fromPid, temp->sig, temp->next );
    }
}

struct segnale *signGetLast(struct segnale *lista){
    if(lista == 0)return 0;
    while(lista != 0 && lista->next != 0){
        lista = lista->next;
    }
    return lista;
}

int signGetLength(struct segnale *lista){
    if(lista == 0)return 0;
    int i = 1;
    while(lista != 0 && lista->next != 0){
        lista = lista->next;
        i++;
    }
    return i;
}
//_____________________________________________________________________
#endif