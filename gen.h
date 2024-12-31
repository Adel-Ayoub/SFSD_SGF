//
// Created by xd on 31/12/2024.
//

#ifndef src_GEN_H
#define src_GEN_H


#include <stdio.h>
#include <stdlib.h>


#define num_of_blocks 20

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
    int nBlocks; // number of blocks in the file
    int global_organs; // 1 -> LIST 0 -> SEQUIENTAL
    int inter_organs; // 1 -> ordered 0-> unordered
    int nRecords;// number of records in the file
} Metadata;

typedef struct {
    Metadata table[num_of_blocks - 2];
    int num_files;
} MetadataBlock;

typedef struct  {
    int arrays[num_of_blocks]; // 0 means block is free 1 means blocks is occupaied
} AllocationTable;

// exemple of how the physical should be in the case of one LOF file in it (Logical file)


////=================================== Primitives functions

void CreateFile(char* filename, int organization);  // Create new file
void DeleteFile(char* filename);                    // Delete entire file
void RenameFile(char* oldname, char* newname);     // Rename file
// LOF








// Allocation Table Operations
AllocationTable* initAllocationTable();  // Initialize table
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



#endif //UNTITLED5_GEN_H
