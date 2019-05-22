#include "Address.h"

Address::Address() {
	Address_info[0] = Address_info[1] = Address_info[2] = 0;
}

Address::~Address() {
}

int Address::getblockID(){
	
	int all=(((Address_info[0]<<8)|Address_info[1])<<8)|Address_info[2];
	int block_id;
	block_id=int(all>>BLOCKOFFSET_SIZE);
	return block_id;
	
}

int Address::getOffset(){
	
	int all=(((Address_info[0]<<8)|Address_info[1])<<8)|Address_info[2];
	int block_offset;
	block_offset=int(all^((all>>BLOCKOFFSET_SIZE)<<BLOCKOFFSET_SIZE));
	return block_offset;
	
}

void Address::setblockID(int id){
	int x[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int temp=id;
	unsigned char tempid[3];
	for(int i=0;temp!=0;i++){
		if(temp%2==1){
			x[i+2]=1;	
		}else{
			x[i+2]=0;			
		}
		temp=temp/2;
	}
	for(int i=0;i<8;i++){
		tempid[0]=(tempid[0]<<1)|x[15-i];
	}
	for(int i=0;i<8;i++){
		tempid[1]=(tempid[1]<<1)|x[7-i];
	}
	unsigned char AND_ITEM2=(1<<1)|1;
	Address_info[0]=tempid[0];
	Address_info[1]=tempid[1]|(Address_info[1]&AND_ITEM2);
}

void Address::setOffset(int offset){
	int x[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int temp=offset;
	unsigned char tempid[3];
	for(int i=0;temp!=0;i++){
		if(temp%2==1){
			x[i]=1;	
		}else{
			x[i]=0;			
		}
		temp=temp/2;
	}
	for(int i=0;i<8;i++){
		tempid[1]=(tempid[1]<<1)|x[15-i];
	}
	for(int i=0;i<8;i++){
		tempid[2]=(tempid[2]<<1)|x[7-i];
	}
	unsigned char AND_ITEM6=((((((1<<1)|1)<<1|1)<<1|1)<<1|1)<<1|1)<<2;
	Address_info[2]=tempid[2];
	Address_info[1]=tempid[1]|(Address_info[1]&AND_ITEM6);
}
