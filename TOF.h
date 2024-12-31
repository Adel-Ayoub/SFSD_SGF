#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef struct record
{
    int Id; // cle de tri
    int deleted; 
} Record, *RecordP;

typedef struct BLock
{
    Record tab[__INT16_MAX__];
    int next;
    int nbrecord;
} Block, *BlockP;


typedef BlockP Buffer;


typedef struct Metadata 
{
    int Firstblock;
    char filename[20];
    int nBlocks;
    int global_organs;
    int inter_organs;
    int nRecords;
    char *del;	
} Metadata, *MetadataP;

typedef struct {
    Metadata* table;
    int num_files;
} MetadataTable;

typedef struct  {
    int array[__INT16_MAX__]; // 0 means block is free 1 means blocks is occupaied
} AllocationTable;


int LireEntete(FILE* F, int param,char* filename);
void MAJEntete(FILE* F, int param, int val);

void LireBloc(FILE* F, int i, Block* buffer, MetadataTable* metadataTable);
void EcrireBloc(FILE* F, int i, Block* buffer, MetadataTable* metadataTable);




bool TOF_SearchRecord(FILE* F,char* filename ,int c, int* i, int* j, MetadataTable* metadataTable);
void TOF_InsertRecord(FILE* F, const char* nomfich, Record e, MetadataTable* metadataTable, AllocationTable* table);
void SuppressionLogiqueTOF(FILE* F, const char* nomfich, int c, MetadataTable* metadataTable);
void SuppressionPhysiqueTOF(FILE* F, const char* nomfich, int c,MetadataTable* metadataTable);
void TOF_DeleteRecord(FILE* f,char* filename, int id, int deleteType,MetadataTable* metadataTable);