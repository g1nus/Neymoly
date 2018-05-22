// - controlla se una striga rappresenta un numero
// 0 ==> false | 1 ==> true
int chk_nmbr(char *str){
    int j;
    for(j = 0; j < strlen(str); j++){
        if(!(str[j] >= 48 && str[j] <= 57)){
            return 0;
        }
    }
    return 1;
}


// - controlla se la stringa contiene solo caratteri ASCII standard
// 0 ==> false | 1 ==> true
int chk_ascii(char *str){
    int j;
    for(j = 0; j < strlen(str); j++){
        if(!(str[j] >= 0 && str[j] <= 127)){
            fprintf(stderr, RED "found not standard ASCII character\n");
            return 0;
        }
    }
    return 1;
}


// - stampa una piccola guida
void print_help(){
    printf(RESET "HOW TO USE START THE PROGRAM : \n");
    printf("\t to set the output file    : --outfile=\"<path>\" or -o <path>\n");
    printf("\t to set the error file     : --errfile=\"<path>\" or -e <path>\n");
    printf("\t to set the code flag      : --code=\"true\" or -c true or --code=\"false\" or -c false \n");
    printf("\t to set the max lenght     : --maxlength=\"<unsingned int>\" or -m <unsingne int>\n");
    printf("\t to start interactive mode : --intercative or -i\n\n");
    printf(RESET "----------------------------------------\n");
}


// - controlla che gli argomenti(singole stringhe suddivise da spazi) passati alla custom shell siano coerenti e li organizza in comandi e pipe, un futuro puo' essere usato per organizzare in altri modi
// arr e' l'array che conterra' gli argomenti mentre cmd i comandi effettivi, il pipe e' considerato un comando
void tok_manager(char *input_buffer, char *(*cmd)[10], int *b,int *c){
    int y=0, p_previous=0, p=0, n=0;    //y e' il contatore di comandi effettivi e pipe, p_previous e' un flag che controlla che non ci siano pipe consecutivi senza comandi in mezzo
    char ch;
    (*cmd)[y] = malloc(100 * sizeof(char));
    int x;
    //for(x=0;x<strlen((*cmd)[y]);x++){
        //(*cmd)[y][x]=00;
    //}
    //(*cmd)[y] = strdup("");
    //memset((*cmd)[y],0,strlen((*cmd)[y]));
    //(*cmd)[y][0]=00;
    printf(CYAN "<tok_manager:info> character vector is %i bytes long\n",strlen(input_buffer));
    printf(CYAN "<tok_manager:info> this is the content of cmd[y]:%s\n",cmd[y]);
    int i;
    for(i = 0; i < strlen(input_buffer); i++){
        printf("(%i) --> (%c)\n", i, input_buffer[i]);
        if(input_buffer[i] == ' '){
            //printf("that's a space ");
            while(input_buffer[i] == ' ' && i<strlen(input_buffer)){
                i++;
            }
            //printf("cmd[y] is %s, input_buffer[i] is %c\n",(*cmd)[y], input_buffer[i]);
            if(input_buffer[i] != '|' && input_buffer[i] != '>' && input_buffer[i] != '<'){
                if( i<strlen(input_buffer)){
                if(p>0){
                    (*cmd)[y][p] = ' ';
                    p++;
                }
                (*cmd)[y][p] = input_buffer[i];
                p++;
                p_previous = 0;
                }
            }else{
                i--;
            }
        }else{
            if(input_buffer[i] == '|' || input_buffer[i] == '>' || input_buffer[i] == '<'){
                if(p_previous == 1){
                    //printf(RED "incoherent piping, p_previous=1\n");
                    exit(1);
                }
                p_previous = 1;
                //printf("now i found a pipe\n");
                y++;
                (*cmd)[y] = malloc(100 * sizeof(char));
                memset((*cmd)[y],0,strlen((*cmd)[y]));
                (*cmd)[y][0] = input_buffer[i];
                //printf("pipe character is here(Y:%i) --> %s\n",y,(*cmd)[y]);
                y++;
                (*cmd)[y] = malloc(100 * sizeof(char));
                memset((*cmd)[y],0,strlen((*cmd)[y]));
                p=0;
            }else{
                p_previous = 0;
                (*cmd)[y][p] = input_buffer[i];
                p++;
            }
        }
        (*cmd)[y][p]=00;
        printf("(Y:%i) --> [%s] - (%i)\n",y,(*cmd)[y], strlen((*cmd)[y]));
    }
    //printf("finshed, y is %i\n",y);
    *b = y + 1;//in b passo l'indice dell'ultimo comando piu' uno, per avere in numero totale di comandi
    n = y / 2;
    n++;

    printf(CYAN "<tok_manager:info> this is what I've done\n");
    for(i=0;i<=y;i++){
        printf("[i:%i] %s\n", i, (*cmd)[i]);
    }
    printf(RESET);
    fflush(stdout);
    *c = n; // in c passo il numero di comandi effettivi da eseguire
    if(strcmp((*cmd)[0], "") == 0 || strcmp((*cmd)[y], "") == 0){//controlla che il primo o l'ultimo comando non sia un pipe
        fprintf(stderr, RED "incoherent pipe\n");
        print_help();
        exit(1);
    }
}


// - controlla e sistema gli argomenti passati al main
void args_manager(int argc, char *argv[], char **out_path, char **err_path, int *max_len, int *code, char *cwd){
    printf("the current working directory is : %s\n", cwd);
    char *opt,*content,*c;  //per gli argomenti composti opt conterra' l'opzione e content il contenuto dell'opzione
    opt = (char *)malloc(9 * sizeof(char));
    content =(char *)malloc(50 * sizeof(char));
    //--> controllo se ho un numero ragionevole di argomenti
    if(argc < 1 || argc > 9){
        fprintf(stderr, RED "incoherent number of arguments\n");
        print_help();
        exit(1);
    }
    //--> controllo se si tratta di un semplice help
    if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)){
        print_help();
        exit(0);    //esco dopo aver stampato l'aiuto
    }
    //--> controllo se si tratta di una richiesta per una modalita' interattiva
    if(argc == 2 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0)){
        printf("insert the out_path(press enter for default value) : ");
        *out_path = (char *)malloc(50 + 1024 * sizeof(char));   //alloco la memoria necessaria per il buffer
        fgets(*out_path, 50, stdin);
        strtok(*out_path, "\n");
        if(strlen(*out_path) == 1){
            *out_path = "/dev/null";
        }else{
            strtok(*out_path, "\n");
        }
        printf(GREEN "setting path for the output log file to : %s\n", *out_path);
        printf(RESET "insert the err_path(press enter for default value) : ");
        *err_path = (char *)malloc(50 + 1024 * sizeof(char));   //alloco la memoria necessaria per il buffer
        fgets(*err_path, 50, stdin);
        if(strlen(*err_path) == 1){
            *err_path = "/dev/null";
        }else{
            strtok(*err_path, "\n");
        }
        printf(GREEN "setting path for the error log file to : %s\n", *err_path);
        printf(RESET "insert the code(f:false, t: true or enter for default value): ");
        fgets(opt, 9, stdin);
        if(strcmp(opt, "t\n") == 0){
            printf(GREEN "setting return code flag to : true\n");
            *code = 1;
        }else if(strcmp(opt, "f\n") == 0){
            printf(GREEN "setting return code flag to : false\n");
            *code = 0;
        }else{
            fprintf(stderr, YELLOW "setting default value(false)\n");
            *code = 0;
        }
        printf(RESET "insert the max_len(press enter for default value) : ");
        fgets(opt, 9, stdin);
        if(strlen(opt) == 1){
            *max_len = 100000;
        }else{
            *max_len = atoi(opt);
        }
        printf(GREEN "setting max output lenght to : %d\n", *max_len);
        fflush(stdout);
        //free(opt);
    }else{
        int i;
        for(i = 1; i < argc; i++){
            //--> prima di tutto controllo che nell'argv[i] non siano presenti caratteri speciali
            if(chk_ascii(argv[i]) == 0){
                print_help();
                exit(1);
            }
            //--> poi controllo le versioni accorciate
            if(strcmp(argv[i], "-o") == 0){
                printf(GREEN "found output file option(-o %s)\n", argv[i+1]);   //stampo anche l'argomento successivo perche' dovrebbe essere il contenuto dell'opzione
                //se esiste il contenuto dell'opzione e non e' stata ancora inizilizzata
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *out_path == NULL && chk_ascii(argv[i+1]) == 1){
                    *out_path = (char *)malloc(strlen(argv[i+1]) + 1 + 1024 * sizeof(char));
                    strcpy(*out_path, argv[i+1]);   //lo salvo
                    i++;    //skippo il prossimo arg che tanto era il contenuto
                }else{
                    fprintf(stderr, RED "error while reading output option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-e") == 0){
                printf(GREEN "found error file option(-e %s)\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *err_path == NULL && chk_ascii(argv[i+1]) == 1){
                    *err_path = (char *)malloc(strlen(argv[i+1]) + 1 + 1024 * sizeof(char));
                    strcpy(*err_path, argv[i+1]);
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading error option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-m") == 0){
                printf(GREEN "found maxlen option(-m %s)\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && strcmp(argv[i+1],"-1") != 0 && chk_nmbr(argv[i+1]) == 1 && *max_len == -1 && chk_ascii(argv[i+1]) == 1){
                    *max_len = atoi(argv[i+1]);
                    if(*max_len < 0){
                        fprintf(stderr, RED "error while reading maxlen option: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading maxlen option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strcmp(argv[i], "-c") == 0){
                printf(GREEN "found code option(-c %s)\n", argv[i+1]);
                if((i + 1 < argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && (strcmp(argv[i+1], "true") == 0 || strcmp(argv[i+1], "false") == 0) && *code==-1 && chk_ascii(argv[i+1]) == 1){
                    if(strcmp(argv[i+1], "true") == 0){
                        *code = 1;
                    }else{
                        *code = 0;
                    }
                    i++;
                }else{
                    fprintf(stderr, RED "error while reading code option\n");
                    print_help();
                    exit(1);
                }
            }
            //--> infine per controllare gli argomenti composti gestisco delle substringhe
            else if(strstr(argv[i], "--outfile=") != NULL){ //se la stringa inizia con '--outfile='
                printf(GREEN "found output file option, ");
                sprintf(content, "%s", argv[i] + 10);   //sprintf mi permette di predere tutti i caratteri fino alla fine(partendo da 10 in questo caso),
                printf("content is : %s \n", content);
                if(*out_path == NULL){
                    *out_path = (char *)malloc(strlen(content) + 1 + 1024 * sizeof(char));
                    strcpy(*out_path, content);
                }else{
                    fprintf(stderr, RED "error while reading output option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--errfile=") != NULL){
                printf(GREEN "found error file option ");
                sprintf(content, "%s", argv[i] + 10);   //sarebbe anche possibile usare strncpy se proprio si vuole
                printf("content is : %s \n", content);
                if(*err_path == NULL){
                    *err_path = (char *)malloc(strlen(content) + 1 + 1024 * sizeof(char));
                    strcpy(*err_path, content);
                }else{
                    fprintf(stderr, RED "error while reading error option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--maxlen=") != NULL){
                printf(GREEN "found maxlen option,");
                sprintf(content, "%s", argv[i] + 9);
                printf("content is : %s \n", content);
                if(chk_nmbr(content) == 1 || *max_len == -1){
                    *max_len = atoi(content);
                    if(*max_len < 0){
                        fprintf(stderr, RED "error while reading maxlen option: value should be greater than 0\n");
                        print_help();
                        exit(1);
                    }
                }else{
                    fprintf(stderr, RED "error while reading maxlen option\n");
                    print_help();
                    exit(1);
                }
            }
            else if(strstr(argv[i], "--code=") != NULL){
                printf(GREEN "found code option, ");
                sprintf(content, "%s", argv[i] + 7);
                printf("content is : %s \n", content);
                if((strcmp(content, "true") == 0 || strcmp(content, "false") == 0) && *code == -1){
                    if(strcmp(content,"true") == 0){
                        *code = 1;
                    }else{
                        *code = 0;
                    }
                }else{
                    fprintf(stderr, RED "error while reading code option\n");
                    print_help();
                    exit(1);
                }
            }
            else{   //se non ho trovato un'opzione valida devo gestire un errore
                fprintf(stderr, RED "found wrong argument\n");
                print_help();
                exit(1);
            }
        }
    }
    //free(content);
    //--> controllo se c'e' qualche opzione non ancora inizializzata(sarebbe possibile fare anche un inserimento interrattivo)
    if(*out_path == NULL){
        printf(YELLOW "\nsetting default outputfile path\n");
        *out_path = "/dev/null";
    }
    if(*err_path == NULL){
        printf(YELLOW "\nsetting default errorfile path\n");
        *err_path = "/dev/null";
    }
    if(*code == -1){
        printf(YELLOW "\nsetting default code flag\n");
        *code = 0;
    }
    if(*max_len == -1){
        printf(YELLOW "\nsetting default max length\n");
        *max_len = 100000;
    }
    char tmp[1024];
    if(*out_path[0] != '/'){
        strcpy(tmp, *out_path);
        strcpy(*out_path, cwd); //Put str2 or anyother string that you want at the begining
        strcat(*out_path, "/"); //concatenate previous str1
        strcat(*out_path, tmp); //concatenate previous str1
    }
    if(*err_path[0] != '/'){
        strcpy(tmp, *err_path);
        strcpy(*err_path, cwd); //Put str2 or anyother string that you want at the begining
        strcat(*err_path, "/"); //concatenate previous str1
        strcat(*err_path, tmp); //concatenate previous str1
    }
    if(strcmp(*err_path, "/dev/null") != 0 && strcmp(*out_path, "/dev/null") != 0 && (strcmp(*err_path, *out_path) == 0)){
        printf("you can't set outfile and errfile to the same path, use only outfile if you want everything on the same file\n");
        print_help();
        exit(1);
    }
}
