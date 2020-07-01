#define _CRT_SECURE_NO_WARNINGS
#define TEXT_FILE_1 "plaintext"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

/*FUNCTION Declaration*/
void RemoveSpaces(char* source)
{
	char* i = source;
	char* j = source;
	while (*j != 0)
	{
		*i = *j++;
		if (*i != ' ')
			i++;
	}
	*i = '\0';
}
void strip(char* str)
{
	int from, to;
	for (from = 0, to = 0; str[from] != '\0'; ++from) {
		if (isascii(str[from])) {
			if ((!isdigit(str[from])) && (!ispunct(str[from])) )
			{
				str[to] = toupper(str[from]);
				++to;
			}
		}
	}
	str[to] = '\0';
}
void stripNewline(char* str)
{
	int from, to;
	for (from = 0, to = 0; str[from] != '\0'; ++from)
	{
		if (str[from] != '\n')
		{
			str[to] = str[from];
			++to;
		}
	}
	str[to] = '\0';
}
char* spaceFive(char* str, size_t file_size) //Error in here
{
	int size = 0;
	for (size = 0; str[size] != 0; ++size) {

	}
	size = size / 5;
	file_size += size;

	char *buff = (char*)malloc(file_size + 1);
	int from, to, counter = 0;

	for (from = 0, to = 0; str[from] != '\0'; ++from)
	{
			//add a space every 5 chars e.g. 01234 56789 ....
			if (counter == 5)
			{
				buff[to] = ' ';
				++to;
				counter = 0;
			}
		buff[to] = str[from];
		++to;
		counter++;
	}
	buff[to] = '\0';
	return buff;
}

/*
FOR OPENCL
move this function into a kernel argument 
will need global and private data
*/
int FileEncrypt(char* input, int userVal)
{
	/*
	ASCII uppercase:A = 65, Z = 90
	char character = input[i]; // This gives the character
	int ascii = (int) character; // This give the ASCII CODE
	*/
	int ascii = 0;
	for (int i = 0; input[i] != '\0'; i++)
	{
		char character = input[i];
		ascii = (int)character;
		if (ascii != 32)
		{
			ascii += userVal;
			if (ascii > 90) {
				ascii -= 25;
			}
		}
		input[i] = ascii;
	}
	//Return zero for success
	return 0; 
}
int FileDecrypt(char* input, int userVal)
{
	//invert File Encrypt function 
	/*
	ASCII uppercase:A = 65, Z = 90
	char character = input[i]; // This gives the character
	int ascii = (int) character; // This give the ASCII CODE
	*/
	int ascii = 0;
	for (int i = 0; input[i] != '\0'; i++)
	{
		char character = input[i];
		ascii = (int)character;
		if (ascii != 32)
		{
			ascii -= userVal;
			if (ascii < 65) {
				ascii += 25;
			}
		}
		input[i] = ascii;
	}
	//Return zero for success
	return 0;
}

void menu(char* spaced) 
{
	/*User Value Variables*/
	int userVal = 0;
	int Version = 0;
	/*Boolean Variables*/
	//Zero = false, One >= TRUE
	int attemptEncrypt = 0, attemptDecrypt = 0;

	printf("Program Versions avaliable to Run:\n Version 1: Single Integer Encryption\n Version 2: Up to 4 Integer Encryption\n Version 3: Lookup table Encryption\n");
	//Prompt user to enter a vaild Version
	for (; Version == 0;)
	{
		printf("Please enter a Version:");
		scanf("%d", &Version);
		//consumes any characters not read by scanf
		while (getchar() != '\n');
		//printf("Number Entered:%d", userVal);
	}
	switch (Version)
	{
	case 1: //Task b
			  //Prompt user for a value
		for (; userVal == 0;)
		{
			printf("Please enter a Value:");
			scanf("%d", &userVal);
			//consumes any characters not read by scanf
			while (getchar() != '\n');
		}

		//encrypt File
		attemptEncrypt = FileEncrypt(spaced, userVal);
		if (attemptEncrypt == 1)
		{
			perror("There was a failure in file Encryption");
			exit(1);
		}

		//Display encrypted File
		printf("Encrypted File:");
		printf("%s\n", spaced);

		//Decrypt and Display
		attemptDecrypt = FileDecrypt(spaced, userVal);
		if (attemptDecrypt == 1)
		{
			perror("There was a failure in file Decryption");
			exit(1);
		}
		printf("Decrypted File:");
		printf("%s\n", spaced);
		break;
	case 2: //Task c
		printf("Multiple Entry Encrytpion Method Will go Here\n");
		break;
	case 3: //Task d
		printf("Table Lookup Encryption Will go Here\n");
		break;
	default:
		printf("invalid version entered. Program will quit.\n");
		break;
	}
}


int main() {
	/*VARIABLE Declaration*/
	FILE *fptr;
	char *file_buffer, *spaced = NULL;
	//char input[20];
	size_t file_size;


	//printf("Begin Program\n");
	/*OPEN file for reading*/
	fptr = fopen(TEXT_FILE_1, "r");
	if (fptr == NULL)
	{
		printf("Cannot open file \n");
		exit(0);
	}
	//Read file into buffer
	fseek(fptr, 0, SEEK_END);
	file_size = ftell(fptr);
	rewind(fptr);
	file_buffer = (char*)malloc(file_size + 1); //remember to free
	file_buffer[file_size] = '\0';
	fread(file_buffer, sizeof(char), file_size, fptr);
	/*CLOSE file*/
	fclose(fptr);

	/* REMOVE spaces */
	RemoveSpaces(file_buffer);

	/* REMOVE non-letters & toUpper */
	strip(file_buffer);

	/*Strip Newline char's*/
	stripNewline(file_buffer);

	/*Add space every 5 char's*/
	spaced = spaceFive(file_buffer,file_size);

	//MENU WRAPPER
	menu(spaced);

	printf("Press ENTER to quit");
	getchar();

	//Deallocate Memory
	free(file_buffer);
	free(spaced);

	return 0;
}