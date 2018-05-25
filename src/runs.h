int going;//variabile usata dalla solo_run per notificare quando il figlio smette di eseguire

char klr[500];//conterra' il path allo script che si occupa di terminare il figlio che si genera con la chiamata system

void terminate_handler(){
    // -> se viene ricevuto un segnale da parte del figlio verra' messo going a 0 perche' vuol dire che non sta piu' eseguendo
    going = 0;
}

int isCD(char *command){
    // -> metodo che controlla se il comando e' cd e se puo' lo esegue
    int i=0;//conterra' la posizione di "cd" nel comando
    int j;//servira' a scorrere fino alla fine del path
    int res=-1;//se sara' 1 vuol dire che ho trovato cd
    int len=strlen(command);
    char path[100];//conterra' il path a cui si vuole accedere
    while((res == -1) && (len > i + 4)){//controllo che non abbia già trovato cd e che ci sia spazio rimanente per un path
        if(command[i] == ' '){//se trovo spazio scorro in avanti di uno
            i++;
        }else if ((command[i] == 'c') && (command[i+1] == 'd') && (command[i+2] == ' ')){//metto res a 1 se trovo "cd "
            res=1;
        }else{//altrimenti rimane a 0
            res=0;
        }
    }
    if(res == 1){//se ho trovato il comando cd provo a cercare il path
        i += 3;//per arrivare alla prima parola dopo lo spazio
        int findparameter=0;//se vale 1 sto passando sopra un parametro
        while(((command[i] == ' ') || (command[i] != ' ' && findparameter == 1) || (command[i] == '-')) && (i < len)){//ciclo sui possibili parametri
            if(command[i] == '-'){
                findparameter=1;
            }else if(findparameter==1 && command[i]==' '){
                findparameter=0;
            }
            i++;
        }
        /*a questo punto l'indice i dovrebbe essere la prima lettera del path*/
        if(i<len){//verifico che si possa effettivamente continuare
            j=i;
            while(command[j] != ' ' && j < len){//scorro con j sul presunto path
                j++;
            }
            memcpy(path, &command[i], j);//mi salvo il path che inizia in i e finisce in j
            strtok(path, " ");//tolgo lo spazio dal path se presente
            findparameter = 0;
            for(; j < len; j++){//controllo che non ci sia qualcos'altro dopo il path
                if(command[j] != ' '){//se c'e' qualcos'altro setto findparameter a 1
                    findparameter = 1;
                    res = 0;
                }
            }
            if(findparameter == 0){//se non c'e' altro provo a fare chdir
                if(chdir(path) < 0){//controlla se ha successo nel cambiare cartella
                    fprintf(stderr, RED "couldn't access directory\n" RESET);
                    res=0;
                }
            }
        }
    }
    return res;//ritorno l'outcome della funzione
}

void solo_run(char *command, char *out_path, char *err_path, int max_len, int code, int timeout, int input, int output, int error, int cc, int numd, int isGT){
    // -> solo_run si occupa di eseguire i comandi, gli vengono passati: comando, path per il log di output, path per il log di errore, lughezza massima dell'output, flag per il codice di ritorno, il tempo di timeout, canali di input, output ed errore, ID del comando, ID del sottocomando e flag controllo per la gestione dei file
    int cd = isCD(command);//prima di tutto controllo se si tratta di un "cd" e se necessario cambio la working directory

    /*dichiaro i file descriptors necessari*/
    int fd;//conterra' il file descriptor per il file di log e output
    int tmp_out;//conterra' temporaneamente l'output del programma(file)
    int tmp_err;//conterra' temporaneamente lo stderr del programma(file)
    int tmp_date;//conterra' temporaneamente la data di esecuzione del programma(file)
    int tmp_ret;//conterra' il codice di ritorno del programma(file)
    int r = 0;//conterra' il valore di ritorno del comando sotto forma di intero, di base impostato a 0, non e' detto che debba eseguire qualcosa

    /*dichiaro ulteriori variabili di supporto*/
    size_t wbytes;//conterra' il numero di byte scritti nel file dal programma
    char *cmd_strout, *cmd_strerr, *cmd_strdate, cmd_strr[10];//conterranno le stringhe restituite dal programma, la data e il codice di ritorno
    int t=0;//indichera' quante volte e' stato letto l'output del programma, serve principalmente per identificare i comandi bloccanti e fare timeout
    int pid, ppid = getpid();//conterrano il pid del figlio e l'attuale pid(padre)
    char tmp[12] = {0x0};//carattere di tabulazione
    char killer[300];char victim[50];//conterrano la stringa per lanciare lo script che termina il figlio e il pid del figlio sotto forma di stringa

    /*preparo i file necessari all'output del comando*/
    if(isGT == 0){//se non si tratta di dover leggere da un file che contiene un output precedente
        remove("/tmp/tmpout.cmd");//rimuove l'output del precedente comando se presente
        remove("/tmp/tmperr.cmd");//rimuove l'output del precedente comando se presente
        remove("/tmp/tmpdate.cmd");//rimuove la date del precedente comando se presente
        remove("/tmp/tmprrr.cmd");//rimuove la date del precedente comando se presente
    }
    tmp_date=open("/tmp/tmpdate.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato temporaneamente la data del programma
    tmp_err=open("/tmp/tmperr.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato lo stderr del programma
    tmp_out=open("/tmp/tmpout.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato lo stdout del programma
    tmp_ret=open("/tmp/tmprrr.cmd", O_RDWR | O_CREAT, 0777);//apro il file in cui verra' salvato il codice di ritorno

    /*eseguo la data sul file di data temporaneo per poi salvarmela*/
    dup2(tmp_date,1);//fa in modo che lo stdout punti al file che conterra' la data di esecuzione
    system("date");//eseguo il comando che mi da la data
    fflush(stdout);

    /*eseguo il comando se necessario altrimenti mi occupo di gestire cd oppure l'accesso ai file*/
    dup2(input, 0);//fa in modo che l'input derivi da tastiera o dal pipe passato alla funzione come input
    if(cd != 1 && isGT == 0){//se si tratta di un comando valido forko
        if ((pid = fork()) == 0){
            /*figlio che si occupera' di eseguire il comando e di stampare il suo output nei file temporanei*/
            dup2(tmp_out,1);//metto il stdout al file temporaneo
            if(strcmp(command, "nano") != 0 && strcmp(command, "pico") != 0){//fix per nano e pico che non accettano tmp_err(?)
                dup2(tmp_err,2);//metto il stderr al file temporaneo
            }
            r = WEXITSTATUS(system(command)); fflush(stdout); fflush(stderr);//eseguo il comando grazie a system, WEXITSTATUS mi ritornera' il codice di ritorno dell'esecuzione
            sprintf(cmd_strr, "%d", r);//converto in una stringa il codice di ritorno per poterla stampare in un file
            write(tmp_ret,cmd_strr,strlen(cmd_strr));
            kill(ppid,SIGUSR1);//invio il segnale a padre per dire che ho finito
            exit(0);//esco
        }else{
            /*padre che si occupera' di leggere cosa scrive il filgio nel file temporaneo e di inoltrarlo ai canali di output ed errore*/
            signal(SIGUSR1, terminate_handler);//assegno al segnale l'esecuzione delle funzione terminate_handler
            int first_time = 1;//inzialmente setto first_time a 1 per specificare che sot facendo la prima lettura
            int x = 0;//flag che mi indica se ho superato la max_len
            going=1;//going a 1 vuol dire che il figlio sta teoricamente eseguendo
            sprintf(victim, "%i", pid);//trasformo il pid del figlio in uns stringa
            dup2(output, 1);//setto l'output dove necessario
            sleep(1.3);//aspetto che il figlio esegua almeno una volta
            while(going || first_time){//se si tratta di un first time oppure se il figlio sta ancora eseguendo ciclo
                if(first_time){//se era un first time ora non lo e' piu'
                    first_time=0;
                }
                /*controllo l'output del figlio e lo stampo dove necessario*/
                x = 0;
                wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di output del figlio
                lseek(tmp_out, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
                //(might be useful) printf("found %i bytes to read\n",wbytes);
                if(wbytes > max_len){//controllo se ho superato max_len
                    wbytes = max_len;
                    x = 1;//notifico che ho dovuto troncare l'output
                }
                if((int)wbytes > 0){//se ho trovato dei bytes da leggere nel file di output
                    cmd_strout=(char *)malloc((wbytes + 1) * sizeof(char));//alloco lo spazio di memoria necessario
                    if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
                        fprintf(stderr, "<solo_run:error> failed to create buffer\n");
                        exit(1);
                    }
                    if(read(tmp_out, cmd_strout, wbytes) < 0){//salvo il contenuto del file nella stringa allocata
                        fprintf(stderr, "<solo_run:error> there was an error reading the out file\n");
                        exit(1);
                    }
                    if(strlen(cmd_strout) != wbytes){//fix per caratteri di fine stringa
                        cmd_strout[wbytes] = 00;
                    }
                    if(x == 1 && cmd_strout[strlen(cmd_strout)-1] != '\n'){//aggiungo un carattere di invio se la stringa e' stata troncata
                        strcat(cmd_strout,"\n");
                    }
                    printf("%s", cmd_strout);//stampo il contenuto della stringa sullo schermo o nel pipe
                    fflush(stdout);
                }
                /*controllo lo stderr del figlio e lo stampo sempre a video stavolta*/
                x = 0;
                wbytes = lseek(tmp_err, (size_t)0, SEEK_END);
                lseek(tmp_err, (off_t)0, SEEK_SET);
                //(might be useful) printf("found %i bytes to read\n",wbytes);
                if(wbytes > max_len){//controllo se ho superato max_len
                    wbytes = max_len;
                    x = 1;//notifico che ho dovuto troncare l'output
                }
                if((int)wbytes > 0){
                    cmd_strerr=(char *)malloc((wbytes + 1) * sizeof(char));
                    if(cmd_strerr == NULL){
                        fprintf(stderr, "<solo_run:error> failed to create buffer\n");
                        exit(1);
                    }
                    if(read(tmp_err, cmd_strerr, wbytes) < 0){
                        fprintf(stderr, "<solo_run:error> there was an error reading the out file\n");
                        exit(1);
                    }
                    if(strlen(cmd_strerr) != wbytes){//fix per l'ultimo carattere
                        cmd_strerr[wbytes] = 00;
                    }
                    if(x == 1 && cmd_strerr[strlen(cmd_strerr)-1] != '\n'){//aggiungo un carattere di invio se la stringa e' stata troncata
                        strcat(cmd_strerr,"\n");
                    }
                    fprintf(stderr, "%s", cmd_strerr);//stampo il contenuto della stringa sullo schermo
                    fflush(stderr);
                }
                if(going){//se il figlio sta ancora eseguendo
                    sleep(1);//aspetto un secondo
                    system("clear");//"pulisco"
                }
                t++;//incrementa il numero di letture
                if(timeout > 0 && (t > timeout && going == 1)){//se e' stato definito un timeout e il figlio lo sta superando
                    strcpy(killer,klr);//preparo la stringa per richiamare lo script
                    strcat(killer,victim);
                    system(killer);//richiamo lo script
                    sleep(1);
                }
            }
            fflush(stdout);
            fflush(stderr);
            wait(NULL);
        }
    }else if(cd == 1){
        /*se si e' trattato di un cd non eseguo niente ma stampo una riga vuota(serve per il log)*/
        cmd_strout = "\n";
    }else if(isGT == 1){
        /*se si tratta di un file per l'ouput stampo al suo interno l'ouput del precedente comando*/
        dup2(output, 1);
        wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte dle file che contiene l'ouput del comando precedente
        lseek (tmp_out, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
        if((int) wbytes > 0){//se il precedente comando ha stampato in output procedo
            cmd_strout=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi il contenuto in una stringa
            if(cmd_strout == NULL){//controlla che il buffer sia stato effettivamente creato
                fprintf(stderr, RED "<solo_run:error> failed to create buffer\n" RESET);
                exit(1);
            }
            if(read(tmp_out, cmd_strout, wbytes) < 0){//salvo il contenuto del file nella stringa creata
                fprintf(stderr, RED "<solo_run:error> there was an error reading the out file\n" RESET);
            }
            if(strlen(cmd_strout) != wbytes){//fix per l'ultimo carattere
                cmd_strout[wbytes] = 00;
            }
            printf("%s", cmd_strout);//stampo il contenuto della stringa sullo schermo o nel pipe
            fflush(stdout);
        }
    }

    /*ora mi cerco il codice di ritorno che il comando eseguito ha salvato nel file tmp_ret*/
    wbytes = lseek(tmp_ret, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
    if((int) wbytes > 0){
        lseek (tmp_ret, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
        if(read(tmp_ret, cmd_strr, wbytes) < 0){//salvo il contenuto del file nella stringa creata
            fprintf(stderr, RED "<solo_run:error> there was an error reading the out file\n" RESET);
        }
        if(strlen(cmd_strr) != wbytes){
            cmd_strr[wbytes] = 00;
        }
    }
    close(tmp_ret);//chiudo il file con il codice di ritorno siccome me lo sono salvato in una stringa

    /*ora accedo al file che contiene la data di esecuzione*/
    wbytes = lseek(tmp_date, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di data
    lseek (tmp_date, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
    cmd_strdate=(char *)malloc(wbytes * sizeof(char));//alloco lo spazio di memoria necessaria per salvarmi la data in una stringa
    if(cmd_strdate == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "<solo_run:error> failed to create buffer");
        exit(1);
    }
    if(read(tmp_date, cmd_strdate, wbytes) < 0){//salvo il contenuto del file nella stringa creata
        fprintf(stderr, "<solo_run:error> there was an error reading the out file\n");
    }
    close(tmp_date);//ora che mi sono salvato la data chiudo il file
    strtok(cmd_strdate, "\n");//tolgo il newline dalla data, non mi serve

    /*gestisco il risulato dello stdout del comando*/
    wbytes = lseek(tmp_out, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek(tmp_out, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
    if((int) wbytes > 0){//se ho trovato dei bytes da leggere mi occupo di stampare le informazioni nel file di log(nel caso del cd trovo solo un invio)
        fd = open(out_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare il log di output
        if(fd < 0){
            fprintf(stderr, RED "<solo_run:info> there was an error accessing the file\n" RESET);
        }
        /*salvo le informazioni sul file di log*/
        write(fd, cmd_strdate, strlen(cmd_strdate));
        if(isGT == 0){//controllo se si tratta di un comando o di un file e stampo cio' che e' necessario
            write(fd, " )-> command printed on STDOUT\n", 32);
        }else{
            write(fd, " )-> FILE content\n", 19);
        }
        write(fd, "ID:", 4);
        sprintf(tmp, "%11d", cc);
        write(fd, tmp, sizeof(tmp));
        write(fd, ".", 2);
        sprintf(tmp, "%11d", numd);
        write(fd, tmp, sizeof(tmp));
        write(fd, "\n", 2);
        if(isGT == 0){
            write(fd, "launched : ", 12);
        }else{
            write(fd, "file name : ", 13);
        }
        write(fd,command,strlen(command));
        write(fd,"\n",1);
        write(fd,"[ - START OF CONTENT - ]\n", 26);
        if(t > 1){
            write(fd, "( - Impossibile stampare il contenuto dei comandi bloccanti - )\n", 65);
        }else{
            write(fd, cmd_strout, strlen(cmd_strout));
        }
        write(fd, "[ - END OF CONTENT - ]\n", 24);
        if(isGT == 0 && code == 1){//se il flag per il codice di ritorno e' true e non si tratta di un file salvo il codice di ritorno del comando
            write(fd, "return code is : ", 18);
            write(fd, cmd_strr, strlen(cmd_strr));
            write(fd, "\n", 2);
        }
        write(fd, "----------------------------\n", 30);
        close(fd);
    }

    /*faccio lo stesso lavoro per il stderr*/
    wbytes = lseek(tmp_err, (size_t)0, SEEK_END);//mi salvo il numero di byte contenuti nel file di stdout
    lseek(tmp_err, (off_t)0, SEEK_SET);//riporto il puntatore all'inizio
    if((int) wbytes > 0){
        if(strcmp(err_path, "/dev/null") == 0){//se il file di errori non e' stato settano stampo nel file di log dello stdout
            fd = open(out_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output di errore in append
            if(fd < 0){
                fprintf(stderr, RED "<solo_run:error> there was an error accessing the file\n" RESET);
            }

        }else{//altrimenti apro il file di errore definito
            fd = open(err_path, O_WRONLY | O_APPEND | O_CREAT, 0777);//apro il file in cui devo salvare l'output di errore in append
            if(fd < 0){
                fprintf(stderr, RED "<solo_run:error> there was an error accessing the file\n" RESET);
            }
        }
        write(fd, cmd_strdate, strlen(cmd_strdate));
        write(fd, " )-> command printed on STDERR\n", 32);
        write(fd, "ID:", 3);
        sprintf(tmp, "%11d", cc);
        write(fd, tmp, sizeof(tmp));
        write(fd, ".", 1);
        sprintf(tmp, "%11d", numd);
        write(fd, tmp, sizeof(tmp));
        write(fd, "\n", 2);
        write(fd, "executed : ", 12);
        write(fd, command, strlen(command));
        write(fd, "\n", 1);
        write(fd, "[ - START OF CONTENT - ]\n", 26);
        if(t > 1){
            write(fd, "( - Impossibile stampare il contenuto dei comandi bloccanti - )\n", 65);
        }else{
            write(fd, cmd_strerr, strlen(cmd_strerr));
        }
        write(fd, "[ - END OF CONTENT - ]\n", 24);
        if(code == 1){
            write(fd, "return code is : ", 18);
            write(fd, cmd_strr, strlen(cmd_strr));
            write(fd, "\n", 2);
        }
        write(fd, "----------------------------\n", 30);
        close(fd);
    }
    fflush(stdout);
    fflush(stderr);

    /*rimetto i canali standard e stampo il codice di ritorno se necessario*/
    dup2(standard_inp, 0);
    dup2(standard_out, 1);
    dup2(standard_err, 2);
    if(isGT == 0 && code == 1){
        printf(YELLOW "\npid of son was %i, return value is : %s\n" RESET, pid, cmd_strr);
    }
    fflush(stdout);
}

void piped_run(char *cmd[], int y, int total, char *out_path, char *err_path, int max_len, int code, int timeout, int *tmppipe, int cc, int num){
    // - si occupa di eseguire comandi multipli passati in input, prende come parametri: l'insieme di argomenti, il numero di comandi rimanenti, il numero di comandi totali, i paramentri di esecuzione e i path, un pipe, il numero di comandi eseguiti fino ad adesso e la posizione del comando da eseguire nella lista. I comandi verrano sempre eseguiti da sinistra verso destra e nella fork il padre rappresenta sempre l'ultimo comando a destra nell'insieme dei sottocomandi
    int child_pid;//conterra' il pid del figlio
    int fd;//quando necessatio conterra' i file specificati dalla stringa
    int fd_pipe[2];//pipe che si prendera l'input di un comando e lo spedira' all'input del successivo
    pipe(fd_pipe);//creo il pipe
    child_pid=fork();//forko
    if(child_pid > 0){
        /*padre che si occupa di eseguire l'ultimo comando a destra della sottolista di comandi*/
        close(fd_pipe[WRITE]);//la scrittura non mi serve, devo solo leggere cosa mi hanno dato i figli e al massimo "inoltrare" al padre superiore
        wait(NULL);//aspetto che i figli finiscano
        if(tmppipe == NULL){//se il pipe temporaneo non e' inizializzato vuol dire che sono il padre principale(quello che dovrebbe eseguire l'ultimo comando)
            /*devo controllare se sono effettivamente un comando o un file*/
            if(strcmp(cmd[y-2], "|") == 0){//se prima di me c'e' un pipe vuol dire che sono un comando
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, fd_pipe[READ], standard_out, standard_err,cc,num,0);//quindi eseguo il comando predendo in input cio' che mi hanno messo i figli nel pipe e stampo a schermo
            }else if(strcmp(cmd[y-2], ">") == 0){//se prima c'e' un > vuol dire che sono un file di output
                remove(cmd[y-1]);//rimuovo il file di output se gia' presente
                fd = open(cmd[y-1], O_RDWR | O_CREAT, 0777);//apro il file
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, fd_pipe[READ], fd, standard_err,cc,num,1);//eseguo mettendo isGT a 1 e passando come ouput il file specificato
            }else{//se prima c'e' un < vuol dire che sono un file di input, il comando precedente si e' gia' occupato di gestirmi, devo comunque stampare nel file di log il mio contenuto
                fd = open("/dev/null",O_WRONLY);//stampo l'output standard a /dev/null tanto non ci faccio niente
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, standard_inp, standard_out, standard_err,cc,num,1);
            }
       }else{//altrimenti vuol dire che esiste il pipe temporaneo e sono un "sotto-padre" che deve eseguire e inoltrare ai superiori
            /*anche qui devo controllare se si tratta di un comando o un file*/
            if(strcmp(cmd[y-2], "|") == 0){//se prima di me c'e' un pipe vuol dire che sono un comando
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, fd_pipe[READ], tmppipe[WRITE], standard_err, cc, num, 0);//quindi leggo dal fd_pipe e mando l'output nel pipe temporaneo
            }else if(strcmp(cmd[y-2], ">") == 0){//se prima c'e' un > vuol dire che sono un file di output
                remove(cmd[y-1]);//rimuovo il file se era gia' presente
                fd = open(cmd[y-1], O_WRONLY | O_CREAT, 0777);
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, fd_pipe[READ], fd, standard_err,cc, num, 1);//setto isGT a 1 per indicare che devo stampare sul file l'output del comando precedente
            }else{//se prima c'e' un < vuol dire che sono un file di input, il comando precedente si e' gia' occupato di gestirmi io devo solo forwardare l'ouput precedente al prossimo comando
                solo_run(cmd[y-1], out_path, err_path, max_len, code, timeout, standard_inp, tmppipe[WRITE], standard_err, cc, num, 1);
            }
        }
        close(fd_pipe[READ]);//quando ho finito chiudo il pipe di lettura
    }else{//figlio che esegue l'ultimo comando della lista
        /*nel caso in cui non sia l'ultimo comando richiama se stesso(piped_run)*/
        close(fd_pipe[READ]);//la lettura non mi serve
        if(y == 3){//se y e' 3 sono l'ultimo figlio(quello che deve eseguire il primo comando)
            if(strcmp(cmd[1], "|") == 0 || strcmp(cmd[1], ">") == 0){//se dopo di me c'e' un pipe o un file di output eseguo e inoltro al padre(cioe' stampo nel pipe, sara' poi il padre a leggere dal pipe)
                solo_run(cmd[0], out_path, err_path, max_len, code, timeout, standard_inp, fd_pipe[WRITE], standard_err, cc, 1, 0);//eseguo il comando prendendo lo standard input e passandolo nel fd_pipe
            }else if(strcmp(cmd[1], "<") == 0){//se e' un < vuol dire che devo prendere in input il suo contenuto
                fd = open(cmd[2], O_RDONLY);//apro il file presente
                if(fd < 0){//controllo se sono riuscito ad aprire i file
                    fprintf(stderr, RED "<solo_run:error> there was an error accessing the file\n" RESET);
                }else{//se sono riuscito ad aprirlo
                    solo_run(cmd[0], out_path, err_path, max_len, code, timeout, fd, fd_pipe[WRITE], standard_err, cc, 1, 0);//eseguo il comando prendendo lo standard fd e passandolo nel fd_pipe
                }
            }
        }else{//altrimenti vuol dire che sono un figlio intermedio e devo richiamare piped_run fino all'ultimo figlio
            piped_run(cmd, y-2, total, out_path, err_path, max_len, code, timeout, fd_pipe,cc,--num);//quindi richiamo la piped_run passando pero' il pipe
        }
        close(fd_pipe[WRITE]);//quando ho finito chiudo il pipe di scrittura
        exit(0);
    }
}
