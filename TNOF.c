//
// Created by xd on 5/1/2025.
//
#include "gen.h"
#include "TNOF.h"
#include "string.h"

void TNOF_InitliazeFile(FILE *F, FILE* MD,const char* filename,int Nrecords){
    AllocationTable* table = malloc(sizeof(AllocationTable));
    if (table == NULL) {
        perror("Memory allocation failed for AllocationTable");
        return;
    }
    ReadAllocationTable(table, F);
    int blocks_needed = (int)(Nrecords / blocking_fact) + 1;
    printf("--------- init\n ");
    printf("%d", blocks_needed);
    printf("--------- init\n ");

    int first_adresss = findFreeBlocks_sequential(table, blocks_needed);
    printf("first adress %d", first_adresss);
    if (first_adresss == -1) {
        printf("Not enough blocks sorry");
        return; // Not enough continuous blocks available
    }
    // Create metadata entry
    Metadata p;
    strcpy(p.filename, filename);
    p.inter_organs = 0;
    p.global_organs = 0;
    p.nRecords = Nrecords;
    p.Firstblock = first_adresss;
    p.nBlocks = blocks_needed;

    //
    printf("Okay creating file \n");
    printf("%s", p.filename);
    fseek(MD, 0, SEEK_END);
    fwrite(&p, sizeof(p),1 , MD);
    table->num_files++;


    fseek(F, sizeof(AllocationTable) + (first_adresss * sizeof(Block)), SEEK_SET);
    int counter = 0;
    int i = 0;

    while (i < blocks_needed){
        Block buffer;
        buffer.nbrecord = 0;

        int j = 0;
        while (j < blocking_fact  && 0 < Nrecords){
            Record student;
            student.deleted = 0;
            student.Id = counter;
            counter++;
            // assembling student takes place in the app!
            buffer.tab[j] = student;
            buffer.nbrecord++;
            j++;
            Nrecords--;
        }
        setBlockStatus(F, i + first_adresss, 1);
        fwrite(&buffer, sizeof(Block),1 , F);
        i++;
    }


}
void TNOF_InsertRecord(FILE* F, FILE* MD, const char* filename, Record e, AllocationTable* table) {
    // First verify all parameters are valid
    if (!F || !MD || !filename || !table) {
        printf("Invalid parameters passed to TNOF_InsertRecord\n");
        return;
    }

    // Search for metadata position
    int pos = search_metadata(filename, MD);
    if (pos == -1) {
        printf("Metadata for file '%s' not found.\n", filename);
        return;
    }

    // Read metadata values
    int fb = read_metadata(pos, 1, MD);
    int nb = read_metadata(pos, 2, MD);
    int nRECO = read_metadata(pos, 3, MD);

    // Verify metadata values
    if (fb < 0 || nb <= 0) {
        printf("Invalid metadata values: fb=%d, nb=%d\n", fb, nb);
        return;
    }

    // Create and initialize buffer
    Block buffer;

    // Calculate position of last block
    long blockPosition = sizeof(AllocationTable) + (fb + nb - 2) * sizeof(Block);
    printf("----------");
    printf("%d", blockPosition);
    printf("----------");

    // Seek to last block
    if (fseek(F, blockPosition, SEEK_SET) != 0) {
        printf("Failed to seek to block position\n");
        return;
    }

    // Read last block
    if (fread(&buffer, sizeof(Block), 1, F) != 1) {
        printf("Failed to read block\n");
        return;
    }

    // If current block has space
    if (buffer.nbrecord < blocking_fact) {
        // Add record to current block
        buffer.tab[buffer.nbrecord] = e;
        buffer.nbrecord++;

        // Seek back to write updated block
        if (fseek(F, -sizeof(Block), SEEK_CUR) != 0) {
            printf("Failed to seek back to block position\n");
            return;
        }

        // Write updated block
        if (fwrite(&buffer, sizeof(Block), 1, F) != 1) {
            printf("Failed to write block\n");
            return;
        }

        // Update record count in metadata
        write_metadata(pos, 3, MD, nRECO + 1);
    }
        // Need new block
    else {
        // Verify we have space for a new block
        if ((fb + nb) >= num_of_blocks) {
            printf("No more blocks available\n");
            return;
        }

        // Verify next block is free
        if (getBlockStatus(table, fb + nb) != 0) {
            printf("Next block is not free\n");
            return;
        }

        // Create new block
        Block newBlock = {0};
        newBlock.tab[0] = e;
        newBlock.nbrecord = 1;

        // Write new block
        if (fseek(F, blockPosition + sizeof(Block), SEEK_SET) != 0) {
            printf("Failed to seek to new block position\n");
            return;
        }

        if (fwrite(&newBlock, sizeof(Block), 1, F) != 1) {
            printf("Failed to write new block\n");
            return;
        }

        // Update allocation table
        setBlockStatus(F, fb + nb, 1);

        // Update metadata
        write_metadata(pos, 3, MD, nRECO + 1);
        write_metadata(pos, 2, MD, nb + 1);
    }
}
coords TNOF_SearchRecord(FILE* F, FILE* MD, const char* filename ,int id){
    int pos = search_metadata(filename,MD);
    Block buffer;
    coords t;
    t.x_block = -1;
    t.y_record = -1;
    t.found = false;
    int fb = read_metadata(pos, 1,MD); // first adresss
    int nb = read_metadata(pos, 2,MD); // number of blocks
    fseek(F, sizeof(Block) * (fb - 1) + sizeof(AllocationTable),SEEK_SET);
    int j = 0;

    while ( j < nb && !t.found){

        fread(&buffer, sizeof(Block), 1 , F);
        for (int i = 0; i < buffer.nbrecord; ++i) {
            if (buffer.tab[i].Id == id){
                t.found = true;
                t.y_record = i;
                t.x_block = j + fb;
                break;
            }
        }
        j++;
    }
    return t;


}

void SuppressionLogiqueTNOF(FILE* F, FILE* MD,const char* filename, int id){
    coords p = TNOF_SearchRecord(F,MD,filename, id);
    if (p.found) {

        Block buffer;
        fseek(F, sizeof(AllocationTable) + sizeof(Block) * (p.x_block - 1), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, F);
        buffer.tab[p.y_record].deleted = 1;
        fseek(F, sizeof(Block) * -1, SEEK_CUR);
        fwrite(&buffer, sizeof(Block), 1, F);
    }
}




void TNOF_Reorganize(char* filename, FILE *ms, FILE *MD) {
    Block buffer, lastBuffer;
    int pos = search_metadata(filename, MD);
    if (pos == -1) return;  // File not found

    int first_address = read_metadata(pos, 1, MD);
    int length = read_metadata(pos, 2, MD);
    int total_products = read_metadata(pos, 3, MD);

    if (length <= 0) return;

    int last_block_idx = first_address + length - 1;
    int current_block_idx = first_address;

    while (current_block_idx < last_block_idx) {
        // Read current block
        ReadBlock(ms, current_block_idx, &buffer);

        // Process each record in current block
        for (int j = 0; j < buffer.nbrecord; j++) {
            if (buffer.tab[j].deleted == 1) {
                // Find last valid record
                do {
                    ReadBlock(ms, last_block_idx, &lastBuffer);

                    while (lastBuffer.nbrecord > 0 &&
                           lastBuffer.tab[lastBuffer.nbrecord - 1].deleted == 1) {
                        lastBuffer.nbrecord--;
                        total_products--;
                    }

                    if (lastBuffer.nbrecord == 0) {
                        length--;
                        last_block_idx--;
                        setBlockStatus(ms, last_block_idx + 1, 1); // Free the block
                    }
                } while (lastBuffer.nbrecord == 0 && last_block_idx > current_block_idx);

                if (last_block_idx > current_block_idx) {
                    // Replace deleted record with last valid record
                    buffer.tab[j] = lastBuffer.tab[lastBuffer.nbrecord - 1];
                    lastBuffer.nbrecord--;
                    WriteBlock(ms, last_block_idx, &lastBuffer);
                }
            }
        }

        WriteBlock(ms, current_block_idx, &buffer);
        current_block_idx++;
    }

    // Update metadata
    write_metadata(pos, 2, MD, length);
    write_metadata(pos, 3, MD, total_products);
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

    printf("\nBlock %d contents:\n", blockNum);
    printf("Number of records in block: %d\n", buffer.nbrecord);
    for (int i = 0; i < buffer.nbrecord; i++) {
        printf("Record %d: ID=%d, Deleted=%d\n",
               i, buffer.tab[i].Id, buffer.tab[i].deleted);
    }
}


void readMetadatax(FILE *MD) {
    if (MD == NULL) {
        printf("Error: Metadata file is not open.\n");
        return;
    }

    fseek(MD, 0, SEEK_END);
    long fileSize = ftell(MD);
    rewind(MD);

    printf("Reading Metadata File:\n");
    int metadataIndex = 0;
    while (ftell(MD) < fileSize) {
        Metadata meta;
        fread(&meta, sizeof(Metadata), 1, MD);

        printf("Metadata %d:\n", metadataIndex++);
        printf("\tFilename: %s\n", meta.filename);
        printf("\tInternal Organs: %d\n", meta.inter_organs);
        printf("\tGlobal Organs: %d\n", meta.global_organs);
        printf("\tNumber of Records: %d\n", meta.nRecords);
        printf("\tFirst Block: %d\n", meta.Firstblock);
        printf("\tNumber of Blocks: %d\n", meta.nBlocks);
    }
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
int main() {
    // Open or create the main storage file and metadata file
    FILE* mainStorage = fopen("storage.bin", "wb+");
    FILE* metadataFile = fopen("metadata.bin", "wb+");

    if (!mainStorage || !metadataFile) {
        printf("Error opening files!\n");
        return 1;
    }

    // 1. Initialize allocation table
    printf("Initializing allocation table...\n");
    AllocationTable* table = initAllocationTable();
    fwrite(table, sizeof(AllocationTable), 1, mainStorage);
    printf("Press Enter to continue...");
    getchar();
    // 2. Create a new TNOF file with 10 records
    printf("\nCreating new TNOF file 'students.dat' with 10 records...\n");
    TNOF_InitliazeFile(mainStorage, metadataFile, "students.dat", 5);
    readMetadatax(metadataFile);
    ReadStorageFile(mainStorage);

    printf("Press Enter to continueXX...");
    getchar();
    printMetadata(metadataFile, "students.dat");
    printf("Press Enter to continue...");
    getchar();
    // 3. Insert some records
    printf("\nInserting test records...\n");
    Record testRecords[5] = {
            {12, 0},  // ID=1, not deleted
            {13, 0},  // ID=2, not deleted
            {14, 0},  // ID=3, not deleted
            {15, 0},  // ID=4, not deleted
            {16, 0}   // ID=5, not deleted
    };

    for (int i = 0; i < 5; i++) {
        printf("Inserting record with ID=%d\n", testRecords[i].Id);
        TNOF_InsertRecord(mainStorage, metadataFile, "students.dat", testRecords[i], table);
    }
    ReadStorageFile(mainStorage);
    printf("Press Enter to continue...");
    getchar();
    // 4. Search for a record
    printf("\nSearching for record with ID=12...\n");
    coords searchResult = TNOF_SearchRecord(mainStorage, metadataFile, "students.dat", 12);
    if (searchResult.found) {
        printf("Found record ID=12 in block %d, position %d\n",
               searchResult.x_block, searchResult.y_record);
        printBlockContents(mainStorage, searchResult.x_block);
    } else {
        printf("Record with ID=3 not found!\n");
    }
    printf("Searching finished now deletion...");
    getchar();
    // 5. Logical deletion
    printf("\nPerforming logical deletion of record with ID=3...\n");
    SuppressionLogiqueTNOF(mainStorage, metadataFile, "students.dat", 12);
    ReadStorageFile(mainStorage);

    printf("Press Enter to continue...");
    getchar();
    // Verify deletion
    searchResult = TNOF_SearchRecord(mainStorage, metadataFile, "students.dat", 12);
    if (searchResult.found) {
        printf("Checking deleted status...\n");
        Block buffer;
        ReadBlock(mainStorage, searchResult.x_block, &buffer);
        printf("Record ID=3 deleted status: %d\n",
               buffer.tab[searchResult.y_record].deleted);
    }
    printf("Press Enter to continue...");
    getchar();
    // 6. Reorganize file
    printf("\nReorganizing file...\n");
    TNOF_Reorganize("students.dat", mainStorage, metadataFile);

    // Print final state
    printf("\nFinal state after reorganization:\n");
    int pos = search_metadata("students.dat", metadataFile);
    int firstBlock = read_metadata(pos, 1, metadataFile);
    int numBlocks = read_metadata(pos, 2, metadataFile);

    for (int i = firstBlock; i < firstBlock + numBlocks; i++) {
        printBlockContents(mainStorage, i);
    }

    // Clean up
    fclose(mainStorage);
    fclose(metadataFile);
    free(table);

    printf("\nTest completed!\n");
    return 0;
}