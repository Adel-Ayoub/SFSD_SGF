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
typedef struct LOF_MS
{
    FILE *f;    // Physical file (Physical fle)
    Metadata H; // the metadata in main memory
} LOF_Ms, *LOF_MsP;

////=================================== Block 
void displayBlock(BlockP R); 

////=================================== Record
void printRecord(RecordP R); // afficher 
void CopyRecord(RecordP R1, RecordP R2); //copier
void createRecord(RecordP R); // lire les informations 


////=================================== Primitives for LOF
LOF_MsP LOF_open(LOF_MsP f, char *Fname, char mode);  //ouvrir le fichier logique
void LOF_close(LOF_MsP f);  //fermer le fichier logique
void writeMetadata(LOF_MsP f, int i, int val); //affecter la valeur val au i ème champ des Metadata
int readMetadata(LOF_MsP f, int i);  //retourner le contenue du i ème champ de les Metadata
void printMetadata(LOF_MsP f, char *Fname);  //afficher le contenue des Metadata
void LOF_writeBlock(LOF_MsP f, int i, Buffer buffer);  //mettre le contenue du tbuffer dans le bloc numero K
void LOF_readBlock(LOF_MsP f, int i, Buffer buffer);  //mettre le contenue du bloc numero i dans le buffer
void allocBlock(LOF_MsP f, int* i, Buffer* buffer);   //allouer un nouveau bloc et l'initialiser avec le contenue du buffer
void displayLOF(LOF_MsP f, char *Fname, char result[]);     //afficher le contenue du fichier



#endif