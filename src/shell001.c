#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"  //colori
#define CYAN    "\x1b[36m"
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"
#define READ 0  // read-side of pipes
#define WRITE 1 // write-side of pipes

#include "big_mngr.h"   //includo le procedure necessarie per il controllo e la gestione degli argomenti iniziali e delle stringhe passate come comandi
#include "runs.h"       //includo le procedure necessarie per eseguire la stringa passata in input

void sigint_handler(int sig){
    // -> la main shell ignorera' il SIGINT (ctrl+c)
    //....
}
int main(int argc, char *argv[]){
    // -> il main si occupa di richiamare le funzioni che metteranno a posto gli argomenti passati in input e poi entrera' nel ciclo che rappresenta la shell
    signal(SIGINT, sigint_handler);//viene assegnata la funzione sigint_handler al segnale sigint
    setstd();//setta i file descriptor standard
    char *err_path, *out_path; err_path = NULL; out_path = NULL;//vengono assegnati momentaneamente i valori null alle stringhe che conterranno il path
    int max_len = -1, code = -1, timeout = -1;//agli interi viene invece attualmente assegnato il valore -1 per specificare che non contengono ancora niente di valido
    int valid=0;//e' un flag che specifichera' se la stringa passata in input alla custom shell contiene un piping coerente
    char cwd[100];//conterra' il path della current working directory
    getcwd(cwd, sizeof(cwd));
    /*viene creato il path assoluto dello script che termina i processi delle chiamate system*/
    strcpy(klr,cwd);
    if(strcmp(klr+strlen(klr)-4,"/bin")==0){//controlla se l'esecuzione parte dalla cartella bin
        //printf("cwd is bin!\n");
        klr[strlen(klr)-4]=00;//se e' vero tronca la stringa togliendo la cartella bin
    }
    strcat(klr,"/src/last_pid.sh");//si aggiunge alla stringa il resto del path necessario
    if(access(klr, F_OK) == -1){//controlla per sicurezza se il path che porta allo script esiste veramente
        fprintf(stderr, RED "the path to the last_pid.sh script does not exist! Please run the program from his root folder or the bin folder\n" RESET);
        exit(2);
    }
    strcat(klr," ");//lo spazio e' necessario per poi poter passare un pid come argomento dello script
    /*viene richiamata la funzione che gestice gli argomenti passati al main*/
    printf(RESET "----------------------------------------\n\n");
    args_manager(argc, argv, &out_path, &err_path, &max_len, &code, &timeout, cwd);
    printf(RESET "\nRECEIVED PARAMETERS----------------------------------------\n");
    printf(BLUE "out : %s - err : %s - max : %i - code : %i - timeout : %i", out_path, err_path, max_len, code, timeout);//vengono stampati a video i parametri ottenuti dall'args_manager
    printf(RESET "\n-----------------------------------------------------------\n");
    if(timeout < 0){
        printf(RED "WARNING! Time out isn't set, this might cause some problems\n");
    }
    remove(out_path);//vegono rimossi i file appena specificati nel caso in cui fossera gia' presenti
    remove(err_path);
    /*si inizializzano le variabili necessarie per far andare la custom shell*/
    char *input_buffer;//buffer di input
    char *cmd[10];//array di stringhe che conterra' il vettore dei comandi passati e i caratteri di pipe e riderizionamento
    int y;//conterra' il numero di token passati in input
    int cont = 0;//conta quanti stringhe di comandi sono state passate alla shell, verra' usato per l'id
    int num_id; //conterrà il numero di comandi effettivi
    char *cl_output = (char *)malloc(100 * sizeof(char));//variabile di supporto per il readline
    input_buffer = (char *)malloc(200 * sizeof(char));//alloca la memoria necessaria per il buffer
    if(input_buffer == NULL || cl_output == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffers\n");
        exit(1);
    }
    /*parte il ciclo while che prende in input i comandi*/
    while(strcmp(input_buffer, "quit")){//continua finche' l'utente non inserisce quit
        fflush(stdout);
        getcwd(cwd, sizeof(cwd));//viene aggiornata la current working directory
        snprintf(cl_output,100, RESET "[%i]shell:%s> ", getpid(), cwd);//salva in cl_output l'output che da la line di comando
        input_buffer = readline(cl_output);//aspetta in input una stringa
        if(strlen(input_buffer) == 0){//se non viene passato niente in input salta alla prossima iterazione
            goto gino;
        }
        add_history(input_buffer);
        valid=tok_manager(input_buffer, &cmd, &y, &num_id, out_path, err_path);//suddivide la stringa presa in input in un vettore di stringhe contenente i comandi e i pipe, inoltre ritorna 0 se la stringa contiene un piping incoerente
        printf("\n");
        if(valid==1){//se i comandi passati risultano validi chiama le opportune funzioni
            if(strcmp(cmd[0], "quit") != 0){//se i comandi inseriti non sono un quit prova a eseguirli
              cont++;//incrementa il contatore degli ID principale
                if(y == 1){//se si tratta di un solo comando richiama la solo_run
                    solo_run(cmd[0], out_path, err_path, max_len, code, timeout, standard_inp, standard_out, standard_err, cont, num_id, 0);
                }else{//altrimenti vuol dire che ci sono dei pipe e quindi richiama la piped_run
                    piped_run(cmd, y, y, out_path, err_path, max_len, code, timeout, NULL, cont, num_id);
                }
            }
        }
        gino:;
    }
    printf(RESET "BYE!\n");
    return 0;
}
