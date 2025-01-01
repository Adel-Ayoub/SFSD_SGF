#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "gen.h"

void TOF_InitliazeFile(FILE *F, int Nrecords);
void TOF_InsertRecord(FILE* F, const char* filename, Record e,  AllocationTable* table);
bool TOF_SearchRecord(FILE* F,char* filename ,int c, int* i, int* j);
void SuppressionLogiqueTOF(FILE* F, const char* filename, int c);
void SuppressionPhysiqueTOF(FILE* F, const char* filename, int c);
void TOF_DeleteRecord(FILE* F,char* filename, int id, int deleteType);
void TOF_Reorganize(char* filename); // Physcal deletion

//