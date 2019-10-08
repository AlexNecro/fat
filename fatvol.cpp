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
/*
    TODO:
	    add multi-cluster read (FAT chain traversal)
		split MakeAPL to couple of func to allow following mods
		add file search by path (by long and short filename)
		add function
			<[vol=x:] act=add out=playlist.apl file="e:\mobile\audio\album\Some music file.mp3">
		add *.pls (with absolute paths) to *.apl converter
			<[vol=x:] act=conv [out=playlist.apl] file="playlist.m3u">
*/
#include <algorithm>
#include <string.h>
#ifdef __BORLANDC__
#include <mem.h>
#endif
#pragma hdrstop
#include "fatvol.h"
//---------------------------------------------------------------------------
#define swap16(word) (((word)&0x00ff)<<8)+(((word)&0xff00)>>8)
#define swap32(dword) (((dword)&0x000000ff)<<24)+(((dword)&0x0000ff00)<<8)+(((dword)&0x00ff0000)>>8)+(((dword)&0xff000000)>>24)
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#pragma package(smart_init)
//extern "C" void PrintError(LPTSTR lpszFunction = "main()");

inline bool operator<(const NPlaylistEntry& arg1, const NPlaylistEntry& arg2)
{
	return (strncmp(arg1.Name,arg2.Name,8)<0);
}//inline bool operator<()

inline char*  replace(char *string, int length=8, char chsrc = 0x20, char chdest = 0x00)
{//for fixed-length strs - DESTROYS SOYRCE STRING
	for (int i=0; i < length; i++) {
		if (string[i]==chsrc) {
			string[i] = chdest;
		}
	}
	return string;
}

inline TDirEntry FindDir(const char* name, const NDirVec& dir, int isDir=1, int isFile=0)
{
	for (unsigned int i=0; i < dir.size(); i++) {
		if (!isDir && (dir[i].Attrib&ATTR_DIRECTORY)) {
			continue;
		}
		if (!isFile && !(dir[i].Attrib&ATTR_DIRECTORY)) {
			continue;
		}
		if (dir[i].Attrib == ATTR_LONG_NAME) {
			continue;
		}
		if (dir[i].Name[0]=='.') {
			continue;			
		}
		if (strncmp(dir[i].Name,name,8)==0) {
		#ifdef _DEBUG
			printf("+[%.8s] / [%s]    CATCH\n",dir[i].Name, name);
		#endif
			return dir[i];
		}
		#ifdef _DEBUG
		printf("-[%.8s] / [%s]\n",dir[i].Name, name);
		#endif
	}
	TDirEntry dir0;
	memset(&dir0,0,sizeof(dir0));
	return dir0;
}

NFATReader::NFATReader(const std::string& vol)
{
	FatType = 0;
	unsigned long nRead;
	path = vol;
	hDrive = fopen(path.c_str(),"rb");
	if (hDrive==NULL) {
        printf("Failed to open volume\n");
		return;
	} else printf("Volume opened\n");
	// Attempt a synchronous read operation.
	nRead = fread(&boot,sizeof(boot),1,hDrive);
	if (nRead != 1)
	{
		// This is the end of the file.
		printf("Bootsector cannot be read\n");
	} else {
		//determine FAT version:
		if (boot.SectorSize>0) {
			RootDirSectors = ((boot.RootDirSize * 32) + (boot.SectorSize - 1)) / boot.SectorSize;
			DataSectors = max(boot.SectorCount32, boot.SectorCount16) -
				(boot.ReservedSectors+(boot.FATCount*boot.FATSize)+RootDirSectors);
			ClusterCount = DataSectors/boot.ClusterSize;
			FirstRootDirSector = boot.ReservedSectors + (boot.FATCount * boot.FATSize);
			FirstDataSector = FirstRootDirSector + boot.RootDirSize*sizeof(TDirEntry)/boot.SectorSize;
			if(ClusterCount < 4085) {
				FatType = FAT12;
				printf("Detected FS type: FAT12");
			} else if(ClusterCount < 65525) {
				FatType = FAT16;
				printf("Detected FS type: FAT16");
			} else {
				FatType = FAT32;
				printf("Detected FS type: FAT32");
			}
			printf("\nStored FS ID: [%.8s]\nOEM: [%.8s]\nLabel: [%.11s]\nCluster size: %d\nRootDir size: %d\nTotal Size: %d\nFAT size: %d\nReserved sectors: %d\n",
				boot.IDFS,
				boot.OEM,
				boot.VolumeLabel,
				boot.ClusterSize*boot.SectorSize,
				boot.RootDirSize,
				max(boot.SectorCount32,boot.SectorCount16)*boot.SectorSize,
				boot.FATSize,
				boot.ReservedSectors);
			BlockSize = boot.ClusterSize*boot.SectorSize;
			//read FAT:
			fat16 = new unsigned short[boot.FATSize*boot.SectorSize];
			unsigned short *ptr = fat16;
			for (int i = 0; i < boot.FATSize; i++) {
				ReadSector(boot.ReservedSectors+i,ptr);
				ptr+=boot.SectorSize;
			}
		} else {
			FatType = FATUNKNOWN;
			BlockSize = 512;
			printf("FAT type cannot be detected!\nStored FS ID: [%.8s]\nOEM: [%.8s]\nLabel: [%.11s]\nSector size: %u\n",boot.IDFS,boot.OEM,boot.VolumeLabel,boot.SectorSize);
		}
    }
}//NFATReader::NFATReader()

NFATReader::~NFATReader()
{
    fclose(hDrive);
	if (fat16) delete[] fat16;
}//NFATReader::~NFATReader()

unsigned long	NFATReader::SaveBoot(const std::string& out)
{
    if (hDrive == NULL)
	{
		printf("Volume is not opened\n",out.c_str());
		return 0;
	}
	printf("Saving boot sector...\n");
	FILE* hImage = fopen(out.c_str(),"wb");
	if (hImage == NULL)
	{
		printf("Cannot create %s\n",out.c_str());
		fclose(hImage);
		return (0);
	} else printf("Created image file: %s\n",out.c_str());
	unsigned long nRead = fwrite(&boot,sizeof(boot),1,hImage);
	if (nRead==0)
		printf("Write error\n");
	else
	   printf("\nSomething seems to be written...\n");
	fclose(hImage);
	return nRead;
}//unsigned long	NFATReader::SaveBoot()

unsigned long 	NFATReader::SaveImage(const std::string& out)
{
    if (hDrive == NULL)
	{
		printf("Volume is not opened\n",out.c_str());
		return 0;
	}
	printf("Saving raw image...\n");
	FILE* hImage = fopen(out.c_str(),"wb");
	if (hImage == NULL)
	{
		printf("Cannot create %s\n",out.c_str());
		fclose(hImage);
		return (0);
	} else printf("Created image file: %s\n",out.c_str());
	
	unsigned long nRead;
    fseek(hDrive,0,SEEK_SET);
	unsigned long TotalRead=0;
	unsigned long TotalWritten=0;
	unsigned long BlockCount;
	if (FatType == FAT12 || FatType == FAT16 || FatType == FAT32) {
		BlockCount = max(boot.SectorCount32,boot.SectorCount16)/boot.ClusterSize;
		printf("Performing cluster transfer (%u blocks of %u bytes, %u bytes total)...\n",
		BlockCount,
		BlockSize,
		BlockCount*BlockSize);
	} else {
		BlockCount = MaxBlocks;//it may work
		printf("Performing block transfer (%u blocks of %u bytes, %u bytes total)...\n",
		BlockCount,
		BlockSize,
		BlockCount*BlockSize);
	}
	char	*buffer = new char[BlockSize];
	for (unsigned long i = 0; i < BlockCount; i++) {
		nRead = fread(buffer,BlockSize,1,hDrive)*BlockSize;
		TotalRead+=nRead;
		if (nRead<1)
		{
			printf("\nEnd of volume or read error\n");
			delete[] buffer;
			fclose(hImage);
			return 0;
		}
		nRead = fwrite(buffer,BlockSize,1,hImage)*BlockSize;
		TotalWritten+=nRead;
		if (nRead<1)
		{
			printf("\nWrite error\n");
			delete[] buffer;
			fclose(hImage);
			return 0;
		}
		if (i%((unsigned long)(BlockCount/1024))==0) {
			printf("* %u/%u, r:%u, w:%u\r", i, BlockCount, TotalRead, TotalWritten);
		}
	}
	printf("\nSomething seems to be written...\n");
	delete[] buffer;
	fclose(hImage);
	return BlockCount;
}//unsigned long 	NFATReader::SaveImage()

unsigned long	NFATReader::ReadSector(unsigned long offs, void* pBuf)
{
    if (hDrive == NULL)
	{
		printf("Volume is not opened\n",out.c_str());
		return 0;
	}
	if (FatType!=FAT16) {

		return 0;
	}
	fseek(hDrive,offs*boot.SectorSize,SEEK_SET);
	unsigned long nRead = fread(pBuf,boot.SectorSize,1,hDrive);
	if (!nRead)
	{
		printf("Read error: ");		
	}
	return nRead*boot.SectorSize;
}//unsigned long	NFATReader::ReadSector()

std::vector<TDirEntry>   NFATReader::ReadDirectory(unsigned long FirstSector, unsigned char attr, int showdeleted)
{
	if (FirstSector==0)
	{
		#ifdef _DEBUG
		printf("Reading root dir at %x\n",FirstRootDirSector*boot.SectorSize);
		#endif
		FirstSector = FirstRootDirSector;
	} else {
		#ifdef _DEBUG
		printf("Reading dir at %x\n",FirstSector*boot.SectorSize);
		#endif
	}
	unsigned long nSectors = boot.ClusterSize;//maximum sectors we can read
	char *buffer = new char[boot.SectorSize];
	TDirEntry *pos;
	std::vector<TDirEntry> dir;
	for (unsigned long i=0; i < nSectors; i++) {
		ReadSector(FirstSector+i,buffer);
		pos = (TDirEntry*)buffer;
		pos--;
		for (unsigned short j=0; j < boot.SectorSize/sizeof(TDirEntry); j++) {
			pos++;
				#ifdef _DEBUG
				if (pos->Attrib == ATTR_LONG_NAME) {
					printf("[LFN %02d] %.10s%.12s%.4s  [%X]==[%X]\n", ((((TDirNameEntry*)pos)->Ord|0x40) - 0x40), ((TDirNameEntry*)pos)->NamePart1, ((TDirNameEntry*)pos)->NamePart2, ((TDirNameEntry*)pos)->NamePart3, (pos->Attrib&attr),attr);
				}  else {
					printf("[%c%c%c%c%c%c] [%c%c%c%c%c%c%c%c] %.3s (%10d b) [%04x] [%X]==[%X] [%X]\n",
						(pos->Attrib & ATTR_READ_ONLY)?('R'):(' '),
						(pos->Attrib & ATTR_HIDDEN)?('H'):(' '),
						(pos->Attrib & ATTR_SYSTEM)?('S'):(' '),
						(pos->Attrib & ATTR_VOLUME_ID)?('V'):(' '),
						(pos->Attrib & ATTR_DIRECTORY)?('D'):(' '),
						(pos->Attrib & ATTR_ARCHIVE)?('A'):(' '),
						pos->Name[0]-u2s,
						pos->Name[1]-u2s,
						pos->Name[2]-u2s,
						pos->Name[3]-u2s,
						pos->Name[4]-u2s,
						pos->Name[5]-u2s,
						pos->Name[6]-u2s,
						pos->Name[7]-u2s,
						pos->Ext, pos->FileSize,
						pos->FirstClusterLo,
						pos->Attrib&attr,attr, ((pos->Attrib&attr)==attr));
				}
				#endif
			if (pos->Name[0] == DIR_END) {
				#ifdef _DEBUG
				printf("[EOF, %u]\n",dir.size());
				#endif
				break;
			}
			if ((pos->Attrib&attr)==attr) {
				if (!showdeleted && pos->Name[0] == DIR_DELETED) {
					continue;
				}
				dir.push_back(*pos);
			}  else {
				if (pos->Name[0] == DIR_DELETED) {
					continue;
				}
			}    
		}
		if (pos->Name[0] == DIR_END) {
			break;
		}
	}
	if (pos->Name[0] != DIR_END) {
    	printf("*** Current directory was not end with EOF marker, so, it spreads to the next cluster\n");
	}
	delete[] buffer;
	return dir;
}//unsigned long   NFATReader::ReadDirectory()

int	NFATReader::ReadMusicFolder(const std::string& dirname, unsigned long FirstSector)
{//recursive; autosaves
#ifdef _DEBUG
	printf("[RMF] entering [%s]\n", dirname.c_str());
#endif
	NPlaylist	playlist;
	NDirVec dir = ReadDirectory(FirstSector);
	unsigned short lfn=0;//last LFN index
	for (unsigned long i=0; i < dir.size(); i++) {
		if ((dir[i].Attrib&ATTR_LONG_NAME) == ATTR_LONG_NAME) {
			if (!lfn) {
				//new last LFN
				//printf("[RMF|%3x] LFN \n", i);
				lfn = i;
			}
			continue;
		};
		if ((dir[i].Attrib&ATTR_DIRECTORY) == ATTR_DIRECTORY) {
			if (dir[i].Name[0] != '.')
				ReadMusicFolder(replace(dir[i].Name), GetDataSectByDir(dir[i].FirstClusterLo));
			//printf("[RMF|%3x] FOLDER: [%.8s] [%x]\n", i,dir[i].Name, dir[i].FirstClusterLo);
			lfn = 0;
			continue;
		}
		//last possibility is a file
//		printf("[RMF|%3x] FILE:   [%.8s] LFN=%x\n", i,dir[i].Name, lfn);
		if (dir[i].Ext[0]=='M' && dir[i].Ext[1]=='P') {
			playlist.AddEntry(replace(dir[i].Name), lfn%(boot.SectorSize/sizeof(TDirEntry)), FirstSector + lfn/(unsigned short)(boot.SectorSize/sizeof(TDirEntry)));
		}
		lfn = 0;
	}
	char Name[9]="\x0\x0\x0\x0\x0\x0\x0\x0";	
	for (int i=0; i < min(9,dirname.length()); i++) {
		Name[i] = dirname[i];
	}
	//Name[8] = '\x0';
	return playlist.SaveToFile(std::string(replace(Name))+".apl");
}//int	NFATReader::ReadMusicFolder()

int NFATReader::MakeAPL(const std::string& out)
{//out is an output folder for playlists, input are always /mobile/audio/
	if (FatType!=FAT16) {
		printf("Only FAT16 supported for playlist creation\n");
		return 0;
	}
	printf("Making playlists...\n");
	NDirVec dir = ReadDirectory();
	TDirEntry entry = FindDir("MOBILE  ", dir);
	if (!entry.FirstClusterLo)
	{
		printf("Can't find folder MOBILE\n");
		return 0;
	}
	#ifdef _DEBUG
	printf("[%.8s] at %d / %x\n",entry.Name, entry.FirstClusterLo, boot.SectorSize*GetDataSectByDir(entry.FirstClusterLo));
	#endif
	dir = ReadDirectory(GetDataSectByDir(entry.FirstClusterLo));

	entry = FindDir("AUDIO   ", dir);
	if (!entry.FirstClusterLo)
	{
		printf("Can't find folder AUDIO\n");
		return 0;
	}
	#ifdef _DEBUG
	printf("[%.8s] at %d / %x\n",entry.Name, entry.FirstClusterLo, boot.SectorSize*GetDataSectByDir(entry.FirstClusterLo));
	#endif
	//now, as we in /mobile/audio/, we need a bit of recursion to walk throught all folders
	ReadMusicFolder(replace(entry.Name), GetDataSectByDir(entry.FirstClusterLo));
	printf("Playlists seem to be good...\n");
	return 1;
}//int NFATReader::MakeAPL()

NPlaylistEntry::NPlaylistEntry(const char *_name, unsigned short _lastlfn, unsigned long _cursector)
{
	for (int i=0; i < 9; i++) {
		Name[i] = _name[i];
	}
	Name[8] = '\x0';
	replace(Name);
	LastLFN = _lastlfn;
	CurrentSector = _cursector;
}//NPlaylistEntry::NPlaylistEntry()

int NPlaylist::AddEntry(const char *name, unsigned short LastLFN, unsigned long CurrentSector)
{
	pls.push_back(NPlaylistEntry(name,LastLFN, CurrentSector));
	return 1;
}//NPlaylist::AddEntry()

int NPlaylist::SaveToFile(const std::string& filename)
{
	unsigned short word = pls.size();
	unsigned long  dword = 0ul;
	if (word<1) {
		printf("! playlist [%s] is empty !\n", filename.c_str());
		return 0;
	}
	FILE* f=fopen(filename.c_str(),"wb");
	if (!f) {
		printf("! can't open [%s] !\n", filename.c_str());
		return 0;
	}
	Sort();
	word = swap16(word);
	fwrite(&word,2,1,f);//size
	word = 0x0001;
	word = swap16(word);
	fwrite(&word,2,1,f);//delay
	dword = swap32(dword);
	fwrite(&dword,4,1,f);//reserved
	for (unsigned int i = 0; i < pls.size(); i++) {
		word = 0x0140;
		word = swap16(word);
		fwrite(&word,2,1,f);//media type
		word = pls[i].LastLFN;
		word = swap16(word);
		fwrite(&word,2,1,f);//lfn index offset
		dword = pls[i].CurrentSector;
		dword = swap32(dword);
		fwrite(&dword,4,1,f);//curent sector of lfn
	}
	fclose(f);
	printf("Playlist [%s] written with %d entries\n", filename.c_str(), pls.size());
	return 1;
}//NPlaylist::SaveToFile()

void NPlaylist::Sort()
{
#ifdef _DEBUG
	printf("SORT-IN\n");
	for (unsigned i=0;i<pls.size();i++)
		printf("%.8s\n",pls[i].Name);
#endif
	sort(pls.begin (), pls.end());
#ifdef _DEBUG
	printf("SORT-OUT\n");
	for (unsigned i=0;i<pls.size();i++)
		printf("%.8s\n",pls[i].Name);
#endif
}//NPlaylist::Sort()
