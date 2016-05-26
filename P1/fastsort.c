#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef struct{
  int key;
  char line[128];
}rec_t;

char* arg2;

int main (int argc, char *argv[]){
  int lineNumber = 0;
  //test command line argument
  if (!(argc == 2 || argc == 3)){
    fprintf(stderr, "Bad command line parameters\n");
    exit(1);
  }
  if (argc == 3){
    arg2 = (char*)malloc(sizeof(char) * strlen(argv[1]));
    strcpy(arg2, argv[1]);
    if (arg2[0] != '-'){
      fprintf(stderr, "Bad command line parameters\n");
      exit(1);
    }
    char arg2sub[sizeof(arg2)-1]; 
    memcpy(arg2sub, &arg2[1], sizeof(arg2)-1);
    arg2sub[sizeof(arg2)] = '\0';
    int const location = atoi(arg2sub);
    if (location == 0){
      fprintf(stderr, "Bad command line parameters\n");
      exit(1);
    }
    lineNumber = 0;
    //test whether the file is valid 
    FILE *fp = fopen(argv[2],"r");
    char line[130];
    char *ptr = line;
    if (fp == NULL){
      fprintf(stderr, "Error: Cannot open file %s \n", argv[1]);
      exit(1);
    }
    char ch;
    //find line number of the file
    while((ch = fgetc(fp)) != EOF)
    {
      if (ch =='\n') {lineNumber ++;}
    }
    fclose(fp);
    //test line length of the file 
    fp = fopen(argv[2],"r");
    int i = 0;
    for (i = 0; i< lineNumber; i++){
      if (fgets(ptr, 130, fp)!= NULL && strlen(ptr)>128){
        fprintf(stderr, "Line too long\n");
        exit(1);
      }
    }
    fclose(fp);
    //reserve space for reading the file
    rec_t *db;
    db = malloc(lineNumber * sizeof(rec_t));
    if (!db){exit(1);}//check malloc
    //read the file
    char line2[128];
    fp = fopen(argv[2],"r+");
    i = 0;
    //printf("%d",lineNumber);
    for (i =0; i< lineNumber; i++){
      if (fgets(line2, 128, fp) != NULL){
        db[i].key = location -1;
        //printf("%d",i);
        strcpy(db[i].line, line2);
        //printf(db[i].line);
      }
    }
    //free(db);
    //printf("%d", lineNumber);
    //printf(db[0].line);
    int change = 1;
    while (change != 0){
      change = 0;
      char lineOne[128];
      char lineTwo[128];
      char sub[4] = "line";
      //printf("%d", lineNumber);
      for (i=0; i<lineNumber-1; i++){
        //printf("%d", lineNumber);
	strcpy(lineOne, db[i].line);
	strcpy(lineTwo, db[i+1].line);
	char *tokenOne;
        tokenOne = strtok(lineOne, " ");
        int j =0;
        for (j=0; j<location-1; j++){
	  tokenOne = strtok(NULL, " ");
          if (tokenOne == NULL){
            tokenOne = sub;
	    break;
          }
	}
	char *tokenTwo;
        tokenTwo = strtok(lineTwo," ");
	int k =0;
	for (k=0; k<location-1; k++){
	  tokenTwo = strtok(NULL, " ");
          if (tokenTwo == NULL){
            tokenTwo = sub;
            break;
          }
	}
	if (strcmp(tokenOne, tokenTwo) < 0){}
	if (strcmp(tokenOne, tokenTwo) == 0){}
	if (strcmp(tokenOne, tokenTwo) > 0){
          //printf("1");
          strcpy(lineOne, db[i].line);
	  strcpy(lineTwo, db[i+1].line);
	  strcpy(db[i+1].line, lineOne);
	  strcpy(db[i].line, lineTwo);
          change ++;
	}
      }
    }
    //print the db
    i=0;
    for (i=0; i<lineNumber; i++){
      printf("%s", db[i].line);
    }
  }
  else{
    lineNumber = 0;
    //test whether the file is valid 
    FILE *fp = fopen(argv[1],"r");
    char line[130];
    char *ptr = line;
    if (fp == NULL){
      fprintf(stderr, "Error: Cannot open file %s \n", argv[1]);
      exit(1);
    }
    char ch;
    //find line number of the file
    while((ch = fgetc(fp)) != EOF)
    {
      if (ch =='\n') {lineNumber ++;}
    }
    fclose(fp);
    //test line length of the file 
    fp = fopen(argv[1],"r");
    int i = 0;
    for (i = 0; i< lineNumber; i++){
      if (fgets(ptr, 130, fp)!= NULL && strlen(ptr)>128){
        fprintf(stderr, "Line too long\n");
        exit(1);
      }
    }
    fclose(fp);
    //reserve space for reading the file
    rec_t *db;
    db = malloc(lineNumber * sizeof(rec_t));
    if (!db){exit(1);}//check malloc
    //read the file
    char line2[128];
    fp = fopen(argv[1],"r+");
    i = 0;
    //printf("%d",lineNumber);
    for (i =0; i< lineNumber; i++){
      if (fgets(line2, 128, fp) != NULL){
        db[i].key = 0;
        //printf("%d",i);
        strcpy(db[i].line, line2);
        //printf(db[i].line);
      }
    }
    //free(db);
    //printf("%d", lineNumber);
    //printf(db[0].line);
    
    //sort
    int change = 1;
    while (change != 0){
      change = 0;
      char lineOne[128];
      char lineTwo[128];
      //printf("%d", lineNumber);
      for (i=0; i<lineNumber-1; i++){
        //printf("%d", lineNumber);
	strcpy(lineOne, db[i].line);
	strcpy(lineTwo, db[i+1].line);
	char *tokenOne;
        tokenOne = strtok(lineOne, " ");
	char *tokenTwo;
        tokenTwo = strtok(lineTwo," ");
	if (strcmp(tokenOne, tokenTwo) < 0){}
	if (strcmp(tokenOne, tokenTwo) == 0){}
	if (strcmp(tokenOne, tokenTwo) > 0){
          //printf("1");
          strcpy(lineOne, db[i].line);
	  strcpy(lineTwo, db[i+1].line);
	  strcpy(db[i+1].line, lineOne);
	  strcpy(db[i].line, lineTwo);
          change ++;
	}
      }
    }
    //print the db
    i=0;
    for (i=0; i<lineNumber; i++){
      printf("%s",db[i].line);
    }
  }
  //free(db);
  //fclose(fp);
  return 0;
}
