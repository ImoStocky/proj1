/*
 * Soubor:  proj1.c
 * Datum:   16.11.2004 21:45
 * Autor:   Imrich Štoffa, xstoff02@stud.fit.vutbr.cz
 * Projekt: Jednoduchý tabuľkový procesor, projekt č.1 pre predmet IZP
 * Popis:   Program spracuváva zadanú časť tabuľky podľa parametrov.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1025

#define CMD_WORD_LEN 7

#define ACCUMULATE 0
#define OUTPUT 1

#define ALL -1

//-----          Structures          ------

enum ERR_CODES {
    ARGC_ERR=2,
    UNKNOWN_ARGV_ERR,
    ARGV_ORDER_ERR,
    ARGV_ERR,

    COL_RNG_ERR,
    ROW_RNG_ERR,
    CELL_TYPE_ERR,
};

const char ERR_MESSAGE[][100] = {
    "Program finished successfully","Unknown error",
    "Argument count error! has: %d required: %d\n", //ARGC_ERR
    "Argument error! Unknown arg: %s\n", //UNKNOWN_ARGV
    "Argument value error! arg: %d argv: %s must be >= than previous arg\n", //ARGV_ORDER_ERR
    "Argument value error! arg: %d argv: %s\n", //ARGV_ERR

    "Range value error! selected range out of table size\nTable has %d cols, range demads: %d\n",
    "Range value error! selected range out of table size\nTable has %d rows, range demads: %d\n",
    "Type error! at cell[col:%d,row:%d]: %s\n %s: %s\n"
};

enum range_mode {
    ROW=1,
    COL,
    ROWS,
    COLS,
    RANGE
};

enum content {
    TEXT=1,
    NUMBER,
    ALL_T,
    UNSET
};

struct task {
    char keywd[CMD_WORD_LEN];
    int (*task)(char* word, int finish);
    int accepts;
    char *error_strng;

};

struct range {
    int rowS;
    int rowE;
    int colS;
    int colE;
};

struct select_command {
    char keywd[CMD_WORD_LEN];
    int args_req;
    int mode;
    //char *error_string;
};
//-----          Structures          ------END

//Funkcia skontroluje ci konverzia skoncila na konci retazca
int check_col(char* word)
{
    char *endptr=NULL;
    //K tejto variacii by nemalo dojst
    if(word == NULL) {
        return UNSET;
    }
    strtod(word,&endptr);
    //Konverzia prebehla na celom stringu
    if(*endptr=='\0') {
        return NUMBER;
    } else
        return TEXT;
}

//-----          TaskFunctions          ------
int help(char* word, int finish)
{
    if(finish==OUTPUT) {
        if(word!=NULL)
            printf( ERR_MESSAGE[UNKNOWN_ARGV_ERR], word);
        printf( "Welcome to simple table processor help page\n"
                "Usage: --help | task selection_type selection\n"
                "   task:\n"
                "       {select | sum | avg | min | max}\n"
                "   selection_type{selection {params}}:\n"
                "       {row {x} | col {x} | rows {x y} | cols {x y} | range{a b x y}}\n\n"
                "Created by Imrich Stoffa - xstoff02@stud.fit.vutbr.cz\n");
    }
    return 0;
}

int sele(char* word, int finish)
{
    if(finish==ACCUMULATE) {
        if(NUMBER==check_col(word))
            printf("%.10g\n",strtod(word,NULL));
        else if(word!=NULL)
            printf("%s\n",word);
    }
    return 0;
}


int aver(char* word, int finish)
{
    static long double avgBuffer=0;
    static int quantity=0;

    if(finish==ACCUMULATE) {
        avgBuffer+=strtod(word, NULL);
        quantity++;
    } else {
        printf("%.10g\n", (double)avgBuffer/quantity);
        avgBuffer=0;
        quantity=0;
        return 0;
    }
    return 0;
}


int summ(char* word, int finish)
{
    static double sumBuffer=0;

    if(finish==ACCUMULATE) {
        sumBuffer+=strtod(word, NULL);
    } else {
        printf("%.10g\n", sumBuffer);
        sumBuffer=0;
        return 0;
    }
    return 0;
}


int mini(char* word, int finish)
{
    static double minBuffer=0;
    double num=0;
    static int init=0;

    if(finish==ACCUMULATE) {
        num=strtod(word, NULL);

        if(!init) {
            minBuffer=num;
            init=1;
        } else if(num<minBuffer) {
            minBuffer=num;
        }
    } else {
        printf("%.10g\n",minBuffer);
        minBuffer=0;
        init=0;
        return 0;
    }
    return 0;
}

int maxi(char* word, int finish)
{
    static double maxBuffer=0;
    double num=0;
    static int init=0;

    if(finish==ACCUMULATE) {
        num=strtold(word, NULL);

        if(!init) {
            maxBuffer=num;
            init=1;
        } else if(num>maxBuffer) {
            maxBuffer=num;
        }

    } else {
        printf("%.10g\n",maxBuffer);
        maxBuffer=0;
        init=0;
        return 0;
    }
    return 0;
}
//-----          TaskFunctions          ------END

int max(int x, int y)
{
    return x>y?x:y;
}

int set_range(int values[], struct range *rng, enum range_mode mode)
{
    switch(mode) {
    case ROW:
            rng->colS=1;
        rng->colE=ALL;
        rng->rowS=values[0];
        rng->rowE=values[0];
        break;

    case COL:
        rng->colS=values[0];
        rng->colE=values[0];
        rng->rowS=1;
        rng->rowE=ALL;
        break;

    case ROWS:
        rng->colS=1;
        rng->colE=ALL;
        rng->rowS=values[0];
        rng->rowE=values[1];
        break;

    case COLS:
        rng->colS=values[0];
        rng->colE=values[1];
        rng->rowS=1;
        rng->rowE=ALL;
        break;

    case RANGE:
        rng->colS=values[2];
        rng->colE=values[3];
        rng->rowS=values[0];
        rng->rowE=values[1];
        break;

    default:
        return 1;
    }
    return 0;
}

int in_col_range(int col, struct range rng)
{
    if(col >= rng.colS && (col <= rng.colE || rng.colE == ALL)) {
        return 1;
    }
    return 0;
}

int in_row_range(int row, struct range rng)
{
    if(row >= rng.rowS && (row <= rng.rowE || rng.rowE == ALL)) {
        return 1;
    }
    return 0;
}

int accepts_content(char *word, int acc_content)
{
    int t = check_col(word);
    if((t == acc_content || acc_content == ALL_T) && t != UNSET) {
        return 1;
    }
    return 0;
}

struct task* init_tasks(void)
{
    static struct task tasks[]= {
        {"--help", help, TEXT, ""},

        {"select", sele, ALL_T, "takes all content types,\nprints one cell per line"},
        {"min", mini, NUMBER, "restritcs cell content to NUMBERS,\ncalculates minimum from given range"} ,
        {"max", maxi, NUMBER, "restritcs cell content to NUMBERS,\ncalculates maximum from given range"},
        {"sum", summ, NUMBER, "restritcs cell content to NUMBERS,\ncalculates sum of given range"},
        {"avg", aver, NUMBER, "restritcs cell content to NUMBERS,\ncalculates avg of given range"},
        {"\0",NULL, UNSET, ""} //Zarazka
    };
    return tasks;
}

struct select_command* init_select_commands(void)
{
    static struct select_command sel[]= {
        {"row",1, ROW},
        {"col",1, COL},
        {"rows",2, ROWS},
        {"cols",2, COLS},
        {"range",4, RANGE},
        {"\0" ,-1, ROW} //Zarazka
    };
    return sel;
}

/**
 * Zpracuje argumenty príkazového riadku. Pokud narazí na chybný argument, vypíše
 * odpovedajúcu hlášku.
 * @param argc Počet argumentov.
 * @param argv Pole textových retazcov s argumenty.
 * @param tskptr Ukazateľ na vybratú štruktúru pre zadanú úlohu.
 * @param selptr Ukazateľ na vybratú štruktúru pre zadaný druh rozsahu.
 * @param rng Struktura kde sa uchovavajú premenne na riadenie vybratého rozsahu
 */
int decode_args(int argc, char **argv, struct task **tskptr, struct select_command **selptr, struct range *rng)
{
    struct task *task_list;
    struct select_command *selection_list;

    task_list=init_tasks();
    selection_list=init_select_commands();

    if(argc>1) {
        int task_sel=0;
        while(strcmp(task_list[task_sel].keywd, argv[1])!=0 && task_list[task_sel].task!=NULL) {
            task_sel++;
        }
        if(task_list[task_sel].task==NULL) {
            help(argv[1],OUTPUT);
            return UNKNOWN_ARGV_ERR;
        }
        *tskptr=(task_list+task_sel);

        if(task_sel==0) {
            help(NULL,OUTPUT);
            return EXIT_SUCCESS;
        }
    } else {
        fprintf(stderr, ERR_MESSAGE[ARGC_ERR] ,0 ,1 );
        return ARGC_ERR;
    }

    if(argc>2) {
        int cmd_sel=0;
        while(strcmp(selection_list[cmd_sel].keywd, argv[2])!=0 && selection_list[cmd_sel].args_req!=-1) {
            cmd_sel++;
        }
        if(selection_list[cmd_sel].args_req==-1) {
            help(argv[2],OUTPUT);
            return UNKNOWN_ARGV_ERR;
        }
        *selptr=(selection_list+cmd_sel);
    } else {
        return ARGC_ERR;
    }

    int verticles[4];
    int insert_i=0;
    int i, si;
    if(argc == 3 + (*selptr)->args_req) {
        for(i=si=3; i < si + (*selptr)->args_req; i++) {
            //Osetrit treba
            char *endptr=NULL;
            if((verticles[insert_i++]=strtol(argv[i],&endptr,10))>0) {
                if( insert_i==2 || insert_i==4 )
                    if(verticles[insert_i-2]>verticles[insert_i-1]) {
                        fprintf(stderr, ERR_MESSAGE[ARGV_ORDER_ERR] ,i,argv[i]);
                        return ARGV_ORDER_ERR;
                    }
                if(*endptr!='\0') {
                    fprintf(stderr, ERR_MESSAGE[ARGV_ERR] ,i,argv[i]);
                    return ARGV_ERR;
                }
            } else {
                fprintf(stderr, ERR_MESSAGE[ARGV_ERR] ,i,argv[i]);
                return ARGV_ERR;
            }
        }
        set_range(verticles, rng, (*selptr)->mode);
    } else {
        fprintf(stderr, ERR_MESSAGE[ARGC_ERR] ,argc, 3+(*selptr)->args_req);
        return ARGC_ERR;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    char line[MAXLINE];
    struct task *tskptr;
    struct select_command *selptr;
    struct range rng= {0,0,0,0};

    int err_code;
    if((err_code=decode_args(argc,argv,&tskptr,&selptr,&rng))!=EXIT_SUCCESS) {
        fprintf(stderr,"Fatal, program abort\n");
        return err_code;
    }else if(tskptr->task==help){
        return EXIT_SUCCESS;
    }

    int row, col, col_m;
    row=col=1;
    col_m = 0;
    int cols_inited=-1;

    char de[]=" \n\t";
    //Nacitavanie riadkov
    while((fgets(line, MAXLINE, stdin))!=NULL) {
        //Spracuj platne riadky
        if(in_row_range(row,rng)) {

            char *word=strtok(line,de);
            for(col=1; word!= NULL; col++) {
                if(in_col_range(col,rng)) {

                    if(accepts_content(word, tskptr->accepts)) {
                        tskptr->task(word,ACCUMULATE);
                    } else {
                        fprintf(stderr, ERR_MESSAGE[CELL_TYPE_ERR],col,row,word,tskptr->keywd,tskptr->error_strng);
                        return CELL_TYPE_ERR;
                    }
                    if(cols_inited==-1)cols_inited=0;
                }
                word=strtok(NULL,de);
            }
            //HACK
            if(cols_inited==-1){
                if(col-1 < rng.colE) {
                    fprintf(stderr, ERR_MESSAGE[COL_RNG_ERR] ,col-1,rng.colE);
                    return COL_RNG_ERR;
                }
            } else if(cols_inited==0) {
                cols_inited=1;
                col_m=max(col-1,col_m);

            } else if(cols_inited==1) {
                if(col_m < rng.colE) {
                    fprintf(stderr, ERR_MESSAGE[COL_RNG_ERR] ,col-1,rng.colE);
                    return COL_RNG_ERR;
                }
            }
        }
        row++;
    }
    if(row-1 < rng.rowE) {
        fprintf(stderr, ERR_MESSAGE[ROW_RNG_ERR] ,row-1,rng.rowE);
        return ROW_RNG_ERR;
    }
    tskptr->task(NULL,OUTPUT);
    return EXIT_SUCCESS;
}
