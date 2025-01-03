#include "LNOF.h"
#include "gen.h"
coords LNOF_SearchRecord(FILE* F,FILE *md ,const char* filename ,int id)//blockpos and recordpos are outputs -1,-1 for error not found
{
	int b=search_metadata(filename,md);//get metadata position for the filename
	b=read_metadata(b,1,md);//read firstblock position from metadata
	Block buffer;
	coords result;
	result.found=false;
	ReadBlock(F, b, &buffer);
	while(true) //traversing the block table until finding the block or end of List
		{
			for(int i=0; i<buffer.nbrecord; i++) //traversing the block searching for record
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
					return result;
				}
			result.x_block=buffer.next;
			ReadBlock(F, buffer.next, &buffer);
		}
};
void LNOF_InsertRecord(FILE* F, FILE* md,const char* filename, Record e){
	coords found;
	found=LNOF_SearchRecord(F,md,filename,e.Id);
	if(found.found){
		printf("record already exists. Try again!!!");
		return;
	};
	int pos_m = search_metadata(filename,md);
	int firstb=read_metadata(pos_m,1,md);
	Block buffer;
	ReadBlock(F,firstb,&buffer);
	while(true){//traverse all the file searching for a place to put record
		if(buffer.nbrecord==blocking_fact){//if current bloc full
			if(buffer.next==-1){//end of file (all blocs are full) add a new bloc
				AllocationTable* t;
				ReadAllocationTable(t,F);
				int blocpos= *findFreeBlocks_list(t,1);//get pos for a new bloc
				
				buffer.next=blocpos;//link the last bloc to the new bloc
				
				ReadBlock(F,blocpos,&buffer);//read th new bloc to modefy it
				
				buffer.nbrecord=1;//filling the new bloc 
				buffer.tab[0]=e;//filling the new bloc 
				buffer.next=-1;//filling the new bloc 
				
				WriteBlock(F, blocpos, &buffer);//parse the data to the bloc
				
				setBlockStatus(F,blocpos,1);//set the new bloc to allocated in allocation table
				break;
			}
			ReadBlock(F,buffer.next,&buffer);//reach here if there are more blocs in the file to check
			continue;
		};
		buffer.tab[buffer.nbrecord]=e;//reach here when it find a place and didnt need to allocate new bloc
		buffer.nbrecord++;
		break;
	}
}

void SuppressionLogiqueLNOF(FILE* F, FILE* md ,const char* filename, int id)
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
				if(buffer.next!=-1)
				{//if the bloc is in the middle: copy next bloc into this one,free the next bloc (we would have to reread the entire file,stop at the previous bloc link it to the next one and set current bloc to empty instead  :( )
					int bloctodelete=buffer.next;
					Block nextbloc;
					ReadBlock(F,buffer.next, &nextbloc);//reading next bloc
					WriteBlock(F, recordcords.x_block, &nextbloc);//copy it to this one
					setBlockStatus(F,bloctodelete,0);//delete next bloc
				}else
				{
					setBlockStatus(F,recordcords.x_block,0);//if this bloc is the last bloc just set it as empty
			    }
			}else{//normal case
				WriteBlock(F, recordcords.x_block, &buffer);
			}
		}
	else
		{
			printf("Erreur : L'enregistrement avec la clé %d n'existe pas.\n", id);
		}
}

