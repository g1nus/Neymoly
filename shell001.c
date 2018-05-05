#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m" //colori carini
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"

#include "big_mngr.h"//includo l'agrs_manager

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code){
    int fd[2];//uno per stdout e un per sterr
    int st_out;//conterra' temporaneamente lo stdout
    int cmd_out;//conterra' temporaneamente l'output del programma
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_str;
    st_out = dup(1);
    printf(BLUE "<solo_run:info> command to run is %s \n", command);
    printf(RESET "\n");
    fd[0]=open(out_path, O_WRONLY | O_CREAT, 0777);//apro il file di output
    cmd_out=open("/tmp/tmpcmd.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente l'output del programma
    dup2(cmd_out,1);//fa in modo che stdout punti allo stesso file puntato da fd[0]
    system(command);
    dup2(st_out,1);
    close(cmd_out);
    cmd_out=open("/tmp/tmpcmd.cmd", O_RDONLY);//apro il file in cui verra' salvato temporaneamente l'output del programma
    wbytes = lseek(cmd_out, (size_t)0, SEEK_END);
    lseek (cmd_out, (off_t) 0, SEEK_SET);
    printf("number of bytes written is : %i \n", wbytes);
    cmd_str = (char *)malloc(wbytes * sizeof(char));
    if(cmd_str == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    if(read(cmd_out, cmd_str, wbytes)<0){
        printf("there was an error reading the file\n");
    }
    printf("I've read this :\n %s", cmd_str);
    close(fd[0]);
    close(st_out);
    close(cmd_out);
    printf("\n");
}

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
        printf(RESET "shell> ");
        scanf ("%[^\n]%*c", input_buffer);//metto in input_buffer la linea presa in input(scanf legge fino a quando non trova invio)
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
        if(strcmp(cmd[0],"quit")!=0){//se i comandi inseriti non sono un quit provo a eseguirli
            if(y==1){
                printf("<main:info> found only one command, going to run it\n");
                solo_run(cmd[0], out_path, err_path, max_len, code);
            }
        }
    }
    return 0;
}
