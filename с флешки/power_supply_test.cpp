// PanoramaCppExample.cpp : ���� ���� �������� ������� "main". ����� ���������� � ������������� ���������� ���������.
//

#include <iostream>
//#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include "visa.h"
#include "windows.h"

#include <stdio.h> // ��� snprintf 

#pragma comment(lib, "E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")
//#pragma comment(linker, "/include:E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")

int main()
{
	std::cout << "Hello World!\n";

	ViSession rm, rsrc;
	ViStatus status;

	status = viOpenDefaultRM(&rm);
	char buff[128];
	//char cmd_str[128];
	// Open conection
	//strcpy(cmd_str, "TCPIP::localhost::8888::SOCKET::VNA");
	status = viOpen(rm, (ViString)"TCPIP::169.254.0.254::8888::SOCKET::VNA", VI_EXCLUSIVE_LOCK, 10000, &rsrc); // directly (via MiVISA only) without Micran Instrument Connector	
	//status = viOpen(rm, "TCPIP::localhost::5025::SOCKET", VI_EXCLUSIVE_LOCK, 10000, &rsrc); // via arbitrary VISA library using Micran Instrument Connector

	// ��� �������� ����� ������������
	const char* config_file_path = "C:\\config.sta";

	// ��� ���������� �����
	const char* save_file_path = "C:\\data.s2p";

	char load_command[256];

	// ��������
	int result = snprintf(load_command, sizeof(load_command), "MMEMory:LOAD \"%s\"\n", config_file_path);

	if (result < 0 || result >= sizeof(load_command)) {
		printf("������: ������� ������� ���� � ����� ������������.\n");
		viClose(rsrc);
		viClose(rm);
		std::cin.get();
		return 1;
	}

	// ������� ����� ������������
	status = viPrintf(rsrc, (ViString)load_command);

	if (status != VI_SUCCESS)
	{
		viStatusDesc(rm, status, buff);
		printf("������ ��� �������� ������������: %s\r\n", buff);
		viClose(rsrc);
		viClose(rm);
		std::cin.get();
		return 1;
	}

	if (status != VI_SUCCESS)
	{
		viStatusDesc(rm, status, buff); // Get status description
		printf("viOpen: %s\r\n", buff);
		std::cin.get();
		return 0;
	}
	// Set timeout
	viSetAttribute(rsrc, VI_ATTR_TMO_VALUE, 5000);
	// Send parameters
	// Device ID
	//strcpy(cmd_str, "*IDN?\n");
	status = viQueryf(rsrc, (ViString)"*IDN?\n", (ViString)"%T", buff);
	printf("IDN: %s\r\n", buff);


	// ������� �������� ����� ������������
	int result = snprintf(load_command, sizeof(load_command), "MMEMory:LOAD \"%s\"\n", config_file_path);
	// ������� �������� ����� ������������
	status = viQueryf(rsrc, (ViString)"MMEMory:LOAD \n", (ViString)"%T", buff);



	// ���������� ������
	// Send "Reset" command
	status = viPrintf(rsrc, (ViString)"*RST\r\n");
	// Send "Clear status" command
	status = viPrintf(rsrc, (ViString)"*CLS\r\n");

	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	//// Delete all traces
	//status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DELete:ALL\n");
	//// Create S11 parameter trace
	//status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S11\",S11\n");
	//// Create S22 parameter trace
	//status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S22\",S22\n");
	//// Select trace
	//status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S11\"\n");
	//// Smoothing
	//status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:STATe ON\n");
	//status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:APERture 3\n");
	// Scale parameters
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:PDIVision 15\n");
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RLEVel -20\n");
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RPOSition 6\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);


	ViUInt32 retCnt = 0;


	// Set RBW
	status = viPrintf(rsrc, (ViString)":SENSe:BANDwidth %d\n", 10000);
	// Set power level value
	status = viPrintf(rsrc, (ViString)"SOURce:POWer -10\n");

	/*unsigned int i = 0;
	status = viQueryf(rsrc, "SENS:LIST:FREQ:POIN?\n", "%T", buff);
	printf("SENS:LIST:FREQ:POIN: %s\r\n", buff);
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Set frequency range values
	double start = 40, stop = 20000;
	status = viPrintf(rsrc, (ViString)"SENSe:FREQuency:STARt %lfMHz;STOP %lfMHz\n", start, stop);
	// Set points count
	int points = 201;
	status = viPrintf(rsrc, (ViString)"SENSe:SWEEp:POINts %d\n", points);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Port 1 input attenuator value
	int att = 20;
	status = viPrintf(rsrc, (ViString)"SENSe:POWer:ATTenuator ARECeiver,%d\n", att);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);


	// Sync gen parameters
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:PERiod 110us\n");
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:WIDTh 10us\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Disable continous sweeping
	status = viPrintf(rsrc, (ViString)"INITiate:CONTinuous OFF\n");
	// Set frequency sweep mode
	status = viPrintf(rsrc, (ViString)":SENSe:SWEep:TYPE LINear\n");
	// Trigger RUN
	status = viPrintf(rsrc, (ViString)":INITiate:IMMediate\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Checking Markers
	// Set diagram active
	status = viPrintf(rsrc, (ViString)"DISPlay:WINDow:ACTivate\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Turn on marker
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:STATe ON\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set marker on frequency
	//status = viPrintf(rsrc, ":CALCulate:MARKer0:X 3GHz\n");
	// Set marker search user domain
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:RANGe 1\n"); // set 0 if full span needed
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STARt 1GHz\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STOP 2GHz\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set marker on maximum
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:SELect MAXimum\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);


	// Wait for measurement complete
	status = viQueryf(rsrc, (ViString)"*OPC?\n", (ViString)"%T", buff);


	if (status == VI_SUCCESS)
	{
		printf("Data ready\r\n");
		status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S11\"\n");
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
		// Set data format		
		status = viPrintf(rsrc, (ViString)":FORMat REAL,64\n");
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
		bool bSData = true;
		// Query data
		if (bSData)
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA? SDATA\n");
		else
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA? FDATA\n");

		// Parse IEEE 488.2 binary block	
		int bitSize = 0;
		char head[32];
		if ((status = viRead(rsrc, (ViPBuf)buff, 2, &retCnt)) < 0) // #n - number of bytes of size record
			return 0;
		buff[retCnt] = '\0';
		memcpy(&head[0], &buff, retCnt);
		bitSize = retCnt;
		int numBytes = atoi(buff + 1);
		if (buff[0] != '#' || numBytes == 0)
			return 0;

		if (viRead(rsrc, (ViPBuf)buff, numBytes, &retCnt) < 0) // read data length
			return 0;
		buff[retCnt] = '\0';
		memcpy(&head[0] + bitSize, &buff, retCnt);
		bitSize += retCnt;
		head[bitSize] = '\0';
		int dataSize = atoi(buff);
		if (dataSize == 0)
			return 0;

		double* data = new double[dataSize + 1];
		if (viRead(rsrc, (ViPBuf)data, dataSize + 1, &retCnt) < 0)
			return 0;

		// Writing data to memory trace
		if (!bSData) // supports only formatted data 
		{
			status = viPrintf(rsrc, (ViString)"*CLS\r\n");
			status = viPrintf(rsrc, (ViString)"MMEMory:CREate \"Mem1\",S11\n");
			status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
			printf("Error: %s\r\n", buff);
			status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Mem1\"\n");
			status = viPrintf(rsrc, (ViString)":CALCulate:FORMat MLOGarithmic\n");
			status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
			printf("Error: %s\r\n", buff);
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA FMEM,%s", head);
			((ViPBuf)data)[retCnt - 1] = '\n';
			status = viWrite(rsrc, (ViPBuf)data, retCnt, &retCnt);
		}
		for (int i = 0; i < points; i++)
		{
			if (bSData)
				printf("%g MHz\tRe: %f\tIm: %f\r\n", start + i * (stop - start) / (points - 1), data[i * 2], data[i * 2 + 1]);
			else
				printf("%g MHz\tRe: %f\t\r\n", start + i * (stop - start) / (points - 1), data[i]);
		}
		delete[] data;
	}
	else
	{
		printf("Data not ready. OPC failed\r\n");
		status = viPrintf(rsrc, (ViString)"abort\n");

		// Check errors
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
	}
	// Get marker value
	status = viQueryf(rsrc, (ViString)"CALCulate:MARKer0:Y? \"Trc_S11\"\n", (ViString)"%T", buff);
	printf("Marker #0 value: %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Hide all markers
	status = viPrintf(rsrc, (ViString)"CALCulate:MARKer:AOFF\n");
	// Wait for user action

	printf("Error: %s\r\n", buff);

	// Wait for user action
	std::cin.get();

	// ���������� ����� 

	int save_result = snprintf(save_command, sizeof(save_command), "CALCulate:DATA:SNP:SAVE \"%s\"\n", save_file_path);

	if (save_result < 0 || save_result >= sizeof(save_command)) {
		printf("������: ������� ������� ���� � ����� ����������.\n");
		viClose(rsrc);
		viClose(rm);
		std::cin.get();
		return 1;
	}

	ViStatus save_status = viPrintf(rsrc, (ViString)save_command);

	char buff[128];

	if (save_status != VI_SUCCESS) {
		viStatusDesc(rm, save_status, buff);
		printf("������ ��� �������� ������� ����������: %s\r\n", buff);
		viClose(rsrc);
		viClose(rm);
		std::cin.get();
		return 1;
	}

	// �������� ����� ����������
	ViStatus error_check_status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error after save: %s\r\n", buff);


	status = viClose(rsrc);
	// Close resource manager
	viClose(rm);

	return 0;
}

// ������ ���������: CTRL+F5 ��� ���� "�������" > "������ ��� �������"
// ������� ���������: F5 ��� ���� "�������" > "��������� �������"

// ������ �� ������ ������ 
//   1. � ���� ������������ ������� ����� ��������� ����� � ��������� ���.
//   2. � ���� Team Explorer ����� ������������ � ������� ���������� ��������.
//   3. � ���� "�������� ������" ����� ������������� �������� ������ ������ � ������ ���������.
//   4. � ���� "������ ������" ����� ������������� ������.
//   5. ��������������� �������� ������ ���� "������" > "�������� ����� �������", ����� ������� ����� ����, ��� "������" > "�������� ������������ �������", ����� �������� � ������ ������������ ����� ����.
//   6. ����� ����� ������� ���� ������ �����, �������� ������ ���� "����" > "�������" > "������" � �������� SLN-����.
