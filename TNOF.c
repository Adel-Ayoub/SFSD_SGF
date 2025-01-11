//
// Created by xd on 5/1/2025.
//
#include "gen.h"
#include "TNOF.h"
#include "string.h"
#include "gen.h"
#include "TNOF.h"
#include "string.h"

// Initialize a TNOF file
void TNOF_InitliazeFile(FILE *F, FILE* MD, const char* filename, int Nrecords, AllocationTable *table) {

    ReadAllocationTable(table, F);
    printf("MD file pointer: %p\n", (void*)MD);

    int blocks_needed = (Nrecords + blocking_fact - 1) / blocking_fact;
    printf("--------- init\n ");
    printf("%d", blocks_needed);
    printf("--------- init\n ");

    int first_address = findFreeBlocks_sequential(table, blocks_needed);
    printf("first address %d", first_address);
    if (first_address == -1) {
        printf("Not enough blocks sorry");
        return; // Not enough continuous blocks available
    }

    // Create metadata entry
    Metadata p;
    strcpy(p.filename, filename);
    p.inter_organs = 0;
    p.global_organs = 0;
    p.nRecords = Nrecords;
    p.Firstblock = first_address;
    p.nBlocks = blocks_needed;
    fflush(stdout);

    printf("BLOCKS: %d\n", blocks_needed);
    printf("OTHER: %d\n", blocking_fact);

    printf("MD file pointer: %p\n", (void*)MD);
    printf("Main storage pointer: %p\n", (void*)F);
    printf("Table pointer: %p\n", (void*)table);
    printf("Table arrays pointer: %p\n", (void*)table->arrays);
    fflush(stdout);
    printf("Okay creating file \n");

    fseek(MD, 0, SEEK_END);
    fwrite(&p, sizeof(p), 1, MD);
    table->num_files++;
    Block buffer;
    buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
    fseek(F, sizeof(table) + (first_address * sizeof(buffer)), SEEK_SET);
    free(buffer.tab);
    int counter = 0;
    int i = 0;

    while (i < blocks_needed) {
        fflush(stdout);

        Block buffer;
        printf("YEAH");
        buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
        printf("\n YEAHHHH");

        if (!buffer.tab) {
            perror("Failed to allocate memory for block records");
            return;
        }
        buffer.nbrecord = 0;

        int j = 0;
        while (j < blocking_fact && Nrecords > 0) {
            Record student;
            student.deleted = 0;
            student.Id = counter;
            counter++;
            buffer.tab[j] = student;
            buffer.nbrecord++;
            j++;
            Nrecords--;
        }

        setBlockStatus_W(F, i + first_address, 1, table);
        fwrite(&buffer, sizeof(Block), 1, F);
        table->arrays[i + first_address] = 1;

        free(buffer.tab); // Free allocated memory
        i++;
    }
    WriteAllocationTable(table, F);
//    free(table); // Free the allocation table
}

// Insert a record into the TNOF file
void TNOF_InsertRecord(FILE* F, FILE* MD, const char* filename, Record e, AllocationTable* table) {
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

    Block buffer;
    buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
    if (!buffer.tab) {
        perror("Failed to allocate memory for block records");
        return;
    }

    // Calculate position of last block
    long blockPosition = sizeof(AllocationTable) + ((fb + nb - 2) * sizeof(Block));

    fseek(F, blockPosition, SEEK_SET);
    if (fread(&buffer, sizeof(Block), 1, F) != 1) {
        printf("Failed to read block\n");
        free(buffer.tab);
        return;
    }

    // If current block has space
    if (buffer.nbrecord < blocking_fact) {
        buffer.tab[buffer.nbrecord] = e;
        buffer.nbrecord++;

        fseek(F, blockPosition, SEEK_SET);
        if (fwrite(&buffer, sizeof(Block), 1, F) != 1) {
            printf("Failed to write block\n");
            free(buffer.tab);
            return;
        }

        // Update metadata
        fseek(MD, pos * sizeof(Metadata), SEEK_SET);
        Metadata meta;
        fread(&meta, sizeof(Metadata), 1, MD);
        meta.nRecords = nRECO + 1;
        fseek(MD, -sizeof(Metadata), SEEK_CUR);
        fwrite(&meta, sizeof(Metadata), 1, MD);
    } else {
        // Need new block
        if ((fb + nb) >= num_of_blocks) {
            printf("No more blocks available\n");
            free(buffer.tab);
            return;
        }

        Block newBlock;
        newBlock.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
        if (!newBlock.tab) {
            perror("Failed to allocate memory for block records");
            free(buffer.tab);
            return;
        }
        newBlock.tab[0] = e;
        newBlock.nbrecord = 1;

        // Write new block
        fseek(F, blockPosition + sizeof(Block), SEEK_SET);
        if (fwrite(&newBlock, sizeof(Block), 1, F) != 1) {
            printf("Failed to write new block\n");
            free(buffer.tab);
            free(newBlock.tab);
            return;
        }

        // Update allocation table
        table->arrays[fb + nb] = 1;
        WriteAllocationTable(table, F);

        // Update metadata
        fseek(MD, pos * sizeof(Metadata), SEEK_SET);
        Metadata meta;
        fread(&meta, sizeof(Metadata), 1, MD);
        meta.nRecords = nRECO + 1;
        meta.nBlocks = nb + 1;
        fseek(MD, -sizeof(Metadata), SEEK_CUR);
        fwrite(&meta, sizeof(Metadata), 1, MD);

        free(newBlock.tab); // Free allocated memory
    }

    free(buffer.tab); // Free allocated memory
}

// Search for a record in the TNOF file
coords TNOF_SearchRecord(FILE* F, FILE* MD, const char* filename, int id) {
    rewind(F);
    rewind(MD);
    fflush(stdout);
    printf("SEARCHI G");

    printf("ID %d", id);
    int pos = search_metadata(filename, MD);
    Block buffer;
    buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
    if (!buffer.tab) {
        perror("Failed to allocate memory for block records");
        coords t = {-1, -1, false};
        return t;
    }

    coords t;
    t.x_block = -1;
    t.y_record = -1;
    t.found = false;

    int fb = read_metadata(pos, 1, MD); // First address
    int nb = read_metadata(pos, 2, MD); // Number of blocks
    fseek(F, sizeof(buffer) * (fb - 1) + sizeof(AllocationTable), SEEK_SET);
    int j = 0;

    while (j < nb && !t.found) {
        fread(&buffer, sizeof(buffer), 1, F);
        for (int i = 0; i < buffer.nbrecord; ++i) {
            printf("ID %d", buffer.tab[i].Id);
            if (buffer.tab[i].Id == id) {
                t.found = true;
                t.y_record = i;
                t.x_block = j + fb;
                break;
            }
        }
        j++;
    }

    free(buffer.tab); // Free allocated memory
    return t;
}

// Logical deletion of a record
void SuppressionLogiqueTNOF(FILE* F, FILE* MD, const char* filename, int id) {
    coords p = TNOF_SearchRecord(F, MD, filename, id);
    if (p.found) {
        Block buffer;
        buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
        if (!buffer.tab) {
            perror("Failed to allocate memory for block records");
            return;
        }

        fseek(F, sizeof(AllocationTable) + sizeof(Block) * (p.x_block - 1), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, F);
        buffer.tab[p.y_record].deleted = 1;
        fseek(F, sizeof(Block) * -1, SEEK_CUR);
        fwrite(&buffer, sizeof(Block), 1, F);

        free(buffer.tab); // Free allocated memory
    }
}

// Print metadata for a file
void printMetadata(FILE* MD, const char* filename) {
    int pos = search_metadata(filename, MD);
    if (pos != -1) {
        printf("\nMetadata for file: %s\n", filename);
        printf("First block: %d\n", read_metadata(pos, 1, MD));
        printf("Number of blocks: %d\n", read_metadata(pos, 2, MD));
        printf("Number of records: %d\n", read_metadata(pos, 3, MD));
        printf("Global organization: %d\n", read_metadata(pos, 4, MD));
        printf("Internal organization: %d\n", read_metadata(pos, 5, MD));
    } else {
        printf("METADATA NOT FOUND");
    }
}

// Reorganize the TNOF file
void TNOF_Reorganize(char* filename, FILE *ms, FILE *MD) {
    Block buffer;
    buffer.tab = (Record*)malloc(blocking_fact * sizeof(Record)); // Allocate memory for records
    if (!buffer.tab) {
        perror("Failed to allocate memory for block records");
        return;
    }

    int pos = search_metadata(filename, MD);
    if (pos == -1) {
        free(buffer.tab);
        return;
    }

    // Read metadata
    Metadata meta;
    fseek(MD, pos * sizeof(Metadata), SEEK_SET);
    fread(&meta, sizeof(Metadata), 1, MD);

    int first_address = meta.Firstblock;
    int length = meta.nBlocks;
    int active_records = 0;

    // First pass: Count active records and compact within blocks
    for (int i = 0; i < length; i++) {
        fseek(ms, sizeof(AllocationTable) + (first_address + i) * sizeof(Block), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        int write_pos = 0;
        for (int j = 0; j < buffer.nbrecord; j++) {
            if (buffer.tab[j].deleted == 0) {
                if (write_pos != j) {
                    buffer.tab[write_pos] = buffer.tab[j];
                }
                write_pos++;
                active_records++;
            }
        }
        buffer.nbrecord = write_pos;

        // Write back the compacted block
        fseek(ms, sizeof(AllocationTable) + (first_address + i) * sizeof(Block), SEEK_SET);
        fwrite(&buffer, sizeof(Block), 1, ms);
    }

    // Calculate new number of blocks needed
    int new_blocks = (active_records + blocking_fact - 1) / blocking_fact;
    if (new_blocks < length) {
        // Mark freed blocks as available
        AllocationTable table;
        ReadAllocationTable(&table, ms);
        for (int i = first_address + new_blocks; i < first_address + length; i++) {
            table.arrays[i] = 0;
        }
        WriteAllocationTable(&table, ms);
    }

    // Update metadata
    meta.nBlocks = new_blocks;
    meta.nRecords = active_records;
    fseek(MD, pos * sizeof(Metadata), SEEK_SET);
    fwrite(&meta, sizeof(Metadata), 1, MD);

    free(buffer.tab); // Free allocated memory
}


void printALLOCATION(AllocationTable* t, FILE* F){
    rewind(F);
    fread(t, sizeof(AllocationTable), 1,F);
    for (int i = 0; i < num_of_blocks; ++i) {
        if (t->arrays[i] == 1){
            printf("block %d -> occupiaed /" , i);
        } else{
            printf("block %d -> free /" , i);
        }

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
            printf("ID=: %d", r.Id);

            // Add more fields to print if necessary
            printf("\n");
        }

        blockIndex++;
    }

    printf("End of storage file.\n");
}
//int main() {
//    printf("==== File System Simulation ====\n");
//
//    // Open or create the main storage and metadata files
//    FILE* mainStorage = fopen("storage.bin", "wb+");
//    FILE* metadataFile = fopen("metadata.bin", "wb+");
//    if (!mainStorage || !metadataFile) {
//        printf("Error: Could not open storage or metadata file.\n");
//        return 1;
//    }
//
//    // 1. Initialize allocation table
//    printf("\n[Step 1] Initializing Allocation Table...\n");
//    AllocationTable* table = initAllocationTable();
//    fwrite(table, sizeof(AllocationTable), 1, mainStorage);
//    printALLOCATION(table, mainStorage);
//
//    printf("Allocation table initialized.\n");
//    getchar(); // Pause
//
//    // 2. Create a new TNOF file with 10 records
//    printf("\n[Step 2] Creating TNOF File...\n");
//    TNOF_InitliazeFile(mainStorage, metadataFile, "students.dat", 5);
//    printf("Metadata and storage initialized for 'students.dat'.\n");
//    readMetadatax(metadataFile);
//    ReadStorageFile(mainStorage);
//    printALLOCATION(table, mainStorage);
//
//    getchar(); // Pause
//
//    // 3. Insert records
//    printf("\n[Step 3] Inserting Records...\n");
//    Record testRecords[5] = {
//            {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}
//    };
//    for (int i = 0; i < 5; i++) {
//        TNOF_InsertRecord(mainStorage, metadataFile, "students.dat", testRecords[i], table);
//    }
//    printf("Records inserted successfully.\n");
//    ReadStorageFile(mainStorage);
//    printALLOCATION(table, mainStorage);
//    readMetadatax(metadataFile);
//
//    getchar(); // Pause
//
//    // 4. Search for a record
//    printf("\n[Step 4] Searching for Record...\n");
//    coords searchResult = TNOF_SearchRecord(mainStorage, metadataFile, "students.dat", 12);
//    if (searchResult.found) {
//        printf("Record ID=12 found in Block=%d, Position=%d.\n",
//               searchResult.x_block, searchResult.y_record);
//        printBlockContents(mainStorage, searchResult.x_block);
//    } else {
//        printf("Record ID=12 not found.\n");
//    }
//    getchar(); // Pause
//
//    // 5. Logical deletion
//    printf("\n[Step 5] Deleting Record...\n");
//    SuppressionLogiqueTNOF(mainStorage, metadataFile, "students.dat", 4);
//
//    SuppressionLogiqueTNOF(mainStorage, metadataFile, "students.dat", 12);
//    printf("Record ID=12 marked as deleted.\n");
//    ReadStorageFile(mainStorage);
//    getchar(); // Pause
//
//    // Verify deletion
//    searchResult = TNOF_SearchRecord(mainStorage, metadataFile, "students.dat", 12);
//    if (searchResult.found) {
//        Block buffer;
//        ReadBlock(mainStorage, searchResult.x_block, &buffer);
//        printf("Deleted Status of Record ID=12: %d\n",
//               buffer.tab[searchResult.y_record].deleted);
//    }
//    getchar(); // Pause
//
//    // 6. Reorganize file
//    readMetadatax(metadataFile);
//
//    printf("\n[Step 6] Reorganizing File...\n");
//
//    TNOF_Reorganize("students.dat", mainStorage, metadataFile);
//    printf("Reorganization complete.\n");
//
//    // Print final state
//    printf("\n[Final State] Blocks after Reorganization:\n");
//    int pos = search_metadata("students.dat", metadataFile);
//    int firstBlock = read_metadata(pos, 1, metadataFile);
//    int numBlocks = read_metadata(pos, 2, metadataFile);
//    for (int i = firstBlock; i < firstBlock + numBlocks; i++) {
//        printBlockContents(mainStorage, i);
//    }
//
//    printALLOCATION(table, mainStorage);
//    readMetadatax(metadataFile);
//
//    // Clean up
//    fclose(mainStorage);
//    fclose(metadataFile);
//    free(table);
//
//    printf("\n==== Test Completed ====\n");
//    return 0;
//}