int chk_nmbr(char *str){
    // - controlla se una striga rappresenta un numero
    for(int j=0;j<strlen(str);j++){
        if(!(str[j] >= 48 && str[j] <= 57)){
            return 0;//0=>false
        }
    }
    return 1;//1=>true
}

int chk_ascii(char *str){
    // - controlla se la stringa contiene solo caratteri ASCII standard
    for(int j=0;j<strlen(str);j++){
        if(!(str[j] >= 0 && str[j] <= 127)){
            printf(RED "found suspicious character\n");
            return 0;//0=>false
        }
    }
    return 1;//1=>true
}

void print_help(){
    // - stampa una mini guida
    printf(RESET "HOW TO USE START THE PROGRAM : \n");
    printf("\t to set the output file : --outfile=\"<path>\" or -o <path>\n");
    printf("\t to set the error file  : --errfile=\"<path>\" or -e <path>\n");
    printf("\t to set the code flag   : --code=\"true\" or -c true or --code=\"false\" or -c false \n");
    printf("\t to set the max lenght  : --maxlength=\"<unsingned int>\" or -m <unsingne int>\n\n");
    printf(RESET "----------------------------------------\n");
}

void tok_manager(char *input_buffer,char *(*arr)[10], char *(*cmd)[10], int *a, int *b){//arr e' l'array che conterra' gli argomenti mentre cmd i comandi effettivi, il pipe e' considerato un comando
    // - controlla che gli argomenti(singole stringhe suddivise da spazi) passati alla custom shell siano coerenti e li organizza in comandi e pipe, un futuro puo' essere usato per organizzare in altri modi
    int x=0, y=0, p_previous=0;//x e' il contatore di argomenti, y e' il contatore di comandi effettivi e pipe, p_previous e' un flag che controlla che non ci siano pipe consecutivi senza comandi in mezzo
    char *tok;//conterra' gli argomenti
    tok = strtok(input_buffer, " ");//tok conterra' in questo primo caso tutto cio' che c'e' prima del primo spazio
    (*cmd)[y] = malloc(100*sizeof(char));//alloco dello spazio nell'array di stringhe che conterra' i comandi
    strcpy((*cmd)[y], "");//metto un carattere vuoto a cui appendera' gli argomenti che fanno parte del comando
    while(tok != NULL){//finche' trovo token
        (*arr)[x] = malloc(50 *sizeof(char));//alloco memoria necessaria per salvare un argomento
        strcpy((*arr)[x],tok);//ci metto dentro il token trovato
        if(chk_ascii((*arr)[x])==0){//se non e' ascii standard me ne esco
            print_help();
            exit(1);
        }
        if(strcmp((*arr)[x],"|")!=0){//se non si tratta di un pipe lo appendo all'attuale comando
            if(strcmp((*cmd)[y],"")!=0){//se il comando non era vuoto aggiungo prima uno spazio
                strcat((*cmd)[y], " ");
                strcat((*cmd)[y], (*arr)[x]);
            }else{//altrimenti ci butto semplicmente l'argomento dentro
                strcat((*cmd)[y], (*arr)[x]);
            }
            p_previous=0;
        }else{//se si tratta di un pipe
            if(p_previous==1){//controllo che non ce ne sia stato uno subito prima
                printf(RED "incoherent pipe\n");
                print_help();
                exit(1);
            }
            y++;//aggiorno il contatore dei comandi trovati e ci salvo dentro il pipe
            (*cmd)[y] = malloc(100*sizeof(char));
            strcpy((*cmd)[y],(*arr)[x]);
            y++;//mi preparo per il successivo comando
            (*cmd)[y] = malloc(100*sizeof(char));
            strcpy((*cmd)[y],"");
            p_previous=1;
        }
        x++;//aggiorno il numero di argomenti totali
        tok = strtok(NULL, " ");//continuo con la restante stringa fino al prossimo pipe o fino alla fine
    }
    *a = x;//in a passo il numero di argomenti
    *b = y+1;//in b passo l'indice dell'ultimo comando piu' uno, per avere in numero totale di comandi
    if(strcmp((*arr)[0],"|")==0 || strcmp((*arr)[x-1],"|")==0){//controlla che il primo o l'ultimo comando non sia un pipe 
        printf(RED "incoherent pipe\n");
        print_help();
        exit(1);    
    }    
}

void args_manager(int argc, char *argv[], char **out_path, char **err_path, int *max_len, int *code){
    // - controlla e sistema gli argomenti passati al main
    char *opt,*content;//per gli argomenti composti opt conterra' l'opzione e content il contenuto dell'opzione
    opt = (char *)malloc(9 * sizeof(char));
    content =(char *)malloc(50 * sizeof(char));
    /*--> controllo se ho un numero ragionevole di argomenti */
    if(argc<1 || argc>9){
        printf(RED "incoherent number of arguments\n");
        print_help();
        exit(1);
    }
    /*--> controllo se si tratta di un semplice help */
    if(argc == 2 && (strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0) ){
        print_help();
        exit(0);//esco dopo aver stampato l'aiuto
    }
    /*--> se non si tratta di un help controllo tutti gli argomenti */
    for(int i=1; i<argc; i++){
        /*--> prima di tutto controllo che nell'argv[i] non siano presenti caratteri speciali */
        if(chk_ascii(argv[i]) == 0){
            print_help();
            exit(1);
        }
        /*--> poi controllo le versioni accorciate */
        if(strcmp(argv[i],"-o") == 0){
            printf(GREEN "found output file option(-o %s)\n", argv[i+1]);//stampo anche l'argomento successivo perche' dovrebbe essere il contenuto dell'opzione
            if((i+1<argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *out_path==NULL && chk_ascii(argv[i+1]) == 1){//se esiste il contenuto dell'opzione e non e' stata ancora inizilizzata
                *out_path = (char *)malloc(strlen(argv[i+1])+1 * sizeof(char));
                strcpy(*out_path, argv[i+1]);//lo salvo
                i++;//skippo il prossimo arg che tanto era il contenuto
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strcmp(argv[i],"-e") == 0){
            printf(GREEN "found error file option(-e %s)\n", argv[i+1]);
            if((i+1<argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && *err_path==NULL && chk_ascii(argv[i+1]) == 1){
                *err_path = (char *)malloc(strlen(argv[i+1])+1 * sizeof(char));
                strcpy(*err_path, argv[i+1]);
                i++;
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strcmp(argv[i],"-m") == 0){
            printf(GREEN "found maxlen option(-m %s)\n", argv[i+1]);
            if((i+1<argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && strcmp(argv[i+1],"-1") != 0 && chk_nmbr(argv[i+1])==1 && *max_len==-1 && chk_ascii(argv[i+1]) == 1){
                *max_len = atoi(argv[i+1]);
                if(*max_len < 0){
                    printf(RED "error while reading args\n");
                    print_help();
                    exit(1);
                }
                i++;
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strcmp(argv[i],"-c") == 0){
            printf(GREEN "found code option(-c %s)\n", argv[i+1]);
            if((i+1<argc) && strstr(argv[i+1], "--") == NULL && strstr(argv[i+1], "-") == NULL && (strcmp(argv[i+1],"true") == 0 || strcmp(argv[i+1],"false") == 0) && *code==-1 && chk_ascii(argv[i+1]) == 1){
                if(strcmp(argv[i+1],"true") == 0){
                    *code = 1;
                }else{
                    *code = 0;
                }
                i++;
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        /*--> infine per controllare gli argomenti composti gestisco delle substringhe */
        else if(strstr(argv[i], "--outfile=") != NULL){//se la stringa inizia con '--outfile='
            printf(GREEN "found output file option, ");
            sprintf(content, "%s", argv[i]+10);//sprintf mi permette di predere tutti i caratteri fino alla fine(partendo da 10 in questo caso),
            printf("content is : %s \n", content);
            if(*out_path==NULL){
                *out_path = (char *)malloc(strlen(content)+1 * sizeof(char));
                strcpy(*out_path, content);
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strstr(argv[i],"errfile=") != NULL){
            printf(GREEN "found error file option ");
            sprintf(content, "%s", argv[i]+10);//sarebbe anche possibile usare strncpy se proprio si vuole
            printf("content is : %s \n", content);
            if(*err_path==NULL){
                *err_path = (char *)malloc(strlen(content)+1 * sizeof(char));
                strcpy(*err_path, content);
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strstr(argv[i],"maxlen=") != NULL){
            printf(GREEN "found maxlen option,");
            sprintf(content, "%s", argv[i]+9);
            printf("content is : %s \n", content);
            if(chk_nmbr(content)==1 || *max_len==-1){
                *max_len = atoi(content);
                if(*max_len < 0){
                    printf(RED "error while reading args\n");
                    print_help();
                    exit(1);
                }
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else if(strstr(argv[i],"code=") != NULL){
            printf(GREEN "found code option, ");
            sprintf(content, "%s", argv[i]+7);
            printf("content is : %s \n", content);
            if((strcmp(content,"true") == 0 || strcmp(content,"false") == 0) && *code == -1){
                if(strcmp(content,"true") == 0){
                    *code = 1;
                }else{
                    *code = 0;
                }
            }else{
                printf(RED "error while reading args\n");
                print_help();
                exit(1);
            }
        }
        else{//se non ho trovato un'opzione valida devo gestire un errore
            printf(RED "found wrong argument\n");
            print_help();
            exit(1);
        }
    }
    /*--> controllo se c'e' qualche opzione non ancora inizializzata(sarebbe possibile fare anche un inserimento interrattivo) */
    if(*out_path==NULL){
        printf(YELLOW "\nsetting default outputfile path\n");
        *out_path="/dev/null";
    }
    if(*err_path==NULL){
        printf(YELLOW "\nsetting default errorfile path\n");
        *err_path="/dev/null";
    }
    if(*code==-1){
        printf(YELLOW "\nsetting default code flag\n");
        *code=0;
    }
    if(*max_len==-1){
        printf(YELLOW "\nsetting default max length\n");
        *max_len=100;
    }
}
