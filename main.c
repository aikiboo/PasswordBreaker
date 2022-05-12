#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

#define KEY_NUMBER 2
#define HASH_NUMBER 9

typedef struct FileLine FileLine;
struct FileLine
{
    char * content;
    FileLine* previous;
    FileLine* nextLine;
};

typedef struct FileContent FileContent;
struct FileContent
{
    FileLine* firstLine;
    int length;
    char* name;
};



/*
  Permet de chercher si dans une liste de hash, notre chaine est dedans
*/
int findCorrespondance(FileContent* fileContent,char * toCompare){
  FileLine* fl = fileContent->firstLine;
  while(fl != NULL){
    char* tmp = fl->content;
    if(strcmp(tmp,toCompare)==0)
      return 1;
    fl = fl->nextLine;
  }
  return 0;
}

/*
Creer un FileContent a partir du nom d'un fichier
*/
FileContent* readFile(char* name){
  FILE* file;
  file = fopen(name,"r");
  if(file == NULL){
    printf("Can't open file : %s\n", name);
    return NULL;
  }
  printf("Lecture de %s\n", name);

  FileContent* fileContent = malloc(sizeof(FileContent));
  fileContent->length = 0;
  fileContent->name = malloc(sizeof(char)*strlen(name));
  strcpy(fileContent->name,name);
  fileContent->firstLine = malloc(sizeof(FileLine));

  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  FileLine* currentFileLine = fileContent->firstLine;
  currentFileLine->previous = NULL;
  while((read = getline(&line, &len, file)) != -1){
        line[strlen(line)-1] = 0;
        currentFileLine->content = malloc(sizeof(char)*strlen(line));
        strcpy(currentFileLine->content,line);
        currentFileLine->nextLine = malloc(sizeof(FileLine));
        FileLine* tmp = currentFileLine;
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
void displayFileContent(FileContent* fileContent){
  printf("Fichier de %d lignes\n",fileContent->length);
  FileLine* fl = fileContent->firstLine;
  printf("%d\n",fl==NULL );
  while(fl !=NULL){
    printf("%s\n",fl->content);
    fl = fl->nextLine;
  }
}


int analyseSystem(FileContent* fileContent,unsigned char*** keyHashs,char** keys){
  for(int i = 0;i <KEY_NUMBER;i++){
    for(int j = 0;j<HASH_NUMBER;j++){
      if(findCorrespondance(fileContent,keyHashs[i][j])){
        printf("Correspondance dans %s en i = %d pour %s\n et %s\n",fileContent->name,j+1,keys[i],keyHashs[i][j]);
        return j+1;
      }
    }
  }
  return -1;
}


void main()
{
    int i=0;
    FileContent* passwordFile = readFile("high_frequency_passwords_list.txt");
    FileContent* sys1 = readFile("systeme_1.phl");
    FileContent* sys2 = readFile("systeme_2.phl");
    FileContent* sys3 = readFile("systeme_3.phl");
    FileContent* sys4 = readFile("systeme_4.phl");
    FileContent* sys5 = readFile("systeme_5.phl");
    char* listPassword[passwordFile->length];
    FileLine* fl = passwordFile->firstLine;
    while(fl!=NULL){
      listPassword[i] = malloc(sizeof(char)*strlen(fl->content));
      strcpy(listPassword[i++],fl->content);
      fl = fl->nextLine;
    }
    unsigned char** listResultat[KEY_NUMBER];



    int m = 40000000;

    unsigned char* mon_hash = malloc(sizeof(unsigned char)*MD5_HASHBYTES); /* le MD5 renvoie 16 octets */
    for (i = 0; i < KEY_NUMBER; i++)
    {
        unsigned char* password = malloc(sizeof(unsigned char)*strlen(listPassword[i]));
        strcpy(password, listPassword[i]);
        printf("mdp : %s \n", password);
        unsigned char* hashs[HASH_NUMBER];
        for (int ind = 1;ind <=HASH_NUMBER*m;ind++){
          if(ind == 1){
            calcul_md5(listPassword[i],strlen(listPassword[i]),mon_hash);
          }
          else{
            calcul_md5(mon_hash,MD5_HASHBYTES,mon_hash);
          }
          if((ind) % m == 0 ){
            int index = ind/m - 1;
            hashs[index] = malloc(sizeof(unsigned char)*MD5_HASHBYTES*2);
            //printf("%d\n", index);
            unsigned char* temp= malloc(sizeof(char)*2) ;
            for(int j=0; j < MD5_HASHBYTES;j++){
                //printf("%.2x", mon_hash[j]); // Affichage sur deux caracteres
                sprintf(temp,"%.2x",mon_hash[j]);
                hashs[index][j*2] = temp[0];
                hashs[index][j*2 +1] = temp[1];
            }
            //printf("\n" );
            //printf("%s\n", hashs[index]);
            if(findCorrespondance(sys1,hashs[index])>0)
              printf("Correspondance dans sys1 en i = %d pour %s\n et %s\n",index+1,password,hashs[index]);

            if(findCorrespondance(sys2,hashs[index])>0)
              printf("Correspondance dans sys2 en i = %d pour %s\n et %s\n",index+1,password,hashs[index]);

            if(findCorrespondance(sys3,hashs[index])>0)
              printf("Correspondance dans sys3 en i = %d pour %s\n et %s\n",index+1,password,hashs[index]);
            if(findCorrespondance(sys4,hashs[index])>0)
              printf("Correspondance dans sys4 en i = %d pour %s\n et %s\n",index+1,password,hashs[index]);
            if(findCorrespondance(sys5,hashs[index])>0)
              printf("Correspondance dans sys5 en i = %d pour %s\n et %s\n",index+1,password,hashs[index]);
          }
          listResultat[i] = &hashs;
        }
    }
    analyseSystem(sys1,listResultat,listPassword);
    analyseSystem(sys2,listResultat,listPassword);
    analyseSystem(sys3,listResultat,listPassword);
    analyseSystem(sys4,listResultat,listPassword);
    analyseSystem(sys5,listResultat,listPassword);



    //free(listResultat);

}
