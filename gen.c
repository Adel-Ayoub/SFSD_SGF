#include <stdio.h>
#include "gen.h"
#include "string.h"
#include "stdbool.h"

// -------- Allocation table

AllocationTable* initAllocationTable(){
    AllocationTable* p = (AllocationTable*)malloc(sizeof(AllocationTable));
    if (!p) {
        printf("Failed to allocate memory for AllocationTable\n");
        exit(1);
    }
    for (int i = 1; i < num_of_blocks; i++) {
        p->arrays[i] = 0; // Mark all blocks as free
    }
    p->arrays[0] = 1;
    p->num_files = 0;
    return p;
}


void setBlockStatus(FILE* F,int blockNum, int status,AllocationTable* t){
	rewind(F);
    fread(t, sizeof(AllocationTable), 1, F);
    t->arrays[blockNum]=status;
    fseek(F, 0 , SEEK_SET);
    fwrite(t, sizeof(AllocationTable), 1, F);
}
int getBlockStatus(AllocationTable *t, int blockNum){
    return t->arrays[blockNum];
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
        //printf("%d ", t->arrays[i]);
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


    // Check if we have enough free blocks

    // Allocate array for storing free block indices
    int* freeBlocks = (int*)malloc(nBlocks * sizeof(int));
    if (freeBlocks == NULL) {
    	printf("couldnt initialize freeblocks");
        return NULL;
    }

    // Fill array with free block indices
    int foundBlocks = 0;
    for (int i = 1; i < num_of_blocks && foundBlocks < nBlocks; i++) {
        if (t->arrays[i] == 0) {  // If block is free
            freeBlocks[foundBlocks] = i;
            foundBlocks++;
        }
    }
	if(foundBlocks==0){
		printf("no block were found");
		return NULL;
	}
	printf("number of found blocks %d\n",foundBlocks);
    return freeBlocks;
}

void traverse_defBlocks(FILE *ms){
    fseek(ms, sizeof(AllocationTable) , SEEK_SET);
}


// -- BLOCKS


void ReadBlock(FILE* F, int i, Block* buffer) {
    // Simulate reading the block `i` from the file
    fseek(F, sizeof(AllocationTable) + (i) * sizeof(Block), SEEK_SET); // Positionnement
    fread(buffer, sizeof(Block), 1, F);
}


void WriteBlock(FILE* F,int i, Block* buffer) {
    // Calculate the offset of the block in the file
   int blockOffset = sizeof(AllocationTable) + (i) * sizeof(Block);
	rewind(F);
    // Move the file pointer to the block's position
    fseek(F, blockOffset, SEEK_SET);

    // Write the block data from the buffer to the file
    fwrite(buffer, sizeof(Block), 1, F);
}

void WriteAllocationTable(AllocationTable *t , FILE *ms){
    rewind(ms);
    fwrite(t, sizeof(AllocationTable), 1, ms);
}



// ----- METADATA

int search_metadata(const char* file_name, FILE *md) {
    // Go to block 2
    int pos = 0;
    Metadata buffer;
    printf("\n searching for metadata");
    rewind(md);

    // Keep reading metadata records until EOF
    while (fread(&buffer, sizeof(Metadata), 1, md) == 1) {
        printf("%s \n", buffer.filename);
        printf("\n %s", file_name);
        printf("\n is it the same?");
        if (strcmp(buffer.filename, file_name) == 0) {
        	printf("file founded in md!!\n");
            return pos;
        }
        pos++;
    }
    return -1;  // Not found
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
    fseek(metadata_file, sizeof(Metadata) * pos, SEEK_SET);
    fwrite(&buffer, sizeof(Metadata), 1, metadata_file);
}
void printALLOCATION(AllocationTable* t){
    for (int i = 0; i < num_of_blocks; ++i) {
        if (t->arrays[i] == 1){
            printf("block %d -> occupiaed " , i+1);
        } else{
            printf("block %d -> free " , i+1);
        }

    }
}

void printMetadata(FILE* MD, const char* filename) {
    int pos = search_metadata(filename, MD);
    if (pos != -1) {
        printf("\nMetadata for file: %s\n", filename);
        printf("First block: %d\n", read_metadata(pos, 1, MD));
        printf("Number of blocks: %d\n", read_metadata(pos, 2, MD));
        printf("Number of records: %d\n", read_metadata(pos, 3, MD));
        printf("Global organization: %d\n", read_metadata(pos, 4, MD));
        printf("Internal organization: %d\n", read_metadata(pos, 5, MD));
    } else{
        printf("METADATA NOT FOUND");
    }
}

void printBlockContents(FILE* F, int blockNum) {
    Block buffer;
    ReadBlock(F, blockNum, &buffer);

    printf("\nBlock %d contents:\n", blockNum-1);
    printf("Number of records in block: %d\n", buffer.nbrecord);
    for (int i = 0; i < blocking_fact; i++) {
        printf("Record %d: ID=%d, Deleted=%d\n",
               i, buffer.tab[i].Id, buffer.tab[i].deleted);
    };
}
void ReadStorageFile(FILE* F) {
    if (F == NULL) {
        printf("Storage file not open.\n");
        return;
    }

    fseek(F, sizeof(AllocationTable), SEEK_SET); // Move to the beginning of the file

    Block buffer;
    int blockIndex = 1;

    printf("Reading storage file block by block...\n");

    while (fread(&buffer, sizeof(Block), 1, F) == 1) {
        printf("Block %d:\n", blockIndex);
        printf("\tNumber of Records: %d\n", buffer.nbrecord);

        for (int i = 0; i < buffer.nbrecord; i++) {
            Record r = buffer.tab[i];
            printf("\tRecord %d: ", i + 1);
            printf("Deleted: %d", r.deleted);
            // Add more fields to print if necessary
            printf("\n");
        }

        blockIndex++;
    }

    printf("End of storage file.\n");
}
