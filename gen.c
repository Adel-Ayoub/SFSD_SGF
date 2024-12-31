#include <stdio.h>
#include "gen.h"

AllocationTable* initAllocationTable(){
    AllocationTable* p = (AllocationTable*)malloc(sizeof(AllocationTable));
    if (p == NULL) { // no memory left
        exit(1);
    }
    p->arrays[0] = 1;
    p->arrays[1] = 1;
    return p;
}

void setBlockStatus(AllocationTable* t,int blockNum, int status){
    t->arrays[blockNum] = status;
}
int getBlockStatus(AllocationTable *t, int blockNum){
    return t->arrays[blockNum];
}

int findFreeBlocks_sequential(AllocationTable *t, int nBlocks){ // will return the index of where you can put n blocks sequnetially
    int sum = 0; // X number of TOTAL blocks
    int i = 0;
    int rt_val = -1;
    while (i < X){
        if (t->arrays[i] == 1){
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

int* findFreeBlocks_list(AllocationTable *t, int nBlocks){ // will return an array of the free blocks avaiable


return NULL;
}

void traverse_defBlocks(FILE *ms){
    fseek(ms, sizeof(AllocationTable) + sizeof(MetadataBlock), SEEK_SET);
}
int search_metadata(const char* file_name, FILE *ms){
    // go to block 2
    int pos = 0;
    MetadataBlock buffer;
    fseek(ms, sizeof(AllocationTable), SEEK_SET);
    fread(&buffer, sizeof(MetadataBlock), 1 , ms);
        for (int i = 0; i < buffer.num_files; ++i) {
            if (buffer.table[i].filename == file_name){
                return pos;
            }
            pos++;
        }
    return -1;
}
int read_metadata(int pos, int id, FILE *ms){
    MetadataBlock buffer;

    fseek(ms, sizeof(AllocationTable), SEEK_SET);
    fread(&buffer, sizeof(MetadataBlock), 1 , ms);
    Metadata p = buffer.table[pos];

    switch (id) {
        case 1: // first block
            return p.Firstblock;
        case 2:  // number of blocks
            return p.nBlocks;
        case 3:
            return p.nRecords; // number of records
        case 4:
            return p.global_organs; // glocal organization (LIST / array)
        case 5:
            return p.inter_organs; // sorted or not sorted
        default:
            return -1;
    }

}
void write_metadata(int pos, int id , FILE *ms, int new_value){
    MetadataBlock buffer;

    fseek(ms, sizeof(AllocationTable), SEEK_SET);
    fread(&buffer, sizeof(MetadataBlock), 1 , ms);
    Metadata p = buffer.table[pos];

    switch (id) {
        case 1: // first block
            p.Firstblock = new_value;
        case 2:  // number of blocks
            p.nBlocks = new_value;
        case 3:
            p.nRecords = new_value;
            // number of records
        case 4:
            p.global_organs = new_value;
// glocal organization (LIST / array)
        case 5:
            p.inter_organs = new_value;

            // sorted or not sorted
        default:
            printf(" "); //...
    }
    buffer.table[pos] = p;
    fseek(ms, sizeof(MetadataBlock) * -1, SEEK_CUR);
    fwrite(&buffer, sizeof(MetadataBlock), 1, ms);
}

int main() {
    printf("Hello, World!\n");
    return 0;
}
