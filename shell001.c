#include<stdio.h>
#include<string.h>
#include<stdlib.h> 

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m" //colori carini
#define MAGENTA "\x1b[35m"
#define RESET   "\x1b[0m"


void args_manager(int argc, char *argv[]){
    printf("number of args received : %i \n", argc);
    printf("here are them: \n");
    char *opt,*content;//per gli argomenti composti opt conterra' l'opzione e content il contenuto dell'opzione
    opt = (char *)malloc(9 * sizeof(char));
    content =(char *)malloc(50 * sizeof(char));
    /*--> prima di tutto controllo se si tratta di un help */
    if(argc == 2 && (strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0) ){
        printf("print help here\n");
        exit(0);//esco dopo aver stampato l'aiuto
    }
    /*--> controllo tutti gli argomenti */
    for(int i=1; i<argc; i++){
        printf(RESET "argv[%i] -> %s \n", i, argv[i]);
        /*--> prima di tutto controllo le versioni accorciate */
        if(strcmp(argv[i],"-o") == 0){
            printf(GREEN "found output file option( -o %s)\n", argv[i+1]);//stampo anche l'argomento successivo perche' dovrebbe essere il contenuto dell'opzione
        }
        else if(strcmp(argv[i],"-e") == 0){
            printf(GREEN "found error file option( -e %s)\n", argv[i+1]);
        }
        else if(strcmp(argv[i],"-m") == 0){
            printf(GREEN "found maxlen option( -m %s)\n", argv[i+1]);
        }
        else if(strcmp(argv[i],"-c") == 0){
            printf(GREEN "found code option( -c %s)\n", argv[i+1]);
        }
        /*--> per controllare gli argomenti composti gestisco delle substringhe */
        else if(strstr(argv[i], "--") != NULL) {
            printf(YELLOW "possible option found!\n");
            strncpy(opt, argv[i]+2, 8);//mi prendo la substringa che potrebbe contere l'opzione piu' lunga("outfile=" oppure "errfile=")
            printf(YELLOW "extracted substring is : %s\n", opt);
            if(strcmp(opt,"outfile=") == 0){
                printf(GREEN "found output file option, ");
                sprintf(content, "%s", argv[i]+10);//sprintf mi permette di predere tutti i caratteri fino alla fine(partendo da 10 in questo caso),
                printf("content is : %s \n", content);
            }
            if(strcmp(opt,"errfile=") == 0){
                printf(GREEN "found error file option ");
                sprintf(content, "%s", argv[i]+10);//sarebbe anche possibile usare strncpy se proprio si vuole
                printf("content is : %s \n", content);
            }
            opt[strlen(opt)-1] = '\0'; //accorcio di uno la stringa per controllare "maxlen="
            printf(YELLOW "extracted substring is : %s\n", opt);
            if(strcmp(opt,"maxlen=") == 0){
                printf(GREEN "found maxlen option,");
                sprintf(content, "%s", argv[i]+9);
                printf("content is : %s \n", content);
            }
            opt[strlen(opt)-2] = '\0'; //accorcio di due per controllare "code="
            printf(YELLOW "extracted substring is : %s\n", opt);
            if(strcmp(opt,"code=") == 0){
                printf(GREEN "found code option, ");
                sprintf(content, "%s", argv[i]+7);
                printf("content is : %s \n", content);
            }
        }
        else{//se non ho trovato un'opzione valida devo gestire un errore
            printf(RED "wrong argument\n");
        }
    }
}

int main(int argc, char *argv[]){
    /*--> controllo che gli argomenti passati all'eseguibile siano corretti */
    printf(RESET "\n----------------------------------------\n");
    args_manager(argc, argv);
    printf(RESET "\n----------------------------------------\n");
    /*--> variabili necessarie per il buffer di input */
    char *input_buffer;//buffer effettivo
    size_t buff_size = 100;//numero di caratteri che il buffer puo' contenere
    size_t n_char;//conterra' il numero effettivo di caratteri presi in input
    /*--> variabili necessarie per fare i token */
    char *tok;//conterra' i token
    int x;//conterra' il numero di token trovati
    /*--> creo buffer per l'input */
    input_buffer = (char *)malloc(buff_size * sizeof(char));//alloco la memoria necessaria per il buffer
    if(input_buffer == NULL){//controlla che il buffer sia stato effettivamente creato
        fprintf(stderr, "failed to create buffer");
        exit(1);
    }
    /*--> prendo in input i comandi */
    while(strcmp(input_buffer, "quit")){//continuo finche' l'utente non inserisce quit
        x = 0;//contatore per il presunto numero di comandi trovati(token)
        printf("shell> ");
        n_char=getline(&input_buffer,&buff_size,stdin);//metto in input_buffer la linea presa in input
        strtok(input_buffer, "\n");//tolgo il carattere invio dall'input(non mi serve)
        printf(BLUE "<info> string: %s - n_char : %i \n", input_buffer, n_char);
        /*--> divisione della stringa in tokens */
        tok = strtok(input_buffer, "|");//tok conterra' in questo primo caso tutto cio' che c'e' prima del primo pipe
        while(tok != NULL){
            printf("<info> tok[%i] : %s \n", x, tok);
            x++;
            tok = strtok(NULL, "|");//continuo con la restante stringa fino al prossimo pipe o fino alla fine
        }
        printf("<info> n_tokens : %i \n" RESET, x);
    }
    return 0;
}
