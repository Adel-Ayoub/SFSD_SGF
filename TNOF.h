//
// Created by xd on 5/1/2025.
//

#ifndef SRC_TNOF_H
#define SRC_TNOF_H


void TNOF_InitliazeFile(FILE *F, FILE* MD,const char* filename,int Nrecords,  AllocationTable* table);
void TNOF_InsertRecord(FILE* F, const char* filename, Record e,  AllocationTable* table);
bool TNOF_SearchRecord(FILE* F,char* filename ,int c, int* i, int* j);
void SuppressionLogiqueTNOF(FILE* F, const char* filename, int c);
void TNOF_Reorganize(char* filename); // Physcal deletion

#endif //SRC_TNOF_H
