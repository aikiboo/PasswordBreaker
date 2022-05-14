#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "md5.h"

#define KEY_NUMBER 3
#define HASH_NUMBER 9
#define SYSTEM_NUMBER 5
#define M 40000000

typedef struct FileLine FileLine;
struct FileLine {
    char *content;
    FileLine *previous;
    FileLine *nextLine;
};

typedef struct FileContent FileContent;
struct FileContent {
    FileLine *firstLine;
    int length;
    char *name;
};


/*
  Permet de chercher si dans une liste de hash, notre chaine est dedans
*/
int findCorrespondance(FileContent *fileContent, char *toCompare) {
    FileLine *fl = fileContent->firstLine;
    while (fl != NULL) {
        char *tmp = fl->content;
        //printf("%s, %s\n",tmp,toCompare);
        if (strcmp(tmp, toCompare) == 0)
            return 1;
        fl = fl->nextLine;
    }
    return 0;
}

/*
Creer un FileContent a partir du nom d'un fichier
*/
FileContent *readFile(char *name) {
    FILE *file;
    file = fopen(name, "r");
    if (file == NULL) {
        printf("Can't open file : %s\n", name);
        exit(2);
    }
    printf("Lecture de %s\n", name);

    FileContent *fileContent = malloc(sizeof(FileContent));
    fileContent->length = 0;
    fileContent->name = malloc(sizeof(char) * strlen(name));
    strcpy(fileContent->name, name);
    fileContent->firstLine = malloc(sizeof(FileLine));

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FileLine *currentFileLine = fileContent->firstLine;
    currentFileLine->previous = NULL;
    while ((read = getline(&line, &len, file)) != -1) {
        line[strlen(line) - 1] = 0;
        currentFileLine->content = malloc(sizeof(char) * strlen(line));
        strcpy(currentFileLine->content, line);
        currentFileLine->nextLine = malloc(sizeof(FileLine));
        FileLine *tmp = currentFileLine;
        currentFileLine = currentFileLine->nextLine;
        currentFileLine->previous = tmp;
        fileContent->length++;
    }
    currentFileLine = currentFileLine->previous;
    free(currentFileLine->nextLine);
    currentFileLine->nextLine = NULL;
    fclose(file);
    return fileContent;
}

/*
  Affiche le contenu du fichier
*/
void displayFileContent(FileContent *fileContent) {
    printf("Fichier de %d lignes\n", fileContent->length);
    FileLine *fl = fileContent->firstLine;
    printf("%d\n", fl == NULL);
    while (fl != NULL) {
        printf("%s\n", fl->content);
        fl = fl->nextLine;
    }
}


int analyseSystem(FileContent *fileContent, unsigned char *keyHashs[KEY_NUMBER][HASH_NUMBER], char **keys) {
    for (int i = 0; i < KEY_NUMBER; i++) {
        for (int j = 0; j < HASH_NUMBER; j++) {
            if (findCorrespondance(fileContent, keyHashs[i][j])) {
                printf("Correspondance dans %s en i = %d pour %s et %s\n", fileContent->name, j + 1, keys[i],
                       keyHashs[i][j]);
                return j + 1;
            }
        }
    }
    return -1;
}
void threadPrint(int tid, char *msg) {
    printf("Thread %d: %s\n", tid, (char *) msg);
}

int main() {
    int i = 0, tid, chunk = 1;
    FileContent *passwordFile = readFile("res/high_frequency_passwords_list.txt");
    FileContent *systems[SYSTEM_NUMBER];
    char file_name[128];
    char c[3];
    for (int x = 0; x < SYSTEM_NUMBER; x++) {
        sprintf(c, "%d", x + 1);
        strncat(file_name, "res/systeme_", (sizeof(file_name) - strlen(file_name) - 1));
        strncat(file_name, c, (sizeof(file_name) - strlen(file_name) - 1));
        strncat(file_name, ".phl", (sizeof(file_name) - strlen(file_name) - 1));
        systems[x] = readFile(file_name);
        memset(file_name, 0, sizeof(file_name));
    }


    char *listPassword[passwordFile->length];
    FileLine *fl = passwordFile->firstLine;
    while (fl != NULL) {
        listPassword[i] = malloc(sizeof(char) * strlen(fl->content));
        strcpy(listPassword[i++], fl->content);
        fl = fl->nextLine;
    }
    unsigned char *listResultat[KEY_NUMBER][HASH_NUMBER];
    int timedHashed[SYSTEM_NUMBER];


    int m = M;
#pragma omp parallel shared(chunk) private(i, tid)


    {
        tid = omp_get_thread_num();
        if(tid == 0){
            printf("Nombre de threads actives : %d\n",omp_get_num_threads());
        }

        unsigned char *mon_hash = malloc(sizeof(unsigned char) * MD5_HASHBYTES); /* le MD5 renvoie 16 octets */

#pragma omp for schedule(static, chunk)
        for (i = 0; i < KEY_NUMBER; i++) {
            threadPrint(tid, listPassword[i]);
            for (int ind = 1; ind <= HASH_NUMBER * m; ind++) {
                if (ind == 1) {
                    calcul_md5(listPassword[i], strlen(listPassword[i]), mon_hash);
                } else {
                    calcul_md5(mon_hash, MD5_HASHBYTES, mon_hash);
                }
                if ((ind) % m == 0) {
                    int index = ind / m - 1;
                    listResultat[i][index] = malloc(sizeof(unsigned char) * MD5_HASHBYTES * 2);
                    //printf("%d\n", index);
                    char *temp = malloc(sizeof(char) * 2);
                    for (int j = 0; j < MD5_HASHBYTES; j++) {
                        //printf("%.2x", mon_hash[j]); // Affichage sur deux caracteres
                        sprintf(temp, "%.2x", mon_hash[j]);
                        listResultat[i][index][j * 2] = temp[0];
                        listResultat[i][index][j * 2 + 1] = temp[1];
                    }
                    //printf("\n" );
                    //threadPrint(tid, listResultat[i][index]);
                }
            }
        }
#pragma omp for schedule(static, chunk)
        for (i = 0; i < SYSTEM_NUMBER; i++) {
            timedHashed[i] = analyseSystem(systems[i], listResultat, listPassword);
        }
    }
    i = 0;
    fl = passwordFile->firstLine;
    while (fl != NULL) {
        listPassword[i] = malloc(sizeof(char) * strlen(fl->content));
        strcpy(listPassword[i++], fl->content);
        fl = fl->nextLine;
    }


    int maxI = 0;
    for(i = 0;i<SYSTEM_NUMBER;i++){
        if(timedHashed[i]>maxI){
            maxI = timedHashed[i];
        }
    }
#pragma omp parallel shared(chunk) private(i, tid)
    {
        tid = omp_get_thread_num();
        unsigned char *mon_hash = malloc(sizeof(unsigned char) * MD5_HASHBYTES); /* le MD5 renvoie 16 octets */
#pragma omp for schedule(static, chunk)
        for (i = KEY_NUMBER; i < passwordFile->length; i++) {
            for (int ind = 1; ind <= maxI * m; ind++) {
                if (ind == 1) {
                    calcul_md5(listPassword[i], strlen(listPassword[i]), mon_hash);
                } else {
                    calcul_md5(mon_hash, MD5_HASHBYTES, mon_hash);
                }
                if ((ind) % m == 0) {
                    int index = ind / m;
                    for (int k = 0; k < SYSTEM_NUMBER; k++) {
                        if (timedHashed[k] == index ) {
                            char *temp = malloc(sizeof(char) * 2);
                            char *tmp = malloc(sizeof(unsigned char) * MD5_HASHBYTES * 2);
                            for (int j = 0; j < MD5_HASHBYTES; j++) {
                                //printf("%.2x", mon_hash[j]); // Affichage sur deux caracteres
                                sprintf(temp, "%.2x", mon_hash[j]);
                                tmp[j * 2] = temp[0];
                                tmp[j * 2 + 1] = temp[1];
                            }
                            if (findCorrespondance(systems[k], tmp)) {
                                printf("Password admin trouvé pour le systeme %d: %s\n",k+1,listPassword[i]);
                            }
                        }
                    }
                }
            }
        }
    }


}
