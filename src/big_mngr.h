// non e' stato definito in maniera esplicita il tipo booleano : 0 ==> false | 1 ==> true

/*variabili globali che conterranno i canali standard*/
int standard_inp;
int standard_out;
int standard_err;

void setstd(){
    // -> salvo i canali standard
    standard_inp = dup(0);//mi salvo lo stdin(tastiera)
    standard_out = dup(1);//mi salvo lo stdout(video)
    standard_err = dup(2);//mi salvo lo stderr(video)

}

int chk_nmbr(char *str){
    // -> passata una stringa controllo che sia composta solo da cifre
    int j;
    for(j = 0; j < strlen(str); j++){
        if(!(str[j] >= '0' && str[j] <= '9')){
            return 0;
        }
    }
    return 1;
}

int chk_ascii(char *str){
    // -> controlla se la stringa contiene solo caratteri ASCII standard
    int j;
    for(j = 0; j < strlen(str); j++){
        if(!(str[j] >= 0 && str[j] <= 127)){
            return 0;
        }
    }
    return 1;
}

void print_help(){
    // -> stampa una piccola guida
    printf(RESET "HOW TO USE START THE PROGRAM : \n");
    printf("\t to set the output file    : --outfile=\"<path>\" or -o <path>\n");
    printf("\t to set the error file     : --errfile=\"<path>\" or -e <path>\n");
    printf("\t to set the code flag      : --code=\"true\" or -c true or --code=\"false\" or -c false \n");
    printf("\t to set the max lenght     : --maxlength=\"<int>\" or -m <int>\n");
    printf("\t to set a timeout          : --timeout=\"<int>\" or -t <int>\n");
    printf("\t to start interactive mode : --intercative or -i\n\n");
    printf("----------------------------------------\n");
}


int tok_manager(char *input_buffer, char *(*cmd)[10], int *b,int *c, char *out_path, char *err_path){
    // -> organizza la stringa passata input in comandi suddivisi dai caratteri '|','<' o '>', ritorna 0 se la stringa non e' valida
    if(chk_ascii(input_buffer) == 0){//controllo che i caratteri siano ASCII standard
        fprintf(stderr, RED "invalid standard ASCII characters found\n");
        return 0;
    }
    int y = 0, p_previous = 0, p = 0;//y conta in quanti pezzi ho suddiviso la stringa, p_previous e' un flag che controlla che non ci siano pipe consecutivi senza comandi in mezzo, p e' l'indice al prossimo carattere da inserire nella stringa
    (*cmd)[y] = malloc(100 * sizeof(char));//alloco memoria necessaria per il primo comando
    int i;
    for(i = 0; i < strlen(input_buffer); i++){//percorro tutti i caratteri della stringa in input
        if(input_buffer[i] == ' '){//se trovo spazi skippo fino al prossimo carattere
            while(input_buffer[i] == ' ' && i<strlen(input_buffer)){
                i++;
            }
            if(input_buffer[i] != '|' && input_buffer[i] != '>' && input_buffer[i] != '<'){//se non si tratta di un carattere di piping mi occupo di aggiungerlo al comando corrente
                if(i < strlen(input_buffer)){
                    if(p > 0){//se il comando conteneva gia' elementi
                        (*cmd)[y][p] = ' ';
                        p++;
                    }
                    (*cmd)[y][p] = input_buffer[i];
                    p++;
                    p_previous = 0;//il carattere appena inserito non e' un pipe quindi setto p_previous a 0
                }
            }else{
                i--;
            }
        }else{//se invece non si trtta di unos spazio
            if(input_buffer[i] == '|' || input_buffer[i] == '>' || input_buffer[i] == '<'){//se si tratta di un carattere di piping o redirezionamento
                if(p_previous == 1){//controllo di non everne incotrato uno subito prima
                    fprintf(stderr, RED "incoherent string of commands\n");
                    return 0;
                }
                p_previous = 1;
                y++;
                (*cmd)[y] = malloc(100 * sizeof(char));//alloco una nuova stringa in cui salvare il pipe
                (*cmd)[y][0] = input_buffer[i];
                (*cmd)[y][1]=00;
                y++;
                (*cmd)[y] = malloc(100 * sizeof(char));//preparo una nuova sringa per il prossimo comando
                p=0;
            }else{//se si tratta di un altro carattere lo aggiungo alla stringa che conterra' il comando
                p_previous = 0;
                (*cmd)[y][p] = input_buffer[i];
                p++;
            }
        }
        (*cmd)[y][p]=00;//aggiungo il carattere di fine stringa
    }
    *b = y + 1;//in b passo il numero di stringhe che ho creato
    *c = y / 2 + 1;// in c passo il numero di comandi effettivi da eseguire
    if(strcmp((*cmd)[0], "") == 0 || strcmp((*cmd)[y], "") == 0){//controlla che il primo o l'ultimo comando non sia un pipe
        fprintf(stderr, RED "incoherent string of commands\n");
        return 0;
    }
    /*controllo i possibili accessi ai file*/
    char cwd[100]; char a_path[150];
    getcwd(cwd, sizeof(cwd));//mi salvo la working directory
    strcat(cwd, "/");
    for(i = 2; i <= y; i += 2){//controllo se i file in input esistono e se si tenta l'accesso ai file di log
        if(strcmp((*cmd)[i-1], "<") == 0){//prima controllo se il file a cui si tenta di accedere in input esiste
            if(access((*cmd)[i], F_OK) == -1){
                fprintf(stderr, RED "warning, you specified a file that doesn't exist\n");
                return 0;
            }
        }
        if(strcmp((*cmd)[i-1], "<") == 0 || strcmp((*cmd)[i-1], ">") == 0){//controllo anche poi se si tenta di accedere ai file di log
            if((*cmd)[i][0] != '/'){
                strcpy(a_path, cwd);
                strcat(a_path,(*cmd)[i]);
            }else{
                strcpy(a_path, (*cmd)[i]);
            }
            if(strcmp(a_path, out_path) == 0 || strcmp(a_path, err_path) == 0 || strcmp(a_path, "/dev/null") == 0){
                fprintf(stderr, RED "warning, you can't access log files during execution or /dev/null\n");
                return 0;
            }
        }
    }
    /*controllo l'ordine dei redirezionamenti e pipe ( < | > )*/
    int pi=0,lt=0,gt=0;
    for(i = 1; i <= y; i += 2){
        if(strcmp((*cmd)[i], "<") == 0){
            lt++;
            if(pi!=0 || gt !=0){
                fprintf(stderr, RED "incoherent string of commands\n");
                return 0;
            }
        }
        if(strcmp((*cmd)[i], "|") == 0){
            pi++; 
            if(gt !=0){
                fprintf(stderr, RED "incoherent string of commands\n");
                return 0;
           }
        }
        if(strcmp((*cmd)[i], ">") == 0){
            gt++; 
        }
    }
    return 1;
}

void args_manager(int argc, char *argv[], char **out_path, char **err_path, int *max_len, int *code, int *timeout, char *cwd){
    // - controlla e sistema gli argomenti passati al main
    char *content;//potra' servire a contenere temporaneamente il contenuto delle opzioni
    content = (char *)malloc(50 * sizeof(char));
    /*prima di tutto controllo se ho un numero ragionevole di argomenti*/
    if(argc < 1 || argc > 11){
        fprintf(stderr, RED "incoherent number of arguments\n");
        print_help();
        exit(1);
    }
    /*controllo se si tratta di un semplice help*/
    if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)){
        print_help();
        exit(0);
    }
    /*controllo se si tratta di una richiesta per la modalita' interrattiva*/
    if(argc == 2 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0)){
        printf("insert the out_path(press enter for default value) : ");
        *out_path = (char *)malloc(200* sizeof(char));//alloco la memoria necessaria per il buffer
        //(???????????????????????????????????????????????????????????)*out_path[0]=00;
        fgets(*out_path, 200, stdin);
        if(strlen(*out_path) == 1){//se l'utente he premuto invio la stringa contiene un unico carattere
            *out_path = "/dev/null";//setto il path al /dev/null
        }else{//se l'utente ha invece inserito qualcosa
            strtok(*out_path, "\n");//tolgo '\n' che tanto non mi serve
        }
        printf(GREEN "setting path for the output log file to : %s\n", *out_path);
        printf(RESET "insert the err_path(press enter for default value) : ");
        *err_path = (char *)malloc(200 * sizeof(char));
        fgets(*err_path, 200, stdin);
        if(strlen(*err_path) == 1){
            *err_path = "/dev/null";
        }else{
            strtok(*err_path, "\n");
        }
        printf(GREEN "setting path for the error log file to : %s\n", *err_path);
        printf(RESET "insert the code(f:false, t: true or enter for default value): ");
        fgets(content, 50, stdin);//uso la variabile content per salvarmi temporaneamente il contenuto
        if(strcmp(content, "t\n") == 0){//controllo se si tratta dei caratteri 't' o 'n'
            printf(GREEN "setting return code flag to : true\n");
            *code = 1;
        }else if(strcmp(content, "f\n") == 0){
            printf(GREEN "setting return code flag to : false\n");
            *code = 0;
        }else{//se non si tratta di uno di quei caratteri metto il valore di dafault
            fprintf(stderr, GREEN "setting default value(false)\n");
            *code = 0;
        }
        printf(RESET "insert the max_len(press enter for default value) : ");
        fgets(content, 50, stdin);//uso la variabile content per salvarmi temporaneamente il contenuto
        if(strlen(content) == 1){
            *max_len = 100000;
        }else{
            *max_len = atoi(content);//se saranno caratteri resituisce 0
        }
        if(*max_len<=0){//se il numero e' negativo o incoerente setto il valore di default
            *max_len=100000;
        }
        printf(GREEN "setting max output lenght to : %d\n", *max_len);
        printf(RESET "insert a timeout(press enter for default value) : ");
        fgets(content, 50, stdin);
        if(strlen(content) != 1){//se c'e' del contenuto lo converto in numero
            *timeout = atoi(content);
        }
        if(*timeout<=0){
            *timeout = -1;
        }
        printf(GREEN "setting timeout to : %d\n", *timeout);

        fflush(stdout);
    }else{
        /*se non si tratta della modalita' interrattiva inizio a scorrere tutti gli argomenti*/
        int i;
        for(i = 1; i < argc; i++){
            /*inizio controllando le versioni accorciate*/
            if(strcmp(argv[i], "-o") == 0){
                printf(GREEN "found output file contention(-o %s)\n\n", argv[i+1]);   //stampo anche l'argomento successivo perche' dovrebbe essere il contenuto dell'opzione
                /*controllo il contenuto dell'opzione e varie condizioni*/
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *out_path == NULL && chk_ascii(argv[i+1]) == 1){
                    *out_path = (char *)malloc((strlen(argv[i+1]) + 100) * sizeof(char));//alloco della memoria extra necessaria per eggiungere il path assoluto se necessario
                    strcpy(*out_path, argv[i+1]);//lo salvo
                    i++;//skippo il prossimo arg che tanto era il contenuto
                }else{//se non ho passato i controlli stampo un errore ed esco
                    fprintf(stderr, RED "error while reading output contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-e") == 0){
                printf(GREEN "found error file contention(-e %s)\n\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *err_path == NULL && chk_ascii(argv[i+1]) == 1){
                    *err_path = (char *)malloc((strlen(argv[i+1]) + 100) * sizeof(char));
                    strcpy(*err_path, argv[i+1]);
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading error contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-m") == 0){
                printf(GREEN "found maxlen contention(-m %s)\n\n", argv[i+1]);
                /*tra i vari check controllo anche che non sia stato inserito -1 che e' il valore di inizializziaaione dell'intero*/
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && strcmp(argv[i+1],"-1") != 0 && chk_nmbr(argv[i+1]) == 1 && *max_len == -1 && chk_ascii(argv[i+1]) == 1){
                    *max_len = atoi(argv[i+1]);
                    if(*max_len <= 0){
                        fprintf(stderr, RED "error while reading maxlen contention: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading maxlen contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-c") == 0){
                printf(GREEN "found code contention(-c %s)\n\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "false") == 0) && *code==-1 && chk_ascii(argv[i+1]) == 1){
                    if(strcmp(argv[i+1], "true") == 0){
                        *code = 1;
                    }else{
                        *code = 0;
                    }
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading code contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-t") == 0){
                printf(GREEN "found timeout contention(-t %s)\n\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && strcmp(argv[i+1],"-1") != 0 && chk_nmbr(argv[i+1]) == 1 && *timeout == -1 && chk_ascii(argv[i+1]) == 1){
                    *timeout = atoi(argv[i+1]);
                    if(*timeout <= 0){
                        fprintf(stderr, RED "error while reading timeout contention: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading timeout contention\n");
                    print_help();
                    exit(1);
                }
            }
            /*infine per controllare gli argomenti composti gestisco delle substringhe*/
            else if(strstr(argv[i], "--outfile=") != NULL){//se la stringa inizia con '--outfile='
                printf(GREEN "found output file contention, ");
                sprintf(content, "%s", argv[i] + 10);//sprintf mi permette di predere tutti i caratteri fino alla fine(partendo da 10 in questo caso),
                printf("content is : %s \n\n", content);
                if(*out_path == NULL){//se il path non e' stato ancora settato mi occupo di salvarlo
                    *out_path = (char *)malloc((strlen(content) + 500) * sizeof(char));
                    strcpy(*out_path, content);
                }else{
                    fprintf(stderr, RED "error while reading output contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--errfile=") != NULL){
                printf(GREEN "found error file contention, ");
                sprintf(content, "%s", argv[i] + 10);
                printf("content is : %s \n\n", content);
                if(*err_path == NULL){
                    *err_path = (char *)malloc((strlen(content) + 500) * sizeof(char));
                    strcpy(*err_path, content);
                }else{
                    fprintf(stderr, RED "error while reading error contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--maxlen=") != NULL){
                printf(GREEN "found maxlen contention,");
                sprintf(content, "%s", argv[i] + 9);
                printf("content is : %s \n\n", content);
                if(chk_nmbr(content) == 1 && *max_len == -1){
                    *max_len = atoi(content);
                    if(*max_len <= 0){
                        fprintf(stderr, RED "error while reading maxlen contention: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                }else{
                    fprintf(stderr, RED "error while reading maxlen contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--code=") != NULL){
                printf(GREEN "found code contention, ");
                sprintf(content, "%s", argv[i] + 7);
                printf("content is : %s \n\n", content);
                if((strcmp(content, "true") == 0 || strcmp(content, "false") == 0) && *code == -1){
                    if(strcmp(content,"true") == 0){
                        *code = 1;
                    }else{
                        *code = 0;
                    }
                }else{
                    fprintf(stderr, RED "error while reading code contention\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--timeout=") != NULL){
                printf(GREEN "found timeout contention,");
                sprintf(content, "%s", argv[i] + 10);
                printf("content is : %s \n\n", content);
                if(chk_nmbr(content) == 1 && *timeout == -1){
                    *timeout = atoi(content);
                    if(*timeout <= 0){
                        fprintf(stderr, RED "error while reading timeout contention: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                }else{
                    fprintf(stderr, RED "error while reading timeout contention\n");
                    print_help();
                    exit(1);
                }
            }
            /*se non ho trovato un'opzione valida esco e stampo un help*/
            else{
                fprintf(stderr, RED "found wrong argument\n");
                print_help();
                exit(1);
            }
        }
    }
    //free(content);//dealloco memoria
    /*controllo se c'e' qualche opzione non ancora inizializzata e quindi setto dei parametri di default se necessario*/
    if(*out_path == NULL){
        printf(YELLOW "setting default outputfile path\n\n");
        *out_path = "/dev/null";
    }
    if(*err_path == NULL){
        printf(YELLOW "setting default errorfile path\n\n");
        *err_path = "/dev/null";
    }
    if(*code == -1){
        printf(YELLOW "setting default code flag\n\n");
        *code = 0;
    }
    if(*max_len == -1){
        printf(YELLOW "setting default max length\n\n");
        *max_len = 100000;
    }
    if(*timeout == -1){
        printf(YELLOW "setting default timeout(-1)\n\n");
    }
    /*mi occupo di convertire i path relativi in assoluti se necessario*/
    char tmp[1024];
    if(*out_path[0] != '/'){
        strcpy(tmp, *out_path);
        strcpy(*out_path, cwd);
        strcat(*out_path, "/");
        strcat(*out_path, tmp);
    }
    if(*err_path[0] != '/'){
        strcpy(tmp, *err_path);
        strcpy(*err_path, cwd);
        strcat(*err_path, "/");
        strcat(*err_path, tmp);
    }
    /*controllo che l'utente non abbiamo inserito path identici e che non abbia inserito quelli di dafault*/
    if(strcmp(*err_path, "/dev/null") != 0 && strcmp(*out_path, "/dev/null") != 0 && (strcmp(*err_path, *out_path) == 0)){
        printf("you can't set outfile and errfile to the same path, use only outfile if you want everything on the same file\n");
        print_help();
        exit(1);
    }
}
