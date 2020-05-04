#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "include/web_request.c"


#define QUEUESIZE 20
#define MAXCHAR 50

//-------------------------------QUEUE-------------------------------------------------------
//Queue Struct
typedef struct {
    char *buffer[QUEUESIZE];
    int head, tail;
    int full, empty;
} queue;

//Queue erzeugen
queue *queueInit (void){

    queue *q;
    q=(queue *)malloc (sizeof ( queue));
    if (q == NULL){
        return (NULL);
    }
    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;
}

//Objekte in die Queue einfügen
void addInQ(queue *q, char *in){

    char* dup = strdup(in);

    q->buffer[q->tail] = dup;
    q->tail++;
    if (q->tail == QUEUESIZE){
        q->tail = 0;
    }
    if (q->tail == q->head){
        q->full = 1;
    }
    return;
}

//Entnehmen bzw. löschen von Objekten aus der Queue
char* delFromQ(queue *q){

    char *out = q->buffer[q->head];

    q->head++;
    if (q->head == QUEUESIZE){
        q->head = 0;
    }
    if (q->head == q->tail){
        q->empty = 1;
    }
    q->full = 0;
    return out;
}

// -----------------------------FILE-READER-----------------------------------------
void fileReader(queue **fifo) {
    char fileName[50];
    FILE *fptr = NULL;
    char line[MAXCHAR];
    char *c;

    printf("Name des einzulesenden Files angeben:\n");
    scanf("%s", fileName);

    fptr = fopen(fileName, "r");
    if (fptr == NULL) {
        printf("Einlesen der Datei nicht möglich");
        return;
    }

    while (fgets(line, MAXCHAR, fptr) != NULL) {
       // if (*c = strchr(line, '\n') != NULL){
       //     *c = '\0';
     //   }
        addInQ(fifo, line);
    }
}

//------------------------------WEB-REQUEST------------------------------------------------------
//lib initalisieren
//int argcounter = 2;
//char *argvalues[argcounter] = {'--webreq-delay 100', '--webreq-path download'};

//webreq_init(argc, argv);





//--------------------------------MAIN------------------------------------------------------------
int main() {

    queue *fifo;
    char *output ;
    pthread_t readerThread;

    //Erzeugen der Queue
    fifo = queueInit();
    if (fifo == NULL){
        printf ("Fehler beim initialisieren der Queue");
    }

    //Einlesen des Files durch den Reader-Thread
    pthread_create(&readerThread, NULL, fileReader, fifo);   //Testen der Threads

   // fileReader(fifo);

    pthread_join(readerThread, NULL);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);
    output = delFromQ(fifo);
    printf("out: %s", output);


}


