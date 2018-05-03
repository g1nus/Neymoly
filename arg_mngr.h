#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m" //colori carini
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"

int chk_nmbr(char *str){//controlla se una striga contiene solo cifre
    for(int j=0;j<strlen(str);j++){
        if(!(str[j] >= 48 && str[j] <= 57)){
            return 0;//0=>false
        }
    }
    return 1;//1=>true
}

int chk_ascii(char *str){//controlla se la stringa contiene solo caratteri ASCII standard
    for(int j=0;j<strlen(str);j++){
        if(!(str[j] >= 0 && str[j] <= 127)){
            printf(RED "found suspicious character\n");
            return 0;//0=>false
        }
    }
    return 1;//1=>true
}

void print_help(){
    printf(RESET "HOW TO USE START THE PROGRAM : \n");
    printf("\t to set the output file : --outfile=\"<path>\" or -o <path>\n");
    printf("\t to set the error file  : --errfile=\"<path>\" or -e <path>\n");
    printf("\t to set the code flag   : --code=\"true\" or -c true or --code=\"false\" or -c false \n");
    printf("\t to set the max lenght  : --maxlength=\"<unsingned int>\" or -m <unsingne int>\n\n");
    printf(RESET "----------------------------------------\n");
}

void args_manager(int argc, char *argv[], char **out_path, char **err_path, int *max_len, int *code){
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
        *out_path="/tmp/shell001output.txt";
    }
    if(*err_path==NULL){
        printf(YELLOW "\nsetting default errorfile path\n");
        *err_path="/tmp/shell001error.txt";
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
