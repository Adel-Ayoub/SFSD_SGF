#ifndef LOF_MODEL_H
#define LOF_MODEL_H

#include <stdio.h>
#include <stdlib.h>

typedef struct record
{
    int Id; // cle de tri
    int deleted; // supression logique
} Record, *RecordP;

//Declaration du bloc
typedef struct BLock
{
    Record tab[__INT16_MAX__];
    int next;
    int nbrecord;
} Block, *BlockP;

// I/O
typedef BlockP Buffer;

//Declaration des metadonnees (caracteristiques)
typedef struct Metadata 
{
    int Firstblock; // First block adresse
    char filename[20];
    int LastBlock; // Last block adresse
    int nBlocks; // number of blocks in the file
    int nRecords;// number of records in the file
    char *del;	// logical erase indicators ('*' erased / ' ' not erased)
} Metadata, *MetadataP;

typedef struct {
    Metadata* table;
    int num_files;
} MetadataTable;

typedef struct  {
    int array[__INT16_MAX__]; // 0 means block is free 1 means blocks is occupaied
} AllocationTable;

// exemple of how the physical should be in the case of one LOF file in it (Logical file)


////=================================== Primitives functions

void CreateFile(char* filename, int organization);  // Create new file
void DeleteFile(char* filename);                    // Delete entire file
void RenameFile(char* oldname, char* newname);     // Rename file
// LOF
void LOF_InsertRecord(char* filename, Record* record);
void LUOF_InsertRecord(char* filename, Record* record);

int LOF_SearchRecord(char* filename, int id);
int LUOF_SearchRecord(char* filename, int id);

void LOF_DeleteRecord(char* filename, int id, int deleteType);
void LUOF_DeleteRecord(char* filename, int id, int deleteType);

void LOF_Reorganize(char* filename);
void LUOF_Reorganize(char* filename);

// TOF
void TOF_InsertRecord(char* filename, Record* record);
void TUOF_InsertRecord(char* filename, Record* record);

int TOF_SearchRecord(char* filename, int id);
int TUOF_SearchRecord(char* filename, int id);

void TOF_DeleteRecord(char* filename, int id, int deleteType);
void TUOF_DeleteRecord(char* filename, int id, int deleteType);

void TOF_Compact(char* filename); // removes gaps mn logical deletions
void TUOF_Compact(char* filename); // removes gaps mn logical deletions

// Allocation Table Operations
void initAllocationTable(AllocationTable *t);  // Initialize table
void setBlockStatus(AllocationTable *t, int blockNum, int status); // Set block status
int getBlockStatus(AllocationTable *t, int blockNum);  // Get block status
int findFreeBlocks(AllocationTable *t, int nBlocks);   // Find n consecutive free blocks
// bach after to decide wether to call elimnate fragementation function or not
void displayAllocationTable(AllocationTable *t);       // Display table status

////=================================== Block 
void displayBlock(BlockP R); 

////=================================== Record
void printRecord(RecordP R); // afficher 
void CopyRecord(RecordP R1, RecordP R2); //copier
void createRecord(RecordP R); // lire les informations 



#endif