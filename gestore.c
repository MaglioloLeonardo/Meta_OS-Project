#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "ipc/ipc.h"
#include "datastructure/datastructure.h"

//_______________Costanti_____________________________
#include "optconf_e_costanti/costanti.h"
//____________________________________________________

//_______________Dichiarazioni_globali________________
time_t tstart;
int tostop = 0;
//____________________________________________________

//_______________Dichiarazioni_globali________________
struct comunicazioni *scomunicazioni;
struct studente *sstudente;
struct conf *sconf;
struct gruppo *slistaGruppi;
int semStudent;             //Semaforo usato dalle fork del programma gestore per immettere i pid dei figli dentro la struttura listaStudenti
//____________________________________________________

//_______________Dichiarazioni_funzioni_______________
void inizialize_structs();	//Inizializza le strutture dati
void studentsGen();			//Genera i POP_SIZE studenti
void studentsReady();		//Attende i segnali "Ready" dai figli
void studentsReply();		//Invia il segnale START ai figli
void simTime();				//Attende sim_time secondi e invia il segnale CONCLUSIONE ai figli
void genVoti();				//Genera i voti immettendoli nelle strutture dati
void studentsVoto();		//Invia il segnale VOTO ai figli
void waitForChilds();		//Aspetta la terminazione dei figli
void statsPrint();			//Stampa le statistiche richieste riguardo i figli
//____________________________________________________

int main(int argc, char *argv[]){
    inizialize_structs();
    setHandler();
    studentsGen();
    studentsReady();
    studentsReply();
    //FASE2___________________________________________________
    simTime();
    test(); //<- Decommentare se si vogliono visionare le strutture condivise una volta finito simTime
    studentsReady();
    //FASE3___________________________________________________
    genVoti();
    studentsVoto();
    waitForChilds();
    statsPrint();
    return 0;
}

//____________________Funzioni______________________________
void inizialize_structs(){
    scomunicazioni = getComunicazioni();
    sstudente = listaStudenti(KEY_LSTUDENTI, POP_SIZE);
    slistaGruppi = listaGruppi(KEY_LGRUPPI, POP_SIZE);

    sconf = getConf(KEY_CONF);
	FILE *fpointer = fopen("optconf_e_costanti/opt.conf", "r");
	confInizialize(sconf, fpointer);
}

void studentsGen(){
    int i, errorexeclp;
    char *name = "studente.o";
    char *path = "./studente.o";
    char *argv2pass[2];
    argv2pass[0] = name; argv2pass[1] = NULL;
    semStudent = getSemaphore(KEY_SEMSTUDENT);
    setSemaphore(semStudent);

    for(i = 1; i <= POP_SIZE; i++ ){
        switch(fork()){
        case -1:
            perror("ERRORE FORK studentsGen");
            i--;
            exit(-1);
            
            
        case 0:
            //printf("PADRE F1: n: %d pid: %d \n", i, getpid());
            Wait_Sem(semStudent);
                struct studente *temp = (struct studente *)malloc(sizeof(struct studente));
                temp->matricola = 0; temp->voto_AdE = 0; temp->nof_elems = 0;
                temp->grupOwner = 0; temp->voto = 0;
                temp->pid = getpid();
                studentAdd(sstudente, temp, POP_SIZE);
            Signal_Sem(semStudent);
            errorexeclp = execvp(path, argv2pass);
            printf("ERRORE EXECLP N: %d", errorexeclp);
            exit(1);
            break;
        }
    }
}

void studentsReady(){
    int counter = 0;

    tstart = time(NULL);
    while(counter < POP_SIZE){
        if(time(NULL)-tstart>=3 && tostop == 1)break;
        struct segnale *thisSegnale = getFifoElem();
        if(thisSegnale != NULL && thisSegnale->sig == SIGREADY){
            counter++;
        }else{
            sleep(1);// Da non editare
        }
    }
    tostop = 1;
}

void studentsReply(){
    int semGruppi = getSemaphore(KEY_SEMGRUPPI);
    setSemaphore(semGruppi);
    int i = 0;
    while(i<POP_SIZE){
        struct studente *temp = studentGetPos(sstudente, i);
        kill(temp->pid, SIGSTART);
        i++;
    }
}

void simTime(){
    int i = 0;
    printf("Attendere %d secondi...\n", sim_time);
    tstart = time(NULL);
    while(time(NULL)-tstart<(sim_time)){
        sleep(1);//Da non editare
    }
    
    while(i<POP_SIZE){
        struct studente *temp = studentGetPos(sstudente, i);
        kill(temp->pid, SIGCONCLUSIONE);
        i++;
    }
}

void genVoti(){
    int i;
    struct studente *tempStudente;
    for(i = 0; i<POP_SIZE; i++){
        struct studente *tempStudente = studentGetPos(sstudente, i);
        struct gruppo *tempGruppo = grupFindPid(slistaGruppi, tempStudente->pid, POP_SIZE);
        if(tempGruppo == -1 || tempGruppo == 0 || tempGruppo->chiuso == 0){
            tempStudente->voto = 0;   
        }else{
            int j, tempVoto=0, maxVoto=0;
            for(j=0; j<4; j++){
            	tempVoto = 0;
                if(tempGruppo->pid[j] > 0)tempVoto = studentFindPid(sstudente, tempGruppo->pid[j], POP_SIZE)->voto_AdE;
                if(tempVoto > maxVoto)maxVoto = tempVoto;
            }
            if(tempStudente->nof_elems != grupPidLen(tempGruppo) && maxVoto>0)maxVoto = maxVoto - 3; 
            tempStudente->voto = maxVoto;
        }
    }
}

void studentsVoto(){
    int i=0;
    while(i<POP_SIZE){
        struct studente *temp = studentGetPos(sstudente, i);
        kill(temp->pid, SIGVOTO);
        i++;
    }
}

void stop(){
    int i=0;
    while(i<POP_SIZE){
        struct studente *temp = studentGetPos(sstudente, i);
        kill(temp->pid, 9);
        i++;
    }
}

void waitForChilds(){
    pid_t child_pid, wpid;
    int status = 0;
    tstart = time(NULL);
    while(time(NULL)-tstart<=3){
        sleep(1); //Da non editare
        continue;
    }
    stop();
}

void statsPrint(){
	int i, media_AdE = 0, media_SO = 0;
	struct studente *tempStudente;
	
	 for(i = 0; i<POP_SIZE; i++){
	 	 tempStudente = studentGetPos(sstudente, i);
	 	 media_AdE = media_AdE + tempStudente->voto_AdE;
	 }
	 media_AdE = media_AdE/POP_SIZE;

	 for(i = 0; i<POP_SIZE; i++){
	 	 tempStudente = studentGetPos(sstudente, i);
	 	 media_SO = media_SO + tempStudente->voto;
	 }
	 media_SO = media_SO/POP_SIZE;

	printf("\nStatistiche gestore:\n");
	printf("	-Numero di studenti:%d\n", POP_SIZE);
	printf("	-Media voto_AdE: %d\n", media_AdE);
	printf("	-Media voto_SO: %d\n", media_SO);
}

void test(){
    //studentsPrint(sstudente, POP_SIZE);
    printGrup(slistaGruppi, POP_SIZE);
}

//___________________________________________________________

