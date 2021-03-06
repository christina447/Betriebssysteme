#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "include/web_request.c"


#define QUEUESIZE 50
#define MAXCHAR 50

//-------------------------------QUEUE-------------------------------------------------------
//Queue Struct
typedef struct {
    char *buffer[QUEUESIZE];
    int head, tail;
    int full, empty;
} queue;

int fileCounter = 0;

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
    q->empty = 0;
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
//Zeilenweises Einlesen der Links aus .txt Datei in die Queue
void *fileReader(void *fifo) {
    queue *fifoPtr = (queue *) fifo;
    char fileName[50];
    FILE *fptr = NULL;
    char line[MAXCHAR];
    char *pos;

    printf("Name des einzulesenden Files angeben:\n");
    scanf("%s", fileName);

    fptr = fopen(fileName, "r");
    if (fptr == NULL) {
        printf("Einlesen der Datei nicht möglich");
        return NULL;
    }

    while (fgets(line, MAXCHAR, fptr) != NULL) {
        if ((pos = strchr(line, '\n')) != NULL){
            *pos = '\0';
        }
        addInQ(fifoPtr, line);
    }
}

//------------------------------WEB-REQUEST------------------------------------------------------
//Download der Websites durch WebRequest
void *webRequestAbruf(void *fifo){

    queue *fifoPtr = (queue*) fifo;

    int argcounter = 2;
    char *argvalues[argcounter];
    argvalues[0] = "--webreq-delay 1000";
    argvalues[1] = "--webreq-path download";

    webreq_init(argcounter, argvalues);

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);

    while (!(fifoPtr->empty)) {

        char* quellURL = strdup(delFromQ(fifo));
        char* downloadURL = strdup(quellURL);

        strtok(quellURL, "/");
        char *website = strtok(NULL, "/");

        char filename[50];
        int threadID = (int) pthread_self();
        fileCounter++;

        printf("Thread ID: %i downloads %s\n", threadID, downloadURL);
        snprintf(filename, sizeof(filename), "%i_%i_%s.html", fileCounter, threadID, website);

        webreq_download(downloadURL, filename);
    }
    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);
}



//--------------------------------MAIN------------------------------------------------------------
int main() {

    queue *fifo;
    char *output ;
    pthread_t readerThread;
    int threadAnzahl = 10;


    //Erzeugen der Queue
    fifo = queueInit();
    if (fifo == NULL){
        printf ("Fehler beim initialisieren der Queue");
    }

    //Einlesen des Files durch den Reader-Thread
    pthread_create(&readerThread, NULL, fileReader, fifo);   //Testen der Threads
    pthread_join(readerThread, NULL);

    //Bestimmung der ClientThread Anzahl
    printf("Anzahl der ClientThreads:\n");
    scanf("%d", &threadAnzahl);

    //Zeitpunkt vor dem Download
    struct timeval timeBegin, timeEnd;
    gettimeofday(&timeBegin, NULL);

    //Download durch webRequest
    pthread_t clientThreadArr[threadAnzahl];
    for (int i = 0; i < threadAnzahl; i++){
       pthread_create(&clientThreadArr[i], NULL, webRequestAbruf, fifo);
    }
    for (int i = 0; i < threadAnzahl; i++){
        pthread_join(clientThreadArr[i], NULL);
    }

    //Zeitpunkt nach dem Download
    gettimeofday(&timeEnd, NULL);

    //Dauer des Downloads in ms
    printf("%lu ms\n", (timeEnd.tv_sec - timeBegin.tv_sec)*1000 + (timeEnd.tv_usec-timeBegin.tv_usec)/1000);

    return 0;

}


