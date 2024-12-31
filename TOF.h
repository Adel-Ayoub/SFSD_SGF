//
// Created by xd on 31/12/2024.
//

#ifndef SRC_TOF_H
#define SRC_TOF_H
#include "gen.h"
// ORDERED
void TOF_InsertRecord(char* filename, Record* record);
int TOF_SearchRecord(char* filename, int id);
void TOF_DeleteRecord(char* filename, int id, int deleteType);
void TOF_Compact(char* filename); // removes gaps mn logical deletions

// UNORDERED
void TUOF_InsertRecord(char* filename, Record* record);
int TUOF_SearchRecord(char* filename, int id);
void TUOF_DeleteRecord(char* filename, int id, int deleteType);
void TUOF_Compact(char* filename); // removes gaps mn logical deletions


#endif //SRC_TOF_H
