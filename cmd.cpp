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
//---------------------------------------------------------------------------

#pragma hdrstop

//#include "cmd.h"
//#include "stdio.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "cmd.h"
NSettings::NSettings()
{
}

int NSettings::Parse(int argc, char* argv[])
{
	std::string s;
	int	pos;
	for (int i=0; i < argc; i++) {
		s = argv[i];
		pos = s.find('=');
		if (pos!=s.npos) {
			cmdline.insert(std::pair<std::string, std::string>(s.substr(0,pos),s.substr(pos+1,s.length()-pos)));
		}
	}
	return cmdline.size();
}

NSettings::~NSettings()
{

}

std::string NSettings::GetArg(std::string& name)
{
	return cmdline[name];
}

void NSettings::SetArg(std::string& name, std::string& value)
{
    cmdline[name]=value;
}