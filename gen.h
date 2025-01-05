//
// Created by xd on 31/12/2024
//

#ifndef SRC_GEN_H
#define SRC_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"
// Constants
#define num_of_blocks 20    // Total number of blocks
#define blocking_fact 4     // Blocking factor (number of records per block)

// Record Structure
typedef struct record {
    int Id;       // Key for sorting
    int deleted;  // Logical deletion flag (1 -> deleted, 0 -> not deleted)
} Record, *RecordP;

// Block Structure
typedef struct BLock {
    Record tab[blocking_fact];  // Array of records in the block
    int next;                   // Address of the next block
    int nbrecord;               // Number of records currently stored in the block
} Block, *BlockP;

// I/O Buffer
typedef BlockP Buffer;

// Metadata Structure
typedef struct Metadata {
    int Firstblock;       // Address of the first block
    char filename[20];    // File name
    int nBlocks;          // Number of blocks in the file
    int global_organs;    // 1 -> LIST, 0 -> SEQUENTIAL
    int inter_organs;     // 1 -> Ordered, 0 -> Unordered
    int nRecords;         // Total number of records in the file
} Metadata;

typedef struct {
    int x_block;
    int y_record;
    bool found;
} coords;

// Metadata Block (to manage multiple files)
typedef struct {
    Metadata table[num_of_blocks - 1];  // Metadata for files
    int num_files;                     // Number of files stored
} MetadataFile;

// Allocation Table Structure
typedef struct {
    int arrays[num_of_blocks];  // 0 -> Block is free, 1 -> Block is occupied
    int num_files;
} AllocationTable;

// Primitive Functions
// =====================

// File Operations
void CreateFile(char* filename, int organization);  // Create a new file
void DeleteFile(char* filename);                    // Delete a file
void RenameFile(char* oldname, char* newname);      // Rename a file

// Metadata Operations
int search_metadata(const char* file_name, FILE *ms);       // Search for metadata by file name
int read_metadata(int pos, int id, FILE *ms);               // Read metadata at a specific position
void write_metadata(int pos, int id, FILE *ms, int new_value); // Write metadata value

// Helper Functions
void traverse_defBlocks(FILE *ms); // Traverse and display default blocks

// Allocation Table Operations
AllocationTable* initAllocationTable();                    // Initialize the allocation table
void setBlockStatus(FILE* F,int blockNum, int status); // Set block status (free/occupied)
int getBlockStatus(AllocationTable *t, int blockNum);       // Get block status
int findFreeBlocks_sequential(AllocationTable *t, int nBlocks);   // Find n consecutive free blocks
int* findFreeBlocks_list(AllocationTable *t, int nBlocks);  // Find n free blocks (non-sequential)
void displayAllocationTable(AllocationTable *t);           // Display allocation table status

// Block Operations
void displayBlock(BlockP R);  // Display a block's contents
void ReadBlock(FILE* ms, int i, Block* buffer);
void WriteBlock(FILE* F, int i, Block* buffer);

// Record Operations
void printRecord(RecordP R);      // Display a record
void CopyRecord(RecordP R1, RecordP R2); // Copy a record
void createRecord(RecordP R);     // Create a record (read information)

#endif // SRC_GEN_H
