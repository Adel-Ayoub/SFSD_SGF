#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "gen.h"






coords TOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id) {
    int pos_meta = search_metadata(filename, md);
    int bi = read_metadata(pos_meta, 1, md);              // Borne inférieure : numéro du premier bloc
    int bs = read_metadata(pos_meta, 2, md) +bi -1; // Borne supérieure : numéro du dernier bloc
    coords position;
    position.found = false;
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
                *j = (inf + sup) / 2;
                if (id == buffer.tab[*j].Id && buffer.tab[*j].deleted == 0) {
                    position.found = true; // Clé trouvée et non supprimée
                } else if (id < buffer.tab[*j].Id) {
                    sup = position.y_record - 1;
                } else {
                    inf = position.y_record + 1;
                }
            }

            if (inf > sup) {
                position.y_record = inf; // Position où `c` devrait se trouver dans le bloc
            }
            stop = true; // Arrêter la RechercheDichotomique externe
        } else if (buffer.nbrecord == 0 || id < buffer.tab[0].Id) {
            bs = position.x_block - 1;
        } else { // c > buffer.tab[buffer.nbrecord - 1].Id
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
    int i, j;


    trouv=TOF_SearchRecord(F, md,filename,e.Id);
    // RechercheDichotomiquer la position où insérer le nouvel enregistrement
    if (getBlockStatus(table, trouv.x_block) == 1){
        printf("block occupied Reorganizing blocks...");
        // call function to reorganize things defrengemntation (interne) then search if still not enough space

        // call defrengemntation external function if still not enough space DISK IS FULL and return
    }
    int pos_m = search_metadata(filename, md);
    if (!trouv.found) {
        Block buffer;
        bool continu = true;

       
        while (continu && i <= read_metadata(pos_m, 2,md)) {
            ReadBlock(F, i, &buffer);

            if (buffer.nbrecord == __INT16_MAX__ ){
                
                Record x = buffer.tab[blocking_fact - 1];

                
                for (int k = __INT16_MAX__  - 2; k >= j; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                
                buffer.tab[j] = e;
                WriteBlock(F, i, &buffer);

                // Passer au bloc suivant
                trouv.x_block++;
                trouv.x_block = 0;
                e = x; // L'enregistrement excédentaire sera inséré dans le bloc suivant
            } else {
                
                for (int k = buffer.nbrecord - 1; k >= j; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                // Insérer le nouvel enregistrement à la position j
                buffer.tab[j] = e;
                buffer.nbrecord++;
                WriteBlock(F, i, &buffer);
                continu = false;
            }
        }

        // Si on dépasse la fin du fichier, ajouter un nouveau bloc
        if (i > read_metadata(pos_m, 2,md)) {
            if (getBlockStatus(table, i) == 1) {  // Vérifier si le bloc est libre
                buffer.nbrecord = 1;
                buffer.tab[0] = e;
                WriteBlock(F, i, &buffer);
                write_metadata(pos_m,2, md, i); // Mettre à jour le numéro du dernier bloc
            } else {
                printf("Erreur : Impossible d'ajouter un nouveau bloc, l'espace est plein.\n");//lzm ndiro defragmentation
            }
        }
    }
}


void SuppressionLogiqueTOF(FILE* F, FILE* md, const char* filename, int id) {
    coords trouv;
    int i, j;

   
    trouv = TOF_SearchRecord(F, md,filename,id);

        if (trouv.found) {
            Block buffer;
            ReadBlock(F, i, &buffer);

            // Supprimer l'enregistrement
            buffer.tab[j].deleted = 1;
            buffer.nbrecord--;

            WriteBlock(F, i, &buffer);

        } else {
            printf("Erreur : L'enregistrement avec la clé %d n'existe pas.\n", c);
        }
}


void TOF_DeleteRecord(FILE* f,char* filename, int id, int deleteType,MetadataTable* metadataTable) {
//...

}
