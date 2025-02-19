#include <stdio.h>
#include "gen.h"
#include "string.h"
#include "stdbool.h"

// -------- Allocation table
int num_of_blocks = 0;    // Default number of blocks
int blocking_fact = 0;     // Default blocking factor
AllocationTable* initAllocationTable() {
    AllocationTable* p = (AllocationTable*)malloc(sizeof(AllocationTable));
    if (!p) {
        printf("Failed to allocate memory for AllocationTable\n");
        exit(1);
    }

    p->arrays = (int*)malloc(num_of_blocks * sizeof(int));
    if (!p->arrays) {
        free(p);
        printf("Failed to allocate memory for arrays\n");
        exit(1);
    }

    for (int i = 1; i < num_of_blocks; i++) {
        p->arrays[i] = 0; // Mark all blocks as free
    }
    p->arrays[0] = 1;
    p->num_files = 0;
    return p;
}


void setBlockStatus(FILE* F,int blockNum, int status){
	rewind(F);
	AllocationTable* t;
	fread(t, sizeof(AllocationTable), 1, F);
	t->arrays[blockNum]=status;

}
void setBlockStatus_W(FILE* F,int blockNum, int status, AllocationTable* t){
    rewind(F);
    fread(t, sizeof(AllocationTable), 1, F);
    t->arrays[blockNum]=status;
    fseek(F, 0 , SEEK_SET);
    fwrite(t, sizeof(AllocationTable), 1, F);

}
int getBlockStatus(AllocationTable *t, int blockNum){
    return t->arrays[blockNum];
}

void WriteAllocationTable(AllocationTable *t , FILE *ms){
    rewind(ms);
    fwrite(t, sizeof(AllocationTable), 1, ms);
}
void ReadAllocationTable(AllocationTable *t , FILE *ms){
    rewind(ms);
    fread(t, sizeof(AllocationTable), 1, ms);
}

int findFreeBlocks_sequential(AllocationTable *t, int nBlocks){ // will return the index of where you can put n blocks sequnetially
    int sum = 0; // X number of TOTAL blocks
    int i = 0;
    int rt_val = -1;
    while (i < num_of_blocks){
        printf("%d ", t->arrays[i]);
        if (t->arrays[i] == 0){
            sum++;
            if(sum >= nBlocks){
                rt_val = i - sum + 1;
                break;
            }
        } else{
            sum = 0;
        }
        i++;
    }
    return rt_val;
}

int* findFreeBlocks_list(AllocationTable *t, int nBlocks) {
    // Validate input
    if (nBlocks <= 0 || t == NULL) {
        return NULL;
    }

    // Count total free blocks first to avoid unnecessary allocation!!!!
    int freeCount = 0;
    for (int i = 1; i < num_of_blocks; i++) {  // nbdaw from 1 since blocks 0 are system(Allocation) blocks
        if (t->arrays[i] == 1) {  // If block is free
            freeCount++;
        }
    }

    // Check if we have enough free blocks
    if (freeCount < nBlocks) {
        return NULL;
    }

    // Allocate array for storing free block indices
    int* freeBlocks = (int*)malloc(nBlocks * sizeof(int));
    if (freeBlocks == NULL) {
        return NULL;
    }

    // Fill array with free block indices
    int foundBlocks = 0;
    for (int i = 1; i < num_of_blocks && foundBlocks < nBlocks; i++) {
        if (t->arrays[i] == 1) {  // If block is free
            freeBlocks[foundBlocks] = i;
            foundBlocks++;
        }
    }

    return freeBlocks;
}

void traverse_defBlocks(FILE *ms){
    fseek(ms, sizeof(AllocationTable) , SEEK_SET);
}


// --------------------------------------- BLOCKS


void ReadBlock(FILE* F, int i, Block* buffer) {
    // Allocate memory for the record array
    buffer->tab = (Record*)malloc(blocking_fact * sizeof(Record));
    if (!buffer->tab) {
        printf("Failed to allocate memory for block records\n");
        exit(1);
    }

    // Simulate reading the block `i` from the file
    fseek(F, sizeof(AllocationTable) + (i - 1) * sizeof(Block), SEEK_SET); // Positionnement
    fread(buffer, sizeof(Block), 1, F);
}


void WriteBlock(FILE* F, int i, Block* buffer) {
    // Calculate the offset of the block in the file
    int blockOffset = sizeof(AllocationTable) + (i - 1) * sizeof(Block);

    // Move the file pointer to the block's position
    fseek(F, blockOffset, SEEK_SET);

    // Write the block data from the buffer to the file
    fwrite(buffer, sizeof(Block), 1, F);
}





// ----- METADATA

int search_metadata(const char* file_name, FILE *md) {
    // Go to block 2
    int pos = 0;
    Metadata buffer;
    rewind(md);

    // Keep reading metadata records until EOF
    while (fread(&buffer, sizeof(Metadata), 1, md) == 1) {
        if (strcmp(buffer.filename, file_name) == 0) {
            return pos;
        }
        pos++;
    }
    return -1;  // Not found
}


void Readmeta_FULL(FILE* MD, Metadata* p, int pos) {
    // Note: Changed from &p to p since we already have a pointer
    fseek(MD, sizeof(Metadata) * pos, SEEK_SET);
    fread(p, sizeof(Metadata), 1, MD);
}
int read_metadata(int pos, int id, FILE *meta_dat) {
    Metadata buffer;

    // Seek to the correct metadata record
    fseek(meta_dat, sizeof(Metadata) * pos, SEEK_SET);
    if (fread(&buffer, sizeof(Metadata), 1, meta_dat) != 1) {
        return -1;  // Error reading
    }
    switch (id) {
        case 1: return buffer.Firstblock;
        case 2: return buffer.nBlocks;
        case 3: return buffer.nRecords;
        case 4: return buffer.global_organs;
        case 5: return buffer.inter_organs;
        default: return -1;
    }
}

void write_metadata(int pos, int id , FILE *metadata_file, int new_value){
    Metadata buffer;

    fseek(metadata_file, sizeof(Metadata) * pos, SEEK_SET);
    fread(&buffer, sizeof(Metadata), 1 , metadata_file);

    switch (id) {
        case 1: // first block
            buffer.Firstblock = new_value;
        case 2:  // number of blocks
            buffer.nBlocks = new_value;
        case 3:
            buffer.nRecords = new_value;
            // number of records
        case 4:
            buffer.global_organs = new_value;
// glocal organization (LIST / array)
        case 5:
            buffer.inter_organs = new_value;

            // sorted or not sorted
        default:
            printf(" "); //...
    }
    fseek(metadata_file, sizeof(Metadata) * -1, SEEK_CUR);
    fwrite(&buffer, sizeof(Metadata), 1, metadata_file);
}

void RenameFile(char* oldname, char* newname, FILE* md){
    int pos = search_metadata(oldname, md);
    Metadata* p;
    Readmeta_FULL(md, p,pos);
    strcpy(p->filename,newname);
    fseek(md, sizeof(Metadata) * pos, SEEK_SET);
    fwrite(p, sizeof(Metadata), 1, md);

}
// FILES-----
