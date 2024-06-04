/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	unsigned int offset;
};

struct RelocationTableEntry {
	unsigned int offset;
	char inst[7];
	char label[7];
	unsigned int file;
};

struct FileData {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	unsigned int textStartingLine; // in final executable
	unsigned int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	int text[MAXSIZE*MAXFILES];
	int data[MAXSIZE*MAXFILES];
	SymbolTableEntry symbolTable[MAXSIZE*MAXFILES];
	RelocationTableEntry relocTable[MAXSIZE*MAXFILES];
};

void globalLabelResolver(FileData *files, CombinedFiles *combinedFiles, char *label, int argc, int *fileIndex, int *newOffset) {
	for (int i = 0; i < argc - 2; i++) {
		for (int j = 0; j < files[i].symbolTableSize; j++) {
			if (!(strcmp(files[i].symbolTable[j].label, label)) && files[i].symbolTable[j].location != 'U') {
				*fileIndex = i;
				if (files[i].symbolTable[j].location == 'T') {
					*newOffset = (files[i].symbolTable[j].offset + files[i].textStartingLine);
				} else {
					*newOffset = (files[i].symbolTable[j].offset + files[i].dataStartingLine);
				}
				goto exit;
			} else if (!strcmp("Stack", label)) {
				*newOffset = (combinedFiles->textSize + combinedFiles->dataSize);
				goto exit;
			}
		}
	}
	exit(1);
exit:
	return;
}

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr; 
	unsigned int i, j;

	if (argc <= 2) {
		printf("error: usage: %s <obj file> ... <output-exe-file>\n",
				argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData files[MAXFILES];

  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; i++) {
		inFileString = argv[i+1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&textSize, &dataSize, &symbolTableSize, &relocationTableSize);

		files[i].textSize = textSize;
		files[i].dataSize = dataSize;
		files[i].symbolTableSize = symbolTableSize;
		files[i].relocationTableSize = relocationTableSize;

		// read in text section
		int instr;
		for (j = 0; j < textSize; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = strtol(line, NULL, 0);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < dataSize; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = strtol(line, NULL, 0);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		unsigned int addr;
		for (j = 0; j < symbolTableSize; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < relocationTableSize; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!

	
	CombinedFiles combinedFiles;

	for (int i = 0; i < argc - 2; i++) {
		combinedFiles.textSize += files[i].textSize;
		combinedFiles.dataSize += files[i].dataSize;
		combinedFiles.symbolTableSize += files[i].symbolTableSize;
		combinedFiles.relocationTableSize += files[i].relocationTableSize;
		
		/* for (int j = 0; j < files[i].textSize; j++) {
			combinedFiles.text[combTextIndex] = files[i].text[j];
			combTextIndex++;
		}
		for (int j = 0; j < files[i].dataSize; j++) {
			combinedFiles.data[combDataIndex] = files[i].data[j];
			combDataIndex++;
		}
		for (int j = 0; j < files[i].symbolTableSize; j++) {
			combinedFiles.symbolTable[combSymIndex] = files[i].symbolTable[j];
			combSymIndex++;
		}
		for (int j = 0; j < files[i].relocationTableSize; j++) {
			combinedFiles.relocTable[combRelocIndex] = files[i].relocTable[j];
			combRelocIndex++;
		} */
	}

	int calculatedTSLine = 0; // text starting line helper
	int calculatedDSLine = combinedFiles.textSize; // data starting line helper

	char globalLabelArray[500][7];
	int globalLabelArraySize = 0;

	for (int i = 0; i < argc - 2; i++)	{
		if (i > 0) {
			calculatedTSLine += files[i-1].textSize;
			calculatedDSLine += files[i-1].dataSize;
		}
		files[i].textStartingLine = calculatedTSLine;
		files[i].dataStartingLine = calculatedDSLine;

		for (int j = 0; j < files[i].symbolTableSize; j++) {
			if (files[i].symbolTable[j].location != 'U') {
				if (!strcmp("Stack", files[i].symbolTable[j].label)) {
					exit(1);
				}
				for (int k = 0; k < globalLabelArraySize; k++) {
					if (!strcmp(globalLabelArray[k], files[i].symbolTable[j].label)) {
						exit(1); // checking for duplicates and wrongful definition of the Stack label
					}
				}
				strcpy(globalLabelArray[globalLabelArraySize], files[i].symbolTable[j].label);
				globalLabelArraySize++;
			}
		}
	}

	//int curCombOffsetT = 0;
	//int curCombOffsetD = combinedFiles.textSize;

	for (int i = 0; i < argc - 2; i++) {
		int newOffset = 0;
		int fileIndex = 0;
		for (int j = 0; j < files[i].relocationTableSize; j++) {
			if (isupper((files[i].relocTable[j].label)[0])) {
				globalLabelResolver(files, &combinedFiles, files[i].relocTable[j].label, argc, &fileIndex, &newOffset);
				if (strcmp(files[i].relocTable[j].inst, ".fill")) {
					int oldOffset = files[i].text[files[i].relocTable[j].offset] & 0xFFFF;
					files[i].text[files[i].relocTable[j].offset] -= oldOffset;
					files[i].text[files[i].relocTable[j].offset] += newOffset;
				} else {
					int oldOffset = files[i].data[files[i].relocTable[j].offset] & 0xFFFF;
					files[i].data[files[i].relocTable[j].offset] -= oldOffset;
					files[i].data[files[i].relocTable[j].offset] += newOffset;

				}
			} else {
				if (strcmp(files[i].relocTable[j].inst, ".fill")) { // if lw, sw
					//check if the label was declared in text or data
					int privateOffset = files[i].text[files[i].relocTable[j].offset] & 0xFFFF;
					if (privateOffset < files[i].textSize) {
						files[i].text[files[i].relocTable[j].offset] += files[i].textStartingLine;
					}
					else {
						files[i].text[files[i].relocTable[j].offset] += (files[i].dataStartingLine - files[i].textSize);
					}
						//if declared in text, real offset = own offset + size of text for all files before this file
						//if declared in data, real offset = own offset + size of text for all files + size of data for all files before this file
				}
				else { // if .fill
					int privateOffset = files[i].data[files[i].relocTable[j].offset];
					if (privateOffset < files[i].textSize) {
						files[i].data[files[i].relocTable[j].offset] += files[i].textStartingLine;
					}
					else {
						files[i].data[files[i].relocTable[j].offset] += (files[i].dataStartingLine - files[i].textSize);
					}
					//check if the label was declared in text or data
						//if declared in text, real offset = own offset + size of text for all files before this file
						//if declared in data, real offset = own offset + size of text for all files + size of data for all files before this file
				}
			}
		}
	}

	int combTextIndex = 0;
	int combDataIndex = 0;
	int combSymIndex = 0;
	int combRelocIndex = 0;

	for (int i = 0; i < argc - 2; i++) {
		for (int j = 0; j < files[i].textSize; j++) {
			combinedFiles.text[combTextIndex] = files[i].text[j];
			combTextIndex++;
		}
		for (int j = 0; j < files[i].dataSize; j++) {
			combinedFiles.data[combDataIndex] = files[i].data[j];
			combDataIndex++;
		}
		for (int j = 0; j < files[i].symbolTableSize; j++) {
			combinedFiles.symbolTable[combSymIndex] = files[i].symbolTable[j];
			combSymIndex++;
		}
		for (int j = 0; j < files[i].relocationTableSize; j++) {
			combinedFiles.relocTable[combRelocIndex] = files[i].relocTable[j];
			combRelocIndex++;
		}
	}

	/*for (int i = 0; i < combinedFiles.textSize; i++) {
		printf("%d\n", combinedFiles.text[i]);
	}

	for (int i = 0; i < combinedFiles.dataSize; i++) {
		printf("%d\n", combinedFiles.data[i]);
	} */

	for (int i = 0; i < combinedFiles.textSize; i++) {
		fprintf(outFilePtr, "%d\n", combinedFiles.text[i]);
	}
	for (int i = 0; i < combinedFiles.dataSize; i++) {
		fprintf(outFilePtr, "%d\n", combinedFiles.data[i]);
	}
	
} // main
