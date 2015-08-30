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
using namespace lr::sstl;

AString Fat32::getFilename( DirectoryEntry Dentries){
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

bool Fat32::FindEntry(AString _Name, DirectoryEntry* _Dir, Fat32Entry& _Res, Vector<Fat32Entry>* _Vec)
{
    uint32_t currentCluster = GET_CLUSTER_BEGIN(*_Dir);
    Fat32Iterator iter(this, sizeof(DirectoryEntry),{currentCluster,0}, false,false);
    DirectoryEntry directoryEntry;
    bool readLongFile = true;
    int count = 0;
    AString temp,name;
    while(!iter.IsEnd()){
        iter.Read(&directoryEntry);

        if( directoryEntry.filename[0] == 0x00)
            break;
        if(directoryEntry.filename[0] == 0xE5){
            count++;
            ++iter;
            continue;
        }
        if(directoryEntry.filename[0] == 0x2E){
            count++;
            ++iter;
            continue;
        }
        if(directoryEntry.attributes == 0xf0)
        {
            ++iter;
            continue;
        }
        //有可能是卷标 Volume label
        if(directoryEntry.attributes & 0x8 && directoryEntry.attributes != 0x0f)
        {
            ++iter;
            continue;
        }

        if(readLongFile){
//            if(_Res.begin.IsEnd())
//            {
//                _Res.begin = iter;
//            }
            temp = getFilename(directoryEntry);
            name = temp + name;
            if((directoryEntry.filename[0] & 0x41) == 0x41 || directoryEntry.filename[0] == 0x01){
                readLongFile = false;
            }
        }
        else{
            _Res.filename = name;
            _Res.dirEntry = directoryEntry;
//            _Res.end = iter;
            if(_Vec)_Vec->Insert(_Vec->Begin(),_Res);
            if(name == _Name)
            {
                return true;
            }
            readLongFile = true;
            name = AString::Empty;
            temp = AString::Empty;
        }
        count++;
        ++iter;
    }
    return false;
}

int Fat32::GetContent(Fat32Entry* _Entry, off_t _Offset, size_t _Size, char* _Buf)
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
            printf("Get dir name : %s\n",dir.d_name);
            dir.d_reclen = (unsigned  short)_Entry->dirEntry.file_size;
            SysCallWriteToPhisicalAddr::Invoke((uint32_t)tmpbuf++, (uint32_t)&dir, sizeof(dir));
        }

        return (tmpbegin - begin) * sizeof(dirent);
    }

    char* buffer = new char[_Size];
    uint32_t cluster = GET_CLUSTER_BEGIN(_Entry->dirEntry);
    uint32_t size = this->ReadCluster(this->NextPosition({cluster,0},_Offset,false), _Size, buffer);
    SysCallWriteToPhisicalAddr::Invoke((uint32_t)_Buf++, (uint32_t)buffer,size);
    return size;
}

uint32_t Fat32::ReadWriteCluster(Fat32Position _Pos, size_t _Size, void* _Buffer,Com rw)
{
    size_t size = _Size;
    size_t bytesPerCluster =  bs.sectors_per_cluster * bs.sector_size ;
    //看当前簇能读多少
    uint32_t currClusterSize = bytesPerCluster - _Pos.currentOffset;
    size_t currSize = currClusterSize > _Size ? _Size : (size_t)currClusterSize;
    this->SeekToDateCluster(_Pos);
    int res =  0;
    if(rw == READ)
        res = fread(_Buffer, currSize, 1, this->fp);
    else
        res = fwrite(_Buffer, currSize, 1, this->fp);
    size -= res;
    if (res < currSize || size == 0)
    {
        return (uint32_t)res;
    }

    do {
        _Pos = this->NextPosition(_Pos, currClusterSize);
        if((_Pos.currentCluster == 0))
        {
            break;
        }
        this->SeekToDateCluster(_Pos);
        size_t sizeToOp = size > bytesPerCluster ? bytesPerCluster :size;
        if(rw == READ)
            res = fread(_Buffer, sizeToOp, 1,  this->fp);
        else
            res = fwrite(_Buffer, sizeToOp, 1,  this->fp);
        if (res > 0)
            size -= sizeToOp;
        else
            break;

        if (size == 0)
            break;
    } while (1);
    return (uint32_t)_Size - size;
}

uint32_t Fat32::WriteCluster(Fat32Position _Pos, size_t _Size, void *_Buffer) {
    return this->ReadWriteCluster(_Pos, _Size, _Buffer, WRITE);
}
uint32_t Fat32::ReadCluster(Fat32Position _Pos, size_t _Size, void* _Buffer)
{
    return this->ReadWriteCluster(_Pos, _Size, _Buffer, READ);
}

void Fat32::WriteFatItem(uint32_t _ClusterNum, uint32_t value) {
    fseek(this->fp , bs.sector_size*bs.reserved_sectors + (long)_ClusterNum*4  ,  SEEK_SET);
    fwrite(&value ,  4  ,1,this->fp);
}

uint32_t Fat32::ReadFatItem(uint32_t _ClusterNum) {
    uint32_t res;
    fseek(this->fp , bs.sector_size*bs.reserved_sectors + (long)_ClusterNum*4  ,  SEEK_SET);
    fread(&res ,  4  ,1,this->fp);
    return res;
}


Fat32Position Fat32::NextPosition(Fat32Position _Pos, uint32_t _Offset, bool _AutoAlloc)
{
    Fat32Position res = _Pos;
    uint32_t bytesPerCluster = bs.sectors_per_cluster * bs.sector_size;
    find:
    if (_Pos.currentOffset + _Offset >= bytesPerCluster)
    {
        //需要读取下一个cluster
        uint32_t nextCluster = ReadFatItem(_Pos.currentCluster);
        if(nextCluster > 0 &&
                nextCluster < this->totalClusters)
        {
            res.currentCluster = nextCluster;
            res.currentOffset = (_Pos.currentOffset + _Offset) % bytesPerCluster;
        }
        else
        {
            res.currentCluster = 0;
        }
    }
    else
    {
        res.currentOffset += _Offset;
    }
    if(_AutoAlloc)
    {
        uint32_t next = this->FindNextAvailCluster(true);
        if (!next)
        {
            WriteFatItem(_Pos.currentCluster, next);
            _AutoAlloc = false;
            goto find;
        }
    }
    return res;
}

void Fat32::SeekToDateCluster(Fat32Position _Pos)
{
    uint32_t firstDataSector = bs.reserved_sectors + (bs.sectos_per_one_FAT * bs.number_of_fats);
    fseek(this->fp
            , bs.sector_size*
              ((long)firstDataSector + ((long)_Pos.currentCluster-2)*bs.sectors_per_cluster)
              + (long)_Pos.currentOffset
            , SEEK_SET);
}

int Fat32Iterator::Read(void *_Buffer) {
    return fat32->ReadCluster(pos, step, _Buffer);
}

int Fat32Iterator::Write(void *_Buffer)
{
    return 0;
}

bool Fat32Iterator::HasNext() {
    return (fat32->NextPosition(this->pos,this->step).currentCluster != 0);
}

Fat32Iterator &Fat32Iterator::operator+=(size_t _Size) {
    this->pos = fat32->NextPosition(this->pos, _Size * this->step, this->autoAlloc);
    return *this;
}

Fat32Iterator &Fat32Iterator::operator++() {
    return *this += 1;
}

const Fat32Iterator Fat32Iterator::operator++(int) {
    auto tmp = *this;
    *this += 1;
    return tmp;
}

bool Fat32Iterator::IsEnd() {
    return this->pos.currentCluster == 0;
}

Fat32::Fat32(FILE *_Fp) {
    this->fp = _Fp;
    fseek(fp, 0, SEEK_SET);
    fread(&bs, sizeof(BootSector), 1, fp);

    // find the root directory
    uint32_t rootCluster = bs.cluster_number_for_the_root_directory;
    this->rootDir.low_starting_cluster = (unsigned short)(rootCluster & 0xffff);
    this->rootDir.high_starting_cluster = 0;
    this->rootDir.attributes = 0;
    this->rootDir.file_size = 0;
    this->rootDir.attributes |= 0x10;

    uint32_t totalSectors = bs.total_sectors - bs.reserved_sectors - bs.fat_size_sectors*bs.number_of_fats;
    this->totalClusters = totalSectors / bs.sectors_per_cluster;
}

uint32_t Fat32::FindNextAvailCluster(bool _Clean) {
    uint32_t i = 0;
    for (; i < totalClusters ; i++)
    {
        if (this->ReadFatItem(this->lastAvailCluster++) == 0)
        {
            break;
        }
        if (this->lastAvailCluster >= this->totalClusters + 2)
            this->lastAvailCluster %= this->totalClusters;
    }
    if (i >= totalClusters)
    {
        return 0;
    }
    else
    {
        if (_Clean)
        {
            uint32_t size = bs.sectors_per_cluster * bs.sector_size;
            char *tmp = new char[size];
            memset(tmp,0,size);
            this->WriteCluster({this->lastAvailCluster, 0},size,tmp);
            delete tmp;
        }
        return this->lastAvailCluster;
    }
}

bool Fat32::GetDirectoryEntry(lr::sstl::AString _Path, DirectoryEntry *_Root, Fat32Entry &_Res)
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
