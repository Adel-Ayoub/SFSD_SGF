#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "gen.h"

void TOF_InitliazeFile(FILE *F, int Nrecords);
void TOF_InsertRecord(FILE* F, const char* filename, Record e, MetadataBlock* metadataTable, AllocationTable* table);
bool TOF_SearchRecord(FILE* F,char* filename ,int c, int* i, int* j, MetadataBlock* metadataTable);
void SuppressionLogiqueTOF(FILE* F, const char* filename, int c, MetadataBlock* metadataTable);
void SuppressionPhysiqueTOF(FILE* F, const char* filename, int c,MetadataBlock* metadataTable);
void TOF_DeleteRecord(FILE* F,char* filename, int id, int deleteType,MetadataBlock* metadataTable);
void LOF_Reorganize(char* filename); // Physcal deletion
