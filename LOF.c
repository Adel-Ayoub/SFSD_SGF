//
// Created by xd on 31/12/2024.
//
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "LOF.h"

#define B_Fact 5

BlockP buffer;

void flushBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
        c = getchar();
}
void printStudent(StudentP R)
{
    printf("Name : %s", R->name);
    printf("Lastname : %s", R->lastname);
    printf("Id : %d \n\n" , R->Id );
}

void StudentCopy(StudentP R1, StudentP R2) {
    strcpy(R1->name, R2->name);
    // printf("Name v2---: %s", R2->name);
    strcpy(R1->lastname, R2->lastname);
    // printf("lastname v2---: %s", R2->lastname);
    R1->Id = R2->Id;
    // printf("Id : %d \n\n" , R1->Id );
    R1->deleted = R2->deleted;
} //copier le contenue de R2 dans  R1

void createStudent(StudentP R) {
    flushBuffer();
    printf("Name : ");
    fgets(R->name, sizeof(R->name), stdin);
    printf("lastname : ");
    fgets(R->lastname, sizeof(R->lastname), stdin); //fgets prends les espaces
    printf("Id : ");
    scanf("%d", &R->Id);
    R->deleted=0;
}

void printBlock(BlockP R) // afficheaderer un block
{
    printf("\t-----Record Information-----\t\n");
    for (int i = 0; i < R->nbrecord; i++) {
        printf("Record %d: \n", i + 1);
        printStudent((R->tab) + i); //afficher metadata les information de le i eme etudiant
    }

}
void createBlock(BlockP R) {
    printf("enter theadere number of records in this block : ");
    scanf("%d",&R->nbrecord);
    printf("\n");
    for(int i=0 ; i<(R->nbrecord) ; i++){
        printf("enter information for record number %d : \n",i+1);
        createStudent((R->tab) + i);
    }

}


void writeMetadata(LOF_MsP f, int i, int val) {
    switch (i)
    {
        case 1:
            f->header->FirstBlock = val;
            break;
        case 2:
            f->header->LastBlock = val;
            break;
        case 3:
            f->header->nBlocks = val;
            break;
        case 4:
            f->header->nRecords = val;
            break;
        default:
            printf("Error in writing header, you cheaderose a wrong number\n");
            break;
    }
}

int readMetadata(LOF_MsP f, int K) {
    switch (K)
    {
        case 1:
            return f->header->FirstBlock;
            break;
        case 2:
            return f->header->LastBlock;
            break;
        case 3:
            return f->header->nBlocks;
            break;
        case 4:
            return f->header->nRecords;
        default:
            printf("reading error, pick a valid number (1-4).\n");
            break;
    }
}

void writeBlock(LOF_MsP f, int i, BlockP buffer) {
    fseek(f->file,sizeof(Metadata)+((i-1)*sizeof(Block)),SEEK_SET);
    fwrite(buffer,sizeof(Block),1,f->file);
    rewind(f->file); // on se repositionne au debut de ficheaderier
}

void readBlock(LOF_MsP f, int i, BlockP buffer){

    // Se deplacer a la position du bloc dans le ficheaderier
    fseek(f->file, sizeof(Metadata) + (i-1) * sizeof(Block), SEEK_SET);
    // Lire le contenu du bloc dans le buffer
    fread(buffer, sizeof(Block), 1, f->file);
    // on se repositionne au debut de ficheaderier
    rewind(f->file);
};

void allocBlock(LOF_MsP f, int* i, BlockP* buffer) {
    *buffer = malloc(sizeof(Block));
    (*buffer)->nbrecord = 0;
    (*buffer)->next = -1;
    *i = readMetadata(f, 3) + 1;
    writeMetadata(f, 3, *i);  //incrementer le nombre de bloc dans le ficheaderier
    Student r;      //declaration d'un record NULL sans aucune valeur
    r.deleted = 1;
    strcpy(r.name, " ");
    strcpy(r.lastname, " ");
    r.Id = 0;
    for (int i = 0; i < B_Fact; i++)
        (*buffer)->tab[i] = r; //initialiser toute les position du bloc avec un etudiant NULL
}   //allouer un nouveau bloc et l'initialiser avec le contenue du tampom

//ouvrir le fichier logique
LOF_MsP LOF_open(LOF_MsP f, char *Fname, char mode) {
    f = malloc(sizeof(LOF_Ms));

    if (mode == 'o') {  // On ouvre le fichier en mode OLD 'o' (le fichier est ancien et existe deja)
        f->file = fopen(Fname, "rb+");
        if (f->file == NULL){
            return NULL;
        }
        f->header = malloc(sizeof(Metadata));
        size_t elements_read = fread(f->header, sizeof(Metadata), 1, f->file); // size_t returns theadere number of elemets read
        if (elements_read != 1) {
            fclose(f->file);
            free(f->header);
            printf("Error reading header from file\n");
            return NULL;
        }
        rewind(f->file);
    } else if (mode == 'n')
    {   // On ouvre le fichier en mode NEW 'n' (le fichier est nouveau et n'existe pas, on le cree et on initialise l'entete)
        f->file = fopen(Fname, "wb+");
        // Initialisation du header a zero
        f->header = malloc(sizeof(Metadata));
        f->header->FirstBlock = 0;
        f->header->LastBlock = 0;
        f->header->nBlocks = 0;
        f->header->nRecords = 0;
        fwrite(f->header, sizeof(Metadata), 1, f->file);
    } else
        printf("You choosed a wrong opening mode\n");
    return f;
}

//fermer le fichier logique
void LOF_close(LOF_MsP f){
    rewind(f->file);   // position le curseur au debut de fichier
    fwrite(f->header, sizeof(Metadata),1,f->file); // la sauvegarde des metadonnees dans le fichier physique
    fclose(f->file);// la fermeture du fichier physique

}

// array creation
StudentP scanTab(StudentP t, int length) {
    t = malloc(sizeof(Block) * length);
    printf("Entrez les informations des %d Student : \n", length);
    for (int i = 0; i < length; i++) {
        printf("Student %d :\n", i+1);
        createStudent(t + i);
    }
    return t;
}


void quickSortTab(StudentP tab, int start, int end) {
    if (start >= end)
        return;

    int pivot = tab[end].Id;
    int i = start - 1;
    Student temp;
    for (int j = start; j < end; j++)
    {
        if (tab[j].Id < pivot)
        {
            i++;
            StudentCopy(&temp, tab + i);
            StudentCopy(tab + i, tab + j);
            StudentCopy(tab + j, &temp);
        }
    }
    i++;
    StudentCopy(&temp, tab + i);
    StudentCopy(tab + i, tab + end);
    StudentCopy(tab + end, &temp);
    quickSortTab(tab, start, i - 1);
    quickSortTab(tab, i + 1, end);
}


void createLOF(LOF_MsP f, char file_name[], int N) {
    StudentP StudentTab;  //*t est le tableau a remplire dés la lecture initial kanet StudentP bedeltha l blockp jsp ida sehiha
    int k;
    BlockP buffer;
    StudentTab = scanTab(StudentTab, N); //scanner les N enregistrements
    quickSortTab(StudentTab, 0, N-1); //trier les N enregistrements selon la cle (QuickSort)

    //insertion des enregistrement en mode FIFO :
    f = LOF_open(f, file_name, 'n');
    allocBlock(f, &k, &buffer);
    writeMetadata(f, 1, k);
    writeMetadata(f, 2, k);   //considerer le nouveau buffer comme firstBlock et lastBlock

    int i = 0;
    int j = 0;
    while (i < N && i < B_Fact )   //remplire le premier bloc
    {
        StudentCopy((buffer->tab) + j, StudentTab + i);
        i++;
        j++;
    }

    while (i < N)
    {
        if (j < B_Fact )
        {
            StudentCopy((buffer->tab) + j, StudentTab + i);  //on reste dans le meme bloc
            j++;
        }
        else
        {
            buffer->nbrecord = j--;
            int oldnext = buffer->next;   //sauvegarder l'ancier bloc
            allocBlock(f, &k, &buffer);
            writeMetadata(f, 2, k);   //allouer un nouveau bloc et l'affecter comme lastBlock
            buffer->next = k;    //mettre a jour le champ svt
            writeBlock(f, oldnext, buffer);  //ecrire buffer dans l'ancien emplacement sauvegarder precedemment
            readBlock(f, buffer->next, buffer);
            StudentCopy((buffer->tab), StudentTab + i);  //affecter S a la premiere case du nouveau buffer
            j = 1;
        }
        i++;
    }
    free(StudentTab);
    buffer->nbrecord = j--;
    buffer->next = -1;   //mettre le svt du dernier bloc a -1 (nil)
    writeBlock(f, k, buffer);    //ecriture du dernier buffer dans le fichier
    writeMetadata(f, 4, N);   //nombre d'enregistrement dans le fichier

    LOF_close(f);    //fermer le fichier
}

void SearchStudent(LOF_MsP f, char file_name[], int id, int* blockNB, int* positionNB, int* exist) {
    f = LOF_open(f, file_name, 'o');
    *exist = 0;
    *blockNB = 0;
    *positionNB = 0;

    // Check if file contains students
    if (readMetadata(f, 4) != 0) {
        int start = 1;  // First block number
        int end = readMetadata(f, 2);  // Last block number

        while (start <= end && !(*exist)) {
            // Read current block
            int mid = (start + end) / 2;
            readBlock(f, mid, buffer);

            // Get first and last id in current block
            int firstid = -1;
            int lastid = -1;
            int i = 0;
            int j = 0;

            // Find first valid id
            while (i < B_Fact && j < buffer->nbrecord) {
                if (buffer->tab[i].deleted == 0) {
                    firstid = buffer->tab[i].Id;
                    break;
                }
                i++;
            }

            // Find last valid id
            i = B_Fact - 1;
            j = buffer->nbrecord - 1;
            while (i >= 0 && j >= 0) {
                if (buffer->tab[i].deleted == 0) {
                    lastid = buffer->tab[i].Id;
                    break;
                }
                i--;
            }

            // Check if id is in current block's range
            if (id >= firstid && id <= lastid) {
                // Search within the block
                for (i = 0; i < B_Fact && i < buffer->nbrecord; i++) {
                    if (buffer->tab[i].deleted == 0 && buffer->tab[i].Id == id) {
                        *exist = 1;
                        *blockNB = mid;
                        *positionNB = i + 1;
                        break;
                    }
                }
            } else if (id < firstid) {
                // Search in previous blocks
                end = mid - 1;
            } else {
                // Search in next blocks
                start = mid + 1;
            }
        }
    }

    LOF_close(f);
}

void SearchInsertionPosition(LOF_MsP f, char file_name[], int id, int* BlockNB, int* PositionNB) {
    f = LOF_open(f, file_name, 'o');

    // Get first block number
    int numbBlock = readMetadata(f, 1);
    *BlockNB = numbBlock;
    *PositionNB = 0;

    // If file is empty
    if (readMetadata(f, 4) == 0) {
        *BlockNB = 1;
        *PositionNB = 0;
        LOF_close(f);
        return;
    }

    // Traverse blocks
    while (numbBlock != -1) {
        readBlock(f, numbBlock, buffer);
        int i = 0;
        int j = 0;

        // Find first valid position in current block
        while (i < B_Fact && j < buffer->nbrecord) {
            if (buffer->tab[i].deleted == 0) {
                if (buffer->tab[i].Id > id) {
                    *BlockNB = numbBlock;
                    *PositionNB = i;
                    LOF_close(f);
                    return;
                }
                j++;
            }
            i++;
        }

        // If we haven't found a position and there's a next block
        if (buffer->next != -1) {
            // If current block is full, go to next block
            if (buffer->nbrecord == B_Fact) {
                numbBlock = buffer->next;
                *BlockNB = numbBlock;
                *PositionNB = 0;
            }
                // If current block has space, insert at end
            else {
                *BlockNB = numbBlock;
                *PositionNB = buffer->nbrecord;
                LOF_close(f);
                return;
            }
        }
            // If this is the last block
        else {
            // If we've gone through all records and id is larger
            if (i >= buffer->nbrecord) {
                // If block is full, create new block
                if (buffer->nbrecord >= B_Fact) {
                    *BlockNB = numbBlock + 1;
                    *PositionNB = 0;
                }
                    // If block has space, add to end
                else {
                    *BlockNB = numbBlock;
                    *PositionNB = buffer->nbrecord;
                }
            }
                // If we found a position in the middle
            else {
                *BlockNB = numbBlock;
                *PositionNB = i;
            }
            LOF_close(f);
            return;
        }
    }

    LOF_close(f);
}

void insertStudent(LOF_MsP f, char file_name[], StudentP student) {
    int blockNum, position, exists;
    BlockP newBlock;
    int newBlockNum;

    f = LOF_open(f, file_name, 'o');

    // Check if id already exists
    SearchStudent(f, file_name, student->Id, &blockNum, &position, &exists);
    while (exists) {
        printf("Ce matricule existe deja.\n");
        printf("donner un autre: \n");
        createStudent(student);
        SearchStudent(f, file_name, student->Id, &blockNum, &position, &exists);
    }

    // Find insertion position
    SearchInsertionPosition(f, file_name, student->Id, &blockNum, &position);
    printf("block num : %d , position : %d\n", blockNum, position);

    // Read the target block
    readBlock(f, blockNum, buffer);

    // Case 1: Block has space and position is valid
    if (buffer->nbrecord < B_Fact && position >= 0) {
        // Shift elements to make space
        for (int i = buffer->nbrecord; i > position; i--) {
            buffer->tab[i] = buffer->tab[i-1];
        }

        // Insert the student
        StudentCopy(&buffer->tab[position], student);
        buffer->nbrecord++;
        writeBlock(f, blockNum, buffer);
    }
        // Case 2: Block is full - need to split
    else if (buffer->nbrecord == B_Fact) {
        // Allocate new block
        allocBlock(f, &newBlockNum, &newBlock);

        int midPoint = B_Fact / 2;

        // Case 2a: Insert in first half
        if (position <= midPoint) {
            // Move second half to new block
            newBlock->nbrecord = 0;
            for (int i = midPoint; i < B_Fact; i++) {
                StudentCopy(&newBlock->tab[newBlock->nbrecord], &buffer->tab[i]);
                buffer->tab[i].deleted = 1;
                newBlock->nbrecord++;
                buffer->nbrecord--;
            }

            // Shift elements in first block to make space
            for (int i = midPoint - 1; i >= position; i--) {
                buffer->tab[i + 1] = buffer->tab[i];
            }

            // Insert the new student
            StudentCopy(&buffer->tab[position], student);
            buffer->nbrecord++;
        }
            // Case 2b: Insert in second half
        else {
            // Calculate position in new block
            int newPosition = position - midPoint;

            // Move second half to new block
            newBlock->nbrecord = 0;
            for (int i = midPoint; i < B_Fact; i++) {
                if (newBlock->nbrecord == newPosition) {
                    StudentCopy(&newBlock->tab[newBlock->nbrecord], student);
                    newBlock->nbrecord++;
                }
                StudentCopy(&newBlock->tab[newBlock->nbrecord], &buffer->tab[i]);
                buffer->tab[i].deleted = 1;
                newBlock->nbrecord++;
                buffer->nbrecord--;
            }

            // Handle case where insertion is at the end
            if (newBlock->nbrecord == newPosition) {
                StudentCopy(&newBlock->tab[newBlock->nbrecord], student);
                newBlock->nbrecord++;
            }
        }

        // Update block links
        newBlock->next = buffer->next;
        buffer->next = newBlockNum;

        // Write both blocks
        writeBlock(f, newBlockNum, newBlock);
        writeBlock(f, blockNum, buffer);
    }

    // Update Metadata information
    writeMetadata(f, 4, readMetadata(f, 4) + 1);  // Increment total number of students

    LOF_close(f);
}

void DeleteStudent(LOF_MsP f, char file_name[], int matricule) {
    f = LOF_open(f, file_name, 'o');

    // Check if file is empty
    if (readMetadata(f, 4) == 0) {
        printf("\nCe fichier est vide! la suppression est impossible!!\n");
        LOF_close(f);
        return;
    }

    // Search for student
    int blockNum, position, exists;
    SearchStudent(f, file_name, matricule, &blockNum, &position, &exists);

    if (exists) {
        position--; // Adjust position (SearchStudent returns 1-based position)
        readBlock(f, blockNum, buffer);

        // Mark record as deleted
        buffer->tab[position].deleted = 1;
        buffer->nbrecord--;

        // Update file header
        int totalStudents = readMetadata(f, 4);
        writeMetadata(f, 4, totalStudents - 1);

        // Save changes
        writeBlock(f, blockNum, buffer);

        printf("\nVous pouvez verifier la suppression dans votre fichier :\n");
    } else {
        printf("\nCe matricule n'existe pas. \n");
    }

    LOF_close(f);
}

void ModifyStudent(LOF_MsP f, char file_name[], StudentP student) {
    f = LOF_open(f, file_name, 'o');

    // Check if file is empty
    if (readMetadata(f, 4) == 0) {
        printf("your file is empty, 0 student\n");
        LOF_close(f);
        return;
    }

    // Search for student
    int blockNum, position, exists;
    SearchStudent(f, file_name, student->Id, &blockNum, &position, &exists);

    if (exists) {
        // Read the block containing the student
        readBlock(f, blockNum, buffer);

        // Update student information (position-1 because SearchStudent returns 1-based position)
        StudentCopy(&buffer->tab[position-1], student);

        // Save changes
        writeBlock(f, blockNum, buffer);
    } else {
        printf("student doesn't exist!\n");
    }

    LOF_close(f);
}