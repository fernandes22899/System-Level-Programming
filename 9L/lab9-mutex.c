#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX_T 24

void *count(void *);
pthread_mutex_t mutex;
int cnt = 0;

typedef struct
{
    char *name;
    int count;
} filecount_t;

filecount_t filec[MAX_T];
pthread_t ptd[MAX_T];

int main(int argc, char *argv[])
{
    int arg = argc - 1;

    if( pthread_mutex_init(&mutex, NULL) != 0 )
        error("ERROR: init mutex");

    for( int j = 0; j < arg; j++ ){
        filec[j].name = argv[j + 1];
        filec[j].count = 0;
    }

    for( int i = 1; i < argc; i++ ){
        if (pthread_create(&ptd[i - 1], NULL, count, (void *)&filec[i - 1]) != 0)
            printf("ERROR\n");
    }

    for( int k = 0; k < argc - 1; k++ ){
        if (pthread_join(ptd[k], NULL))
            error("ERROR: thread join");
    }

    sort(argc - 1);
    for( int l = 0; l < argc - 1; l++ ){
        printf("%s=%d\n", filec[l].name, filec[l].count);
    }

    printf("Total Count=%d\n", cnt);
    return 0;
}

void sort(int parm)
{
    filecount_t tmp_if;
    for( int i = 0; i < parm - 1; i++ ){
        for( int j = i + 1; j < parm; j++ ){
            if( strcmp(filec[i].name, filec[j].name) > 0 ){
                tmp_if = filec[i];
                filec[i] = filec[j];
                filec[j] = tmp_if;
            }
        }
    }
}

void *count(void *parm)
{

    filecount_t *fc_t = (filecount_t *)parm;
    char *fcName = fc_t->name;
    FILE *f;
    char ch;

    if( fopen(fcName, "r") == NULL )
        return 0;

    f = fopen(fcName, "r");

    while( !feof(f) ){
        ch = getc(f);
        if( ch == '\n' )
            ((filecount_t *)parm)->count = ((filecount_t *)parm)->count + 1;
    }

    if( pthread_mutex_lock(&mutex) != 0 )
        error("ERROR: mutex_lock in child");

    cnt += ((filecount_t *)(parm))->count;
    if( pthread_mutex_unlock(&mutex) != 0 )
        error("ERROR: mutex_unlock in child");

    fclose(f);
    return 0;
}
