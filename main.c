#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include IMPL

#define DICT_FILE "./dictionary/words.txt"
#define TABLE_SIZE 1024


static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    struct timespec start, end;
    double cpu_time1, cpu_time2;

    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (fp == NULL) {
        printf("cannot open the file\n");
        return -1;
    }

    /* build the entry */
#if defined(opt_hash)
    entry pHead[TABLE_SIZE], *e[TABLE_SIZE];
    printf("size of entry : %lu bytes\n", sizeof(entry));
    for(i=0; i<TABLE_SIZE; i++) {
        e[i] = &pHead[i];
        e[i]->pNext=NULL;
    }
    i=0;
#else
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));
    printf("size of entry : %lu bytes\n", sizeof(entry));
    e = pHead;
    e->pNext = NULL;
#endif

#if defined(__GNUC__)
#if defined(opt_hash)
    __builtin___clear_cache((char *) pHead, (char *) pHead + TABLE_SIZE * sizeof(entry));
#else
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
#endif
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
#if defined(opt_hash)
        unsigned int hash_key = hash(line) % TABLE_SIZE;
        e[hash_key] = append(line, e[hash_key]);
#else
        e = append(line, e);
#endif
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);

    /* close file as soon as possible */
    fclose(fp);
#if defined(opt_hash)
    for(i = 0; i < TABLE_SIZE; i++) {
        e[i] = &pHead[i];
    }
#else
    e = pHead;
#endif

    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
#if defined(opt_hash)
    unsigned int hash_key = hash(input) % TABLE_SIZE;
    e[hash_key] = &pHead[hash_key];
    assert(findName(input, e[hash_key]) && "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e[hash_key])->lastName, "zyxel"));
#else

    e = pHead;

    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));
#endif

#if defined(__GNUC__)
#if defined(opt_hash)
    __builtin___clear_cache((char *) pHead, (char *) pHead + TABLE_SIZE * sizeof(entry));
#else
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
#if defined(opt_hash)
    findName(input, e[hash_key]);
#else
    findName(input, e);
#endif
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    FILE *output;
#if defined(OPT)
    output = fopen("opt.txt", "a");
#elif defined(opt_hash)
    output = fopen("hash.txt", "a");
#else
    output = fopen("orig.txt", "a");

#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);
#if defined(opt_hash)
    for(i = 0; i < TABLE_SIZE ; i++) {
        if(pHead[i].pNext) free(pHead[i].pNext);
    }
#else
    if (pHead->pNext) free(pHead->pNext);
    free(pHead);
#endif
    return 0;
}
