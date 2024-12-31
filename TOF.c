#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef struct record
{
    int Id; // cle de tri
    int deleted; 
} Record, *RecordP;

typedef struct BLock
{
    Record tab[__INT16_MAX__];
    int next;
    int nbrecord;
} Block, *BlockP;


typedef BlockP Buffer;


typedef struct Metadata 
{
    int Firstblock;
    char filename[20];
    int nBlocks;
    int global_organs;
    int inter_organs;
    int nRecords;
} Metadata, *MetadataP;

typedef struct {
    Metadata* table;
    int num_files;
} MetadataTable;

typedef struct  {
    int array[__INT16_MAX__]; // 0 means block is free 1 means blocks is occupaied
} AllocationTable;




int LireEntete(FILE* F, int param, char* filename) {
    MetadataTable metadataTable;
    fseek(F, sizeof(AllocationTable), SEEK_SET);
    fread(&metadataTable, sizeof(MetadataTable), 1, F);

    for (int i = 0; i < metadataTable.num_files; i++) {
        if (strcmp(metadataTable.table[i].filename, filename) == 0) {
            switch (param) {
                case 1:
                    return metadataTable.table[i].Firstblock;
                case 2:
                    return metadataTable.table[i].nBlocks;
                case 3:
                    return metadataTable.table[i].global_organs;
                case 4:
                    return metadataTable.table[i].inter_organs;
                case 5:
                    return metadataTable.table[i].nRecords;
                default:
                    printf("Erreur : Paramètre invalide.\n");
                    return -1;
            }
        }
    }

    printf("Erreur : Fichier non trouvé.\n");
    return -1;
}

void MAJEntete(FILE* F, int param, int val);

void LireBloc(FILE* F, int i, Block* buffer, MetadataTable* metadataTable   ) {
    // Simuler la lecture du bloc `i` depuis le fichier
    fseek(F, sizeof(metadataTable) + sizeof(int) + (metadataTable->num_files * sizeof(Metadata)) + ((i - 1) * sizeof(Block)), SEEK_SET); // Positionnement
    fread(buffer, sizeof(Block), 1, F);         
}

void EcrireBloc(FILE* F, int i, Block* buffer, MetadataTable* metadataTable) {
    // Calculate the offset of the block in the file
    int blockOffset = sizeof(metadataTable) + sizeof(int) + 
                      (metadataTable->num_files * sizeof(Metadata)) + 
                      ((i - 1) * sizeof(Block));

    // Move the file pointer to the block's position
    fseek(F, blockOffset, SEEK_SET);

    // Write the block data from the buffer to the file
    fwrite(buffer, sizeof(Block), 1, F);
}





bool TOF_SearchRecord(FILE* F,char* filename ,int c, int* i, int* j, MetadataTable* metadataTable) {
    int bi = LireEntete(F,1,filename);                // Borne inférieure : numéro du premier bloc
    int bs = LireEntete(F, 2,filename)+bi -1; // Borne supérieure : numéro du dernier bloc

    bool Trouv = false, stop = false;

    while (bi <= bs && !Trouv && !stop) {
        *i = (bi + bs) / 2; // Bloc du milieu
        Block buffer;
        LireBloc(F, *i, &buffer, metadataTable);

        // Vérifier si `c` est dans des clés du bloc 
        if (buffer.nbrecord > 0 && c >= buffer.tab[0].Id && c <= buffer.tab[buffer.nbrecord - 1].Id) {
            // RechercheDichotomique dans le bloc 
            int inf = 0, sup = buffer.nbrecord - 1;

            while (inf <= sup && !Trouv) {
                *j = (inf + sup) / 2;
                if (c == buffer.tab[*j].Id && buffer.tab[*j].deleted == 0) {
                    Trouv = true; // Clé trouvée et non supprimée
                } else if (c < buffer.tab[*j].Id) {
                    sup = *j - 1;
                } else {
                    inf = *j + 1;
                }
            }

            if (inf > sup) {
                *j = inf; // Position où `c` devrait se trouver dans le bloc
            }
            stop = true; // Arrêter la RechercheDichotomique externe
        } else if (buffer.nbrecord == 0 || c < buffer.tab[0].Id) {
            bs = *i - 1;
        } else { // c > buffer.tab[buffer.nbrecord - 1].Id
            bi = *i + 1;
        }
    }

    if (bi > bs) {
        *i = bi;
        *j = 0;  
    }

    return Trouv;
}


void TOF_InsertRecord(FILE* F, const char* nomfich, Record e, MetadataTable* metadataTable, AllocationTable* table) {
    bool trouv;
    int i, j;

    // RechercheDichotomiquer la position où insérer le nouvel enregistrement
    trouv=TOF_SearchRecord(F,nomfich,e.Id, &i, &j,metadataTable);

    if (!trouv) {
        Block buffer;
        bool continu = true;

       
        while (continu && i <= LireEntete(F, 2,nomfich)) {
            LireBloc(F, i, &buffer,metadataTable);

            if (buffer.nbrecord == __INT16_MAX__ ){
                
                Record x = buffer.tab[__INT16_MAX__ - 1];

                
                for (int k = __INT16_MAX__  - 2; k >= j; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                
                buffer.tab[j] = e;
                EcrireBloc(F, i, &buffer,metadataTable);

                // Passer au bloc suivant
                i++;
                j = 0;
                e = x; // L'enregistrement excédentaire sera inséré dans le bloc suivant
            } else {
                
                for (int k = buffer.nbrecord - 1; k >= j; k--) {
                    buffer.tab[k + 1] = buffer.tab[k];
                }

                // Insérer le nouvel enregistrement à la position j
                buffer.tab[j] = e;
                buffer.nbrecord++;
                EcrireBloc(F, i, &buffer,metadataTable);
                continu = false;
            }
        }

        // Si on dépasse la fin du fichier, ajouter un nouveau bloc
        if (i > LireEntete(F, 2,nomfich)) {
            if (getBlockStatus(table, i) == 1) {  // Vérifier si le bloc est libre
                buffer.nbrecord = 1;
                buffer.tab[0] = e;
                EcrireBloc(F, i, &buffer,metadataTable);
                MAJEntete(F, 2, i); // Mettre à jour le numéro du dernier bloc
            } else {
                printf("Erreur : Impossible d'ajouter un nouveau bloc, l'espace est plein.\n");//lzm ndiro defragmentation
            }
        }
    }
}


void SuppressionLogiqueTOF(FILE* F, const char* nomfich, int c, MetadataTable* metadataTable) {
    bool trouv;
    int i, j;

   
    trouv=TOF_SearchRecord(F, nomfich,c,&i, &j,metadataTable);

        if (trouv) {
            Block buffer;
            LireBloc(F, i, &buffer,metadataTable);

            // Supprimer l'enregistrement
            buffer.tab[j].deleted = 1;
            buffer.nbrecord--;

            EcrireBloc(F, i, &buffer,metadataTable);

        } else {
            printf("Erreur : L'enregistrement avec la clé %d n'existe pas.\n", c);
        }
}

void SuppressionPhysiqueTOF(FILE* F, const char* nomfich, int c,MetadataTable* metadataTable) {
    bool trouv;
    int i, j;

    
    trouv = TOF_SearchRecord(F, nomfich, c, &i, &j, metadataTable);

    if (trouv) {
        Block buffer;
        LireBloc(F, i, &buffer, metadataTable);

        // suppression de l'enregistrement
        for (int k = j; k < buffer.nbrecord - 1; k++) {
            buffer.tab[k] = buffer.tab[k + 1];
        }
        buffer.nbrecord--;

        EcrireBloc(F, i, &buffer, metadataTable);

        // Propager les décalages externes si nécessaire
        while (i < LireEntete(F, 2, nomfich)) {
            Block nextBuffer;
            LireBloc(F, i + 1, &nextBuffer, metadataTable);

            if (nextBuffer.nbrecord > 0) {
                // Déplacer le premier enregistrement du bloc suivant dans le bloc courant
                buffer.tab[buffer.nbrecord] = nextBuffer.tab[0];
                buffer.nbrecord++;

                // Décaler les enregistrements dans le bloc suivant
                for (int k = 0; k < nextBuffer.nbrecord - 1; k++) {
                    nextBuffer.tab[k] = nextBuffer.tab[k + 1];
                }
                nextBuffer.nbrecord--;

                EcrireBloc(F, i, &buffer, metadataTable);
                EcrireBloc(F, i + 1, &nextBuffer, metadataTable);

                // Lire le bloc suivant pour continuer
                buffer = nextBuffer;
                i++;
            }
        }
        if (buffer.nbrecord == 0) {
            // Supprimer le dernier bloc s'il est vide
            MAJEntete(F, 2, LireEntete(F, 2, nomfich) - 1);
        }
        
        printf("Enregistrement avec la clé %d supprimé physiquement.\n", c);
    } else {
        printf("Erreur : L'enregistrement avec la clé %d n'existe pas.\n", c);
    }
}


void TOF_DeleteRecord(FILE* f,char* filename, int id, int deleteType,MetadataTable* metadataTable) {
    if (deleteType == 0) {
        SuppressionLogiqueTOF(f, filename, id,metadataTable);
    } else if (deleteType == 1) {
        SuppressionPhysiqueTOF(f, filename, id,metadataTable);
    } else {
        printf("Erreur : Type de suppression invalide.\n");
    }

}
