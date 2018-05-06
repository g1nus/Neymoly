#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
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

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code){
    int fd;//uno per il file di log di output  e uno per il file di log di errori
    int standard_out;//conterra' temporaneamente lo stdout(video)
    int standard_err;//conterra' temporaneamente lo stderr(video)
    int tmp_out;//conterra' temporaneamente l'output del programma(file)
    int tmp_err;//conterra' temporaneamente lo stderr del programma(file)
    int tmp_date;//conterra' temporaneamente la data di esecuzione del programma(file)
    int r;//conterra' il valore di ritorno della funzione
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_strout, *cmd_strerr, *cmd_strdate, cmd_strr[10];//conterra' le stringhe restituite dal programma

    //* --> stampo l'esecuzione dei comandi sul file temporaneo */
    standard_out = dup(1);//mi salvo lo stdout(video)
    standard_err = dup(2);//mi salvo lo stderr(video)
    remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmperr.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmpdate.cmd");//rimuove la date del precedente comando se presente
    printf(BLUE "<solo_run:info> command to run is :  %s \n", command);
    printf(RESET "\n");
    tmp_out=open("/tmp/tmpout.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stdout del programma
    tmp_err=open("/tmp/tmperr.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stderr del programma
    tmp_date=open("/tmp/tmpdate.cmd", O_WRONLY | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente la data del programma
    dup2(tmp_date,1);//fa in modo che lo stdout punti al file che conterra' la data di esecuzione
    system("date");//eseguo il comando che mi da la data
    dup2(tmp_out,1);//fa in modo che stdout punti al file temporaneo
    dup2(tmp_err,2);//fa in modo che stderr punti al file temporaneo
    r=WEXITSTATUS(system(command));//esegue il comando e salva in r la variabile di stato $?
    dup2(standard_out,1);//rimette lo stdout a schermo
    dup2(standard_err,2);//rimette lo stderr a schermo
    close(tmp_out);//chiudo la scrittura del file che contiene lo stdout
    close(tmp_err);//chiudo la scrittura del file che contiene lo stderr
    close(tmp_date);//chiudo la scrittura del file che contiene la data
    close(standard_out);//chiudo l'fd che mi ha salvato temporaneamente l'indice del video
    close(standard_err);//chiudo l'fd che mi ha salvato temporaneamente l'indice del video
    
    //* --> converto il valore di ritorno in una stringa */
    sprintf(cmd_strr, "%d", r);
    
    //* --> mi salvo la data in una stringa */
    tmp_date=open("/tmp/tmpdate.cmd", O_RDONLY);//apro il file che contiene la data in lettura
    wbytes = lseek(tmp_date, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
    lseek (tmp_date, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    cmd_strdate=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi la data in una stringa
    if(cmd_strdate == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "<solo_run:error> failed to create buffer");
        exit(1);
    }
    if(read(tmp_date, cmd_strdate, wbytes)<0){//salvo il contenuto del file nella stringa creata
        printf("<solo_run:error> there was an error reading the out file\n");
    }
    close(tmp_date);
    strtok(cmd_strdate, "\n");//tolgo il newline dalla data, non mi serve
    
    //* --> gestisco il risulato dello stdout del comando */
    tmp_out=open("/tmp/tmpout.cmd", O_RDONLY);//apro il file in cui e' stato salvato temporaneamente lo stdout del programma
    wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    printf("<solo_run:info> number of bytes written on the cmd_out is : %i \n", wbytes);
    if(wbytes>0){//se ho letto dei dati vuol dire che il comando ha stampato qualcosa nel suo stdout
        cmd_strout = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file che contiene lo stdout
        if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "<solo_run:error> failed to create buffer");
            exit(1);
        }
        if(read(tmp_out, cmd_strout, wbytes)<0){//salvo il contenuto del file nella stringa creata
            printf("<solo_run:error> there was an error reading the out file\n");
        }
        close(tmp_out);//chiudo il file da cui ho fatto la lettura
        printf("<solo_run:info> I've read this from tmp_out:\n%s", cmd_strout);//stampo il contenuto della stringa su schermo
        remove(out_path);//rimuove i file di log se gia' presente, altrimenti sovrascrive su dati gia' presenti
        fd=open(out_path, O_WRONLY | O_CREAT, 0777);//apro il file in cui devo salvare il log di output
        if(fd<0){
            printf("<solo_run:info> there was an error accessing the file\n");
        }
        /*-> salvo le informazioni sul file di log */
        write(fd,cmd_strdate,strlen(cmd_strdate));
        write(fd," )-> successful execution\n",27);
        write(fd," -----------\n", 14);
        write(fd,cmd_strout,strlen(cmd_strout));
        write(fd," -----------\n", 14);
        write(fd,"return code is : ",18);
        write(fd,cmd_strr,strlen(cmd_strr));
        write(fd,"\n",2);
        write(fd,"----------------------------\n", 30);
        close(fd);
    }

    //* --> gestisco il risulato dello stderr del comando(allo stesso modo come ho fatto per il stdout) */
    tmp_err=open("/tmp/tmperr.cmd", O_RDONLY);//apro il file in cui e' stato salvato temporaneamente lo stderr del programma
    wbytes = lseek(tmp_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_err, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    printf("<solo_run:info> number of bytes written on the cmd_err is : %i \n", wbytes);
    if(wbytes>0){
        cmd_strerr = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file
        if(cmd_strerr == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "<solo_run:error> failed to create buffer");
            exit(1);
        }
        if(read(tmp_err, cmd_strerr, wbytes)<0){//salvo il contenuto del file in una stringa
            printf("<solo_run:error> there was an error reading the err file\n");
        }
        close(tmp_err);
        printf("<solo_run:info> I've read this from cmd_err:\n%s", cmd_strerr);//stampo il contenuto della string
        fd=open(err_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output in append
        if(fd<0){
            printf("<solo_run:info> there was an error accessing the file\n");
        }
        write(fd,cmd_strdate,strlen(cmd_strdate));
        write(fd," )-> failed execution\n",23);
        write(fd," -----------\n", 14);
        write(fd,cmd_strerr,strlen(cmd_strerr));
        write(fd," -----------\n", 14);
        write(fd,"return code is : ",18);
        write(fd,cmd_strr,strlen(cmd_strr));
        write(fd,"\n",2);
        write(fd,"----------------------------\n", 30);
        close(fd);
        close(fd);
    }
    printf("\nreturn value is : %i ", r);
    printf("\n");
}


void duo_pipedrun(char *cmd[], int y, char *out_path, char *err_path, int max_len, int code){
    printf("<duo_pipedrun:info> getting into a basic piping\n");
    printf("<duo_pipedrun:info> first command to run is: %s\n", cmd[0]);
    int tmp_out;//conterra' temporaneamente l'output del programma(file)
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_strout;//conterra' le stringhe restituite dal programma

    remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente

    int child_pid;
    int fd_pipe[2];
    pipe(fd_pipe);
    child_pid=fork();
    if(child_pid>0){//padre
        printf("PADRE:esecuzione del comando\n");
        int standard_out;
        standard_out = dup(1);
        close(fd_pipe[READ]);
        dup2(fd_pipe[WRITE],1);
        system(cmd[0]);
        close(fd_pipe[WRITE]);
        dup2(standard_out,1);
        printf("PADRE:fine esecuzione comando\n");
        sleep(3);
        kill(child_pid,SIGKILL);
    }else{
        printf("FIGLIO:attesa di comando\n");
        int standard_in;
        standard_in = dup(0);
        close(fd_pipe[WRITE]);
        dup2(fd_pipe[READ],0);
        system(cmd[2]);
        close(fd_pipe[READ]);
        dup2(standard_in,0);
        printf("FIGLIO:fine escuzione comando\n");
    }
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
            if(y==3 && strcmp(cmd[1],"|")==0){
                duo_pipedrun(cmd, y, out_path, err_path, max_len, code);
            }
        }
    }
    return 0;
}
