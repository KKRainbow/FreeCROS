//
// Created by ssj on 15-8-24.
//

#include <Type.h>
#include <dirent.h>
#include <SystemCalls.h>
#include <UserLog.h>
#include "fat32lib.h"
#include "stdio.h"
#include "stl/sstring.h"
#include "stl/svector.h"
#include "fat32.h"

static BootSector bs;
static DirectoryEntry rootDir;
FILE* fat32img;

using namespace lr::sstl;

int Fat32Init(const char* _Dev)
{
    fat32img = fopen(_Dev,"rb");
    if (!fat32img)
    {
        return false;
    }

    fseek(fat32img, 0, SEEK_SET);
    fread(&bs, sizeof(BootSector), 1, fat32img);

    // find the root directory
    uint32_t rootCluster = bs.cluster_number_for_the_root_directory;
    rootDir.low_starting_cluster = (unsigned short)(rootCluster & 0xffff);
    rootDir.high_starting_cluster = 0;
    rootDir.attributes = 0;
    rootDir.file_size = 0;
    rootDir.attributes |= 0x10;

    return true;
}

static AString getFilename( DirectoryEntry Dentries){
    AString filename;


    for(int j=0;j<10;j++){
        if(Dentries.filename[j+1] != 0xff && Dentries.filename[j+1] != 0x00  )
            filename += Dentries.filename[j+1];
    }
    for(int j=0;j<12;j++){
        if(Dentries.filename[j+14] != 0xff && Dentries.filename[j+14] != 0x00)
            filename += (Dentries.filename[j+14]);
    }
    for(int j=0;j<4;j++){
        if(Dentries.filename[j+28] != 0xff && Dentries.filename[j+28] != 0x00)
            filename += (Dentries.filename[j+28]);
    }


    return filename;
}

static bool FindEntry(AString _Name, DirectoryEntry* _Dir, Fat32Entry& _Res, Vector<Fat32Entry>* _Vec = nullptr)
{
    uint32_t CurrentCluster;
    CurrentCluster = (uint32_t)(
            _Dir->high_starting_cluster*256*256 + _Dir->low_starting_cluster);

    unsigned int FirstDataSector 	=    bs.reserved_sectors + ( bs.number_of_fats * bs.sectos_per_one_FAT) ;
    unsigned int FirstSectorofCluster  =  (CurrentCluster-2) * bs.sectors_per_cluster  + FirstDataSector;
    unsigned int FAT_First_Sector = bs.reserved_sectors;
    DirectoryEntry directory_entry ;
    unsigned int count=0;
    unsigned int NextCluster;
    fseek(fat32img,(FirstSectorofCluster)*bs.sector_size,SEEK_SET);
    AString name,temp;
    Fat32Entry fat32entry;
    bool readLongFile = true;
    while(1){

        if(count * 32 >= bs.sector_size * bs.sectors_per_cluster){

            count = 0;

            fseek(fat32img , bs.sector_size*FAT_First_Sector + CurrentCluster*4  ,  SEEK_SET);

            fread(&NextCluster ,  4  ,1,fat32img );

            if(NextCluster > 0 &&  NextCluster < ( bs.total_sectors - bs.reserved_sectors - bs.fat_size_sectors*bs.number_of_fats)/bs.sectors_per_cluster ){

                fseek(fat32img , bs.sector_size*(FirstDataSector + (NextCluster-2)*bs.sectors_per_cluster) ,  SEEK_SET);
                CurrentCluster = NextCluster;

            }
            else{
                break;
            }
        }

        fread(&directory_entry,sizeof(directory_entry),1,fat32img);

        if( directory_entry.filename[0] == 0x00)
            break;
        if(directory_entry.filename[0] == 0xE5){
            count++;
            continue;
        }
        if(directory_entry.filename[0] == 0x2E){
            count++;
            continue;
        }
        if(directory_entry.attributes == 0xf0)
        {
            continue;
        }
        //有可能是卷标 Volume label
        if(directory_entry.attributes & 0x8 && directory_entry.attributes != 0x0f)
        {
            continue;
        }

        if(readLongFile){
            temp = getFilename(directory_entry);
            name = temp + name;
            if((directory_entry.filename[0] & 0x41) == 0x41 || directory_entry.filename[0] == 0x01){
                readLongFile = false;

            }
        }
        else{
            _Res.filename = name;
            _Res.dirEntry = directory_entry;
            if(_Vec)_Vec->PushBack(_Res);
            char tmp[40];
            char tmp2[40];
            name.CStr(tmp);
            _Name.CStr(tmp2);
            if(name == _Name)
            {
                return true;
            }
            readLongFile = true;
            name = AString::Empty;
            temp = AString::Empty;
        }

        count++;
    }
    return false;
}


bool GetDirectoryEntry(lr::sstl::AString _Path, DirectoryEntry* _Root, Fat32Entry& _Res)
{
    if (!_Root)
    {
        _Root = &rootDir;
    }
    AString currName;
    Vector<AString> pathSplit;
    _Path.Split(pathSplit, '/');

    Fat32Entry nextEntry;
    nextEntry.dirEntry = *_Root;

    for (auto& tmpname : pathSplit)
    {
        if(tmpname.Length() == 0)continue;
        char tmp[50];
        tmpname.CStr(tmp);
        if(!FindEntry(tmpname, &nextEntry.dirEntry, nextEntry))
        {
            return false;
        }
    }
    _Res = nextEntry;
    return true;
}

int GetContent(Fat32Entry* _Entry, off_t _Offset, size_t _Size, char* _Buf)
{
    if(IS_DIR(_Entry->dirEntry))
    {
        if (_Size % sizeof(dirent) != 0 || _Offset % sizeof(dirent) != 0)
        {
            return 0;
        }
        Vector<Fat32Entry> vec;
        Fat32Entry tmp;
        FindEntry(AString::Empty, &_Entry->dirEntry, tmp,&vec);

        int begin = _Offset / sizeof(dirent);
        int end = (_Offset + _Size) / sizeof(dirent);
        end = end > vec.Size() ? vec.Size() : end;

        dirent* tmpbuf = (dirent*)_Buf;

        int tmpbegin = begin;
        for(;tmpbegin < end; tmpbegin++)
        {
            dirent dir;
            _Entry = &vec[tmpbegin];
            dir.d_ino = GET_CLUSTER_BEGIN(_Entry->dirEntry);
            _Entry->filename.CStr(dir.d_name);
            dir.d_reclen = (unsigned  short)_Entry->dirEntry.file_size;
            SysCallWriteToPhisicalAddr::Invoke((uint32_t)tmpbuf++, (uint32_t)&dir, sizeof(dir));
        }

        return (tmpbegin - begin) * sizeof(dirent);
    }

    unsigned int FirstDataSector 	=    bs.reserved_sectors + ( bs.number_of_fats * bs.sectos_per_one_FAT) ;
    unsigned int fileStartCluster = GET_CLUSTER_BEGIN(_Entry->dirEntry);

    unsigned int FirstSectorofThisFile  =  (fileStartCluster-2) * bs.sectors_per_cluster  + FirstDataSector;
    unsigned int FAT_First_Sector = bs.reserved_sectors;
    unsigned int filesize = _Entry->dirEntry.file_size;
    unsigned int NextCluster;

    fseek(fat32img,(FirstSectorofThisFile)*bs.sector_size,SEEK_SET);

    size_t size = _Size;
    unsigned char c;
    for(unsigned i=0;i<filesize;i++){
        if (i < _Offset)continue;
        if( i > 0 && i % (bs.sector_size*bs.sectors_per_cluster) == 0)
        {
            fseek(fat32img , bs.sector_size*FAT_First_Sector + fileStartCluster*4  ,  SEEK_SET);
            fread(&NextCluster , 4 ,1,fat32img );
            if(NextCluster > 0 && NextCluster < ( bs.total_sectors - bs.reserved_sectors - bs.fat_size_sectors*bs.number_of_fats)/bs.sectors_per_cluster ){
                fseek(fat32img , bs.sector_size*(FirstDataSector + (NextCluster-2)*bs.sectors_per_cluster) ,  SEEK_SET);
                fileStartCluster = NextCluster;
            }
        }
        fread(&c,sizeof(c),1,fat32img);
        SysCallWriteToPhisicalAddr::Invoke((uint32_t)_Buf++, (uint32_t)&c, sizeof(c));
        size--;
    }

    return _Size - size;
}
