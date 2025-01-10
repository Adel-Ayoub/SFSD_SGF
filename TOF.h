#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "gen.h"

void TOF_InitliazeFile(FILE *F,FILE *md, const char *filename, int Nrecords);
void TOF_InsertRecord(FILE* F, const char* filename, Record e,  AllocationTable* table);
coords TOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id);
void TOF_DeleteRecord(FILE* F,FILE* md,char* filename,AllocationTable *t, int id);
void TOF_Reorganize(FILE* F,FILE* md ,const char* nomfich);