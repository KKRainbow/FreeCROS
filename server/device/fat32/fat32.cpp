//
// Created by ssj on 15-8-24.
//

#include <Device.h>
#include <stl/sstl/sstl_idgen.h>
#include "fat32.h"
#include"fsserver/format.h"
#include "stl/sstring.h"
#include "stl/smap.h"
#include "fat32lib.h"

using namespace lr::sstl;

class Fat32Device : public DeviceOperation
{
private:
    Map<ino_t, Fat32Entry> inodeTable;
    IDGenerator<int> idGen;
    AString dir;
    Fat32* fat32;
private:
    AString GetRealPath(AString _Path)
    {
        AString tmp = _Path.Sub(0, dir.Length());
        if (dir == tmp)
        {
            return _Path.Sub(dir.Length() + 1, _Path.Length());
        }
        else
        {
            return AString::Empty;
        }
    }
public:
    Fat32Device(const char* _Dir, const char* _Dev ):idGen(1)
    {
        FILE* fp = fopen(_Dev,"rb");
        fat32 = new Fat32(fp);
        dir = _Dir;
        SysCallMountFs::Invoke((uint32_t)_Dir, (uint32_t)_Dev);
        Fat32Entry entry;
        fat32->MakeDirectory("/makabdafa/a/b/c/d", nullptr,true,entry);
    };
    virtual Message Open(Message &_Msg)
    {
        Message msg;
        char name[1024];
        char mode[20];
        //name在内核空间，当然要这么取0 0
        SysCallReadFromPhisicalAddr::Invoke((uint32_t) name,
                                            (uint32_t)_Msg.content[FsMsg::O_PATH],
                                            (uint32_t)_Msg.content[FsMsg::O_PATH_SIZE]);
        SysCallReadFromPhisicalAddr::Invoke((uint32_t) mode,
                                            (uint32_t)_Msg.content[FsMsg::O_MODE],
                                            (uint32_t)_Msg.content[FsMsg::O_MODE_SIZE]);

        int rootId = _Msg.content[FsMsg::O_ROOTID];
        Fat32Entry* root = nullptr;
        if(rootId > 0)
        {
            auto iter = inodeTable.Find(rootId);
            if(iter == inodeTable.End())
            {
                msg.content[0] = 0;
                return msg;
            }
            root = &iter->second;
        }
        Fat32Entry entry;
        bool res = fat32->GetDirectoryEntry(this->GetRealPath(name), root, entry);

        if(!res)
        {
            msg.content[0] = -1;
            return msg;
        }

        int id = idGen.GetID();
        inodeTable.Insert(MakePair(id, entry));
        msg.content[0] = id;

        return msg;
    }

    virtual Message Read(Message &_Msg)
    {
        Message msg;
        auto iter = inodeTable.Find(_Msg.content[FsMsg::R_FD]);
        if(iter == inodeTable.End())
        {
            msg.content[0] = 0;
            return msg;
        }
        Fat32Entry* root = &iter->second;

        msg.content[0] = fat32->GetContent(root, _Msg.content[FsMsg::R_POS],
                                    _Msg.content[FsMsg::R_SIZE], (char*)_Msg.content[FsMsg::R_BUF]);
        return msg;
    }

    virtual Message Write(Message &_Msg)
    {
        Message msg;
        msg.content[0] = 0;
        return msg;
    }
};


int fat32()
{
    SysCallMkdir::Invoke((uint32_t)"/mnt");
    Fat32Device device("/mnt","/dev/hda1");
    device_loop(device);
    SysCallExit::Invoke();
    return 1;
}
