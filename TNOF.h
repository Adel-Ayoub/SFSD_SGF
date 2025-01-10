//
// Created by xd on 5/1/2025.
//

#ifndef SRC_TNOF_H
#define SRC_TNOF_H


void  TNOF_InitliazeFile(FILE *F, FILE* MD,const char* filename,int Nrecords);
coords TNOF_SearchRecord(FILE* F, FILE* MD, const char* filename ,int id);
void TNOF_InsertRecord(FILE* F, FILE* MD,const char* filename, Record e,  AllocationTable* table);
void TNOF_Reorganize(char* filename,FILE *ms, FILE *MD); // Physcal deletion

#endif //SRC_TNOF_H
