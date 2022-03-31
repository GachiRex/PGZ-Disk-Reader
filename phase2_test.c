#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Partition {
	int number;					//iteration
	int bootable_partition;		//byte offset 0x00
	char file_system_type[14];	//byte offset 0x04
	int start_sector;			//byte offset 0x08
	int size;					//byte offset 0x0C
};

struct FAT {
	int sectors_per_cluster;	//byte offset 0x0D

	int FAT_area_size;
		//Size of FAT in sect. x No. of FAT copies ==> (0x10) * (0x16)(word)

	int RootDir_size;
		//Max No. of entries x Dir. entry size (bytes) / sector size ==> (0x11)(word) * 32 / 512

	int cluster2_address;
		//[First sector of FAT volume + Size of R.A + Size of FAT area] + Size of Root dir.
		//[ 63 + 0x0E(word) +  FAT_area_size ] + RootDir_Size
};

struct Deleted_file {
	char filename[11];			//11 byte filename
	int size;
	int startingCluster;
	char content[16];			//the 16 first characters of the content of the file
	int cluster_nb;
	int csa;
};

struct NTFS {
	int partition_number;
	int address;
	int size;

	int bytes_per_sector;
	int sectors_per_cluster;

	long long mft_logical_cluster_nb;
	int mft_sector_address;

	int mft_first_AttributeOffset;
	int mft_first_AttributeAddress;
	int mft_first_AttributeType_val;
	char mft_first_AttributeType[22];
	int mft_first_AttributeLength;

	int mft_second_AttributeOffset;
	int mft_second_AttributeAddress;
	int mft_second_AttributeType_val;
	char mft_second_AttributeType[22];
	int mft_second_AttributeLength;
};

struct Partition partitions[4];
struct FAT fat;
struct Deleted_file delfile;
struct NTFS ntfs;

void mft_attribute(int code, char * attributeType) {
	switch(code) {
		case 0x10 : strcpy ( attributeType, "STANDARD-INFORMATION" ); 	break;
		case 0x20 : strcpy ( attributeType, "ATTRIBUTE-LIST" ); 		break;
		case 0x30 : strcpy ( attributeType, "FILE-NAME" ); 				break;
		case 0x40 : strcpy ( attributeType, "OBJECT-ID" ); 				break;
		case 0x50 : strcpy ( attributeType, "SECURITY-DESCRIPTOR" ); 	break;
		case 0x60 : strcpy ( attributeType, "VOLUME-NAME" ); 			break;
		case 0x70 : strcpy ( attributeType, "VOLUME-INFORMATION" ); 	break;
		case 0x80 : strcpy ( attributeType, "DATA" ); 					break;
		case 0x90 : strcpy ( attributeType, "INDEX-ROOT" ); 			break;
		case 0xA0 : strcpy ( attributeType, "INDEX-ALLOCATION" ); 		break;
		case 0xB0 : strcpy ( attributeType, "BITMAP" ); 				break;
		case 0xC0 : strcpy ( attributeType, "REPARSE-POINT" ); 			break;
		case 0x100 : strcpy ( attributeType, "LOGGED-UTILITY-STREAM" ); break;
	}
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		fprintf(stderr, "Usage: 1 argument needed (disk image)\n");
		return 0;
	}

//=============================================[Partition]=======================================

	int offset = 16, invalid = 0, type, bootable;
	char partitions_data[64];
		//each partition is 16 bytes long and there are 4 partitions --hence 64

	FILE *disk_image;
	disk_image = fopen(argv[1],"rb");
	fseek(disk_image, 0x1BE, SEEK_SET);

	fread(partitions_data, 1, 64, disk_image);
		//fread reads '64' items of data, each '1' byte
		//long, from the stream pointed to by 'disk_image', storing them at the
		//location given by 'partitions_data'

	printf("------------------------------------------------------------------------------\n");

	for (int i = 0; i < 4; i++) {
		bootable = *(int*)(partitions_data + offset * i); ///
		partitions[i].bootable_partition = (bootable) ? 1 : 0; ///
		type = *(char*)(partitions_data + 4 + offset * i);
		partitions[i].start_sector = *(int*)(partitions_data + 8 + offset * i);
		partitions[i].size = *(int*)(partitions_data + 0x0C + offset * i);

		switch (type) {
			case 0 : strcpy ( partitions[i].file_system_type, "NOT-VALID"); invalid++ ; break;
			case 1 : strcpy ( partitions[i].file_system_type, "12-BIT FAT"); break;
			case 4 : strcpy ( partitions[i].file_system_type, "16-BIT FAT"); break;
			//case 5 : strcpy ( partitions[i].file_system_type, "EXTENDED MS-DOS"); break;
			case 6 : strcpy ( partitions[i].file_system_type, "FAT-16"); break;
			case 7 : strcpy ( partitions[i].file_system_type, "NTFS"); 
			ntfs.partition_number = i; break;
			case 0xB : strcpy ( partitions[i].file_system_type, "FAT-32"); break;				//FAT-32 (CHS)
			case 0xC : strcpy ( partitions[i].file_system_type, "FAT-32"); break;				//FAT-32 (LBA)
			case 0xE : strcpy ( partitions[i].file_system_type, "FAT-16"); break;				//FAT-16 (LBA)
			case 0x10 : strcpy (partitions[i].file_system_type, "FAT-16"); break;				//Hidden FAT-16
			case 0x11 : strcpy (partitions[i].file_system_type, "NTFS") ; break;				//Hidden NTFS
			default: strcpy ( partitions[i].file_system_type, "NOT-RECOGNIZED"); break;
		}

	printf("Partition %d: Type: %-14s Start: %-9d Size: %-9d Bootable: %-d\n",
			i,
			partitions[i].file_system_type,
			partitions[i].start_sector,
			partitions[i].size,
			partitions[i].bootable_partition);
 	}

	printf("\nTotal number of valid partitions is: %d\n", 4-invalid);
	printf("------------------------------------------------------------------------------\n");

//==========================================[FAT Infos]==========================================
//in this tool we assume that the first partition will be formatted as a FAT-16 always
//int, char, short: reads certain nb of bytes (8, 1, 2 resp.)

	fseek(disk_image, partitions[0].start_sector * 512, SEEK_SET);			//jump to sect 0 of FAT
	char FAT_volume_data[64];												//FAT-16 volume's boot sector is 64 bytes long
	fread(FAT_volume_data, 1, 64, disk_image);

	fat.sectors_per_cluster = *(char*)(FAT_volume_data + 0x0D);

	int FAT_size = *(short*)(FAT_volume_data + 0x16);
	int nb_FAT_copies = *(short*)(FAT_volume_data + 0x10);
	fat.FAT_area_size = FAT_size * nb_FAT_copies;

	int RootDir_max_nbEntries = *(int*)(FAT_volume_data + 0x11);
	fat.RootDir_size = RootDir_max_nbEntries * 32 / 512;

	int RA_size = *(char*)(FAT_volume_data + 0x0E);
	int dataArea_start_sector = ( 63 + RA_size + fat.FAT_area_size );
	fat.cluster2_address = dataArea_start_sector + fat.RootDir_size;

	printf("Partition 1 -- FAT Volume Information:\n\nSectors per cluster:   %d\n------FAT Area size:   %d\nRoot Directory size:   %d\n--Sector #2 Address:   %d\n",
		fat.sectors_per_cluster,
		fat.FAT_area_size,
		fat.RootDir_size,
		fat.cluster2_address);
	printf("------------------------------------------------------------------------------\n");

//========================================[Deleted files]========================================

	int dataArea_address = dataArea_start_sector * 512;
	int RootDir_curr_data, nb_deleted_files;

	fseek(disk_image, dataArea_address, SEEK_SET);
	char deletedFiles_data[RootDir_max_nbEntries * 32];
	fread(deletedFiles_data, 1, RootDir_max_nbEntries * 32, disk_image);

	printf("Search for a deleted file:\n\n");
	printf("--Begins on Sector %d (0x%x)\n", dataArea_address/512, dataArea_address);

	int j = 0, flag = 0;								//Flag for first deleted file encountered
	int first_deletedFile_address;

	for (j ; j <= RootDir_max_nbEntries ; j++) {
		RootDir_curr_data = *(unsigned char*)(deletedFiles_data + j * 32);
		if ( RootDir_curr_data == 0xE5 ) { 
			if (!flag) { first_deletedFile_address = dataArea_address + j * 32; }
			nb_deleted_files++;
			flag = 1;

			printf("Deleted file found on 0x%x...\n", dataArea_address + j * 32);
		}
	}
	printf("---Ended on Sector %d (0x%x)\n\nNumber of deleted file(s):%6d\n",
		(dataArea_address+j*32)/512, dataArea_address+j*32,
		nb_deleted_files);
	printf("------------------------------------------------------------------------------\n");

//=====================================[Deleted files info]======================================
//in this tool we assume that the deleted file is a simple text file

	fseek(disk_image, first_deletedFile_address, SEEK_SET);
	char first_deletedFile_data[32];					//An entry is 32-bytes long
	fread(first_deletedFile_data, 1, 32, disk_image);

	printf("First Deleted File Information:\n\n");

	for (int i = 0 ; i < 11 ; i++) { delfile.filename[i] = *(char*)(first_deletedFile_data + i); } 
	delfile.startingCluster = *(short*)(first_deletedFile_data + 0x1A);
	delfile.size = *(int*)(first_deletedFile_data + 0x1C);

	delfile.cluster_nb = fat.cluster2_address + ((delfile.startingCluster - 2) * 8);
	delfile.csa = delfile.cluster_nb * 512;

	fseek(disk_image, delfile.csa, SEEK_SET);
	char deletedFile_content[16];
	fread(deletedFile_content, 1, 16, disk_image);

	for (int i = 0; i < 16 ; i++) { delfile.content[i] = *(char*)(deletedFile_content + i) ;}

	printf("----File name: %s\n---------Size: %d\nFirst cluster: %d\n-Content peek:%s\n",
		delfile.filename,
		delfile.size,
		delfile.startingCluster,
		delfile.content);
	printf("------------------------------------------------------------------------------\n");

//==========================================[NTFS Infos]=========================================



	ntfs.address = partitions[ntfs.partition_number].start_sector * 512;
	ntfs.size = partitions[ntfs.partition_number].size;

	fseek(disk_image, ntfs.address, SEEK_SET);
	char ntfs_data[ntfs.size];
	fread(ntfs_data, 1, ntfs.size, disk_image);

	printf("NTFS Volume Information:\n\n");																		//NTFS starts on sector 1606500

	ntfs.bytes_per_sector = *(short*)(ntfs_data + 0x0B);
	ntfs.sectors_per_cluster = *(char*)(ntfs_data + 0x0D);
	ntfs.mft_logical_cluster_nb = *(long long*)(ntfs_data + 0x30);

	int mft_offset = ntfs.mft_logical_cluster_nb * 8 * 512; 													//MFT offset: 32 sectors
	ntfs.mft_sector_address = ntfs.address + mft_offset;													//MFT on sector 16065032

	ntfs.mft_first_AttributeOffset = *(short*)(ntfs_data + mft_offset + 0x14);
	ntfs.mft_first_AttributeAddress = ntfs.address + ntfs.mft_first_AttributeOffset * 512; 						//1st Attribute offset : 0x38 bytes (56) from MFT start
	ntfs.mft_first_AttributeType_val = *(int*)(ntfs_data + mft_offset + ntfs.mft_first_AttributeOffset);
	mft_attribute(ntfs.mft_first_AttributeType_val, ntfs.mft_first_AttributeType);
	ntfs.mft_first_AttributeLength = *(int*)(ntfs_data + mft_offset + ntfs.mft_first_AttributeOffset + 0x04);

	ntfs.mft_second_AttributeOffset = ntfs.mft_first_AttributeOffset + ntfs.mft_first_AttributeLength;
	ntfs.mft_second_AttributeAddress = ntfs.address + ntfs.mft_second_AttributeOffset * 512; 
	ntfs.mft_second_AttributeType_val = *(int*)(ntfs_data + mft_offset + ntfs.mft_second_AttributeOffset);
	mft_attribute(ntfs.mft_second_AttributeType_val, ntfs.mft_second_AttributeType);
	ntfs.mft_second_AttributeLength = *(int*)(ntfs_data + mft_offset + ntfs.mft_second_AttributeOffset + 0x04);

	printf("---Bytes per sector: 0x%x (%d)\nSectors per Cluster: %d\n-----Sector address: %lld\n",
		ntfs.bytes_per_sector,
		ntfs.bytes_per_sector,
		ntfs.sectors_per_cluster,
		ntfs.mft_logical_cluster_nb);

	printf("\nMFT: Two First Attributes Information:\n");
	printf("\nAttribute 1:\n---Type: %-22s\n-Length: 0x%x (%3d)\nAddress: 0x%x\n-Sector: %d\n",
		ntfs.mft_first_AttributeType,
		ntfs.mft_first_AttributeLength,
		ntfs.mft_first_AttributeLength,
		ntfs.mft_first_AttributeAddress,
		ntfs.mft_sector_address/512);
	printf("\nAttribute 2:\n---Type: %-22s\n-Length: 0x%x (%3d)\nAddress: 0x%x\n-Sector: %d\n",
		ntfs.mft_second_AttributeType,
		ntfs.mft_second_AttributeLength,
		ntfs.mft_second_AttributeLength,
		ntfs.mft_second_AttributeAddress,
		ntfs.mft_sector_address/512);

	printf("------------------------------------------------------------------------------\n");

	fclose(disk_image);
	return 0;
}