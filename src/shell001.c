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
#define BLUE    "\x1b[34m"  //colori carini
#define CYAN    "\x1b[36m"
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"
#define READ 0  // read-side of pipes
#define WRITE 1 // write-side of pipes

#include "big_mngr.h"   //includo le procedure necessarie per il controllo e la gestione degli argomenti iniziali e delle stringhe passate come comandi
#include "runs.h"       //includo le procedure necessarie per eseguire i comandi

void kill_handler(int sig){
  //....
}
int main(int argc, char *argv[]){
    signal(SIGINT, kill_handler);
    setstd();   //setta i file descriptor standard
    char *err_path, *out_path; err_path = NULL; out_path = NULL;
    int max_len = -1, code = -1;    //conterranno i contenuti delle opzioni che verranno usati
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //--> controllo che gli argomenti passati all'eseguibile siano corretti
    printf(RESET "----------------------------------------\n\n");
    args_manager(argc, argv, &out_path, &err_path, &max_len, &code, cwd);   //args_manager andra' a mettere all'interno delle variabili gli opportuni valori che trova
    printf(RESET "\nRECEIVED PARAMETERS----------------------------------------\n");
    printf(BLUE "out : %s - err : %s - max : %i - code : %i", out_path, err_path, max_len, code);   //stampo i parametri ottenuti dall'args_manager
    printf(RESET "\n-----------------------------------------------------------\n");
    remove(out_path);   //rimuove i file di log se gia' presente, altrimenti sovrascrive su dati gia' presenti
    remove(err_path);
    //--> variabili necessarie per il buffer di input
    char *input_buffer; //buffer effettivo
    size_t buff_size = 100; //numero di caratteri che il buffer puo' contenere
    //--> variabili necessarie per fare i token e i comandi
    char *cmd[10];  //conterra' il vettore dei comandi passati(puo' essere composto da piu' strighe)
    int y;  //conterra' il numero di comandi
    int cont = 0;   //conterrà qunati comandi sono stati passati dalla shell
    int num_id; //conterrà il numero di comandi effettivi
    //--> creo buffer per l'input
    char *tmp = (char *)malloc(100 * sizeof(char));     //variabile di supporto per il readline
    input_buffer = (char *)malloc(100 * sizeof(char));  //alloco la memoria necessaria per il buffer
    if(input_buffer == NULL){   //controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    //--> prendo in input i comandi

    while(strcmp(input_buffer, "quit")){    //continuo finche' l'utente non inserisce quit
        fflush(stdout);
        getcwd(cwd, sizeof(cwd));
        snprintf(tmp,buff_size, RESET "[%i]shell:%s> ", getpid(), cwd);
        input_buffer = readline(tmp);
        if(strlen(input_buffer) == 0){
            goto gino;
        }
        add_history(input_buffer);
        if(strlen(input_buffer) == 1){
            continue;
        }else{
            strtok(input_buffer, "\n");
        }
        //printf(BLUE "\n<main:info> string: %s - n_char : %i \n", input_buffer, strlen(input_buffer));
        //--> divisione della stringa in tokens
        tok_manager(input_buffer, &cmd, &y, &num_id);//dentro arr mi trovero le diverse stringhe passare in input e in cmd i comandi passati
        printf("\n<main:info> total commands received : %i --> ", y);
        int i;
        for(i = 0; i < y; i++){
            printf("(%s) ", cmd[i]);
        }
        printf("\n");
        if(strcmp(cmd[0], "quit") != 0){    //se i comandi inseriti non sono un quit provo a eseguirli
          cont++;
            if(y == 1){
                solo_run(cmd[0], out_path, err_path, max_len, code, standard_inp, standard_out, standard_err, cont, num_id, 0);
            }else{
                pipedrun(cmd, y, y, out_path, err_path, max_len, code, NULL, cont, num_id);
            }
        }
        sleep(0.2);
        gino:;
    }
    //free(input_buffer);
    printf(RESET "BYE!\n");
    return 0;
}
