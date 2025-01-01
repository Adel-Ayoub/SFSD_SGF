//
// Created by xd on 31/12/2024.
//

#ifndef SRC_LOF_H
#define SRC_LOF_H

#include "gen.h"

void LOF_InsertRecord(char* filename, Record* record);
int LOF_SearchRecord(char* filename, int id);
void LOF_DeleteRecord(char* filename, int id, int deleteType);
void LOF_Reorganize(char* filename); // Physcal deletion

void LUOF_InsertRecord(char* filename, Record* record);
int LUOF_SearchRecord(char* filename, int id);
void LUOF_DeleteRecord(char* filename, int id, int deleteType);
void LUOF_Reorganize(char* filename);


#endif //SRC_LOF_H
