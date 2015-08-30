//
// Created by ssj on 15-8-24.
//
#pragma once

#include <stl/sstl/sstl_wstring.h>
#include <stl/sstl/sstl_vector.h>
#include"fat32.h"

#define IS_DIR(dir) (dir.attributes & 0x10)
#define IS_REG(dir) (dir.attributes & 0x20)

#define GET_CLUSTER_BEGIN(dir) ((uint32_t)(dir).high_starting_cluster*256*256 + \
                                    (dir).low_starting_cluster)

struct Fat32Position
{
    uint32_t currentCluster;
    uint32_t currentOffset;
};

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

class Fat32;

class Fat32Iterator
{
private:
    Fat32* fat32;
    friend class Fat32;
    size_t step;
    bool autoAlloc;
    bool crossCluster;

    Fat32Position pos;

    Fat32Iterator(Fat32* _Fat32, size_t _Step,Fat32Position _Pos, bool _CrossCluster,bool _AutoAlloc)
    :fat32(_Fat32),step(_Step),pos(_Pos),crossCluster(_CrossCluster),autoAlloc(_AutoAlloc)
    {}
public:
    int Read(void* _Buffer);
    int Write(void* _Buffer);
    bool HasNext();
    bool IsEnd();
    Fat32Iterator& operator+=(size_t _Size);
    Fat32Iterator& operator++();
    const Fat32Iterator operator++(int);
    Fat32Iterator(){
        pos = {0,0};
        step = 0;
        fat32 = nullptr;
    }
};

typedef struct {
    DirectoryEntry dirEntry;
    lr::sstl::AString filename;
//    Fat32Iterator begin;
//    Fat32Iterator end;
}  Fat32Entry  ;

class Fat32
{
private:
    BootSector bs;
    FILE* fp;
    DirectoryEntry rootDir;
    uint32_t lastAvailCluster = 2;
    uint32_t totalClusters = 0;
    friend class Fat32Iterator;
    void SeekToDateCluster(Fat32Position _Pos);
    enum Com{READ,WRITE};
    uint32_t ReadWriteCluster(Fat32Position _Pos, size_t _Size, void* _Buffer, Com rw);
    uint32_t FindNextAvailCluster(bool _Clean);
protected:
    uint32_t ReadFatItem(uint32_t _ClusterNum);
    uint32_t ReadCluster(Fat32Position _Pos, size_t _Size, void* _Buffer);
    void WriteFatItem(uint32_t _ClusterNum,uint32_t value);
    uint32_t WriteCluster(Fat32Position _Pos, size_t _Size, void* _Buffer);
    Fat32Position NextPosition(Fat32Position _Pos, uint32_t _Offset, bool _AutoAlloc = false);
    bool FindEntry(lr::sstl::AString _Name, DirectoryEntry* _Dir,
                   Fat32Entry& _Res,
                   lr::sstl::Vector<Fat32Entry>* _Vec = nullptr);
    lr::sstl::AString getFilename( DirectoryEntry Dentries);
public:
    Fat32(FILE* _Fp);

    bool GetDirectoryEntry(lr::sstl::AString _Path, DirectoryEntry* _Root, Fat32Entry& _Res);
    int GetContent(Fat32Entry* _Entry, off_t _Offset, size_t _Size, char* _Buf);
};
