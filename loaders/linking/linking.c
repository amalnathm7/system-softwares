#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct map
{
	char *key;
	char *value;
} map;

int PROGADDR;
int CSADDR;
int CSLTH;
int EXECADDR;
map ESTAB[100];
int ESTAB_SIZE = 0;

int getAddress()
{
	return 4096;
}

int containsKey(map arr[], int size, char *key)
{
	if (key == NULL)
		return 0;

	for (int i = 0; i < size; i++)
		if (!strcmp(arr[i].key, key))
			return 1;

	return 0;
}

void updateKey(map arr[], int size, char *key, char *value)
{
	if (key == NULL)
		return;

	for (int i = 0; i < size; i++)
		if (!strcmp(arr[i].key, key))
			arr[i].value = value;
}

char *getValue(map arr[], int size, char *key)
{
	if (key == NULL)
		return NULL;

	for (int i = 0; i < size; i++)
		if (!strcmp(arr[i].key, key))
		{
			char *temp = strdup(arr[i].value);
			return temp;
		}

	return NULL;
}

char *intToHex(int num)
{
	char *buff = malloc(7 * sizeof(char));
	sprintf(buff, "%X", num);
	return buff;
}

int hexToInt(char *buff)
{
	int num;
	sscanf(buff, "%X", &num);
	return num;
}

void displayObj(char *filename)
{
	FILE *fp = fopen(filename, "r");
	char buff[100];

	if (!fp)
	{
		printf("No such file: %s\n", filename);
		exit(0);
	}

	printf("\nObject program:\n");
	while (1)
	{
		fgets(buff, 100, fp);
		if (feof(fp))
		break;
		fputs(buff, stdout);
	}
	printf("\n");

	fclose(fp);
}

void displayMem()
{
	FILE *memFile = fopen("memory", "r");

	char buff[10];
	fgets(buff, 10, memFile);

	printf("\nMemory Allocation:\n");

	while (!feof(memFile))
	{
		printf("%s", buff);

		fgets(buff, 10, memFile);
	}
}

void displayESTAB()
{
	printf("ESTAB:\n");
	for (int i = 0; i < ESTAB_SIZE; i++)
	{
		fputs(ESTAB[i].key, stdout);
		fputs("\t", stdout);
		fputs(ESTAB[i].value, stdout);
		fputs("\n", stdout);
	}
}

void displayEstabFile()
{
	FILE *fp = fopen("estab", "r");

	printf("ESTAB:\n");
	if (fp)
	{
		char buff[20];

		while (1)
		{
			fgets(buff, 20, fp);
			if (feof(fp))
				break;

			fputs(strtok(buff, "\t"), stdout);
			printf("\t");

			char *temp = strtok(NULL, "\t");
			if (temp[strlen(temp) - 1] == '\n')
				temp[strlen(temp) - 1] = '\0';

			fputs(temp, stdout);
			printf("\n");
		}

		fclose(fp);
	}
}

void pass1(char *filename)
{
	FILE *objFile = fopen(filename, "r");

	PROGADDR = getAddress();
	CSADDR = PROGADDR;

	char buff[100], CSNAME[6];
	fgets(buff, 100, objFile);

	while (!feof(objFile))
	{
		if (buff[0] == 'H')
		{
			for (int i = 2; i < 8; i++)
			{
				CSNAME[i - 2] = buff[i];

				if (buff[i + 1] == ' ')
				{
					CSNAME[i - 1] = '\0';
					break;
				}
			}

			if (containsKey(ESTAB, ESTAB_SIZE, CSNAME))
			{
				printf("\nDuplicate control section name \"%s\". Aborted.\n", CSNAME);
				fclose(objFile);
				remove("estab");
				exit(0);
			}
			else
			{
				ESTAB[ESTAB_SIZE].key = strdup(CSNAME);
				ESTAB[ESTAB_SIZE].value = intToHex(CSADDR);

				ESTAB_SIZE++;

				printf("Control Section Name: %s\n", CSNAME);
			}

			char temp[6];
			for (int i = 0; i < 6; i++)
				temp[i] = buff[i + 16];
			temp[6] = '\0';

			CSLTH = hexToInt(temp);
			
			printf("Control Section Start Address: %X\n", CSADDR);
			printf("Control Section Length: %XH\n", CSLTH);
		}

		fgets(buff, 100, objFile);

		while (buff[0] != 'E')
		{
			char *token = strtok(buff, "^");

			if (!strcmp(token, "D"))
			{
				while (1)
				{
					token = strtok(NULL, "^");

					if (!token)
						break;

					for (int i = 0; i < 6; i++)
						if (token[i] == ' ')
							token[i] = '\0';

					if (containsKey(ESTAB, ESTAB_SIZE, token))
					{
						printf("\nDuplicate external symbol \"%s\". Aborted.\n", token);
						fclose(objFile);
						remove("estab");
						exit(0);
					}

					ESTAB[ESTAB_SIZE].key = strdup(token);

					token = strtok(NULL, "^");

					if (!token)
					{
						printf("Bad Define record. Aborted.\n");
						fclose(objFile);
						exit(0);
					}

					ESTAB[ESTAB_SIZE].value = strdup(intToHex(hexToInt(token) + CSADDR));

					ESTAB_SIZE++;
				}
			}

			fgets(buff, 100, objFile);
		}

		printf("\n%s pass 1 completed.\n\n", CSNAME);
		CSADDR += CSLTH;

		fgets(buff, 100, objFile);
	}

	fclose(objFile);
}

void pass2(char *filename)
{
	FILE *objFile = fopen(filename, "r");
	FILE *memFile = fopen("memory", "w+");
	int addr = PROGADDR, textLen;
	long seek1;

	CSADDR = PROGADDR;

	char buff[100], CSNAME[6];
	fgets(buff, 100, objFile);

	while (!feof(objFile))
	{
		EXECADDR = CSADDR;
		
		if (buff[0] == 'H')
		{
			for (int i = 2; i < 8; i++)
			{
				CSNAME[i - 2] = buff[i];
			
				if (buff[i + 1] == ' ')
				{
					CSNAME[i - 1] = '\0';
					break;
				}
			}
			
			char temp[] = {buff[9], buff[10], buff[11], buff[12], buff[13], buff[14], '\0'};

			int csAddrInObj = hexToInt(temp); //always 000000

			for (int i = 0; i < 6; i++)
				temp[i] = buff[i + 16];

			CSLTH = hexToInt(temp);
		}

		fgets(buff, 100, objFile);

		while (buff[0] != 'E')
		{
			char *token = strtok(buff, "^");

			if (!strcmp(token, "T"))
			{
				token = strtok(NULL, "^");

				while (addr < CSADDR + hexToInt(token))
				{
					fputs(intToHex(addr), memFile);
					fputs(": XX\n", memFile);
					addr++;
				}

				addr = CSADDR + hexToInt(token);

				token = strtok(NULL, "^");

				textLen = hexToInt(token);

				while (1)
				{
					token = strtok(NULL, "^");

					if (!token)
						break;
					else
					{
						if (token[strlen(token) - 1] == '\n')
							token[strlen(token) - 1] = '\0';

						for (int i = 0; i < strlen(token); i = i + 2)
						{
							fputs(intToHex(addr), memFile);
							fputs(": ", memFile);

							char temp[] = {token[i], token[i + 1], '\n', '\0'};
							fputs(temp, memFile);

							addr++;
 						}
					}
				}
			}
			else if (!strcmp(token, "M"))
			{
				seek1 = ftell(memFile);
				
				token = strtok(NULL, "^");

				int modAddr = hexToInt(token) + CSADDR;

				token = strtok(NULL, "^");

				int bytes = ceil(hexToInt(token) / 2.0);

				token = strtok(NULL, "^");

				char sign = token[0];

				char *symbol = strdup(++token);
				
				symbol[strlen(symbol) - 1] = '\0';

				if (containsKey(ESTAB, ESTAB_SIZE, symbol))
				{
					rewind(memFile);

					while (!feof(memFile))
					{
						long seek = ftell(memFile);
						fgets(buff, 100, memFile);

						token = strtok(buff, ": ");

						if (!strcmp(token, intToHex(modAddr)))
						{
	 						char newAddr[bytes * 2];
	 						
	 						strcpy(newAddr, strtok(NULL, ": "));
	 						
	 						newAddr[2] = '\0';

							for (int i = 1; i < bytes; i++)
							{
								fgets(buff, 100, memFile);
								token = strtok(buff, ": ");
								token = strtok(NULL, ": ");
								token[2] = '\0';

								strcat(newAddr, token);
							}
							
							char *mod;
							
							if(sign == '+')
								mod = intToHex(hexToInt(newAddr) + hexToInt(getValue(ESTAB, ESTAB_SIZE, symbol)));
							else
								mod = intToHex(hexToInt(newAddr) - hexToInt(getValue(ESTAB, ESTAB_SIZE, symbol)));

							int len = strlen(mod);

							char temp[bytes * 2];
							strcpy(temp, "0");

							for (int i = bytes * 2; i > len; i--)
							{
								strcat(temp, mod);
								strcpy(mod, temp);
								strcpy(temp, "0");
							}

							fseek(memFile, seek, SEEK_SET);
							
							for (int i = 0; i < bytes * 2; i = i + 2, modAddr++)
							{
								char modStr[] = {mod[i], mod[i + 1], '\n', '\0'};
								fputs(intToHex(modAddr), memFile);
								fputs(": ", memFile);
								fputs(modStr, memFile);
							}
							
							break;
						}
					}
				}
				else
				{
					printf("\nUndefined external symbol \"%s\". Aborted.\n", symbol);
					fclose(objFile);
					fclose(memFile);
					remove("estab");
					exit(0);
				}
				
				fseek(memFile, seek1, SEEK_SET);
			}

			fgets(buff, 100, objFile);
		}

		while (addr <= CSADDR + CSLTH)
		{
			fputs(intToHex(addr), memFile);
			fputs(": XX\n", memFile);
			addr++;
		}

		strtok(buff, "^");

		if(!strtok(NULL, "^")) {
			char temp[] = {buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], '\0'};

			EXECADDR = CSADDR + hexToInt(temp);
		}

		CSADDR += CSLTH;

		fgets(buff, 100, objFile);

		printf("\n%s pass 2 completed.\n", CSNAME);
			
		printf("\nExecution of %s starts at %XH\n", CSNAME, EXECADDR);
	}

	fclose(objFile);
	fclose(memFile);
	remove("estab");
}

void initialise()
{
	FILE *fp = fopen("estab", "r");

	if (fp)
	{
		char buff[20];

		while (1)
		{
			fgets(buff, 20, fp);
			
			if (feof(fp))
				break;

			ESTAB[ESTAB_SIZE].key = strdup(strtok(buff, "\t"));

			char *temp = strtok(NULL, "\t");
			
			if (temp[strlen(temp) - 1] == '\n')
				temp[strlen(temp) - 1] = '\0';

			ESTAB[ESTAB_SIZE].value = strdup(temp);

			ESTAB_SIZE++;
		}

		fclose(fp);
	}
}

void writeESTAB()
{
	FILE *fp = fopen("estab", "w");

	for (int i = 0; i < ESTAB_SIZE; i++)
	{
		fputs(ESTAB[i].key, fp);
		fputs("\t", fp);
		fputs(ESTAB[i].value, fp);
		fputs("\n", fp);
	}

	fclose(fp);
}

void main()
{
	char *filename = malloc(20 * sizeof(char));

	printf("Enter filename of object program: ");
	fgets(filename, 20, stdin);

	for (int i = 0; i < 20; i++)
		if (filename[i] == '\n')
			filename[i] = '\0';

	initialise();

	displayObj(filename);

	pass1(filename);

	writeESTAB();

	displayEstabFile();

	pass2(filename);

	displayMem();
}
