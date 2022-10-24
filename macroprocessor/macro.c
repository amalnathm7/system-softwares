#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct table {
	char* name;
	int start;
	int end;
};

int expanded;
FILE* fp1;
FILE* fp2;
char* buff;
struct table NAMTAB[20];
int NAMTAB_LEN;
char* DEFTAB[100];
int DEFTP;
int DEFTC;

void processLine();

void displayPgm() {
	char buff[64];
	fseek(fp1, SEEK_SET, 0);
	
	printf("\nPROGRAM:\n\n");
	while(1) {
		fgets(buff, 64, fp1);
		
		if(feof(fp1))
			break;
		
		fputs(buff, stdout);
	}
}

void displayDeftab() {
	printf("\n\nDEFTAB:\n\n");
	
	for(int i = 0; i < DEFTC; i++)
		printf("%d: %s", i, DEFTAB[i]);
}

void displayNamtab() {
	printf("\n\nNAMTAB:\n\n");
	
	for(int i = 0; i < NAMTAB_LEN; i++)
		printf("%s\t%d\t%d\n", NAMTAB[i].name, NAMTAB[i].start, NAMTAB[i].end);
}

void displayOp() {
	char buff[64];
	fseek(fp2, SEEK_SET, 0);
	
	printf("\n\nOUTPUT:\n\n");
	while(1) {
		fgets(buff, 64, fp2);
		
		if(feof(fp2))
			break;
		
		fputs(buff, stdout);
	}
}

void getLine() {
	if(expanded) {
		buff = strdup(DEFTAB[DEFTP]);
		DEFTP++;
	}
	else 
		fgets(buff, 100, fp1);
}

int* searchNamtab(char* key) {
	for(int i = 0; i < NAMTAB_LEN; i++) {
		if(!strcmp(NAMTAB[i].name, key)) {
			int* arr = malloc(2 * sizeof(int));
			
			arr[0] = NAMTAB[i].start;
			arr[1] = NAMTAB[i].end;
			
			return arr;
		}
	}
	
	return NULL;
}

void insertNamtab(char* key) {
	NAMTAB[NAMTAB_LEN].name = strdup(key);
	NAMTAB[NAMTAB_LEN].start = DEFTC;
}

void updateNamtab() {
	NAMTAB[NAMTAB_LEN].end = DEFTC - 1;
	NAMTAB_LEN++;
}

void insertDeftab(char* str) {
	DEFTAB[DEFTC] = strdup(str);
	DEFTC++;
}

void expand(int start, int end) {
	expanded = 1;
	
	DEFTP = start;
	
	char* temp = strdup(DEFTAB[DEFTP]);
	DEFTP++;
	
	char* str = malloc(66 * sizeof(char));
	strcpy(str, "//");
	strcat(str, temp);
	fputs(str, fp2);
	
	char* token = strtok(buff, "\t");
	
	if(buff[0] != '\t')
		token = strtok(NULL, "\t");
	
	token = strtok(NULL, ",");
	
	char* ARGTAB[10];
	int ARGTAB_LEN = 0;
	
	while(token != NULL) {
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';
	
		ARGTAB[ARGTAB_LEN] = strdup(token);
		ARGTAB_LEN++;
		
		token = strtok(NULL, ",");
	}
	
	while(DEFTP != end) {
		getLine();
		processLine(ARGTAB, ARGTAB_LEN);
	}
	
	expanded = 0;
}

void define() {
	char* temp = strdup(buff);
	char* token = strtok(buff, "\t");

	insertNamtab(token);

	insertDeftab(temp);
	
	token = strtok(NULL, "\t");
	
	token = strtok(NULL, ",");
	
	char* ARGTAB[10];
	int ARGTAB_LEN = 0;
	
	while(token != NULL) {
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';
	
		ARGTAB[ARGTAB_LEN] = strdup(token);
		ARGTAB_LEN++;
		
		token = strtok(NULL, ",");
	}
	
	int level = 1;
	
	while(level > 0) {
		char* str = malloc(64 * sizeof(char));
		getLine();
		
		token = strtok(buff, "\t");
		
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';
		
		strcpy(str, token);
		
		if(buff[0] != '\t') {
			token = strtok(NULL, "\t");
			
			if(token[strlen(token) - 1] == '\n')
				token[strlen(token) - 1] = '\0';
			
			strcat(str, "\t");
			strcat(str, token);
		}
		
		token = strtok(NULL, ",");
				
		int flag1 = 0;
		
		while(token != NULL) {
			if(token[strlen(token) - 1] == '\n')
				token[strlen(token) - 1] = '\0';
			
			int flag = -1;
			
			for(int i = 0; i < ARGTAB_LEN; i++) {
				if(!strcmp(ARGTAB[i], token)) {
					flag = i;
					break;
				}
			}
			
			if(flag == -1) {
				strcat(str, "\t");
				strcat(str, token);
			} else {
				char str1[10];
				sprintf(str1, "%d", flag);
								
				if(flag1)
					strcat(str, ",");
				else {
					flag1 = 1;
					strcat(str, "\t");
				}
				
				strcat(str, "?");
				strcat(str, str1);
			}
			
			token = strtok(NULL, ",");
		}
		
		strcat(str, "\n");
		
		insertDeftab(str);
		
		token = strtok(buff, "\t");
		
		if(buff[0] != '\t')
			token = strtok(NULL, "\t");
		
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';
		
		if(!strcmp(token, "MACRO"))
			level++;
		else if(!strcmp(token, "MEND"))
			level--;
	}
	
	updateNamtab();
}

void processLine(char* ARGTAB[], int ARGTAB_LEN) {
	char* temp = strdup(buff);
	char* token = strtok(buff, "\t");
	
	if(buff[0] != '\t')
		token = strtok(NULL, "\t");
	
	if(token != NULL) {
		if(token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';

		int* index = searchNamtab(token);

		if(index != NULL) {
			buff = strdup(temp);
			expand(index[0], index[1]);
		}
		else if (!strcmp(token, "MACRO")) {
			buff = strdup(temp);
			define();
		}
		else {
			if(ARGTAB == NULL) {
				fputs(temp, fp2);
				return;
			}
			
			char* str = malloc(64 * sizeof(char));
			
			token = strtok(temp, "\t");
			
			if(token[strlen(token) - 1] == '\n')
				token[strlen(token) - 1] = '\0';
			
			strcpy(str, "\t");
			strcat(str, token);
			
			token = strtok(NULL, ",");
			
			int flag = 0;
			
			while(token != NULL) {
				if(token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = '\0';
				
				if(token[0] == '?') {
					for(int i = 1; i < strlen(token); i++)
						token[i-1] = token[i];
					
					token[strlen(token) - 1] = '\0';
					
					int index = atoi(token);
					
					if(flag)
						strcat(str, ",");
					else {
						flag = 1;
						strcat(str, "\t");
					}
					
					strcat(str, ARGTAB[index]);
				} else {
					strcat(str, "\t");
					strcat(str, token);
				}
				
				token = strtok(NULL, ",");
			}
			
			strcat(str, "\n");
			
			fputs(str, fp2);
		}
	}
}

void main() {
	char* filename = malloc(20 * sizeof(char));
	buff = malloc(64 * sizeof(char));
	
	printf("Enter filename of object program: ");
	fgets(filename, 20, stdin);
	
	for(int i = 0; i < 20; i++)
		if(filename[i] == '\n')
			filename[i] = '\0';
	
	fp1 = fopen(filename, "r+");
	fp2 = fopen("expanded", "w+");
	
	expanded = 0;
	NAMTAB_LEN = 0;
	DEFTC = 0;
	DEFTP = 0;
	
	if(fp1 != NULL) {
		while(1) {
			getLine();
			
			if(feof(fp1))
				break;
			
			processLine(NULL, 0);
		}
		
		displayPgm();
		displayDeftab();
		displayNamtab();
		displayOp();
	}
	
	fclose(fp1);
	fclose(fp2);
}
