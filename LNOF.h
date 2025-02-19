#include <stdio.h>
#include <stdlib.h>

#include "gen.h"

coords LNOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id);
void LNOF_InsertRecord(FILE* F, FILE* md,const char* filename, Record e);
void LNOF_SuppressionLogique(FILE* F, FILE* md ,const char* filename, int id);
void LNOF_SuppressionPhysique(FILE* F, FILE* md ,const char* filename, int id);
void LNOF_DeleteRecord(FILE* F, FILE* md ,const char* filename, int id,int deleteType);
void LNOF_reorgenize(FILE* F, FILE* md ,const char* filename);
////=================================== Primitives for LO-F ===================================


