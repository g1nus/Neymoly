int standard_inp;
int standard_out;
int standard_err;

void setstd(){
    standard_inp = dup(0);//mi salvo lo stdin(tastiera)
    standard_out = dup(1);//mi salvo lo stdout(video)
    standard_err = dup(2);//mi salvo lo stderr(video)

}
//metodo che controlla se il comando è cd e se può lo esegue
int isCD(char *command)
{
    int i=0; //la posizione di "cd" nel comando
    int res=-1; //controllo cosa so (se è 1 ho trovato cd)
    int len=strlen(command);
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
            if(strstr(command,"..")!=NULL)
            {
                chdir("..");
            }
            else
            { 
                if(chdir(&(command[i]))<0){ //controlla se ha successo nel cambiare cartella
                    printf("<isCD:error> couldn't access directory\n");
                    res=0;
                }
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("<isCD:info> cartella di lavoro dopo il comando:: %s\n", cwd);
        }
    }
    return res;
}

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code, int input, int output, int error){//contiene input, output e error
    int cd = isCD(command);
    printf("<solo_run:info>cd = %i\n", cd);
    int fd;//uno per il file di log di output  e uno per il file di log di errori
    int tmp_out;//conterra' temporaneamente l'output del programma(file)
    int tmp_err;//conterra' temporaneamente lo stderr del programma(file)
    int tmp_date;//conterra' temporaneamente la data di esecuzione del programma(file)
    int r=0;//conterra' il valore di ritorno della funzione
    size_t wbytes;//conterra' il numero di byte scritti dal programma
    char *cmd_strout, *cmd_strerr, *cmd_strdate, cmd_strr[10];//conterra' le stringhe restituite dal programma

    //* --> preparo i file per l'esecuzione dei comandi sul file temporaneo */
    remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmperr.cmd");//rimuove l'output del precedente comando se presente
    remove("/tmp/tmpdate.cmd");//rimuove la date del precedente comando se presente
    tmp_out=open("/tmp/tmpout.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stdout del programma
    tmp_err=open("/tmp/tmperr.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente lo stderr del programma
    tmp_date=open("/tmp/tmpdate.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente la data del programma

    //* --> eseguo la data su un file temporaneo per poi salvarmele */
    dup2(tmp_date,1);//fa in modo che lo stdout punti al file che conterra' la data di esecuzione
    system("date");//eseguo il comando che mi da la data

    dup2(standard_out, 1);
    printf(YELLOW "[%i]<solo_run:info> command to run is (%s)\n" RESET,getpid(), command);
    fflush(stdout);

    //* --> eseguo il comando passato sui file temporanei e poi rimetto stout e stderr a quelli necessari */
    dup2(input, 0);//fa in modo che l'input derivi da tastiera o dal pipe passato
    dup2(tmp_out,1);//fa in modo che stdout punti al file temporaneo
    dup2(tmp_err,2);//fa in modo che stderr punti al file temporaneo
    if(cd!=1){//se il comando non e' un cd, oppure se e' un cd sbagliato
        r=WEXITSTATUS(system(command));//esegue il comando e salva in r la variabile di stato $?
    }else{
        printf(" \n");
    }
    dup2(output,1);//rimette lo stdout dove necessario
    dup2(error,2);//rimette lo stderr dove necessario

    //* --> converto il valore di ritorno in una stringa */
    sprintf(cmd_strr, "%d", r);
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

    //* --> gestisco il risulato dello stdout del comando */
    wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_out, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    if(wbytes>0){//se ho letto dei dati vuol dire che il comando ha stampato qualcosa nel suo stdout
        dup2(standard_out,1);
        printf(YELLOW "found %i bytes to read on the standard output", wbytes);
        cmd_strout = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file che contiene lo stdout
        if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "<solo_run:error> failed to create buffer");
            exit(1);
        }
        if(wbytes>max_len){
            printf("(exceded max_len, truncating output)");
            wbytes=max_len;
        }
        printf("\n" RESET);
        if(read(tmp_out, cmd_strout, wbytes)<0){//salvo il contenuto del file nella stringa creata
            printf("<solo_run:error> there was an error reading the out file\n");
        }
        close(tmp_out);//chiudo il file da cui ho fatto la lettura
        //* -> fix per l'ultimo carattere */
        if(strlen(cmd_strout)!=wbytes){
            printf(RED "suspicious character found at end of string, gonna fix\n" RESET);
            cmd_strout[wbytes] = 00;
        }
        fflush(stdout);
        dup2(output,1);
 
        printf("%s", cmd_strout);//stampo il contenuto della stringa sullo schermo o nel pipe
        fflush(stdout);
        fd=open(out_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare il log di output
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
    wbytes = lseek(tmp_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek (tmp_err, (off_t) 0, SEEK_SET);//riporto il puntatore all'inizio
    if(wbytes>0){
        dup2(standard_out,1);
        printf(YELLOW "found %i bytes to read on the standard error", wbytes);
        cmd_strerr = (char *)malloc(wbytes * sizeof(char));//alloco memoria necesseria per contenere il file che contiene lo stdout
        if(cmd_strerr == NULL){//controlla che il buffer sia stato effettivamente creato
            fprintf(stderr, "<solo_run:error> failed to create buffer");
            exit(1);
        }
        if(wbytes>max_len){
            printf("(exceded max_len, truncating output)");
            wbytes=max_len;
        }
        printf("\n" RESET);
        if(read(tmp_err, cmd_strerr, wbytes)<0){//salvo il contenuto del file nella stringa creata
            printf("<solo_run:error> there was an error reading the out file\n");
        }
        close(tmp_err);//chiudo il file da cui ho fatto la lettura
        //* -> fix per l'ultimo carattere */
        if(strlen(cmd_strerr)!=wbytes){
            printf(RED "suspicious character found at end of string, gonna fix\n" RESET);
            cmd_strout[wbytes] = 00;
        }
        fflush(stdout);
        dup2(output,1);
 
        fprintf(stderr, "%s", cmd_strerr);//stampo il contenuto della string sullo schermo o nel pipe
        fd=open(err_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output in append
        if(fd<0){
            printf("<solo_run:error> there was an error accessing the file\n");
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
    dup2(standard_inp,0);
    dup2(standard_out,1);
    dup2(standard_err,2);
    printf(YELLOW "\nreturn value is : %i ", r);
    printf(RESET "\n");
}

void pipedrun(char *cmd[], int y, char *out_path, char *err_path, int max_len, int code, int *tmppipe){
    printf(MAGENTA "[%i]<pipedrun:info>I'm a piped run, y is %i. Commands to run are: \n", getpid(), y);
    for(int i=0; i<y;i++){
        printf("(%s) ",cmd[i]);
    }
    printf("\n" RESET);
    fflush(stdout);
    int child_pid;//conterra' il pid del figlio
    int fd_pipe[2];//conterra' il pipe
    pipe(fd_pipe);//creo il pipe
    child_pid=fork();//forko
    if(child_pid>0){//padre
        //padre che fa l'ultima funzione quando il resto è finito
        close(fd_pipe[WRITE]);//la scrittura non mi serve, devo solo leggere cosa mi hanno dato i figli e al massimo "inoltrare" al padre superiore
        wait(NULL);//aspetto che i figli finiscano
        if(tmppipe==NULL){//se il pipe temporaneo non e' inizializzato ful dire che sono il padre principale(quello che deve eseguire l'ultimo comando)
            solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], standard_out, standard_err);//quindi eseguo il comando predendo in input cio' che mi hanno messo i figli nel pipe e stampo a schermo
        }else{//altrimenti vuol dire che esiste il pipe temporaneo e sono un "sotto-padre" che deve solo inoltrare ai superiori
            solo_run(cmd[y-1], out_path, err_path, max_len, code, fd_pipe[READ], tmppipe[WRITE], standard_err);//quindi leggo dal fd_pipe e mando l'output nel pipe temporaneo
        }
        close(fd_pipe[READ]);//quando ho finito chiudo il pipe
    }else{//figlio
        close(fd_pipe[READ]);//la lettura non mi serve
        if(y==3){//se c'e' praticamente solo un pipe vuol dire che sono l'ultimo figlio(quello che deve eseguire il primo comando)
            solo_run(cmd[0], out_path, err_path, max_len, code, standard_inp, fd_pipe[WRITE], standard_err);//eseguo il comando prendendo lo standard input e passandolo nel fd_pipe
        }else{//altrimenti vuol dire che sono un comando intermedio
            pipedrun(cmd, y-2, out_path, err_path, max_len, code, fd_pipe);//quindi richiamo la pipedrun passando pero' il pipe
        }
        close(fd_pipe[WRITE]);
        exit(0);
    }
}

