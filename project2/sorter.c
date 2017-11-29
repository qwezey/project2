#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "sorter.h"
#include "memTools.h"
#include "queue.h"

int CsvCounter = 0;
pthread_mutex_t CCM = PTHREAD_MUTEX_INITIALIZER;

int odd;
pthread_mutex_t om = PTHREAD_MUTEX_INITIALIZER;

int DoneSorting = 0;
pthread_cond_t DSCV = PTHREAD_COND_INITIALIZER;
pthread_mutex_t DSM = PTHREAD_MUTEX_INITIALIZER;

char * Header;
unsigned int SortIndex;
int IsNumeric;
char * OutputDir;

void increment() {
    
    pthread_mutex_lock(&CCM);
    CsvCounter++;
    pthread_mutex_unlock(&CCM);
}

void decrement() {
    
    pthread_mutex_lock(&CCM);
    CsvCounter--;
    pthread_mutex_unlock(&CCM);
}

int isOdd() {
    
    pthread_mutex_lock(&om);
    
    int ret = odd;
    
    if (odd) {
        odd = 0;
    } else {
        odd = 1;
    }
    
    pthread_mutex_unlock(&om);
    printf("i am odd: %d\n", odd);
    return ret;
}

// Merges sub rows from <table>, where subrows1 is from row at index <start>
// inclusive to row at index <mid> exclusive, and subrows2 is from row at
// index <mid> inclusive to row at index <end> exclusive. As the sub rows
// merge, they are sorted ascendingly according to the column at index
// <columnIndex>. If areNumbers is set to 0, the sort is done numerically
// otherwise it is done lexicographically.
void merge(char *** table, unsigned int start, unsigned int mid, unsigned int end) {
    
    char ** temp[end - start];
    unsigned int s = start;
    unsigned int m = mid;
    unsigned int i = 0;
    
    while(s < mid && m < end) {
        
        if(isXBeforeY(table[s][SortIndex], table[m][SortIndex])) {
            
            temp[i] = (table)[s];
            s++;
            
        } else {
            
            temp[i] = (table)[m];
            m++;
        }
        
        i++;
    }
    
    while (s < mid) {
        
        temp[i] = (table)[s];
        s++;
        i++;
    }
    
    while (m < end) {
        
        temp[i] = (table)[m];
        m++;
        i++;
    }
    
    s = start;
    
    for (int j = 0; j < i; j++) {
        
        table[s] = temp[j];
        s++;
    }
}

// Ascendingly sorts <table> according to the column at index <columnIndex> from row at
// index <start> to row at index <end> - 1. If areNumbers is set to 0, the sort
// is done numerically otherwise it is done lexicographically.
void mergeSort(char *** table, unsigned int start, unsigned int end) {
    
    if ((end - start) > 1 ) {
        
        unsigned int mid = ((end - start) / 2) + start;
        
        mergeSort(table, start, mid);
        mergeSort(table, mid, end);
        
        merge(table, start, mid, end);
    }
}

// Sorts a the CSV file at <csvPath> in ascending order on the
// column header <columnHeader> at index <sortIndex>. Saves the
// sorted csv file in <outputDir>.
void * sortCsv(void * path) {
    
    if (!isCsv(path)) {
        
        fprintf(stderr, "Not a CSV file: %s\n", path);
        fflush(stderr);
        free(path);
        decrement();
        pthread_exit(NULL);
    }
    
    struct table * table = malloc(sizeof(struct table));
    
    if (!fillTable(path, table)) {
        
        fprintf(stderr, "Not a proper movie_metadata CSV file: %s\n", path);
        fflush(stderr);
        free(path);
        decrement();
        pthread_exit(NULL);
    }
    
    mergeSort(table->table, 1, table->numRows);
    mergeThreads(table);
    
    return NULL;
}

void mergeThreads(struct table * table) {
    
    if (CsvCounter == 1) {
        printf("hello qelsm %d\n", QElements);
        printToSortedCsvPath("in/all.csv", table->table, table->numRows);
        
        pthread_mutex_lock(&DSM);
        DoneSorting = 1;
        pthread_cond_signal(&DSCV);
        pthread_mutex_unlock(&DSM);
        
        pthread_exit(NULL);
    }
    
    if (isOdd()) {

        void * ret;
        
        pthread_mutex_lock(&QM);
        while(QElements == 0) {
            pthread_cond_wait(&QCV, &QM);
        }
        pthread_join(popTid(), &ret);
        pthread_mutex_unlock(&QM);
        
        struct table * table2 = (struct table *) ret;
        
        mergeTables(table, table2);
    }

    pushTid(pthread_self());
    pthread_exit(table);
    
}


void mergeTables(struct table * table1, struct table * table2) {
    
    struct table * table = (struct table *) malloc(sizeof(struct table));
    table->table = (char ***) malloc(sizeof(char **) * (table1->numRows + table2->numRows - 1));
    table->numRows = table1->numRows + table2->numRows - 1;
    
    unsigned int fc = 1;
    unsigned int sc = 1;
    unsigned int tc = 1;
    
    table->table[0] = table1->table[0];
    
    while (fc < table1->numRows && sc < table2->numRows) {
        
        if (isXBeforeY(table1->table[fc][SortIndex], table2->table[sc][SortIndex])) {
            
            table->table[tc] = table1->table[fc];
            fc++;
            
        } else {
            
            table->table[tc] = table2->table[sc];
            sc++;
        }
        
        tc++;
    }
    
    while (fc < table1->numRows) {
        
        table->table[tc] = table1->table[fc];
        fc++;
        tc++;
    }
    
    while (sc < table2->numRows) {
        
        table->table[tc] = table2->table[sc];
        sc++;
        tc++;
    }
    
    free(table1->table);
    free(table2->table);
    
    table->rowsMems = (char * **) malloc(sizeof(char **) * (table1->numRowsMems + table2->numRowsMems));
    unsigned int trmc = 0;
    for (int i = 0; i < table1->numRowsMems; i++) {
        table->rowsMems[trmc] = table1->rowsMems[i];
        trmc++;
    }
    for (int i = 0; i < table2->numRowsMems; i++) {
        table->rowsMems[trmc] = table2->rowsMems[i];
        trmc++;
    }
    table->numRowsMems = trmc;
    free(table1->rowsMems);
    free(table2->rowsMems);
    
    table->cellsMems = (char * *) malloc(sizeof(char *) * (table1->numCellsMems + table2->numCellsMems));
    unsigned int tcmc = 0;
    for (int i = 0; i < table1->numCellsMems; i++) {
        table->cellsMems[tcmc] = table1->cellsMems[i];
        tcmc++;
    }
    for (int i = 0; i < table2->numCellsMems; i++) {
        table->cellsMems[tcmc] = table2->cellsMems[i];
        tcmc++;
    }
    table->numCellsMems = tcmc;
    free(table1->cellsMems);
    free(table2->cellsMems);
    
//    pthread_exit(NULL);
    decrement();
    mergeThreads(table);
}
