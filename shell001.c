#include<stdio.h>
#include<string.h>
#include<stdlib.h> 

#include "arg_mngr.h"//includo l'agrs_manager

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m" //colori carini
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"

int main(int argc, char *argv[]){
    char *err_path, *out_path; int max_len=-1, code=-1; err_path=NULL; out_path=NULL;//conterranno i contenuti delle opzioni che verranno usati
    /*--> controllo che gli argomenti passati all'eseguibile siano corretti */
    printf(RESET "----------------------------------------\n\n");
    args_manager(argc, argv, &out_path, &err_path, &max_len, &code);//args_manager andra' a mettere all'interno delle variabili gli opportuni valori che trova
    printf(RESET "\nRECEIVED PARAMETERS----------------------------------------\n");
    printf(BLUE "out : %s - err : %s - max : %i - code : %i", out_path, err_path, max_len, code);//stampo i parametri ottenuti dall'args_manager
    printf(RESET "\n-----------------------------------------------------------\n");
    /*--> variabili necessarie per il buffer di input */
    char *input_buffer;//buffer effettivo
    size_t buff_size = 100;//numero di caratteri che il buffer puo' contenere
    /*--> variabili necessarie per fare i token */
    char *tok;//conterra' i token
    int x;//conterra' il numero di token trovati
    /*--> creo buffer per l'input */
    input_buffer = (char *)malloc(buff_size * sizeof(char));//alloco la memoria necessaria per il buffer
    if(input_buffer == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    /*--> prendo in input i comandi */
    while(strcmp(input_buffer, "quit")){//continuo finche' l'utente non inserisce quit
        x = 0;//contatore per il presunto numero di comandi trovati(token)
        printf("shell> ");
        scanf ("%[^\n]%*c", input_buffer);//metto in input_buffer la linea presa in input(scanf legge fino a quando non trova invio)
        printf(BLUE "<info> string: %s - n_char : %i \n", input_buffer, strlen(input_buffer));
        /*--> divisione della stringa in tokens */
        tok = strtok(input_buffer, "|");//tok conterra' in questo primo caso tutto cio' che c'e' prima del primo pipe
        while(tok != NULL){
            printf("<info> tok[%i] : %s \n", x, tok);
            x++;
            tok = strtok(NULL, "|");//continuo con la restante stringa fino al prossimo pipe o fino alla fine
        }
        printf("<info> n_tokens : %i \n" RESET, x);
    }
    return 0;
}
