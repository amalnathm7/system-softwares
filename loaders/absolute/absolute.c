#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* intToHex(int num) {
	char* buff = malloc(7 * sizeof(char));
	sprintf(buff, "%X", num);
	return buff;
}

int hexToInt(char* buff) {
	int num;
	sscanf(buff, "%X", &num);
	return num;
}

void displayObj(char* filename) {
	FILE* fp = fopen(filename, "r");
	
	if(!fp) {
		printf("No such file: %s\n", filename);
		exit(0);
	}
	
	char buff[100];
	
	printf("\nObject program:\n");
	while(1) {
		fgets(buff, 100, fp);
		if(feof(fp))
			break;
		fputs(buff, stdout);
	}
	printf("\n");
	
	fclose(fp);
}

void displayMem() {
	FILE* memFile = fopen("memory", "r");
	
	char buff[10];
	fgets(buff, 10, memFile);
	
	printf("Memory Allocation:\n");
	
	while(!feof(memFile)) {
		printf("%s", buff);
		
		fgets(buff, 10, memFile);
	}
}

void load(char* filename) {
	FILE* objFile = fopen(filename, "r");
	FILE* memFile = fopen("memory", "w");
	char pgName[6];
	int pgAddr;
	int pgLength;
	
	char buff[100];
	fgets(buff, 100, objFile);
	
	if(buff[0] == 'H') {
		for(int i = 2; i < 8; i++) {
			pgName[i-2] = buff[i];
			
			if(buff[i+1] == ' ') {
				pgName[i-1] = '\0';
				break;
			}
		}
		
		printf("Program Name: %s\n", pgName);
		
		char temp[] = {buff[9], buff[10], buff[11], buff[12], buff[13], buff[14], '\0'};
		
		pgAddr = hexToInt(temp);
		
		printf("Program Start Address: %XH\n", pgAddr);
		
		for(int i = 0; i < 6; i++)
			temp[i] = buff[i+16];
		
		pgLength = hexToInt(temp);
		
		printf("Program Length: %XH\n", pgLength);
	} else {
		printf("Invalid object program: %s\n", filename);
		fclose(objFile);
		fclose(memFile);
		return;
	}
	
	fgets(buff, 100, objFile);
	
	int addr = pgAddr, textLen;
	
	while(buff[0] != 'E') {
		char* token = strtok(buff, "^");
		
		token = strtok(NULL, "^");
		
		while(addr != hexToInt(token)) {
			fputs(intToHex(addr), memFile);
			fputs(": XX\n", memFile);
			addr++; 
		}
		
		addr = hexToInt(token);
		
		token = strtok(NULL, "^");
		
		textLen = hexToInt(token);
		
		while(1) {
			token = strtok(NULL, "^");
			if(token == NULL)
				break;
			else {
				if (token[strlen(token) - 1] == '\n')
					token[strlen(token) - 1] = '\0';
				
				for(int i = 0; i < strlen(token); i = i + 2) {
					fputs(intToHex(addr), memFile);
					fputs(": ", memFile);
					
					char temp[] = {token[i], token[i+1], '\n', '\0'};
					fputs(temp, memFile);
					
					addr++;
				}
			}
		}
		
		fgets(buff, 100, objFile);
	}
	
	while(addr <= (pgAddr + pgLength)) {
		fputs(intToHex(addr), memFile);
		fputs(": XX\n", memFile);
		addr++; 
	}
	
	char temp[] = {buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], '\0'};
	
	int execAddr = atoi(temp);
	
	printf("Program Execution Address: %dH\n", execAddr);
	
	fclose(objFile);
	fclose(memFile);
	displayMem();
}

void main() {
	char* filename = malloc(20 * sizeof(char));
	
	printf("Enter filename of object program: ");
	fgets(filename, 20, stdin);
	
	for(int i = 0; i < 20; i++)
		if(filename[i] == '\n')
			filename[i] = '\0';
	
	displayObj(filename);
	load(filename);
}
