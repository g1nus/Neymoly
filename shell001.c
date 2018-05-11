#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <signal.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m" //colori carini
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"
#define READ 0 /* read-side of pipes */
#define WRITE 1 /* write-side of pipes */

#include "big_mngr.h"//includo le procedure necessarie per il controllo e la gestione degli argomenti iniziali e delle stringhe passate come comandi
#include "runs.h"//includo le procedure necessarie per eseguire i comandi

int main(int argc, char *argv[]){
    setstd();//setta i file descriptor standard
    char *err_path=NULL, *out_path=NULL; int max_len=-1, code=-1;//conterranno i contenuti delle opzioni che verranno usati
    /*--> controllo che gli argomenti passati all'eseguibile siano corretti */
    printf(RESET "----------------------------------------\n\n");
    args_manager(argc, argv, &out_path, &err_path, &max_len, &code);//args_manager andra' a mettere all'interno delle variabili gli opportuni valori che trova
    printf(RESET "\nRECEIVED PARAMETERS----------------------------------------\n");
    printf(BLUE "out : %s - err : %s - max : %i - code : %i", out_path, err_path, max_len, code);//stampo i parametri ottenuti dall'args_manager
    printf(RESET "\n-----------------------------------------------------------\n");
    remove(out_path);//rimuove i file di log se gia' presente, altrimenti sovrascrive su dati gia' presenti
    remove(err_path);
    /*--> variabili necessarie per il buffer di input */
    char *input_buffer;//buffer effettivo
    size_t buff_size = 100;//numero di caratteri che il buffer puo' contenere
    /*--> variabili necessarie per fare i token e i comandi*/
    char *arr[10];//conterra' il vettore delle stringe passate(attualmente puo' contenerene 10)
    char *cmd[10];//conterra' il vettore dei comandi passati(puo' essere composto da piu' strighe)
    int x;//conterra' il numero di stringhe(argomenti) passati
    int y;//conterra' il numero di comandi
    /*--> creo buffer per l'input */
    input_buffer = (char *)malloc(buff_size * sizeof(char));//alloco la memoria necessaria per il buffer
    if(input_buffer == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    /*--> prendo in input i comandi */
    while(strcmp(input_buffer, "quit")){//continuo finche' l'utente non inserisce quit
        printf(RESET "[%i]shell> ", getpid());
        scanf (" %[^\n]%*c", input_buffer);//metto in input_buffer la linea presa in input(scanf legge fino a quando non trova invio)
        printf(BLUE "\n<main:info> string: %s - n_char : %i \n", input_buffer, strlen(input_buffer));
        /*--> divisione della stringa in tokens */
        tok_manager(input_buffer, &arr, &cmd, &x, &y);//dentro arr mi trovero le diverse stringhe passare in input e in cmd i comandi passati
        printf("<main:info> total arguments received  : %i --> ", x);
        for(int i=0; i<x; i++){
            printf("(%s) ", arr[i]);
        }
        printf("\n<main:info> total commands received : %i --> ", y);
        for(int i=0; i<y; i++){
            printf("(%s) ", cmd[i]);
        }
        printf("\n");
        sleep(1);
        if(strcmp(cmd[0],"quit")!=0){//se i comandi inseriti non sono un quit provo a eseguirli
            if(y==1){
                printf(BLUE "<main:info> found only one command, going to run it\n\n" RESET);
                solo_run(cmd[0], out_path, err_path, max_len, code, standard_inp, standard_out, standard_err);
            }
            if(y>2){
                printf(BLUE "<main:info> found pipe, calling pipedrun\n\n" RESET);
                pipedrun(cmd, y, out_path, err_path, max_len, code,NULL);
            }
        }
    }
    return 0;
}
