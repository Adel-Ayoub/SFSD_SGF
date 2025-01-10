#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "gen.h"




void trierTableau(Record tableau[], int taille) {
    for (int i = 0; i < taille - 1; i++) {
        int minIndex = i;
        
        for (int j = i + 1; j < taille; j++) {
            if (tableau[j].Id < tableau[minIndex].Id) {
                minIndex = j;
            }
        }
        
        if (minIndex != i) {
            int temp = tableau[i].Id;
            tableau[i].Id = tableau[minIndex].Id;
            tableau[minIndex].Id = temp;
        }
    }
}


void TOF_InitializeFile(FILE *F,FILE *md, const char *filename, int Nrecords){

    AllocationTable* table = malloc(sizeof(AllocationTable));
    if (table == NULL) {
        perror("Memory allocation failed for AllocationTable");
        return;
    }
    ReadAllocationTable(table, F);
    int blocks_needed = (Nrecords + blocking_fact - 1) / blocking_fact;
    int first_adresss =findFreeBlocks_sequential(table, blocks_needed); 
    Block buffer;
    int i;

    if (first_adresss==-1)
    {
        //defragmentation externe
        first_adresss = findFreeBlocks_sequential(table, blocks_needed);
        if (first_adresss==-1)
        {
            printf("Espace insufisant\n");
            return;
        }
        
    }

    Metadata p;
    strcpy(p.filename, filename);
    p.inter_organs = 0;
    p.global_organs = 0;
    p.nRecords = Nrecords;
    p.Firstblock = first_adresss;
    p.nBlocks = blocks_needed+first_adresss-1; 

    
    Record* tabOfID = (Record*)malloc(Nrecords * sizeof(Record));
    if (tabOfID == NULL) { // Vérifier si l'allocation a échoué
        printf("Erreur d'allocation mémoire.\n");
        return ;
    }
    
    // Remplir le tableau
    for (i = 0; i < Nrecords; i++) {
        printf("Entrez l'ID pour l'enregistrement n%d : ", i);
        scanf("%d", &(tabOfID[i].Id));
        (tabOfID+i)->deleted=0;
    }

    fseek(md,sizeof(Metadata)*table->num_files,SEEK_SET);
    fwrite(&p,sizeof(p),1,md);
    trierTableau(tabOfID,Nrecords);

    int k=0;
    i=0;
    while (i < blocks_needed){
        Block buffer;
        int j = 0;
        while (j < blocking_fact && 0 < Nrecords){
            buffer.tab[j] = tabOfID[k];
            j++;
            Nrecords--;
            k++;
        }
        WriteBlock(F,i+first_adresss,&buffer);
        setBlockStatus(table,i+first_adresss,1);
        i++;
    }
    free(tabOfID);
    free(table);
}




coords TOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id) {
    coords position;
    position.found = false;
    position.x_block=-1;
    position.y_record=-1;

    int pos_meta = search_metadata(filename, md);
    if (pos_meta==-1)
    {
        printf("Le fichier n'existe pas");
        return position;
    }
    
    int bi = read_metadata(pos_meta, 1, md);              // Borne inférieure : numéro du premier bloc
    int bs = read_metadata(pos_meta, 2, md) +bi -1; // Borne supérieure : numéro du dernier bloc
    
    bool stop = false;

    while (bi <= bs && !position.found && !stop) {
        position.x_block = (bi + bs) / 2; // Bloc du milieu
        Block buffer;
        ReadBlock(F, position.x_block, &buffer);

        // Vérifier si `c` est dans des clés du bloc 
        if (buffer.nbrecord > 0 && id >= buffer.tab[0].Id && id <= buffer.tab[buffer.nbrecord - 1].Id) {
            // RechercheDichotomique dans le bloc 
            int inf = 0, sup = buffer.nbrecord - 1;

            while (inf <= sup && !position.found) {
                position.y_record = (inf + sup) / 2;
                if (id == buffer.tab[position.y_record].Id && buffer.tab[position.y_record].deleted == 0) {
                    position.found = true; // id trouvée et non supprimée
                } else if (id < buffer.tab[position.y_record].Id) {
                    sup = position.y_record - 1;
                } else {
                    inf = position.y_record + 1;
                }
            }

            if (inf > sup) {
                position.y_record = inf; // Position où `id` devrait se trouver dans le bloc
            }
            stop = true; // Arrêter la RechercheDichotomique externe
        } else if (buffer.nbrecord == 0 || id < buffer.tab[0].Id) {
            bs = position.x_block - 1;
        } else { // id > buffer.tab[buffer.nbrecord - 1].Id
            bi = position.x_block + 1;
        }
    }

    if (bi > bs) {
        position.x_block = bi;
        position.y_record = 0;
    }


    return position;
}


void TOF_InsertRecord(FILE* F, FILE* md,const char* filename, Record e,  AllocationTable* table) {
    coords trouv;


    trouv=TOF_SearchRecord(F, md,filename,e.Id);
    // RechercheDichotomique la position où insérer le nouvel enregistrement
    if (trouv.x_block==-1)
    {
        return;
    }

    if (trouv.found) {
        printf("Record with ID %d already exists\n", e.Id);
        return;
    }
    
    bool continu = true;
    int pos_m = search_metadata(filename, md);
    if (!trouv.found) {
        Block buffer;

        
        int fb = read_metadata(pos_m, 1,md);
        int nb = read_metadata(pos_m, 2,md);
        int last_block = fb + nb - 1;

       
        while (continu && trouv.x_block <= read_metadata(pos_m, 2,md)+read_metadata(pos_m,1,md)-1) {
            ReadBlock(F, trouv.x_block, &buffer);

            if (buffer.nbrecord == blocking_fact ){
                if (getBlockStatus(table,fb + nb) == 1)
                {
                    TOF_Reorganize(F, md, filename);
                        if (getBlockStatus(table,fb + nb) == 1){
                        printf("Not enough space");
                        break;
                        }
                }
                
                Record x = buffer.tab[blocking_fact - 1];

                
                for (int k = blocking_fact  - 2; k >= trouv.y_record; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                
                buffer.tab[trouv.y_record] = e;
                WriteBlock(F, trouv.x_block, &buffer);

                // Passer au bloc suivant
                trouv.x_block++;
                trouv.x_block = 0;
                e = x; // L'enregistrement excédentaire sera inséré dans le bloc suivant
            } else {
                
                for (int k = buffer.nbrecord - 1; k >= trouv.y_record; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                // Insérer le nouvel enregistrement à la position j
                buffer.tab[trouv.y_record] = e;
                buffer.nbrecord++;
                WriteBlock(F, trouv.x_block, &buffer);
                continu = false;
            }
        }

        int current_block=trouv.x_block;
        // Si on dépasse la fin du fichier, ajouter un nouveau bloc
        if (continu) {
            if (getBlockStatus(table, current_block) == 1) {
            // No free block available, attempt reorganization
            printf("Attempting file reorganization to free space...\n");
            TOF_Reorganize(F, md, filename);
            
            // Re-read metadata after reorganization
            nb = read_metadata(pos_m, 2, md);
            last_block = fb + nb - 1;
            
            // Check if we have a free block after reorganization
            if (getBlockStatus(table, last_block + 1) == 0) {
                // Try insertion in the new block
                memset(&buffer, 0, sizeof(Block));
                buffer.nbrecord = 1;
                buffer.tab[0] = e;
                WriteBlock(F, last_block + 1, &buffer);
                setBlockStatus(table, last_block + 1, 1);
                write_metadata(pos_m, 2, md, nb + 1);
                continu = false;
            } else {
                printf("Error: No space available even after reorganization\n");
                return;
            }
        } else {
            // Free block available, use it
            memset(&buffer, 0, sizeof(Block));
            buffer.nbrecord = 1;
            buffer.tab[0] = e;
            WriteBlock(F, current_block, &buffer);
            setBlockStatus(table, current_block, 1);
            write_metadata(pos_m, 2, md, nb + 1);
            continu = false;
        }
    }

    if (!continu) {
        // Update record count
        int nRecords = read_metadata(pos_m, 3, md);
        write_metadata(pos_m, 3, md, nRecords + 1);
        printf("Record inserted successfully\n");
    }

    }

}


void TOF_DeleteRecord(FILE* F,FILE* md,const char* filename, int id) {
    coords trouv;

    AllocationTable t;
    ReadAllocationTable(&t,md);
    if (!F || !md) {
    printf("Erreur : Fichier non valide.\n");
    return;
    }
   
    trouv = TOF_SearchRecord(F, md,filename,id);

        if (trouv.found) {
            Block buffer;
            ReadBlock(F, trouv.x_block, &buffer);

            // Supprimer l'enregistrement
            buffer.tab[trouv.y_record].deleted = 1;

            int validrecod=0;

            for (int i = 0; i < buffer.nbrecord; i++)
            {
                if (buffer.tab[i].deleted==0)
                {
                    validrecod++;
                }
                
            }
            

            if (validrecod == 0) {
                setBlockStatus(&t, trouv.x_block,0);

                int pos_meta = search_metadata(filename, md);
                
                int lastBlock = read_metadata(pos_meta, 2, md)+read_metadata(pos_meta,1,md)-1;

                if (trouv.x_block == lastBlock) {
                    int newLastBlock = read_metadata(pos_meta, 2, md) - 1;
                    write_metadata(pos_meta, 2,md,newLastBlock);
                }

                printf("Bloc %d est maintenant vide et marqué comme libre.\n", trouv.x_block);
            }

            WriteBlock(F, trouv.x_block, &buffer);
            

            // Update metadata (e.g., decrement nRecords)
            int pos_meta = search_metadata(filename, md);
            int Nrecords=read_metadata(pos_meta,3,md)-1;
            write_metadata(pos_meta,3,md,Nrecords);


        printf("Enregistrement avec la clé %d supprimé logiquement.\n", id);
    } else {
        printf("Erreur : L'enregistrement avec la clé %d n'existe pas.\n", id);
    }

}


void TOF_Reorganize(FILE* F, FILE* md, const char* filename) {
    int pos_meta = search_metadata(filename, md);
    if (pos_meta == -1) {
        printf("File not found\n");
        return;
    }

    // Get file metadata
    int first_block = read_metadata(pos_meta, 1, md);
    int num_blocks = read_metadata(pos_meta, 2, md);
    int last_block = first_block + num_blocks - 1;
    
    // Create temporary array to store valid records
    Record* valid_records = NULL;
    int valid_count = 0;
    Block buffer;

    // First pass: Count valid records and allocate array
    for (int i = first_block; i <= last_block; i++) {
        ReadBlock(F, i, &buffer);
        for (int j = 0; j < buffer.nbrecord; j++) {
            if (buffer.tab[j].deleted == 0) {
                valid_count++;
            }
        }
    }

    valid_records = (Record*)malloc(valid_count * sizeof(Record));
    if (!valid_records) {
        printf("Memory allocation failed\n");
        return;
    }

    // Second pass: Collect all valid records
    int idx = 0;
    for (int i = first_block; i <= last_block; i++) {
        ReadBlock(F, i, &buffer);
        for (int j = 0; j < buffer.nbrecord; j++) {
            if (buffer.tab[j].deleted == 0) {
                valid_records[idx++] = buffer.tab[j];
            }
        }
    }

    // Sort the valid records by ID (to maintain TOF property)
    for (int i = 0; i < valid_count - 1; i++) {
        for (int j = 0; j < valid_count - i - 1; j++) {
            if (valid_records[j].Id > valid_records[j + 1].Id) {
                Record temp = valid_records[j];
                valid_records[j] = valid_records[j + 1];
                valid_records[j + 1] = temp;
            }
        }
    }

    // Calculate new number of blocks needed
    int new_num_blocks = (valid_count + blocking_fact - 1) / blocking_fact;
    
    // Rewrite blocks with compacted data
    int record_idx = 0;
    for (int i = 0; i < new_num_blocks; i++) {
        memset(&buffer, 0, sizeof(Block));  // Clear buffer
        buffer.nbrecord = 0;
        
        // Fill block with records
        while (buffer.nbrecord < blocking_fact && record_idx < valid_count) {
            buffer.tab[buffer.nbrecord++] = valid_records[record_idx++];
        }
        
        // Write block back to file
        WriteBlock(F, first_block + i, &buffer);
    }

    // Update metadata
    write_metadata(pos_meta, 2, md, new_num_blocks);  // Update number of blocks
    write_metadata(pos_meta, 3, md, valid_count);     // Update number of records

    // Mark any remaining blocks as free in allocation table
    AllocationTable table;
    ReadAllocationTable(&table, F);
    
    // Mark used blocks
    for (int i = first_block; i < first_block + new_num_blocks; i++) {
        table.arrays[i] = 1;  // Mark as used
    }
    
    // Mark freed blocks
    for (int i = first_block + new_num_blocks; i <= last_block; i++) {
        table.arrays[i] = 0;  // Mark as free
    }
    
    // Write updated allocation table
    rewind(F);
    fwrite(&table, sizeof(AllocationTable), 1, F);

    // Clean up
    free(valid_records);
    
    printf("File reorganized: %d records compacted into %d blocks\n", 
           valid_count, new_num_blocks);
}
