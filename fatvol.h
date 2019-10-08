/*
* Copyright (c) 2010, Alexey "Necro" Kurdyukov
*
* Разрешается повторное распространение и использование как в виде исходного
* кода, так и в двоичной форме, с изменениями или без, при соблюдении
* следующих условий:
*
*     * При повторном распространении исходного кода должно оставаться
*       указанное выше уведомление об авторском праве, этот список условий и
*       последующий отказ от гарантий.
*     * При повторном распространении двоичного кода должна сохраняться
*       указанная выше информация об авторском праве, этот список условий и
*       последующий отказ от гарантий в документации и/или в других
*       материалах, поставляемых при распространении.
*     * Ни название программы, ни имя её разработчика не могут быть
*       использованы в качестве поддержки или продвижения продуктов,
*       основанных на этом ПО без предварительного письменного разрешения. 
*
* ЭТА ПРОГРАММА ПРЕДОСТАВЛЕНА ВЛАДЕЛЬЦАМИ АВТОРСКИХ ПРАВ И/ИЛИ ДРУГИМИ
* СТОРОНАМИ "КАК ОНА ЕСТЬ" БЕЗ КАКОГО-ЛИБО ВИДА ГАРАНТИЙ, ВЫРАЖЕННЫХ ЯВНО
* ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ, ПОДРАЗУМЕВАЕМЫЕ
* ГАРАНТИИ КОММЕРЧЕСКОЙ ЦЕННОСТИ И ПРИГОДНОСТИ ДЛЯ КОНКРЕТНОЙ ЦЕЛИ. НИ В
* КОЕМ СЛУЧАЕ, ЕСЛИ НЕ ТРЕБУЕТСЯ СООТВЕТСТВУЮЩИМ ЗАКОНОМ, ИЛИ НЕ УСТАНОВЛЕНО
* В УСТНОЙ ФОРМЕ, НИ ОДИН ВЛАДЕЛЕЦ АВТОРСКИХ ПРАВ И НИ ОДНО  ДРУГОЕ ЛИЦО,
* КОТОРОЕ МОЖЕТ ИЗМЕНЯТЬ И/ИЛИ ПОВТОРНО РАСПРОСТРАНЯТЬ ПРОГРАММУ, КАК БЫЛО
* СКАЗАНО ВЫШЕ, НЕ НЕСЁТ ОТВЕТСТВЕННОСТИ, ВКЛЮЧАЯ ЛЮБЫЕ ОБЩИЕ, СЛУЧАЙНЫЕ,
* СПЕЦИАЛЬНЫЕ ИЛИ ПОСЛЕДОВАВШИЕ УБЫТКИ, ВСЛЕДСТВИЕ ИСПОЛЬЗОВАНИЯ ИЛИ
* НЕВОЗМОЖНОСТИ ИСПОЛЬЗОВАНИЯ ПРОГРАММЫ (ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ
* ПОТЕРЕЙ ДАННЫХ, ИЛИ ДАННЫМИ, СТАВШИМИ НЕПРАВИЛЬНЫМИ, ИЛИ ПОТЕРЯМИ
* ПРИНЕСЕННЫМИ ИЗ-ЗА ВАС ИЛИ ТРЕТЬИХ ЛИЦ, ИЛИ ОТКАЗОМ ПРОГРАММЫ РАБОТАТЬ
* СОВМЕСТНО С ДРУГИМИ ПРОГРАММАМИ), ДАЖЕ ЕСЛИ ТАКОЙ ВЛАДЕЛЕЦ ИЛИ ДРУГОЕ
* ЛИЦО БЫЛИ ИЗВЕЩЕНЫ О ВОЗМОЖНОСТИ ТАКИХ УБЫТКОВ.
*/
//---------------------------------------------------------------------------
//
// Necro, 25.01.2k10
//---------------------------------------------------------------------------
#ifndef fatvolH
#define fatvolH
//---------------------------------------------------------------------------
//#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <limits.h>
//---------------------------------------------------------------------------
const unsigned char ATTR_READ_ONLY = 0x01;
const unsigned char ATTR_HIDDEN = 0x02;
const unsigned char ATTR_SYSTEM = 0x04;
const unsigned char ATTR_VOLUME_ID = 0x08;
const unsigned char ATTR_DIRECTORY = 0x10;
const unsigned char ATTR_ARCHIVE = 0x20;
const unsigned char ATTR_LONG_NAME = ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID;
#define u2s		0x0
#define	DIR_DELETED (0xe5-0x80)
#define DIR_END (0x0)
const unsigned char FAT12 = 12;
const unsigned char FAT16 = 16;
const unsigned char FAT32 = 32;
const unsigned char FATUNKNOWN = 200;

//const unsigned long MaxBlocks = 32*1024*1024*1024/512;//32GiB, 67108864 512-byte sectors
const unsigned long MaxBlocks = ULONG_MAX/512;//maximum number of blocks try to read

#pragma pack(1)
struct TBootSector16
{
	//common to all FAT:
	unsigned char	_jmp[3]; //jump to boot code
	char			OEM[8]; //ASCII OEM-name
	unsigned short	SectorSize; //bytes per sector
	unsigned char	ClusterSize; //cluster size, in sectors
	unsigned short	ReservedSectors; // reserved area size, sectors
	unsigned char	FATCount; // number of FATs
	unsigned short	RootDirSize; // number of entries in root dir
	unsigned short	SectorCount16; // number of sectors on the volume
	unsigned char	MediaType; //0xf8 = HDD, 0xf0=portable
	unsigned short	FATSize; //size of each FAT, sectors
	unsigned short	SectorsPerTrack; //number of sectors per track
	unsigned short	HeadCount; //number of heads in device
	unsigned long	HiddenSectors; //number of sectors before partition
	unsigned long	SectorCount32; // number of sectors on the volume ???
	//specific to FAT12/FAT16:
	unsigned char	BIOSNumber; //BIOS disk number
	unsigned char	Reserved;
	unsigned char	ExtendedSignature; //=0x29 if next 3 fields are valid
	unsigned long	VolumeID; //volume serial number
	char			VolumeLabel[0xB]; //text label
	char			IDFS[8]; //ID of FS, maybe FAT, FAT16, FAT32 or nothing
	unsigned char	BootCode[450]; //executable code
};

struct TDirEntry
{
	char			Name[8];//filename
	char			Ext[3];//fileext
	unsigned char	Attrib; //attributes
	unsigned char	NTReserved; //WindowsNT reserved field
	unsigned char	CreationTenth; // tenthseconds of file datetime
	unsigned short	CreationTime; //creation time
	unsigned short	CreationDate; //creation date
	unsigned short	AccessedDate; //date file was last accessed
	unsigned short	FirstClusterHi; //number of first cluster, HIWORD (=0 for FAT12/16)
	unsigned short	ModifiedTime; //time file was last modified
	unsigned short	ModifiedDate; //date file was last modified
	unsigned short	FirstClusterLo;//number of first cluster, LOWORD
	unsigned long	FileSize; //itself
};//struct TDirEntry
/*
	if Name[0] = 0xE5, entry is deleted
	if Name[0] = 0x00, entry is empty & last in dir
*/

struct TDirNameEntry
{
	unsigned char	Ord; //sequental number of part of the name; must contain 0x40 for last part
	char			NamePart1[10];//name part
	unsigned char	Attrib; //attributes
	unsigned char	Type; //must be 0
	unsigned char	Checksum; //short filename checksum
	char			NamePart2[12];//name part
	unsigned short	FirstClusterLo;//must be 0
	char			NamePart3[4];//name part
};//struct TDirNameEntry
//---------------------------------------------------------------------------
/*class NDirEntry : public TDirEntry
{
public:
	NDirEntry(){;}
	~NDirEntry(){;}
};//class NDirEntry*/
//---------------------------------------------------------------------------
typedef std::vector<TDirEntry> NDirVec;
//---------------------------------------------------------------------------
class NPlaylistEntry
{
public:
	NPlaylistEntry(const char *_name, unsigned short _lastlfn, unsigned long _cursector);
	~NPlaylistEntry(){;}

	char	Name[9];//null-terminated shortname
	unsigned short LastLFN;
	unsigned long  CurrentSector;
//in-file entry structure:
// 1. [2] memory type (0x0140=SD)
// 2. [2] last LFN index from top of cur dir sector
// 3. [4] cur dir sector
};//class NPlaylistEntry
//---------------------------------------------------------------------------
typedef std::vector<NPlaylistEntry> NPlaylistVec;
//---------------------------------------------------------------------------
class NPlaylist
{
public:
	NPlaylist(){;}
	~NPlaylist(){;}
	int SaveToFile(const std::string& filename);
	//need to add SaveToBuffer() to inject to the FAT16
	int AddEntry(const char *name, unsigned short LastLFN, unsigned long CurrentSector);//name is always 8-byte length!!!!
	//CurrentSector always can be calculated from FirstDirSector & LFN inder
	void Sort();
//header structure:
// 1. [2] entry count
// 2. [2] slideshow delay, always 1
// 3. [4] reserved
protected:
	NPlaylistVec	pls;
};//class NPlaylist
//---------------------------------------------------------------------------
class NFATReader
{
public:
	NFATReader(const std::string& vol);
	~NFATReader();
	unsigned long	SaveBoot(const std::string& out);
	unsigned long 	SaveImage(const std::string& out);
	NDirVec			ReadDirectory(unsigned long FirstSector=0, unsigned char attr=0x0, int showdeleted=0);//what to supply? FAT id, first cluster, name?
	unsigned long	GetFAT(unsigned long clusterid) {return clusterid*2/*FAT16*//boot.SectorSize;}
	//cluster to sector...
	unsigned long 	GetDataSectByDir(unsigned short FirstClusterLo){return FirstDataSector+(FirstClusterLo-2)*boot.ClusterSize;}
	//...and back
	unsigned short 	GetClusterBySect(unsigned long Sect){return (Sect - FirstDataSector)/boot.ClusterSize + 2;}
	unsigned short  GetNextCluster(unsigned short Cluster) {return fat16[Cluster];}
	int				MakeAPL(const std::string& out);//out is an output folder for playlists, input are always /mobile/audio/

	unsigned char	FatType;

protected:
//	unsigned long	ReadCluster(unsigned long offs, void* pBuf);
	unsigned long	ReadSector(unsigned long offs, void* pBuf);
	int				ReadMusicFolder(const std::string& dirname, unsigned long FirstSector);//recursive

	std::string		path;
	std::string		out;
	TBootSector16	boot;
	unsigned long	BlockSize;
//	HANDLE			hDrive;
	FILE*			hDrive;
	unsigned long 	RootDirSectors;
	unsigned long 	DataSectors;
	unsigned long	ClusterCount;
	unsigned long	FirstRootDirSector;
	unsigned long 	FirstDataSector;

	NDirVec			RootDir;
	unsigned short	*fat16;
};//class NFATReader
//---------------------------------------------------------------------------
#endif
