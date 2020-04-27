/*
* Copyright (c) 2020 Jonathan D. Delaney, and  
* The University of North Alabama.
* Dr. Roden CS 421
* Last updated: 4/26/2020
*/

/* 
* scanner.c will be in charge of scanning a file for
* the tokens that are present in the simplified Pascal
* programming language. The associated token numbers will 
* also be tracked. The goal of the scanner is to scan the
* file line by line, printing a line number and the line
* contents as you go. This will be followed by the token 
* types and specifiers associated with the tokens found 
* in the line. Tokens will be found by scanning the line until
* a space, then what type of token(s) are before that space.
* When a new line is detected, another line will be read in.
* This program will use a struct to hold the contents and index
* of a line, a struct to store a token type and specifer, and a
* array of structs to hold the identifiers(symbols) and hashnumbers.
* The symbol table will be printed at the end.
*/

/*
* Test files input
* test0.txt -	test embedded comments in a line
* test1.txt	-	test single line comments with and without spaces 
* test2.txt	-	test multi line comment mode
* test3.txt	-	test case insensitivity
* test4.txt -	test minimal whitespace
* test5.txt	-	test lots of whitespace
* test6.txt -	test invalid symbols 
* test7.txt	-	space after ( and before )
* test8.txt	-	test invalid symbols and invalid identifiers
* test9.txt - 	other test invalid symbols
* test10.txt -  token too long
* test11.txt - 	invalid identifiers
* test12.txt -  too many identifiers
* test13.txt -	test empty lines
* test14.txt -  exactly 32 identifiers
*/

#define PROGRAM		1
#define VAR			2
#define BEGIN 		3
#define END			4
#define ENDDOT		5
#define INTEGER		6
#define FOR			7
#define READ		8
#define WRITE  		9
#define TO  		10
#define DO  		11
#define SEMIC  		12
#define COLON  		13
#define COMMA  		14
#define COLONEQ  	15
#define ADD			16
#define SUBT	    17
#define MULT		18
#define DIV			19
#define LPAREN		20
#define RPAREN		21
#define ID			22
#define INT			23 

#define MAX 81 
#define IDLIMIT 32
#define TOKLENGTH 12
#define TOKAMMOUNT 23

#include <stdio.h> 
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

struct Token 
{
	int tokenType;
	char tokenSpecifier[TOKLENGTH];
};

struct Line
{
	int index;
	char strRead[MAX];
	bool newLine;	
};

typedef struct Identifier
{
	int key;
	char id[TOKLENGTH];	
} IDENTIFIER;

const char KEY[24][10] =
{
	"","PROGRAM", "VAR", "BEGIN", "END", "ENDDOT", 
	"INTEGER", "FOR", "READ", "WRITE", "TO", "DO",
	"SEMIC", "COLON", "COMMA", "COLONEQ", "ADD",
	"SUBT", "MULT", "DIV", "LPAREN", "RPAREN", 
	"ID", "INT"
};

void addToSym(IDENTIFIER *ident_p, struct Token *tok_p);
void checkForToken(char *tokString_p, struct Token *tok);
bool checkWhiteSpace(char c);
void closeFile(FILE **file_p);
void commentMode(FILE **file_p, struct Line *lineRead_p, int *lineCount_p);
struct Token getatoken(FILE **file_p, struct Token tok, struct Line *lineRead_p, int *lineCount_p);
int hash(struct Token tok);
bool isNumber(char *tkString_p);
void isSpecial(FILE **file_p, struct Token *tok, struct Line *lineRead_p, int *lineCount_p);
void openFile(FILE **file_p); 
void printIdentifiers(IDENTIFIER *ident_p);
void printToken(struct Token tok);
void readLine(FILE **file_p, struct Line *lineRead_p, int *lineCount_p);
bool searchSymTab(IDENTIFIER *ident_p, struct Token tok);
bool special(struct Line lineRead);
void symbolPush(struct Line *lineRead_p, struct Token *tok_p, char *tokName_p);

int main(int argc, char *argv[])
{	
	int 		lineC = 1;
	struct 		Token token;
	struct 		Line lineR;
	IDENTIFIER 	identifier[IDLIMIT];	
	bool 		beforeBegin = true;
	bool 		end = false;
	
	memset(&identifier, 0, sizeof identifier);		
	lineR . newLine = true;

	FILE *file;
	openFile(&file);
		 
	while(!end)
	{		
		token = getatoken(&file, token, &lineR, &lineC); 
		printToken(token);

		if(token . tokenType == ENDDOT)
		{
			end = true;
		}
		if(token . tokenType == BEGIN)
		{
			beforeBegin = false;
		}
		if(token . tokenType == ID)
		{
			if(beforeBegin)
			{	
				addToSym(identifier, &token);						
			}
			else if(!beforeBegin)
			{
				if(!searchSymTab(identifier, token))
				{
					printf("%s%s%s","Symbol ", token.tokenSpecifier,
						   " was not found in the symbol table.");
				}	
			}
		}
		
		memset(token . tokenSpecifier, 0, sizeof(token . tokenSpecifier));
		token . tokenType = 0;		
	}
	
	printIdentifiers(identifier);			
	closeFile(&file);

return 0;	
}

/*
* addToSym() takes in a struct IDENTIFIER pointer and a struct Token pointer.
* This function does not return anything. It's purpose is to call searchSymTab()
* to see if a symbol is already in the table, if it is the function will end.
* If the symbol is not in the symbol table it will add it at the hashNumber index.
* If that spot is taken it will increment until it finds a spot in the table.
* If all spots are taken then the symbol table is full and the program will
* exit gracefully.
/*--------------------------------------------------------------------*/
void addToSym(IDENTIFIER *ident_p, struct Token *tok_p)
{
	int hashNumber;
	int i = 0;
	bool added = false;
	hashNumber = hash(*tok_p);

	if(!searchSymTab(ident_p, *tok_p))
	{
		while(!added)
		{
			if(strlen(ident_p[hashNumber].id) == 0)
			{	
				strcpy(ident_p[hashNumber] . id, tok_p -> tokenSpecifier);		
				ident_p[hashNumber] . key = hashNumber;
				added = true;
			}
			/* go back to 0 if 31 is hit and nothing in ident_p*/			
			if(hashNumber == (IDLIMIT - 1) && strlen(ident_p[hashNumber] . id) != 0)
			{
				hashNumber = 0;
				i++;
			}
			/* case for too many symbols*/
			if(i == (IDLIMIT + 1) && strlen(ident_p[hashNumber] . id) != 0)
			{
				printf("\n\tThere are too many symbols for the symbol table.");
				exit(-1);
			}
			hashNumber++;
		}		
	}			
}

/*
* checkForToken() takes in a char array pointer, and a struct Token. 
* This function will check the char array that is passed
* against the KEY global array. If there is a match then we know that the
* char array token is a token. If it's not then we know that it is either
* the ID token or the INT token. We set our struct Token tokenType equal 
* to where it was found in the KEY, then the tokenSpecifier equal to 
* the char array that was passed.
*/
/*--------------------------------------------------------------------*/
void checkForToken(char *tokString_p, struct Token *tok_p)  
{
	int flag = 1;
	int i = 0;
	
	/* token in key */
	for(i; i < TOKAMMOUNT; i++)
	{
		if(strcmp(tokString_p, KEY[i]) == 0)
		{		
			flag = 0;			
			tok_p -> tokenType = i;					
			strcpy(tok_p -> tokenSpecifier, tokString_p);							
		}
	}	
	
	/* identifier or int */	
	if(flag)
	{
		if(isNumber(tokString_p))
		{
			tok_p -> tokenType = INT;	
			strcpy(tok_p -> tokenSpecifier, tokString_p);
		}
		else
		{
			tok_p -> tokenType = ID;	
			strcpy(tok_p ->tokenSpecifier, tokString_p);
		}
	}				
}

/*
* checkWhiteSpace takes in a character. This character will be checked
* for a space, tab, or new line and return true if one of those three
* is found.
*/
/*--------------------------------------------------------------------*/
bool checkWhiteSpace(char c)
{	 
	if(c == ' ' || c == '\t' || c == '\n') 
	{
		return true;
	}
}

/*
* closeFile() will take in a file pointer, attempt to close the file,
* and let the user know whether or not it was successful.
* No returns
*/
/*--------------------------------------------------------------------*/
void closeFile(FILE **file_p)
{
	fclose(*file_p);
	printf("file closed successfully \n");
}

/*
* commentMode() will take in a file pointer, a struct Line pointer, and
* a int pointer and will not return anything. This function will be 
* called when there is a (* detected in a line, then ignore all characters
* untill the *) is found. If the *) is not detected on the same line,
* readLine() will be called to grab the next line. This function will end
* once *) is found.
/*--------------------------------------------------------------------*/
void commentMode(FILE **file_p, struct Line *lineRead_p, int *lineCount_p)
{
	bool commentMode = true;
	lineRead_p -> index++;
	int count = 0;
	
	while(commentMode)
	{
		/* end comment mode if *) is found */
		if(lineRead_p -> strRead[lineRead_p -> index] == '*' &&
		   lineRead_p -> strRead[lineRead_p -> index + 1] == ')')
		{
			commentMode = false;
		}
		
		/* if new line we need to read another line */
		if(lineRead_p -> strRead[lineRead_p -> index] == '\n' && commentMode)
		{
			readLine(*&file_p, lineRead_p, *&lineCount_p);
		}
		
		lineRead_p -> index++;		
	}
	lineRead_p -> index++;	
}

/*
* getatoken() will take in a file pointer, a Token struct, a Line struct
* pointer, and integer pointer. This function will return a Token. getatoken()
* will call readLine then traverse a line stored in the Line struct character
* by character determining whether it is alpabetical, numeric, or a symbol.
* The function will return a token after checkForToken() is called. The struct
* Line will keep track of the index of the line in -> index, and whether or not
* it's time to read a new line with the bool -> newLine.
*/
/*--------------------------------------------------------------------*/
struct Token getatoken(FILE **file_p, struct Token tok, struct Line *lineRead_p, int *lineCount_p)
{
	char tokenData[TOKLENGTH];
	int  k = 0, count = 0;
	bool done = false;
	
	memset(tokenData, 0, sizeof tokenData);
	
	if(lineRead_p -> newLine)
	{
		readLine(*&file_p, lineRead_p, *&lineCount_p);
		while(strlen(lineRead_p -> strRead) == 1)
		{
			readLine(*&file_p, lineRead_p, *&lineCount_p);
		}
	}
	
	/* if index > 0 we know line is not finished so increment*/
	if(lineRead_p -> index > 0)
	{		
		lineRead_p -> index++;
	}

	while(!done)
	{			
		while(!checkWhiteSpace(lineRead_p -> strRead[lineRead_p -> index]) && !done) 
		{	
			/* alphabet */		
			if(isalpha(lineRead_p -> strRead[lineRead_p -> index]))
			{
				tokenData[k] = toupper(lineRead_p -> strRead[lineRead_p -> index]);
				k++;
				lineRead_p -> index++;
			}
			/* numeric */
			else if(isdigit(lineRead_p -> strRead[lineRead_p -> index]))
			{
				tokenData[k] = lineRead_p -> strRead[lineRead_p -> index];
				k++;
				lineRead_p -> index++;
			}
			/* special */
			else if(special(*lineRead_p)) 
			{
				if(strlen(tokenData) != 0 && !done)
				{	
					if(lineRead_p -> strRead[lineRead_p -> index] != '.')
					{
						checkForToken(tokenData, &tok);
						memset(tokenData, 0, sizeof(tokenData));
						lineRead_p -> index--;
						done = true;
					}
					else
					{
						memset(tokenData, 0, sizeof(tokenData));
						lineRead_p -> index--;
						done = true;
					}
				}
				else if(strlen(tokenData) == 0 && !done)
				{
					isSpecial(*&file_p, &tok, *&lineRead_p, *&lineCount_p);
					done = true;
				}
			}				
		}
				
		if(strlen(tokenData) != 0 && !done)
		{
			checkForToken(tokenData, &tok);
			memset(tokenData, 0, sizeof(tokenData));
			done = true;
		}
		
		/* if newline we need to clean up memory */				
		if(lineRead_p -> strRead[lineRead_p -> index] == '\n')
		{
			memset(lineRead_p -> strRead, 0, sizeof(lineRead_p -> strRead));
			lineRead_p -> index = 0;
			lineRead_p -> newLine = true;
		}
		
		/* get through white space patches */				
		if(checkWhiteSpace(lineRead_p -> strRead[lineRead_p -> index]) && !done)
		{
			lineRead_p -> index++;
		}
		
	}	
	return tok; 
}

/*
* hash() will take in a Token struct and return an integer. 
* This function will calculate the hashNumber where an identifier
* will be stored. It does this by converting the tok . tokenSpecifier's
* character to integers and adding them all together. If the string 
* length of the tokenSpecifier is longer than the Global TOKLENGTH
* then the token length limit has been exceeded.
/*--------------------------------------------------------------------*/
int hash(struct Token tok)
{
	int i;
	int sum;
	
	if(strlen(tok . tokenSpecifier) < TOKLENGTH)
	{
		for(i = 0; i < strlen(tok . tokenSpecifier); i++)
		{
			sum += (int)(tok . tokenSpecifier[i]);
		}
	}
	else /* if not in if statment then tokenlength too long */
	{
		printf("\n\tThe token length limit has been exceeded.\n");
	}
	
	sum = sum % IDLIMIT;
	return sum;	
}

/*
* isNumber() takens in a char string pointer and returns a bool.
* isNumber() will check the char string that is passed for digits, and
* if there all digits will return true, if not will return false
*/
/*--------------------------------------------------------------------*/
bool isNumber(char *tokString_p)
{
	int i;
	
	for(i=0; i < strlen(tokString_p); i++)
	{
		if(isdigit(tokString_p[i]) == false)
		{
			return false;
		}
	}
	return true;
}

/*
* isSpecial() will take in a file pointer, a Token struct, a Line struct
* pointer and a integer pointer. This function will lookes at the char in
* the lineRead_p -> strRead[lineRead_p -> index] to determine what case
* to go into. Some cases call symbolPush for the sake of shortening the
* isSpecial() function. The ones that don't have special cases to handle,
* like ( handling comment mode if * is the next character, and : being 
* colonEq if = is the next char. This function does not return anything.
/*--------------------------------------------------------------------*/
void isSpecial(FILE **file_p, struct Token *tok_p, struct Line *lineRead_p, int *lineCount_p)
{
	switch(lineRead_p -> strRead[lineRead_p -> index])
	{
		case ';': ;
			char semiToken[] = "SEMIC";
			symbolPush(lineRead_p, tok_p, semiToken);					
			break;
		case ':':
			if(lineRead_p -> strRead[lineRead_p -> index + 1] == '=')
			{
				char assignToken[] = "COLONEQ";
				checkForToken(assignToken, tok_p);
				lineRead_p -> index++;
			}
			else
			{
				char colonToken[] = "COLON";
				checkForToken(colonToken, tok_p);
			}
			break;
		case ',': ;
			char commaToken[] = "COMMA";
			symbolPush(lineRead_p, tok_p, commaToken);					
			break;
		case '+': ;
			char addToken[] = "ADD";
			symbolPush(lineRead_p, tok_p, addToken);
			break;
		case '-': ;
			char subtToken[] = "SUBT";
			symbolPush(lineRead_p, tok_p, subtToken);
			break;
		case '*': ;
			char multTok[] = "MULT";
			symbolPush(lineRead_p, tok_p, multTok);
			break;
		case '(':
			if(lineRead_p -> strRead[lineRead_p -> index + 1] == '*')
				commentMode(*&file_p, *&lineRead_p, *&lineCount_p);
			else
			{
				char leftPToken[] = "LPAREN";
				checkForToken(leftPToken, tok_p);
			}
			break;
		case ')': ;
			char rightPToken[] = "RPAREN";
			symbolPush(lineRead_p, tok_p, rightPToken);
			break;
		case '.': ;
			char endToken[] = "ENDDOT";
			checkForToken(endToken, tok_p);
			break;
		default:
			printf("\n\t%c ", lineRead_p -> strRead[lineRead_p -> index]);
			printf("Is not a valid symbol\n");
			break;
	}
}

/*
* openFile will take in a file pointer, attempt to open the file,
* and let the user know whether or not it was successful.
* No returns
*/
/*--------------------------------------------------------------------*/
void openFile(FILE **file_p)
{
	char fileName[TOKLENGTH];
	
	printf("Enter the filename to be opened \n");
	scanf("%s", fileName);
	
	*file_p = fopen(fileName, "r");
	
	if(!*file_p)
	{
		printf("oops, file can't be read \n");
		exit(-1); 
	}
}

/*
* printIdentifiers() will take in a struct IDENTIFIER pointer and does
* not return anything. This function prints the identifiers stored in 
* the array of structures.
/*--------------------------------------------------------------------*/
void printIdentifiers(IDENTIFIER *ident_p)
{
	int i;
	
	printf("\n*******************SYMBOL TABLE*******************");
	for(i = 0; i < IDLIMIT; i++)
	{
		printf("\n%d ", ident_p[i] . key);
		printf("%s", ident_p[i] . id);
	}
	printf("\n");
}

/*
* printToken() will take in a struct Token and does not return anything.
* This function will print the tokenType and tokenSpecifier stored in
* a token.
/*--------------------------------------------------------------------*/
void printToken(struct Token tok)
{
	if(strlen(tok.tokenSpecifier) != 0)
	{
		printf("\n\t%d", tok.tokenType);
		printf("\t%s\n\n", tok.tokenSpecifier);
	}
}

/*
* readLine() takes in a file pointer, a struct Line pointer and a
* integer pointer. This function will read a line from the file then
* print the line number and the line contents stored in the Line struct.
* newLine is set to false since we just read a line and have to traverse
* it in getatoken(). This function does not return anything.
/*--------------------------------------------------------------------*/
void readLine(FILE **file_p, struct Line *lineRead_p, int *lineCount_p)
{	
	int count;
	
	fgets(lineRead_p -> strRead, MAX, *file_p);
	count = *lineCount_p;
	lineRead_p -> index = 0;
	printf("****************************************************\n");
	printf("%d %s\n", count, lineRead_p -> strRead);
	printf("****************************************************\n");
	count++;
	*lineCount_p = count;
	lineRead_p -> newLine = false;
}

/*
* searchSymTab takes in a struct IDENTIFIER token and a struct Token.
* This function returns true or false depending on whether a tok .
* tokenSpecifier has been found if the symbol table.
/*--------------------------------------------------------------------*/
bool searchSymTab(IDENTIFIER *ident_p, struct Token tok)
{
	int i;	

	for(i = 0; i < IDLIMIT; i++)
	{
		if(strcmp(tok . tokenSpecifier, ident_p[i] . id) == 0)
		{
			return true;
		}
	}
	
	return false;
}

/*
* special() takes in a struct Line and returns true or false depending
* on whether a symbol is found.
/*--------------------------------------------------------------------*/
bool special(struct Line lineRead)
{
	int c = lineRead . strRead[lineRead . index];
	
	if(c == ';' || c == ':' || c == ',' || c == '+' || 
	   c == '-' || c == '*' || c == '(' || c == '.' || c == '<' || 
	   c == '>' || c == '?' || c == '/' || c == '"' || c == ']' ||
	   c == '[' || c == '{' || c == '}' || c == '=' || c == '_' || 
	   c == '&' || c == '^' || c == '%' || c == '$' || c == '#' ||
	   c == '@' || c == '!')
	{
		return true;
	}

}

/*
* symbolPush() takes in a Line struct pointer, a struct Token pointer,
* and a char array pointer. This function calls checkForToken to push
* what is currently in the tokName_p then increments the index in
* lineRead_p if the next character is a new line. This function does not
* return anything.
/*--------------------------------------------------------------------*/
void symbolPush(struct Line *lineRead_p, struct Token *tok_p, char *tokName_p)
{
	checkForToken(tokName_p, tok_p);

	if(lineRead_p -> strRead[lineRead_p -> index + 1] == '\n')
	{
		lineRead_p -> index++;
	}
}
