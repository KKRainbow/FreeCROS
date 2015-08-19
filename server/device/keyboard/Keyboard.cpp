#include <stl/sstl/sstl_vector.h>
#include"stdio.h"
#include"stl/sstring.h"
using namespace lr::sstl;

#define endl '\n'
class ostream {
public:
    friend ostream& operator<<(ostream& o,AString& i);
    friend ostream& operator<<(ostream& o,const char* i);
    friend ostream& operator<<(ostream& o,int i);
    friend ostream& operator<<(ostream& o,char i);
    friend ostream& operator<<(ostream& o,unsigned int i);
}cout;
ostream& operator<<(ostream& o,int i)
{
    printf("%d",i);
    return o;
}
ostream& operator<<(ostream& o,unsigned int i)
{
    printf("%d",i);
    return o;
}
ostream& operator<<(ostream& o,char i)
{
    printf("%c",i);
    return o;
}
ostream& operator<<(ostream& o,const char* i)
{
    printf("%s",i);
    return o;
}
ostream& operator<<(ostream& o,AString& i)
{
    char* tmp = new char[i.Length()];
    printf("%s",i.CStr(tmp));
    delete tmp;
    return o;
}
class istream {
public:
    friend istream& operator>>(istream& o, AString& i);
}cin;
istream& operator>>(istream& o, AString& i)
{
    char buf[512];
    scanf("%s",buf);
    i = buf;
    return o;
}
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
    AString filename;
}  Fat32Entry  ;


int byteArrayToInt(unsigned char b[]) {

    return (b[0] & 0xFF) 		|
           (b[1] & 0xFF) << 8 	|
           (b[2] & 0xFF) << 16 |
           (b[3] & 0xFF) << 24	;

}
short byteArrayToShort(unsigned char b[]){
    return (short) ((b[0] & 0xFF) 	|
                    (b[1] & 0xFF) << 8) ;
}
void info(BootSector boot_sector){
    cout << boot_sector.sector_size << endl;
    cout << boot_sector.oem_name <<endl;
    cout << int(boot_sector.sectors_per_cluster) << endl;
    cout << boot_sector.reserved_sectors << endl;
    cout << int(boot_sector.number_of_fats) << endl;
    cout << boot_sector.sectos_per_one_FAT << endl;
    cout << boot_sector.total_sectors << endl;
}

AString getFilename( DirectoryEntry Dentries){
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
void loadEntries(BootSector bs , FILE* fat32img , Vector<Fat32Entry> &DEntries, unsigned int CurrentCluster  ){

    DEntries.Clear();

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

        if(readLongFile == true){
            temp = getFilename(directory_entry);
            name = temp + name;
            if((directory_entry.filename[0] & 0x41) == 0x41 || directory_entry.filename[0] == 0x01){
                readLongFile = false;

            }
        }
        else{
            fat32entry.filename = name;
            fat32entry.dirEntry = directory_entry;
            DEntries.PushBack(fat32entry);
            readLongFile = true;
            name = AString::Empty;
            temp = AString::Empty;

        }

        count++;
    }
}


int keyboard() {

    FILE* fat32img = fopen("/dev/hda1","rb");
    AString	command,dir,entry,file;
    char cbuf[100];

    Vector<unsigned int> directoryStack;
    BootSector bs;

    fseek(fat32img, 0, SEEK_SET);
    fread(&bs, sizeof(BootSector), 1, fat32img);

    // find the root directory

    unsigned int CurrentCluster = bs.cluster_number_for_the_root_directory;
    Vector<Fat32Entry> Dentries;
    loadEntries(bs,fat32img,Dentries,CurrentCluster);
    directoryStack.PushBack(CurrentCluster);

    Vector<AString> directory ;

    unsigned int i;
    AString arg;

    while(1){

        if(directory.Size() == 0){
            cout << "/> ";
        }
        else if(directory.Size() == 1){
            cout << "/" << directory[0] << "> ";
        }
        else{
            cout << ".../" << directory[directory.Size()-1] << "> ";
        }

        cin >> command;
        Vector<AString> tmpv;
        command.Split(tmpv,' ');
        command = tmpv[0];
        if(tmpv.Size() > 1)
        {
            arg = tmpv[1];
        }


        if(command == "quit"){
            return 0;
        }
        else if(command == "info"){
            cout<<"info is"<<endl;
            info(bs);
        }
        else if(command == "ls"){
            for(unsigned int i=0;i<Dentries.Size();i=i+1){

                if(Dentries[i].dirEntry.attributes & 0x10){
                    cout << "d " << Dentries[i].filename << endl;
                }
                else if(Dentries[i].dirEntry.attributes & 0x20) {
                    cout << "- " << Dentries[i].filename << endl;
                }
            }
        }
        else if(command == "pwd"){
            if(directory.Size() == 0){
                cout << "/" << endl;
                continue;
            }
            for(unsigned int i=0;i<directory.Size();i++){
                cout << "/" << directory[i] ;
            }
            cout << endl;
        }
        else if(command == "cd"){
            entry = arg;
            if(entry == ".."){
                if(directory.Size() == 0){
                    continue;
                }
                directory.PopBack();
                directoryStack.PopBack();
                loadEntries(bs,fat32img,Dentries,directoryStack[directoryStack.Size()-1]);
                continue;
            }
            if(entry == ".")
                continue;

            bool found = false;
            for(i=0;i<Dentries.Size();i=i+1){
                if ( (entry == ( Dentries[i].filename))  && Dentries[i].dirEntry.attributes == 0x10 ){
                    unsigned int nextCluster = Dentries[i].dirEntry.high_starting_cluster*256*256 + Dentries[i].dirEntry.low_starting_cluster;
                    directoryStack.PushBack(nextCluster);
                    loadEntries(bs,fat32img,Dentries,nextCluster);
                    directory.PushBack(entry);
                    found = true;
                    break;
                }
            }
            if( found == false){
                cout << "No such a directory" <<  endl;
            }
        }
        else if(command == "size"){
            file = arg;
            bool found = false;
            for(i=0;i<Dentries.Size();i=i+1){
                if ( file == ( Dentries[i].filename  ) ){
                    cout << Dentries[i].dirEntry.file_size << endl;
                    found = true;
                    break;
                }
            }
            if(found == false)
                cout << "No such a file" << endl;
        }
        else if(command == "cat"){
            file = arg;
            bool found = false;
            for(i=0;i<Dentries.Size();i=i+1){

                if ( file == ( Dentries[i].filename   )  ){
                    unsigned int FirstDataSector 	=    bs.reserved_sectors + ( bs.number_of_fats * bs.sectos_per_one_FAT) ;
                    unsigned int fileStartCluster = Dentries[i].dirEntry.high_starting_cluster*256*256 + Dentries[i].dirEntry.low_starting_cluster;

                    unsigned int FirstSectorofThisFile  =  (fileStartCluster-2) * bs.sectors_per_cluster  + FirstDataSector;
                    unsigned int FAT_First_Sector = bs.reserved_sectors;
                    unsigned int filesize = Dentries[i].dirEntry.file_size;
                    unsigned int NextCluster;

                    fseek(fat32img,(FirstSectorofThisFile)*bs.sector_size,SEEK_SET);

                    while(1){
                        unsigned char c;
                        for(unsigned i=0;i<filesize;i++){
                            if( i != 0 && i % (bs.sector_size*bs.sectors_per_cluster) == 0){
                                fseek(fat32img , bs.sector_size*FAT_First_Sector + fileStartCluster*4  ,  SEEK_SET);
                                fread(&NextCluster , 4 ,1,fat32img );
                                if(NextCluster > 0 && NextCluster < ( bs.total_sectors - bs.reserved_sectors - bs.fat_size_sectors*bs.number_of_fats)/bs.sectors_per_cluster ){
                                    fseek(fat32img , bs.sector_size*(FirstDataSector + (NextCluster-2)*bs.sectors_per_cluster) ,  SEEK_SET);
                                    fileStartCluster = NextCluster;
                                }
                            }
                            fread(&c,sizeof(c),1,fat32img);
                            printf("%c",c);
                        }
                        break;
                    }
                    found = true;
                    cout << endl;
                    break;
                }
            }
            if(found == false)
                cout << "No such a file" << endl;

        }
        else if(command == "crdt"){
            bool found = false;
            entry = arg;
            for(i=0;i<Dentries.Size();i=i+1){
                if ( entry == ( Dentries[i].filename )  ){
                    printf("%04d.%02d.%02d %02d:%02d\n",
                           1980 + (Dentries[i].dirEntry.created_day >> 9), (Dentries[i].dirEntry.created_day >> 5) & 0xF, Dentries[i].dirEntry.created_day & 0x1F,
                           (Dentries[i].dirEntry.created_time_hms >> 11), (Dentries[i].dirEntry.created_time_hms >> 5) & 0x3F);
                    found = true;
                    break;
                }
            }
            if(found == false)
                cout << "No such an entry" << endl;


        }
        else if(command == "moddt"){
            entry = arg;
            bool found = false;
            for(i=0;i<Dentries.Size();i=i+1){
                if ( entry == ( Dentries[i].filename    ) ){
                    printf("%04d.%02d.%02d %02d:%02d\n",
                           1980 + (Dentries[i].dirEntry.modify_date >> 9), (Dentries[i].dirEntry.modify_date >> 5) & 0xF, Dentries[i].dirEntry.modify_date & 0x1F,
                           (Dentries[i].dirEntry.modify_time >> 11), (Dentries[i].dirEntry.modify_time >> 5) & 0x3F);
                    found = true;
                    break;
                }
            }
            if(found == false)
                cout << "No such an entry" << endl;
        }
    }
    return 0;
}
