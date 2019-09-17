#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "ipc/ipc.h"
#include "datastructure/datastructure.h"

//_______________Costanti_____________________________
#include "optconf_e_costanti/costanti.h"
//____________________________________________________


//_______________Dichiarazioni_globali________________
struct comunicazioni *scomunicazioni;
struct studente *sstudente;
struct gruppo *slistaGruppi;
struct conf *sconf;                           
int semGruppi;                                //Semaforo usato dagli studenti per accedere a slistaGruppi

time_t tstart;              
struct studente *thisStudente;                //Rappresentazione in listastudenti del corrente studente
int elemGrup = 1;                             //Elementi appartenenti al gruppo
int conclusione = 0, start = 0, stop = 0;     //Variabili utili per interrompere cicli in caso di aventi
int currentInvites = 0, currentRejects = 0;   //Inviti / rifiuti correntemente inviati
struct gruppo *thisGruppo = 0;                //Rappresentazione in listagruppi del corrente studente
//____________________________________________________

//_______________Dichiarazioni_funzioni_______________
void inizialize_structs();                    //Inizializza le strutture dati
void initialComunications();                  //Si occupa dell'invio dei segnali READY
void parametersGenerator();                   //Genera i parametri runtime e li immette dentro la struttura listastudenti
void gestioneSegnale(struct segnale *elem);   //Esegue delle operazioni sulle code per gestire l'occorrenza del singolo segnale
void controlloSegnali();                      //Gestisce tutti i segnali accumulati in coda
void close();                                 //Termina la fase della formazione dei gruppi
void searchAndInvite();                       //Si occupa dell'invio degli inviti
void denyEveryRequest();                      //Elimina tutte le richieste in pendenza in arrivo
void conferma();                              //Si occupa dell'invio delle conferme
void createGroup(int owner);                  //Crea un gruppo e aggiorna invarianti
void waitVoto();                              //Aspetta il segnale "voto" dal padre
void testAndRefresh();                        //Si occupa di aggiornare delle invarianti 
int stessoTurno(struct studente *elem);       //Si occupa di deteminare se i due studenti sono dello stesso turno
//____________________________________________________

//______________Funzioni_test_________________________
void optSimulator();                           //Da usare per test in caso di bug da parte della gestione di opt.conf
                                               //all'interno di inizialize_structs()
//____________________________________________________







int main(int argc, char *argv[]){
    //FASE 1
    inizialize_structs();
    setHandler();
    parametersGenerator();
    initialComunications();
    //FASE 2_______________________________________________________________________ 
    semGruppi = getSemaphore(KEY_SEMGRUPPI);                                        
    setSemaphore(semGruppi);
    while(!conclusione){
        controlloSegnali();

        if(time(NULL)-tstart>=(sim_time-1)){
            close();
        }


         struct segnale *temp = temp = signGetLast(scomunicazioni->fifoQueue);
        
         Wait_Sem(semGruppi);
         if(temp == signGetLast(scomunicazioni->fifoQueue) && !conclusione){ //Esegue il blocco solo se Wait non è stato interrotto 
                                                                             //dalla ricezione di un segnale
            testAndRefresh();
            
            struct segnale *listS = scomunicazioni->listPendingAggToSend;      //Lista richieste a cui rispondere
            
            if(signGetLength(listS) > 1 ){
                    conferma();
                    goto OUT;
            }

            searchAndInvite();
         }
         OUT:
         Signal_Sem(semGruppi);
    }
    //FASE 3________________________________________________________________________
    waitVoto();
    printf("PID: %d MATR:%d voto_AdE:%d voto_SO:%d IDGruppo:%d \n", getpid(), 
        thisStudente->matricola , thisStudente->voto_AdE, thisStudente->voto,
        grupGetIndex(slistaGruppi, thisGruppo));
    exit(1);
    return 0;
}









//____________________Funzioni______________________________
int stessoTurno(struct studente *elem){
    if(thisStudente->matricola %2 == 0){
        if(elem->matricola %2 == 0){
            return 1;
        }else return 0;
    }else{
        if(elem->matricola %2 != 0){
            return 1;
        }else return 0;
    }
}

void testAndRefresh(){ 
    if(grupFindPid(slistaGruppi, getpid(), POP_SIZE)!=-1){
        thisGruppo = grupFindPid(slistaGruppi, getpid(), POP_SIZE);
    }

    if(time(NULL)-tstart>=(sim_time-1) ||!(thisGruppo == 0 || thisGruppo->chiuso == 0)){
        close();
    }

    if(thisGruppo != 0)elemGrup = grupPidLen(thisGruppo);

    if(elemGrup >= thisStudente->nof_elems ){
         close();                                           
    }
}

void createGroup(int owner){
        struct gruppo *temp = (struct gruppo *)malloc(sizeof(struct gruppo));
        temp->pid[1] = 0; temp->pid[2] = 0; temp->pid[3] = 0; temp->chiuso = 0;
        temp->pid[0] = owner;
        //thisGruppo = grupGetLast(slistaGruppi, POP_SIZE);
        grupAdd(slistaGruppi, temp, POP_SIZE);
        thisGruppo = grupFindPid(slistaGruppi, owner, POP_SIZE);
        thisStudente->grupOwner = owner;
        //printf(" ZCREAGRUPPI: THISPID:%d OWNER:%d", getpid(),owner);
}

void conferma(){ //Conferma lo studente con il voto più alto e rifiuta gli altri
    int i, pos = 0, temp_voto, next_voto = 0;
    struct segnale *lista = scomunicazioni->listPendingAggToSend;
    struct segnale *temp = 0;//signGetPos(lista, 1); 

    if(thisGruppo != 0 || thisStudente->grupOwner != 0)goto END;
    for(i=1; i<signGetLength(lista); i++){
        temp_voto = 0;
        if(temp != 0){
            temp_voto = studentFindPid(sstudente, temp->fromPid, POP_SIZE)->voto_AdE;
        }
        
        struct studente *next_studente = studentFindPid(sstudente, signGetPos(lista, i)->fromPid, POP_SIZE);
        struct gruppo *next_gruppo = grupFindPid(slistaGruppi, signGetPos(lista, i)->fromPid,POP_SIZE);
        next_voto = next_studente->voto_AdE;
        if(next_studente->nof_elems != thisStudente->nof_elems) next_voto = next_voto - 3; //toglie alla valutazione del voto 3 punti se i due nof_elems sono diversi

        if(next_voto >=temp_voto &&(next_studente->grupOwner == 0 || next_studente->grupOwner == next_studente->pid) )  {
            if(next_gruppo == -1 || (grupPidLen(next_gruppo) < next_studente->nof_elems && next_gruppo->chiuso == 0)){
                temp = signGetPos(lista, i);
                pos = i;
            }
        }
        
    }

    //printf("POS:%d LEN:%d\n", pos, signGetLength(lista)-1);
    
   
    if(temp != -1 && temp != 0 && temp->fromPid != getpid() && temp->fromPid != 0){  
        conclusione = 1;
        //printf("%d INVIOCONFERMAA: %d", getpid(), temp->fromPid);
        thisStudente->grupOwner = temp->fromPid; //setto il mio gowner
        struct studente *grupStudente = studentFindPid(sstudente, temp->fromPid, POP_SIZE);  //cerco il suo
        grupStudente->grupOwner=temp->fromPid; //setto il suo
        kill(temp->fromPid, SIGCONFERMA);   
        thisGruppo = grupFindPid(slistaGruppi, temp->fromPid, POP_SIZE);
        if(grupFindPid(slistaGruppi, temp->fromPid, POP_SIZE)==-1){
             createGroup(temp->fromPid);
        }
        grupInsertPid(thisGruppo, getpid());
        struct studente *tempStudente = studentFindPid(sstudente, temp->fromPid, POP_SIZE);
        if(tempStudente != 0)tempStudente->grupOwner = temp->fromPid;

        scomunicazioni->listPendingAggToSend = signRemovePos(lista, pos);
    }

    END:
    denyEveryRequest(); 
    lista = scomunicazioni->listPendingAggToRec;
    for(i=0; i<signGetLength(lista); i++){
        struct segnale *ttemp = signGetPos(lista,i);
        if(ttemp != 0 && ttemp->fromPid != 0 && ttemp->fromPid){
            kill(ttemp->fromPid, SIGRIFIUTO);
            //printf("YYpid: %d INVRIFIUTOTO: %d", getpid(), temp->fromPid);
            }
    }
    
}

void denyEveryRequest(){
    int i;
    struct segnale *lista = scomunicazioni->listPendingAggToSend;
    for(i=0; i<signGetLength(lista); i++){
        struct segnale *temp = signGetPos(lista,i);
        if(temp != 0 && temp->fromPid != 0){
            kill(temp->fromPid, SIGRIFIUTO);
            //printf("pid: %d INVRIFIUTOTO: %d", getpid(), temp->fromPid);
        }
    }
    //signPrint(lista);
    scomunicazioni->listPendingAggToSend = getSegnale(); //elimina tutte le richieste in sospeso*/
}

void searchAndInvite(){
struct segnale *lista = scomunicazioni->listPendingAggToRec;
while(thisStudente->nof_elems - signGetLength(lista)>0){
    int i;
    struct studente *maxAdE = (struct studente *)malloc(sizeof(struct studente));
    maxAdE->matricola = 0; maxAdE->voto_AdE = 0; maxAdE->nof_elems = 0;
    maxAdE->grupOwner = 0; maxAdE->voto = 0;
    for(i=0; i<POP_SIZE; i++){
        struct studente *temp = studentGetPos(sstudente, i);
        if(temp->voto_AdE >= maxAdE->voto_AdE && temp->grupOwner == 0 && temp->pid != getpid() && grupFindPid(slistaGruppi, temp->pid, POP_SIZE) == -1){
            if(signFind(lista, temp->pid, SIGAGGREGAZIONE) == -1 && stessoTurno(temp))maxAdE = temp; //controlla che non lo abbia già invitato 
                                                                                                     //e che abbia stessa matricola
        }
        
    }

    //if(maxAdE != 0 && maxAdE->pid != 0)printf("\nthisPid:%d MaxAdE: %p pid:%d\n", getpid(),maxAdE, maxAdE->pid);// printf("\n\n"); DA NON TOGLIERE
    if(maxAdE == 0|| maxAdE->grupOwner != 0 || maxAdE->pid == getpid() || maxAdE->pid == 0 || !stessoTurno(maxAdE) || currentInvites >= sconf->nof_invites){
        if(time(NULL)-tstart>=(sim_time-1)){
        //printf("\n____________CLOSE_________________\n");
        close();  //Non ci sono nuovi elementi da inviare 
        }
        break;
    }else if(maxAdE != 0 && maxAdE->pid != getpid() && maxAdE->grupOwner == 0){
        //printf("\nthisPid: %d maxAdE: %d\n", getpid(), maxAdE->pid);
        currentInvites++;
        kill(maxAdE->pid, SIGAGGREGAZIONE);
        struct segnale *toSend = getSegnale();
        toSend->fromPid = maxAdE->pid;
        toSend->sig = SIGAGGREGAZIONE; 
        toSend->next = 0;
        if(lista == 0){
            lista = toSend;
        }else{
            signAdd(lista, toSend); //Aggiungo il segnale nelle richieste inviate in pendenza
        }
        
    }
}

}

void close(){
    denyEveryRequest();
    int i; 
    struct segnale *lista = scomunicazioni->listPendingAggToRec;
    for(i=0; i<signGetLength(lista); i++){
        struct segnale *temp = signGetPos(lista,i);
        if(temp != 0 && temp->fromPid != 0){
            kill(temp->fromPid, SIGRIFIUTO);
            //printf("pid: %d INVRIFIUTOTO: %d", getpid(), temp->fromPid);
            }
    }
    thisGruppo = grupFindPid(slistaGruppi, getpid(), POP_SIZE);
    if(thisGruppo != -1){
        thisGruppo->chiuso = 1;
    }else{
        createGroup(getpid());
        thisGruppo->chiuso = 1;
    }
    conclusione = 1;
}

void controlloSegnali(){
    struct segnale *temp = getFifoElem();
    while(temp != NULL){
        gestioneSegnale(temp);
        temp = getFifoElem();
         //printf("PID: %d CONTROLLOSEGNALI", getpid());
    }
   
}

void gestioneSegnale(struct segnale *current){ //Testato
    if(current != 0){
        switch(current->sig){

            case SIGSTART:
                start = 1;
                break;

            case SIGAGGREGAZIONE:{
                //printf("\n->AGGR:THISPID:%d FROMTO: %d |len: %d|\n", getpid() , current->fromPid , signGetLength(scomunicazioni->listPendingAggToSend));
                if(scomunicazioni->listPendingAggToSend== 0){
                    scomunicazioni->listPendingAggToSend = current;
                }else{
                    signAdd(scomunicazioni->listPendingAggToSend, current);
                }
                //printf("\n---len: %d|\n", signGetLength(scomunicazioni->listPendingAggToSend));
                break;
            }

            case SIGRIFIUTO:{
                struct segnale *lista = scomunicazioni->listPendingAggToRec;
                while(lista != 0 && signFind(lista, current->fromPid,SIGAGGREGAZIONE) != -1){
                    lista = signRemovePos(lista, signFind(lista, current->fromPid,SIGAGGREGAZIONE));
                }
                scomunicazioni->listPendingAggToRec = lista;
                
                lista = scomunicazioni->listPendingAggToSend;
                while(lista != 0 && signFind(lista, current->fromPid,SIGAGGREGAZIONE) != -1){
                lista = signRemovePos(lista, signFind(lista, current->fromPid,SIGAGGREGAZIONE));
                }
                scomunicazioni->listPendingAggToSend = lista;
                
                //printf("\n->RIFIUTO:THISPID:%d FROMTO: %d |len: %d|\n", getpid() , current->fromPid , signGetLength(scomunicazioni->listPendingAggToRec));
                break;
            }

            case SIGCONFERMA:{
                struct segnale *lista = scomunicazioni->listPendingAggToRec;
                //printf("\nPID: %d CONFERMAFROM: %d\n", getpid(), current->fromPid);
                while(lista != 0 && signFind(lista, current->fromPid,SIGAGGREGAZIONE) != -1){
                     lista = signRemovePos(lista, signFind(lista, current->fromPid,SIGAGGREGAZIONE));
                }
                scomunicazioni->listPendingAggToRec = lista;
                    
                lista = scomunicazioni->listPendingAggToSend;
                while(lista != 0 && signFind(lista, current->fromPid,SIGAGGREGAZIONE) != -1){
                lista = signRemovePos(lista, signFind(lista, current->fromPid,SIGAGGREGAZIONE));
                }
                scomunicazioni->listPendingAggToSend = lista;
                break;
            }

            case SIGCONCLUSIONE:
                conclusione = 1;
                break;

            case SIGVOTO:
                stop = 1;
                break;
        }
    }
}

void waitVoto(){
    stop = 0;
    kill(getppid(),SIGREADY);
    while(!stop){
        controlloSegnali();
    }
    
}

void initialComunications(){
    kill(getppid(),SIGREADY);
    while(!start){
        controlloSegnali();
    }
    tstart = time(NULL);
}

void parametersGenerator(){
    srand(getpid()*time(NULL)); 
    int this_nof_elems, this_voto_AdE, this_matricola, r;
    int t1 = sconf->typeOfGroups[0], t2 = sconf->typeOfGroups[1], t3 = sconf->typeOfGroups[2], t4 = sconf->typeOfGroups[3];
    r = 1 + rand() % 100;  
    //Tipi di gruppi--
    if(t1+t2+t3+t4 != 100){
        printf("ERROR in opt.conf, the sum of probabilities is not equal to 100");
        exit(1);
    }
    if(r<=t1){
        this_nof_elems = 1;
    }else if(r>t1 && r<=t1+t2){
        this_nof_elems = 2;
    }else if(r>t1+t2 && r<=t1+t2+t3){
        this_nof_elems = 3;
    }else if(r>t1+t2+t3){
        this_nof_elems = 4;
    }
    //--

    this_voto_AdE = 18 +  rand() % 13;
    this_matricola = getpid(); 

    //immette i parametri all'interno della struttura listastudenti
    thisStudente = studentFindPid(sstudente, getpid(), POP_SIZE);
    if(thisStudente != -1){
        thisStudente->matricola = this_matricola;
        thisStudente->voto_AdE = this_voto_AdE;
        thisStudente->nof_elems = this_nof_elems;
        thisStudente->grupOwner = 0;
    }else{
        printf("ERROR PID NOT FOUND");
        exit(-1);
    }
    //----------------------------------------------------------------
    //printf("\nelems = %d votoA = %d mart = %d\n",thisStudente->nof_elems, thisStudente->voto_AdE, thisStudente->matricola);
}

void inizialize_structs(){
    scomunicazioni = getComunicazioni();
    sstudente = attachStudenti(KEY_LSTUDENTI, POP_SIZE);
    slistaGruppi = attachGruppi(KEY_LGRUPPI, POP_SIZE);
    sconf = attachConf(KEY_CONF);
    //sstudente = listaStudenti(KEY_LSTUDENTI, POP_SIZE);
    //slistaGruppi = listaGruppi(KEY_LGRUPPI, POP_SIZE);
    //sconf = getConf();
}
//_____________________________________________________________

void optSimulator(){
    sconf->typeOfGroups[0] = 0;
    sconf->typeOfGroups[1] = 0;
    sconf->typeOfGroups[2] = 0;
    sconf->typeOfGroups[3] = 100;
    sconf->nof_invites = 3;
}
