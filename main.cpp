/*
* Copyright (c) 2010, Alexey "Necro" Kurdyukov
*
* ����������� ��������� ��������������� � ������������� ��� � ���� ���������
* ����, ��� � � �������� �����, � ����������� ��� ���, ��� ����������
* ��������� �������:
*
*     * ��� ��������� ��������������� ��������� ���� ������ ����������
*       ��������� ���� ����������� �� ��������� �����, ���� ������ ������� �
*       ����������� ����� �� ��������.
*     * ��� ��������� ��������������� ��������� ���� ������ �����������
*       ��������� ���� ���������� �� ��������� �����, ���� ������ ������� �
*       ����������� ����� �� �������� � ������������ �/��� � ������
*       ����������, ������������ ��� ���������������.
*     * �� �������� ���������, �� ��� � ������������ �� ����� ����
*       ������������ � �������� ��������� ��� ����������� ���������,
*       ���������� �� ���� �� ��� ���������������� ����������� ����������. 
*
* ��� ��������� ������������� ����������� ��������� ���� �/��� �������
* ��������� "��� ��� ����" ��� ������-���� ���� ��������, ���������� ����
* ��� ���������������, �������, �� �� ������������� ���, ���������������
* �������� ������������ �������� � ����������� ��� ���������� ����. �� �
* ���� ������, ���� �� ��������� ��������������� �������, ��� �� �����������
* � ������ �����, �� ���� �������� ��������� ���� � �� ����  ������ ����,
* ������� ����� �������� �/��� �������� �������������� ���������, ��� ����
* ������� ����, �� ��Ѩ� ���������������, ������� ����� �����, ���������,
* ����������� ��� ������������� ������, ���������� ������������� ���
* ������������� ������������� ��������� (�������, �� �� �������������
* ������� ������, ��� �������, �������� �������������, ��� ��������
* ������������ ��-�� ��� ��� ������� ���, ��� ������� ��������� ��������
* ��������� � ������� �����������), ���� ���� ����� �������� ��� ������
* ���� ���� �������� � ����������� ����� �������.
*/
//Necro, 29.01.2010
//to allow debugging output, use
//#define _DEBUG
//which is set by Borland C++ Builder 10 on debug builds 
//---------------------------------------------------------------------------
//#include <windows.h>
#include <stdio.h>
#pragma hdrstop
#include "cmd.h"
#include "fatvol.h"
//---------------------------------------------------------------------------
//messages, string constants:
char	eInvParams[] = "\nUSAGE: appname.exe vol=x: act={img|boot|apl} out=c:\\MyImage.img";
std::string	cVol = "vol";
std::string	cAct = "act";
std::string	cOut = "out";

std::string drive;
NSettings		cmdline;

#pragma argsused
int main(int argc, char* argv[])
{
  printf("Motorola Phone Playlist Generator\nNecro, 2010 (http://necro.nightmail.ru; necro@nightmail.ru)\nCrossplatform GCC version available\n");
  #ifdef __BORLANDC__
  printf("*This version compiled using Borland C++\n");
  #endif
  printf("\n");
  cmdline.Parse(argc, argv);
  printf("BPB size=%d, dir entry size=%d\n",sizeof(TBootSector16), sizeof(TDirEntry));
  if (cmdline.GetArg(cVol)=="" || cmdline.GetArg(cVol).length()<2) {
	  printf("(vol) %s\n",eInvParams);
	  return 0;
  } else {
    if (cmdline.GetArg(cVol).length()>2) {
		//image
		drive = cmdline.GetArg(cVol);
		printf("Image: %s\n",drive.c_str());
	} else {
		drive = std::string("\\\\.\\")+cmdline.GetArg(cVol);
		printf("Volume: %s\n",drive.c_str());
	}
  }

  if ((cmdline.GetArg(cAct)=="img" || cmdline.GetArg(cAct)=="boot")&& cmdline.GetArg(cOut)=="") {
	  printf("(img/boot) %s\n",eInvParams);
	  return 0;
  } else {
	printf("Output: %s\n",cmdline.GetArg(cOut).c_str());
  }
  NFATReader	*fat = new NFATReader(drive);

  if (cmdline.GetArg(cAct)=="boot") {
	fat->SaveBoot(cmdline.GetArg(cOut));
  }//if boot

  if (cmdline.GetArg(cAct)=="img") {
	fat->SaveImage(cmdline.GetArg(cOut));
  }//if img
  if (cmdline.GetArg(cAct)=="apl") {
	if (fat->FatType!=FAT16) {
		printf("Playlists supported only for FAT16\n");
		return 0;
	}
	fat->MakeAPL(cmdline.GetArg(cOut));
  }
  return 0;
}
//---------------------------------------------------------------------------
