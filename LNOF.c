#include "LNOF.h"
#include "gen.h"
int LNOF_InitializeFile(FILE *F, FILE* MD,const char* filename,int Nrecords,AllocationTable *table){
    /*if (table == NULL) {
        perror("Memory allocation failed for AllocationTable\n");
        return -1;
    }*/
    ReadAllocationTable(table, F);
    
    printf("%d\n",table->arrays[0]);
    int blocks_needed = (int)(Nrecords / blocking_fact);
    printf("--------- init\n ");
    printf("%d\n", blocks_needed);
    printf("--------- init\n ");
	
	int* blocks=findFreeBlocks_list(table, blocks_needed);
	/*for(int i=0;i<blocks_needed;i++){
		printf("free blocks %d !!!!!\n",blocks[i]);
	}*/
    
    if (blocks == NULL) {
        printf("Not enough blocks sorry\n");
        return -1; // Not enough continuous blocks available
    }
    int first_adresss=blocks[0];
    
    //printf("first adress %d\n", first_adresss);
    Metadata p;
    strcpy(p.filename, filename);
    p.inter_organs = 0;
    p.global_organs = 0;
    p.nRecords = Nrecords;
    p.Firstblock = first_adresss;
    p.nBlocks = blocks_needed;

    
    printf("Okay creating file ");
    printf("%s\n", p.filename);
    
    fseek(MD, 0, SEEK_END);
    fwrite(&p, sizeof(p), 1, MD);
    printMetadata(MD,filename);
    table->num_files++;
     // Allocate memory for records
    WriteAllocationTable(table, F);
	
    Block buffer;
    fseek(F, sizeof(AllocationTable) + (blocks[0] * sizeof(buffer)), SEEK_SET);
    int counter = 0;
    int i = 0;
	int j = 0;
    while (i < blocks_needed){
         // Allocate memory for records
        buffer.nbrecord = 0;
        j=0;
        while (j < blocking_fact  && Nrecords>0){
            Record student;
            student.deleted = 0;
            student.Id = counter;
            counter++;
            buffer.tab[j] = student;
           
            buffer.nbrecord++;
            j++;
            Nrecords--;
        }    
        if(i==blocks_needed-1){
        	buffer.next=-1;
		}else{
			buffer.next=blocks[i+1];
		}
        fwrite(&buffer, sizeof(Block), 1, F);
        table->arrays[blocks[i]] = 1;
        i++;
    };
    return 0;
}





coords LNOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id)//blockpos and recordpos are outputs -1,-1 for error not found
{
	coords result;
	result.found=false;
	result.	x_block=-2;
	result.y_record=-1;
	int b=search_metadata(filename,md);//get metadata position for the filename
	if (b == -1) {
        printf("Metadata for file '%s' not found.\n", filename);
        return result ;
    }
	int posb=read_metadata(b,1,md);//read firstblock position from metadata
	int numb=read_metadata(b,2,md);
	printf("numb in md %d !!!!!!!!!!!!!!\n",numb);
	result.found=false;
	Block buffer;
	ReadBlock(F, posb, &buffer);
	int c=0;
	while(c<numb&& result.x_block!=-1) //traversing the block table until finding the block or end of List
		{
			
			for(int i=0; i<blocking_fact; i++) //traversing the block searching for record
				{
					if(buffer.tab[i].Id==id)
						{
							result.y_record=i;
							result.found=true;
							return result;
						}
				}
			if(buffer.next==-1)
				{
					printf("c at the end%d !!!!!!!!\n",c);
					return result;
				}
			result.x_block=buffer.next;
			printf("num block %d, next %d\n",c,buffer.next);
			ReadBlock(F, buffer.next, &buffer);
			
			c++;
		}
};
int LNOF_InsertRecord(FILE* F, FILE* md,const char* filename, Record e,AllocationTable *table){
	coords found;
	found=LNOF_SearchRecord(F,md,filename,e.Id);
	if(found.found){
		printf("record already exists. Try again!!!\n");
		return 1;
	};
	int pos_m = search_metadata(filename,md);
	int firstb=read_metadata(pos_m,1,md);
	int numb=read_metadata(pos_m,2,md);
	printf("first block pos %d !!!\n",firstb);
	Block buffer;
	Record def;
	def.Id=0;
	def.deleted=1;
	// Allocate memory for records
	int i=0;                      
	ReadBlock(F, firstb, &buffer);
	int currpos=firstb;
	while(i<numb){//traverse all the file searching for a place to put record
		if(buffer.nbrecord==blocking_fact){//if current bloc full
			if(buffer.next==-1){//end of file (all blocs are full) add a new bloc
				
				int blocpos= findFreeBlocks_list(table,1)[0];//get pos for a new bloc
				buffer.next=blocpos;//link the last bloc to the new bloc
				WriteBlock(F, currpos, &buffer);
				printf("bloc pos %d !!!!!!!!!!!\n",blocpos);
				printf("inserting new block at %d!!!!!!!!\n",blocpos);
				buffer.nbrecord=1;//filling the new bloc
				buffer.tab[0]=e;//filling the new bloc
				buffer.tab[1]=def;
				buffer.tab[2]=def;
				buffer.tab[3]=def;
				buffer.next=-1;//filling the new bloc
				WriteBlock(F, blocpos, &buffer);
				write_metadata(pos_m,2,md,numb+1);
				write_metadata(pos_m,3,md,(numb*4)+1);
				setBlockStatus(F,blocpos,1,table);
				return 0;
			}
			printBlockContents(F,i);
			currpos=buffer.next;
			ReadBlock(F,buffer.next,&buffer);
			
			i++;
			continue;
		}
			for(int j=0;i<blocking_fact;i++){  //set the new bloc to allocated in allocation table
			if(buffer.tab[j].deleted==1){
				buffer.tab[j]=e;
				buffer.nbrecord++;
				return 0;
			}
		}                         
		return -1;
	};
};
void LNOF_SuppressionLogique(FILE* F, FILE* md ,const char* filename, int id)
{
	coords recordcords=LNOF_SearchRecord(F,md,filename,id);
	if(recordcords.found) //searching for record enter if found else exit
		{
			//read bloc
			Block buffer;
			ReadBlock(F,recordcords.x_block , &buffer);
			// delete record
			buffer.tab[recordcords.y_record].deleted = 1;
			buffer.nbrecord--;
			if(buffer.nbrecord==0)
			{//if bloc became empty after deletion of record (single record case) set bloc to empty
				AllocationTable *t;
				ReadAllocationTable(t ,F);
				if(buffer.next!=-1)
				{//if the bloc is in the middle: copy next bloc into this one,free the next bloc (we would have to reread the entire file,stop at the previous bloc link it to the next one and set current bloc to empty instead  :( )
					int bloctodelete=buffer.next;
					Block nextbloc;
					ReadBlock(F,buffer.next, &nextbloc);//reading next bloc
					WriteBlock(F, recordcords.x_block, &nextbloc);//copy it to this one
					setBlockStatus(F,bloctodelete,0,t);//delete next bloc
				}else
				{
					setBlockStatus(F,recordcords.x_block,0,t);//if this bloc is the last bloc just set it as empty
			    }
			}else{//normal case
				WriteBlock(F, recordcords.x_block, &buffer);
			}
		}
	else
		{
			printf("Erreur : L'enregistrement with key %d n'existe pas.\n", id);
		}
}
void LNOF_SuppressionPhysique(FILE* F, FILE* md ,const char* filename, int id){
	coords recordcords=LNOF_SearchRecord(F,md,filename,id);
	if(recordcords.found)
	{
		Block buffer;
		ReadBlock(F,recordcords.x_block , &buffer);
		buffer.tab[recordcords.y_record].deleted=1;
		int r =blocking_fact-1;//right pointer
		int l=-1;//left pointer
		int i =0;//number of records deleted
		
		while(l<r &&buffer.tab[++l].deleted==0);
		while(l<r &&buffer.tab[--r].deleted==1);
		if(l<r)
		{
			buffer.tab[l]=buffer.tab[r];
			buffer.tab[r].deleted=1;
		};
		buffer.nbrecord-=1;
	};
		
}
void LNOF_DeleteRecord(FILE* F, FILE* md ,const char* filename, int id,int deleteType){
	if(deleteType==0){
		LNOF_SuppressionLogique(F,md,filename,id);
	}else if(deleteType==1){
		LNOF_SuppressionPhysique(F,md,filename,id);
	}else{
		printf("incorect delete type!!!!\n");
		printf("0 for logical\n1 for physical");
	}
};
void LNOF_reorgenize(FILE* F, FILE* md ,const char* filename){
	int blocpos=search_metadata(filename,md);//get metadata position for the filename
	blocpos=read_metadata(blocpos,1,md);//get first bloc position
	Block buffer;
	ReadBlock(F,1,&buffer);//read first bloc
	while(true){
		
		int r =blocking_fact-1;//right pointer
		int l=-1;//left pointer
		
		while(l<r){
			while(l<r && buffer.tab[++l].deleted==0);
			while(l<r && buffer.tab[--r].deleted==1);
			if(l<r)
			{
				buffer.tab[l]=buffer.tab[r];
				buffer.tab[r].deleted=1;
			};
		};
		if(buffer.next==-1){
			break;
		};
		ReadBlock(F,buffer.next,&buffer);
	};
}

