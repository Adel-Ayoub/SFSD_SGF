#include <stdio.h>
#include <stdlib.h>

#include "gen.h"

coords LNOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id);
void LNOF_InsertRecord(FILE* F, FILE* md,const char* filename, Record e);
void LNOF_SuppressionLogique(FILE* F, FILE* md ,const char* filename, int id);

////=================================== Primitives for LO-F ===================================


