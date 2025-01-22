#include "LNOF.h"
#include "gen.h"

int main() {
    const char* ms_filename = "ms_file.bin";
    const char* md_filename = "md_file.bin";
    const char* testfile_name = "testfile";
    int Nrecords = 100; // Number of records to initialize
    printf("fffs");
    // Open files in binary mode
    FILE *ms = fopen(ms_filename, "wb+");
    FILE *md = fopen(md_filename, "wb+");
    if (!ms || !md) {
        printf("Error opening files.\n");
        return 1;
    }
    // Initialize allocation table
    AllocationTable *table = initAllocationTable();
    WriteAllocationTable(table, ms);// Write allocation table to block 0

    // Test TNOF_InitializeFile
    printf("Initializing file with %d records...\n", Nrecords);
    if(LNOF_InitializeFile(ms, md, testfile_name, Nrecords,table)==1){
    	printf("Error initializing file!!!!!!!!!!!!\n");
    	return 1;
	};
	WriteAllocationTable(table, ms);
	ReadAllocationTable(table,ms);
    // Test TNOF_InsertRecord
    printf("my seaching method!!!!!\n");
    /*coords p = LNOF_SearchRecord(ms, md, testfile_name, 15);
    if(p.found && p.x_block==4&&p.y_record==3){
    	printf("searching works!!!!!!!!");
	}*/
    //printf(p.found);
    printf("Inserting additional records...\n");
    Record e1, e2;
    e1.Id = 101; e1.deleted = 0;
    e2.Id = 102; e2.deleted = 0;
    LNOF_SuppressionLogique(ms, md, testfile_name, 98);
    if(LNOF_InsertRecord(ms, md, testfile_name, e1,table)==1){
    	printf("error inserting record 101!!!!!!");
	}else{
		printf("record 101 inserted  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	printBlockContents(ms,26);
    if(LNOF_InsertRecord(ms, md, testfile_name, e2,table)==1){
    	printf("error inserting record 102!!!!!!");
	};

    // Test TNOF_SearchRecord
    printf("Searching for records...\n");
    coords p1 = LNOF_SearchRecord(ms, md, testfile_name, 101);
    if (p1.found)
        printf("Found record 101 at block %d,!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", p1.x_block);
    else
        printf("Record 101 not found.  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    coords p2 = LNOF_SearchRecord(ms, md, testfile_name, 102);
    if (p2.found)
        printf("Found record 102 at block %d, record %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", p2.x_block, p2.y_record);
    else
        printf("Record 102 not found.\n");
    
    printf("Deleting record 98 logically...\n");
    LNOF_SuppressionLogique(ms, md, testfile_name, 98);

    // Re-search for deleted record
    coords p3 = LNOF_SearchRecord(ms, md, testfile_name, 98);
    Block buffer;
	ReadBlock(ms,p3.x_block , &buffer);
    if (p3.found && buffer.tab[p3.y_record].deleted)
        printf("Record 98 is logically deleted.\n");
    else
        printf("Record 98 is not deleted or not found.\n");

    // Test reorganiserBlocs
    printf("Reorganizing the file...\n");

    fclose(ms);
    fclose(md);

    printf("All tests completed.\n");
    return 0;
}
