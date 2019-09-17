#define POP_SIZE 500
#define KEY_LSTUDENTI 9000
#define KEY_LGRUPPI 9001
#define KEY_CONF 9002
#define KEY_SEMSTUDENT 9003 //Key semaforo semStudent
#define KEY_SEMGRUPPI 9004 //Key semaforo semGruppi

int sim_time = 30; 

/*
1)Per generare i gruppi come richiesto il programma ha bisogno di un
tempo sim_time suff. grande. Nel caso in cui sim_time risulti
sproporazionato rispetto al numero di studenti POP_SIZE richiesti
è possibile che i gruppi non riescano a formarsi completamente e che quindi
la media dei voti si abbassi di conseguenza.

2)Se vengono cambiate le chiavi soprastanti è necessario modificare la funziona
removeIPCS() all'internno di gestore.c

3)I processi figli chiudono un secondo prima che il tempo sim_time sia esaurito
dunque per garantiche che i gruppi siano ben formati: sim_time > 1
*/
