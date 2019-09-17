#ifndef SIGNALS
#define SIGNALS 

#include <stdio.h>
#include <signal.h>
#include "structures/comunicazioni.h"


#define FIFOBUFFERLEN 1000000 //lunghezza massima del buffer FIFO contenuto in comunicazioni.

/*_____________________________NOTE_IMPORTANTI________________________
/DICHIARARE UNA STRUCT COMUNICAZIONI 'scomunicazioni'IN SPAN PUBBLICO
 E INIZIALIZZARE LA STRUTTURA PRIMA DI CHIAMARE setHandler()
_______________________________________________________________________*/
struct comunicazioni *scomunicazioni;



//___________________SEGNALI_____________________
#define SIGREADY 57
#define SIGSTART 58
#define SIGAGGREGAZIONE 59
#define SIGCONFERMA 60
#define SIGRIFIUTO 61
#define SIGVOTO 62
#define SIGCONCLUSIONE 63
//_________________________________________________

//___________________Segmento_Dichiarazioni__________________________________
void setHandler();//Funzione setta gli handler per i segnali, genera un errore se non si rispettano le note
struct segnale *getFifoElem();//Funzione preleva l'ultimo elemento della FIFO eliminandolo dalla coda stessa

void handler(int sig, siginfo_t *info, void * vp);
int setupHandler(int sig, void (*h)(int, siginfo_t *, void *), int options);
//____________________________________________________________________________


//_______________________________________Metodi_Handler______________
void setHandler(){
    int i;
    for(i = SIGREADY; i<=SIGCONCLUSIONE; i++){
        setupHandler(i, handler, SA_SIGINFO);
    }
}

struct segnale *getFifoElem(){
    int len = signGetLength(scomunicazioni->fifoQueue);
    if(len == 0){
        //DEBUG---
        //printf("\n fifoQueue is empty");
        //--------
        return NULL;
    }else if(len == 1){
        struct segnale *toReturn = getSegnale();
        toReturn->fromPid = scomunicazioni->fifoQueue->fromPid;
        toReturn->sig = scomunicazioni->fifoQueue->sig;
        toReturn->next = scomunicazioni->fifoQueue->next;
        //free(scomunicazioni->fifoQueue);
        scomunicazioni->fifoQueue = NULL;
        return toReturn;
    }else{
        struct segnale *temp = scomunicazioni->fifoQueue;
        struct segnale *toReturn = getSegnale();
        while(temp->next->next != 0){
            temp = temp->next;
        }

        toReturn->fromPid = temp->next->fromPid;
        toReturn->sig = temp->next->sig;
        toReturn->next = temp->next->next;
        
        //free(temp->next);

        temp->next = 0;
        return toReturn;
    }
}
//___________________________________________________________________


//______________________________Metodi_per_implementazione_____________
int setupHandler (int sig, void (*h)(int, siginfo_t *, void *), int options)
{
    int r;
    struct sigaction s;
    s.sa_sigaction = h;
    sigemptyset (&s.sa_mask);
    s.sa_flags = options;
    r = sigaction (sig, &s, NULL);
    if (r < 0) perror (__func__);
    return r;
}


void handler(int sig, siginfo_t *info, void * vp){
    int len = signGetLength(scomunicazioni->fifoQueue);
    if(len == 0){
        scomunicazioni->fifoQueue = getSegnale();
        scomunicazioni->fifoQueue->fromPid = info->si_pid;
        scomunicazioni->fifoQueue->sig = sig;
        scomunicazioni->fifoQueue->next = 0;
    }else if(len == 1 && scomunicazioni->fifoQueue->fromPid == 0){
        scomunicazioni->fifoQueue->fromPid = info->si_pid;
        scomunicazioni->fifoQueue->sig = sig;
    }else{
        struct segnale *thisSegnale = getSegnale();
        thisSegnale->fromPid = info->si_pid;
        thisSegnale->sig = sig;
        thisSegnale->next = 0;
        scomunicazioni->fifoQueue = signAddFifo(scomunicazioni->fifoQueue,thisSegnale,FIFOBUFFERLEN );
    }    
    //DEBUG---
    //signPrint(scomunicazioni->fifoQueue);
    //printf("\n\n\n\n\n\n\n\n\n\n");
    //-------
}
//______________________________________________________________________
#endif