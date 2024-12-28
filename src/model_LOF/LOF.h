#ifndef LOF_MODEL_H
#define LOF_MODEL_H

#include <stdio.h>
#include <stdlib.h>

typedef struct record
{
    int Id; // cle de tri
    int deleted; // supression logique
} Record, *RecordP;

//Declaration du bloc
typedef struct BLock
{
    Record tab[__INT16_MAX__];
    int next;
    int nbrecord;
} Block, *BlockP;

// I/O
typedef BlockP Buffer;

//Declaration des metadonnees (caracteristiques)
typedef struct Metadata 
{
    int Firstblock; // First block adresse
    int LastBlock; // Last block adresse
    int nBlocks; // number of blocks in the file
    int nRecords;// number of records in the file
    char *del;	// logical erase indicators ('*' erased / ' ' not erased)

} Metadata, *MetadataP;

// exemple of how the physical should be in the case of one LOF file in it (Logical file)


////=================================== Primitives for LOF
// -------

////=================================== Block 
void displayBlock(BlockP R); 

////=================================== Record
void printRecord(RecordP R); // afficher 
void CopyRecord(RecordP R1, RecordP R2); //copier
void createRecord(RecordP R); // lire les informations 



#endif