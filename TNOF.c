//
// Created by xd on 5/1/2025.
//
#include "gen.h"
#include "TNOF.h"



void TNOF_InitliazeFile(FILE *F, FILE* MD,const char* filename,int Nrecords){
    AllocationTable table;
    ReadAllocationTable(&table, F);
    int blocks_needed = (int)(Nrecords / blocking_fact) + 1;
    int first_adresss = start_findFreeBlocks_sequential(table, blocks_needed);
    Metadata p;
    p.filename = filename;
    p.inter_organs = 0;
    p.global_organs = 0;
    p.nRecords = Nrecords;
    p.Firstblock = first_adresss;
    p.nBlocks = blocks_needed;
    Metadata Buffer_mt;
    fseek(MD, sizeof(Buffer_mt) * table.num_files, SEEK_SET);
    fwrite(&Buffer_mt, sizeof(Buffer_mt),1 , MD);
    fseek(F, sizeof(Block) * first_adresss, SEEK_SET);

    while (0 < blocks_needed){
        Block buffer;
        int j = 0;
        while (j < blocking_fact && 0 < Nrecords){
            record student;
            student.deleted = 0;
            // assembling student takes place in the app!
            buffer.tab[j] = student;
            j++;
            Nrecords--;
        }
        fwrite(&buffer, sizeof(Block),1 , F);
        blocks_needed--;
    }


}

void TNOF_InsertRecord(FILE* F, FILE* MD,const char* filename, Record e,  AllocationTable* table){
    int pos = search_metadata(filename,MD);
    Metadata p;
    Block buffer;
    int fb = read_metadata(pos, 1,MD);
    int nb = read_metadata(pos, 2,MD);
    int nRECO = read_metadata(pos, 3,MD);

    int avaiable = 0;
    fseek(F, sizeof(Block) * (fb + nb - 1), SEEK_SET);
    fread(&buffer, sizeof(Block), 1 , F);
    if (buffer.nbrecord < blocking_fact){
        buffer[buffer.nbrecord] = e;
        buffer.nbrecord++;
        fseek(F, sizeof(Block) * -1, SEEK_CUR);
        fwrite(&buffer, sizeof(Block), 1 , F);
        write_metadata(pos,3,MD,nRECO+1);
    } else{
        if (getBlockStatus(fb + nb) == 1){
            // reorganize
            if (getBlockStatus(fb + nb) == 1){
                // External frgementation
                if (getBlockStatus(fb + nb) == 1){
                    printf("Not enough space")
                }
            }
        }
        if (getBlockStatus(fb + nb) == 0){
            buffer.tab[0] = e;
            buffer.nbrecord++;
            fseek(F, sizeof(Block) * -1, SEEK_CUR);
            fwrite(&buffer, sizeof(Block), 1 , F);
            write_metadata(pos,3,MD,nRECO+1);
            write_metadata(pos,2,MD,nb+1);

        }
    }
}
coords TNOF_SearchRecord(FILE* F, FILE* MD, char* filename ,int id){
    int pos = search_metadata(filename,MD);
    Metadata p;
    Block buffer;
    coords t;
    t.x_block = -1;
    t.y_record = -1;
    t.found = false
    int fb = read_metadata(pos, 1,MD);
    int nb = read_metadata(pos, 2,MD);
    int nRecords = read_metadata(pos, 3,MD);
    fseek(F, sizeof(Block) * fb, SEEK_SET);
    int found = 0;
    int j = 0;

    while ( j < nb && t.found){

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
    Block buffer;
    fseek(F, sizeof(Block) * p.x_block, SEEK_SET);
    fread(&buffer, sizeof(Block), 1 , F);
    buffer.tab[p.y_record].deleted = 1;
    fseek(F, sizeof(Block) * -1, SEEK_SET);
    fwrite(&buffer, sizeof(Block), 1 , F);

}

void TNOF_Reorganize(char* filename){

}


void reorganiserBlocs(char* filename,FILE *ms, FILE *MD) {
    Block buffer, lastBuffer;
    int pos = search_metadata(filename, MD);
    int first_address = read_metadata(pos, 1, MD);
    int length = read_metadata(pos, 2, MD);
    int total_products = read_metadata(pos, 3, MD);

    if (length <= 0) return;

    for (int i = 0; i < length; i++) {
        // Read current block
        fseek(ms, sizeof(Block) * (first_address + i), SEEK_SET);
        fread(&buffer, sizeof(Block), 1, ms);

        // Check each product in current block
        for (int j = 0; j < buffer.nbrecord; j++) {
            if (buffer.tab[j].deleted == 1) { // If product is marked as deleted
                // Read the last block
                fseek(ms, sizeof(Block) * (first_address + length - 1), SEEK_SET);
                fread(&lastBuffer, sizeof(Block), 1, ms);

                // Find last valid product
                while (lastBuffer.nbrecord > 0 && lastBuffer.tab[lastBuffer.nbrecord - 1].deleted == 1) {
                    lastBuffer.nbrecord--;
                    total_products--;
                }

                if (lastBuffer.nbrecord > 0) {
                    // Replace deleted product with last valid product
                    buffer.tab[j] = lastBuffer.nbrecord[lastBuffer.numb - 1];
                    lastBuffer.nbrecord--;
                    total_products--;

                    // Write back last block
                    fseek(ms, sizeof(Block) * (first_address + length - 1), SEEK_SET);
                    fwrite(&lastBuffer, sizeof(Block), 1, ms);

                    // If last block is empty, decrease length
                    if (lastBuffer.nbrecord == 0) {
                        length--;
                        write_metadata()
                        MAJMeta(f, 3, length);
                    }
                }
            }
        }

        // Write back current block
        fseek(ms, sizeof(Block) * (first_address + i), SEEK_SET);
        fwrite(&buffer, sizeof(Block), 1, ms);
    }

    // Update metadata
    MAJMeta(f, 2, total_products);
}


