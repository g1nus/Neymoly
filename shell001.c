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

#include "big_mngr.h"//includo le procedure necessarie per il controllo e la gestione degli argomenti iniziali e delle stringhe passate come comandi

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code){
    int fd[2];//uno per il file di log di output  e uno per il file di log di errori
    int st_out;//conterra' temporaneamente lo stdout(video)
    int st_err;//conterra' temporaneamente lo stderr(video)
    int cmd_out;//conterra' temporaneamente l'output del programma(file)
    int cmd_err;//conterra' temporaneamente lo stderr del programma(file)
    int cmd_date;//conterra' tempraneamente la data di esecuzione del programma(file)
    int r = 127;//conterra' il valore di ritorno della funzione
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_strout, *cmd_strerr, *cmd_strdate, cmd_r[10];//conterra' le stringhe restituite dal programma
    st_out = dup(1);//mi salvo lo stdout(video)
    st_err = dup(2);//mi salvo lo stderr(video)
    remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmperr.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmpdate.cmd");//rimuove la date del precedente comando se presente
    printf(BLUE "<solo_run:info> command to run is :  %s \n", command);
    printf(RESET "\n");
    cmd_out=open("/tmp/tmpout.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stdout del programma
    cmd_err=open("/tmp/tmperr.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stderr del programma
    cmd_date=open("/tmp/tmpdate.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente la data del programma
    dup2(cmd_out,1);//fa in modo che stdout punti al file temporaneo
    dup2(cmd_err,2);//fa in modo che stderr punti al file temporaneo
    r=system(command);//esegue il comando
    dup2(cmd_date,1);//fa in modo che lo stdout punti al file che conterra' la data di esecuzione
    system("date");//eseguo il comando che mi da la data
    dup2(st_out,1);//rimette lo stdout a schermo
    dup2(st_err,2);//rimette lo stderr a schermo
    close(cmd_out);//chiudo la scrittura del file che contiene lo stdout
    close(cmd_err);//chiudo la scrittura del file che contiene lo stderr
    close(cmd_date);//chiudo la scrittura del file che contiene la data
    close(st_out);//chiudo l'fd che mi ha salvato temporaneamente l'indice del video
    close(st_err);//chiudo l'fd che mi ha salvato temporaneamente l'indice del video

    //* --> mi salvo la data in una stringa */
    cmd_date=open("/tmp/tmpdate.cmd", O_RDONLY);
    wbytes = lseek(cmd_date, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
    lseek (cmd_date, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    cmd_strdate=(char *)malloc(wbytes * sizeof(char));
    if(cmd_strdate == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    if(read(cmd_date, cmd_strdate, wbytes)<0){//salvo il contenuto del file nella stringa creata
        printf("there was an error reading the out file\n");
    }
    close(cmd_date);
    strtok(cmd_strdate, "\n");

    //* --> converto il valore di ritorno in una stringa
    sprintf(cmd_r, "%d", r);
    
    /* --> gestisco il risulato dello stdout del comando */
    cmd_out=open("/tmp/tmpout.cmd", O_RDONLY);//apro il file in cui e' stato salvato temporaneamente lo stdout del programma
    wbytes = lseek(cmd_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (cmd_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    printf("number of bytes written on the cmd_out is : %i \n", wbytes);
    if(wbytes>0){//se ho letto dei dati vuol dire che il comando ha stampato qualcosa nel suo stdout
        cmd_strout = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file che contiene lo stdout
        if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "failed to create buffer");
            exit(1);
        }
        if(read(cmd_out, cmd_strout, wbytes)<0){//salvo il contenuto del file nella stringa creata
            printf("there was an error reading the out file\n");
        }
        printf("I've read this from cmd_out:\n%s", cmd_strout);//stampo il contenuto della stringa su schermo
        remove(out_path);//rimuove i file di log se gia' presente, altrimenti sovrascrive su dati gia' presenti
        fd[0]=open(out_path, O_WRONLY | O_CREAT, 0777);//apro il file in cui devo salvare il log di output
        if(fd[0]<0){
            printf("there was an error accessing the file\n");
        }
        write(fd[0],cmd_strdate,strlen(cmd_strdate));
        write(fd[0]," )-> successful execution\n",27);//salvo le informazioni necessarie sul file
        write(fd[0]," -----------\n", 14);
        write(fd[0],cmd_strout,strlen(cmd_strout));
        write(fd[0]," -----------\n", 14);
        write(fd[0],"return code is : ",18);
        write(fd[0],cmd_r,strlen(cmd_r));
        write(fd[0],"\n",2);
        write(fd[0]," ---------------------------\n", 30);
        close(fd[0]);
    }
    close(cmd_out);
    /* --> gestisco il risulato dello stderr del comando(allo stesso modo come ho fatto per il stdout) */
    cmd_err=open("/tmp/tmperr.cmd", O_RDONLY);//apro il file in cui e' stato salvato temporaneamente lo stderr del programma
    wbytes = lseek(cmd_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (cmd_err, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    printf("number of bytes written on the cmd_err is : %i \n", wbytes);
    if(wbytes>0){
        cmd_strerr = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file
        if(cmd_strerr == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "failed to create buffer");
            exit(1);
        }
        if(read(cmd_err, cmd_strerr, wbytes)<0){//salvo il contenuto del file in una stringa
            printf("there was an error reading the err file\n");
        }
        printf("I've read this from cmd_err:\n%s", cmd_strerr);//stampo il contenuto della string
        fd[1]=open(err_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output in append
        if(fd[1]<0){
            printf("there was an error accessing the file\n");
        }
        write(fd[1],cmd_strdate,strlen(cmd_strdate));
        write(fd[1]," )-> failed execution\n",23);
        write(fd[1]," -----------\n", 14);
        write(fd[1],cmd_strerr,strlen(cmd_strerr));
        write(fd[1]," -----------\n", 14);
        write(fd[1],"return code is : ",18);
        write(fd[1],cmd_r,strlen(cmd_r));
        write(fd[1],"\n",2);
        write(fd[1]," ---------------------------\n", 30);
        close(fd[1]);
        close(fd[1]);
    }
    printf("\nreturn value is : %i ", r);
    close(cmd_err);
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
