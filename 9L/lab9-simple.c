#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX_T 24

void *count(void *);

typedef struct
{
    int count;
    char *name;
} filecount_t;

pthread_t pthread[MAX_T];
filecount_t filecount[MAX_T];

void sort(int parm)
{
    filecount_t tmp_if;
    for( int i = 0; i < parm - 1; i++ ){
        for( int j = i + 1; j < parm; j++ ){
            if( strcmp(filecount[i].name, filecount[j].name) > 0 ){
                tmp_if = filecount[i];
                filecount[i] = filecount[j];
                filecount[j] = tmp_if;
            }
        }
    }
}

void *count(void *parm)
{
    char ch;
    FILE *f;
    filecount_t *fc = (filecount_t *)parm;
    char *fc_name = fc->name;

    if( fopen(fc_name, "r") == NULL )
        return 0;

    f = fopen(fc_name, "r");
    while( !feof(f) ){
        ch = getc(f);
        if( ch == '\n' )
            ((filecount_t *)parm)->count = ((filecount_t *)parm)->count + 1;
    }

    fclose(f);
    return 0;
}

int main( int argc, char *argv[] )
{
    int cnt = 0;
    int arg = argc - 1;

    for( int v = 0; v < arg; v++ ){
        filecount[v].name = argv[v + 1];
        filecount[v].count = 0;
    }

    for( int i = 1; i < argc; i++ ){
        if( pthread_create(&pthread[i - 1], NULL, count, (void *)&filecount[i - 1]) != 0 )
            printf("ERROR\n");
    }

    for( int j = 0; j < argc - 1; j++ ){
        if (pthread_join(pthread[j], NULL))
            error("ERROR: joining thread");
    }

    sort(arg);
    for( int k = 0; k < argc - 1; k++ ){
        printf("%s=%d\n", filecount[k].name, filecount[k].count);
        cnt += filecount[k].count;
    }

    printf("Total Count=%d\n", cnt);
    return 0;
}
