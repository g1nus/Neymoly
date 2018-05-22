int cont=1;

int standard_inp;
int standard_out;
int standard_err;

void setstd(){
    standard_inp = dup(0);//mi salvo lo stdin(tastiera)
    standard_out = dup(1);//mi salvo lo stdout(video)
    standard_err = dup(2);//mi salvo lo stderr(video)
    //standard_err = open("/tmp/doyourjob");//mi salvo lo stderr(video)

}

void terminate_handler(){
  cont=0;
}

/*void k_handler(){
    system("clear");
    fflush(stdin);
    remove("/tmp/tmpout.cmd");
    exit(0);
}*/

//metodo che controlla se il comando è cd e se può lo esegue
int isCD(char *command)
{
    int i=0; //la posizione di "cd" nel comando
    int j;
    int res=-1; //controllo cosa so (se è 1 ho trovato cd)
    int len=strlen(command);
    char path[1024];
    while((res==-1)&&(len>i+4)) //controllo che non abbia già trovato cd e che ci sia spazio rimanente per un path
    {
        if(command[i]==' ')
            i++;
        else if ((command[i]=='c')&&(command[i+1]=='d')&&(command[i+2]==' '))
            res=1;
        else
            res=0;
    }
    if(res==1)//se è il comando cd entra qui
    {
	printf("<isCD:info> ho trovato il comando cd\n");
        //aumento i fino ad togliere tutti i parametri
        i+=3; //per arrivare alla prima parola dopo lo spazio
        int findparameter=0;    //se vale 1 sto passando sopra un parametro
        while(((command[i]==' ')||(command[i]!=' ' && findparameter==1)||(command[i]=='-'))&&(i<len))
        {
            if(command[i]=='-')
                findparameter=1;
            else if(findparameter==1 && command[i]==' ')
                findparameter=0;
            i++;
        }
        //a questo punto i dovrebbe essere la prima lettera del path
        if(i<len)    //verifico che si possa effettivamente continuare
        {
            j=i;
            while(command[j]!=' ' && j<len){
                j++;
            }
            printf("<isCD:info> last character of folder is %c\n", command[j-1]);
            //strncpy(path, command+i, j-1);
            memcpy( path, &command[i], j);
            strtok(path," ");
            printf("<isCD:info> path is: (%s)\n", path);
            findparameter=0;
            for(;j<len;j++){
                if(command[j]!=' '){
                    printf("<isCD:error> there's something fishy going on here\n");
                    findparameter=1;
                    res=0;
                }
            }
            if(findparameter==0){
                if(chdir(path)<0){ //controlla se ha successo nel cambiare cartella
                    printf("<isCD:error> couldn't access directory\n");
                    res=0;
                }
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("<isCD:info> cartella di lavoro dopo il comando:: %s\n", cwd);
        }
    }
    printf("<isCD:info> i=%i len=%i\n", i, len);
    return res;
}

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code, int input, int output, int error, int cc, int numd, int isGT){
    /*contiene comando, path per il log di output, path per il log di errore, lughezza massima dell'output, flag per il codice di ritorno,
    canali di fd, output ed errore, codice del comando, codice del sottocomando e controllo per la gestione dei file*/
    int cd = isCD(command);
    printf("<solo_run:info>cd = %i\n", cd);
    int fd;//uno per il file di log di output  e uno per il file di log di errori
    int tmp_out;//conterra' temporaneamente l'output del programma(file)
    int tmp_err;//conterra' temporaneamente lo stderr del programma(file)
    int tmp_date;//conterra' temporaneamente la data di esecuzione del programma(file)
    int tmp_ret;//conterra' il codice di ritorno del programma
    int r=0;//conterra' il valore di ritorno della funzione
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_strout, *cmd_strerr, *cmd_strdate, cmd_strr[10];//conterra' le stringhe restituite dal programma
    char tmp[12]={0x0};
    int t=0;//indica il numero delle letture
    int timeout=2;
    int pid, ppid,pppid=getpid(), status;
    char killer[300];char victim[50];
    //killer=(char *)malloc(30 * sizeof(char));
    strcpy(killer, "./last_pid.sh ");

    //* --> preparo i file per l'esecuzione dei comandi sul file temporaneo */
    dup2(standard_out, 1);
    if(isGT==0){
        remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente
        remove("/tmp/tmperr.cmd");//rimuove l'output del precedente comando se presente
        remove("/tmp/tmpdate.cmd");//rimuove la date del precedente comando se presente
        remove("/tmp/tmprrr.cmd");//rimuove la date del precedente comando se presente
        printf(YELLOW "[%i]<solo_run:info> command to run is (%s), cc=%i, numd=%i\n" RESET,getpid(), command, cc, numd);
    }else{
        printf(YELLOW "[%i]<solo_run:info> this is a file: %s\n, cc=%i, numd=%i\n" RESET,getpid(), command, cc, numd);
    }
    tmp_date=open("/tmp/tmpdate.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente la data del programma
    tmp_err=open("/tmp/tmperr.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stderr del programma
    tmp_out=open("/tmp/tmpout.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stdout del programma
    tmp_ret=open("/tmp/tmprrr.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato il codice di ritorno


    
    fflush(stdout);
    //* --> eseguo la data su un file temporaneo per poi salvarmele */
    dup2(tmp_date,1);//fa in modo che lo stdout punti al file che conterra' la data di esecuzione
    system("date");//eseguo il comando che mi da la data

    fflush(stdout);

    //* --> eseguo il comando passato sui file temporanei e poi rimetto stout e stderr a quelli necessari */
    dup2(input, 0);//fa in modo che l'fd derivi da tastiera o dal pipe passato
    if(cd!=1 && isGT==0){//se il comando non e' un cd, oppure se e' un cd sbagliato
        if ((pid = fork()) == 0){//son
            
            //signal(SIGKILL, k_handler);
            dup2(tmp_out,1);
            if(strcmp(command,"nano")!=0 && strcmp(command,"pico")!=0){
                dup2(tmp_err,2);
            }
            r=WEXITSTATUS(system(command));fflush(stdout);fflush(stderr);
            close(tmp_err);
            close(tmp_out);
            //* --> converto il valore di ritorno in una stringa */
            sprintf(cmd_strr, "%d", r);
            write(tmp_ret,cmd_strr,strlen(cmd_strr));
            kill(pppid,SIGUSR1);
            exit(0);
          }else{//father
            int first_time=1;
            cont=1;
            sprintf(victim, "%i", pid);
            
            signal(SIGUSR1, terminate_handler);
            dup2(output,1);
            //printf("waiting for magic to happen\n");fflush(stdout);
            sleep(1.3);
            while(cont || first_time){
                if(!first_time){
                    system("clear");
                }else{
                    first_time=0;
                }

                wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
                lseek (tmp_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
                //printf("found %i bytes to read\n",wbytes);
                if((int) wbytes > 0){
                    cmd_strout=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi la data in una stringa
                    if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
                        fprintf(stderr, "<solo_run:error> failed to create buffer\n");
                        exit(1);
                    }
                        if(read(tmp_out, cmd_strout, wbytes)<0){//salvo il contenuto del file nella stringa creata
                        printf("<solo_run:error> there was an error reading the out file\n");
                    }
                    if(strlen(cmd_strout)!=wbytes){
                        cmd_strout[wbytes] = 00;
                    }
                    printf("%s", cmd_strout);//stampo il contenuto della stringa sullo schermo o nel pipe
                    fflush(stdout);
                }

                wbytes = lseek(tmp_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
                lseek (tmp_err, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
                //printf("found %i bytes to read\n",wbytes);
                if((int) wbytes > 0){
                    cmd_strerr=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi la data in una stringa
                    if(cmd_strerr == NULL){//controlla che il buffer sia stato effettivamente creato
                        fprintf(stderr, "<solo_run:error> failed to create buffer\n");
                        exit(1);
                    }
                        if(read(tmp_err, cmd_strerr, wbytes)<0){//salvo il contenuto del file nella stringa creata
                        printf("<solo_run:error> there was an error reading the out file\n");
                    }
                    if(strlen(cmd_strerr)!=wbytes){
                        cmd_strerr[wbytes] = 00;
                    }
                    fprintf(stderr, "%s", cmd_strerr);//stampo il contenuto della string sullo schermo o nel pipe
                    fflush(stderr);
                }
                if(cont){
                    sleep(1);
                    system("clear");
                }
                t++;//incrementa il numero di letture
                if(t>timeout && cont==1){

                    strcpy(killer,klr);
                    strcat(killer,victim);
                    system(killer);
                    sleep(1);
                    //kill(pid,SIGQUIT);
                }
            }
            close(tmp_out);
            close(tmp_err);
            fflush(stdout);
            fflush(stderr);
            wait(&status);
          }
    }else if(cd==1){
        printf(" \n");
    }else if(isGT==1){
        dup2(output,1);
        wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
        lseek (tmp_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
        //printf("found %i bytes to read\n",wbytes);
        if((int) wbytes > 0){
            cmd_strout=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi la data in una stringa
            if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
                fprintf(stderr, "<solo_run:error> failed to create buffer\n");
                exit(1);
            }
                if(read(tmp_out, cmd_strout, wbytes)<0){//salvo il contenuto del file nella stringa creata
                printf("<solo_run:error> there was an error reading the out file\n");
            }
            if(strlen(cmd_strout)!=wbytes){
                cmd_strout[wbytes] = 00;
            }
            printf("%s", cmd_strout);//stampo il contenuto della stringa sullo schermo o nel pipe
            fflush(stdout);
        }
    }
    dup2(output,1);//rimette lo stdout dove necessario
    dup2(error,2);//rimette lo stderr dove necessario


    //* --> mi salvo il codice di ritorno in una stringa */
    wbytes = lseek(tmp_ret, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
    lseek (tmp_ret, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    if(read(tmp_ret, cmd_strr, wbytes)<0){//salvo il contenuto del file nella stringa creata
        printf("<solo_run:error> there was an error reading the out file\n");
    }
    if(strlen(cmd_strr)!=wbytes){
        cmd_strr[wbytes] = 00;
    }

    close(tmp_ret);



    //* --> mi salvo la data in una stringa */
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

    tmp_out=open("/tmp/tmpout.cmd", O_RDONLY);//apro il file in cui verra' salvato temporaneamente lo stdout del programma
    tmp_err=open("/tmp/tmperr.cmd", O_RDONLY);//apro il file in cui verra' salvato temporaneamente lo stderr del programma

    //* --> gestisco il risulato dello stdout del comando */
    wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    if((int)wbytes>0){
        if(strcmp(out_path, "/dev/null") != 0){
            fd=open(out_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare il log di output
            if(fd<0){
                printf("<solo_run:info> there was an error accessing the file\n");
            }
            /*-> salvo le informazioni sul file di log */
            write(fd,cmd_strdate,strlen(cmd_strdate));
            if(isGT==0){
                write(fd," )-> command printed on STDOUT\n",32);
            }else{
                write(fd," )-> FILE content\n", 19);
            }
            write(fd,"ID:",3);
            sprintf(tmp,"%11d", cc);
            write(fd,tmp,sizeof(tmp));
            write(fd,".",1);
            sprintf(tmp,"%11d", numd);
            write(fd,tmp,sizeof(tmp));
            write(fd,"\n",1);
            if(isGT==0){
                write(fd,"launched : ",12);
            }else{
                write(fd,"file name : ",13);
            }
            write(fd,command,strlen(command));
            write(fd,"\n",1);
            write(fd,"[ - START OF CONTENT - ]\n", 26);
            if(t>1){
                write(fd,"( - Impossibile stampare il contenuto dei comandi bloccanti - )\n", 65);
            }else{
                write(fd,cmd_strout,strlen(cmd_strout));
            }
            write(fd,"[ - END OF CONTENT - ]\n", 24);
            if(isGT==0 && code==1){
                write(fd,"return code is : ",18);
                write(fd,cmd_strr,strlen(cmd_strr));
                write(fd,"\n",2);
            }
            write(fd,"----------------------------\n", 30);
            close(fd);
        }
    }
    //free(cmd_strout);



    //* --> gestisco il risulato dello stderr del comando(allo stesso modo come ho fatto per il stdout) */
    wbytes = lseek(tmp_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_err, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    if((int)wbytes>0){
        if((strcmp(out_path, "/dev/null") != 0)&&(strcmp(err_path, "/dev/null") == 0))
        {
            fd=open(out_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output di errore in append
            if(fd<0){
                printf("<solo_run:error> there was an error accessing the file\n");
            }
            
        }
        if(strcmp(err_path, "/dev/null") != 0)
        {
            fd=open(err_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output di errore in append
            if(fd<0){
                printf("<solo_run:error> there was an error accessing the file\n");
            }
        }
        write(fd,cmd_strdate,strlen(cmd_strdate));
        write(fd," )-> command printed on STDERR\n",32);
        write(fd,"ID:",3);
        sprintf(tmp,"%11d", cc);
        write(fd,tmp,sizeof(tmp));
        write(fd,".",1);
        sprintf(tmp,"%11d", numd);
        write(fd,tmp,sizeof(tmp));
        write(fd,"\n",1);
        write(fd,"executed : ",12);
        write(fd,command,strlen(command));
        write(fd,"\n",1);
        write(fd,"[ - START OF CONTENT - ]\n", 26);
        if(t>1){
            write(fd,"( - Impossibile stampare il contenuto dei comandi bloccanti - )\n", 65);
        }else{
            write(fd,cmd_strerr,strlen(cmd_strerr));
        }
        write(fd,"[ - END OF CONTENT - ]\n", 24);
        if(code==1){
            write(fd,"return code is : ",18);
            write(fd,cmd_strr,strlen(cmd_strr));
            write(fd,"\n",2);
        }
        write(fd,"----------------------------\n", 30);
        close(fd);
        //free(cmd_strerr);
    }
    //free(cmd_strdate);
    fflush(stdout);
    dup2(standard_inp,0);
    dup2(standard_out,1);
    dup2(standard_err,2);
    if(isGT==0 && code==1){
        printf(YELLOW "\npid of son was %i, return value is : %s ", pid, cmd_strr);
    }
    printf(RESET "\n");
    fflush(stdout);
}

void pipedrun(char *cmd[], int y, int total, char *out_path, char *err_path, int max_len, int code, int *tmppipe, int cc, int num){
    int i;
    printf(MAGENTA "[%i]<pipedrun:info>I'm a piped run, y is %i, total is %i. Commands to run are: \n", getpid(), y, total);
    for(i=0; i<y;i++){
        printf("(%s) ",cmd[i]);
    }
    printf("\n[%i]<pipedrun:info>cc=%i, num=%i", getpid(),cc,num);
    printf("\n" RESET);
    fflush(stdout);
    int child_pid;//conterra' il pid del figlio
    int fd;
    int fd_pipe[2];//conterra' il pipe
    pipe(fd_pipe);//creo il pipe
    child_pid=fork();//forko
    if(child_pid>0){//padre
        //padre che fa l'ultima funzione quando il resto è finito
        close(fd_pipe[WRITE]);//la scrittura non mi serve, devo solo leggere cosa mi hanno dato i figli e al massimo "inoltrare" al padre superiore
        wait(NULL);//aspetto che i figli finiscano
        if(tmppipe==NULL){//se il pipe temporaneo non e' inizializzato ful dire che sono il padre principale(quello che deve eseguire l'ultimo comando)
            printf(MAGENTA "[%i]<pipedrun:info> [FIRST FATHER] The tmppipe is null, so I'm the main father\n", getpid());
            if(strcmp(cmd[y-2],"|")==0){
                printf(MAGENTA "[%i]<pipedrun:info> [FIRST FATHER] There is a pipe before the command\n", getpid());
                solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], standard_out, standard_err,cc,num,0);//quindi eseguo il comando predendo in fd cio' che mi hanno messo i figli nel pipe e stampo a schermo
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FIRST FATHER] There is a pipe before the command\n", getpid());
            }else if(strcmp(cmd[y-2],">")==0){
                printf(MAGENTA "[%i]<pipedrun:info> [FIRST FATHER] There is a greater before the command, opening %s\n", getpid(), cmd[y-1]);
                remove(cmd[y-1]);
                fd=open(cmd[y-1], O_WRONLY | O_CREAT, 0777);
                solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], fd, standard_err,cc,num,1);//quindi eseguo il comando predendo in fd cio' che mi hanno messo i figli nel pipe e stampo a schermo
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FIRST FATHER] There is a greater before the command, opening %s\n", getpid(), cmd[y-1]);
            }else{
                printf(MAGENTA "[%i]<pipedrun:info> [FIRST ATHER] I'm an fd file(%s), I don't have to do nothing(maybe)\n", getpid(), cmd[y-1]);
                if(y>3 && y==total){
                    printf(MAGENTA "[%i]<pipedrun:info> [FIRST FATHER] I'm the last fd file(%s), I need to print what was done on screen\n", getpid(), cmd[y-1]);
                    solo_run(cmd[y-1], out_path, err_path, max_len, code, standard_inp, standard_out, standard_err,cc,num,1);
                    printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FIRST FATHER] I'm the last fd file(%s), I need to print what was done on screen\n", getpid(), cmd[y-1]);
                }
            }
        }else{//altrimenti vuol dire che esiste il pipe temporaneo e sono un "sotto-padre" che deve solo inoltrare ai superiori
            printf(MAGENTA "[%i]<pipedrun:info> [FATHER] The tmppipe is already created, so I'm a random father\n", getpid());
            if(strcmp(cmd[y-2],"|")==0){
                printf(MAGENTA "[%i]<pipedrun:info> [FATHER] There is a pipe before the command\n", getpid());
                solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], tmppipe[WRITE], standard_err,cc,num,0);//quindi leggo dal fd_pipe e mando l'output nel pipe temporaneo
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FATHER] There is a pipe before the command\n", getpid());
            }else if(strcmp(cmd[y-2],">")==0){
                printf(MAGENTA "[%i]<pipedrun:info> [FATHER] There is a greater before the command, opening %s\n", getpid(), cmd[y-1]);
                remove(cmd[y-1]);
                fd=open(cmd[y-1], O_WRONLY | O_CREAT, 0777);
                solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], fd, standard_err,cc,num,1);
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FATHER] There is a greater before the command, opening %s\n", getpid(), cmd[y-1]);
            }else{
                //...scorrere fino al prossimo comando e passargli fd_pipe[READ]
                printf(MAGENTA "[%i]<pipedrun:info> [FATHER] I'm an fd file(%s), I need to forward to the next command\n", getpid(), cmd[y-1]);
                solo_run(cmd[y-1], out_path, err_path, max_len, code, standard_inp, tmppipe[WRITE], standard_err,cc,num,1);
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:FATHER] I'm an fd file(%s), I need to forward to the next command\n", getpid(), cmd[y-1]);
            }
        }
        close(fd_pipe[READ]);//quando ho finito chiudo il pipe
    }else{//figlio
        close(fd_pipe[READ]);//la lettura non mi serve
        if(y==3){//se c'e' praticamente solo un pipe vuol dire che sono l'ultimo figlio(quello che deve eseguire il primo comando)
            printf(MAGENTA "[%i]<pipedrun:info> [LAST SON] y is 3 so I'm the first command\n", getpid());
            if(strcmp(cmd[1],"|")==0 || strcmp(cmd[1],">")==0){
                printf(MAGENTA "[%i]<pipedrun:info> [LAST SON] Found pipe or output file\n", getpid());
                solo_run(cmd[0], out_path, err_path, max_len, code, standard_inp, fd_pipe[WRITE], standard_err,cc,1,0);//eseguo il comando prendendo lo standard fd e passandolo nel fd_pipe
                printf(MAGENTA "[%i]<pipedrun:info> [FINISH:LAST SON] Found pipe or output file\n", getpid());
            }else if(strcmp(cmd[1],"<")==0){
                printf("[%i]<piperun:info> [LAST SON] Found an fd file\n", getpid());
                fd=open(cmd[2], O_RDONLY, 0777);
                if(fd<0){
                    printf("[%i]<piperun:info> [LAST SON] Couldn't open the file\n", getpid());
                }else{
                    //...controllare se si tratta di dover fare uno standard output o un pipe output
                    if(total==3){
                        solo_run(cmd[0], out_path, err_path, max_len, code, fd, standard_out, standard_err,cc,1,0);//eseguo il comando prendendo lo standard fd e passandolo nel fd_pipe
                    }else{
                        solo_run(cmd[0], out_path, err_path, max_len, code, fd, fd_pipe[WRITE], standard_err,cc,1,0);//eseguo il comando prendendo lo standard fd e passandolo nel fd_pipe
                    }
                }
                printf("[%i]<piperun:info> [FINISH:LAST SON] Found an fd file\n", getpid());
            }
        }else{//altrimenti vuol dire che sono un comando intermedio
            printf(MAGENTA "[%i]<pipedrun:info> [SON] y is %i gonna call myself\n", getpid(),y);
            pipedrun(cmd, y-2, total, out_path, err_path, max_len, code, fd_pipe,cc,--num);//quindi richiamo la pipedrun passando pero' il pipe
            printf(MAGENTA "[%i]<pipedrun:info> [FINISH:SON] y is %i gonna call myself\n", getpid(),y);
        }
        close(fd_pipe[WRITE]);
        exit(0);
    }
    printf(MAGENTA "[%i]<pipedrun:info>I'm a piped run, y is %i and I've finished \n", getpid(), y);
}
