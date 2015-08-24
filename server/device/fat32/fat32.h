//
// Created by ssj on 15-8-24.
//
#pragma once
typedef struct {

    unsigned char 	jump_instruction[3];
    char 		    oem_name[8];
    unsigned short 	sector_size; // in bytes
    unsigned char 	sectors_per_cluster;
    unsigned short	reserved_sectors;
    unsigned char 	number_of_fats;
    unsigned short	root_dir_entries; //NA
    unsigned short 	total_sectors_short; // if zero, later field is used
    unsigned char 	media_descriptor;
    unsigned short 	fat_size_sectors;
    unsigned short 	sectors_per_track;
    unsigned short 	number_of_heads;
    unsigned int 	hidden_sectors;
    unsigned int 	total_sectors;

    unsigned int	sectos_per_one_FAT;
    unsigned short	flags_for_FAT;
    unsigned short	file_system_version_number;
    unsigned int	cluster_number_for_the_root_directory;
    unsigned short	sector_number_FSInfo_structure;
    unsigned short 	sector_number_for_backupcopy;
    char		reserved_for_future_expansion[12];

    unsigned char 	drive_number;
    unsigned char 	current_head;
    unsigned char 	boot_signature;
    unsigned int 	volume_ID;
    char 		volume_label[11];
    char 		fs_type[8];
    char 		boot_code[420];
    unsigned short 	boot_sector_signature;

} __attribute((packed)) BootSector;

typedef struct {
    unsigned char filename[8];
    unsigned char extension[3];
    unsigned char attributes;
    unsigned char reserved;
    unsigned char created_time_tensOFsecond;
    unsigned short created_time_hms;
    unsigned short created_day;
    unsigned short accessed_day;
    unsigned short high_starting_cluster;
    unsigned short modify_time;
    unsigned short modify_date;
    unsigned short low_starting_cluster;
    unsigned int file_size;
} __attribute((packed)) DirectoryEntry;

typedef struct {
    DirectoryEntry dirEntry;
    lr::sstl::AString filename;
}  Fat32Entry  ;


