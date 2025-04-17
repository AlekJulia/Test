#include <iostream>
//#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include "visa.h"
#include "windows.h"

// подключение библиотеки
#pragma comment(lib, "E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")

void AskError(ViSession rsrc, ViSession status, char buff[128]) {
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
}


int main()
{
	// rm - дескриптор ресурса visa, rsrc - дескриптор подключения к прибору
	ViSession rm, rsrc;
	ViStatus status;
	// открываем соединение с системой visa
	status = viOpenDefaultRM(&rm);
	// буфер для хранения текста
	char buff[128];
	// подключаемся к прибору по адресу
	status = viOpen(rm, (ViString)"TCPIP::169.254.0.254::8888::SOCKET::VNA", VI_EXCLUSIVE_LOCK, 10000, &rsrc); // directly (via MiVISA only) without Micran Instrument Connector	

	// если не удалось подключится к устройству, выводим ошибку
	if (status != VI_SUCCESS)
	{
		viStatusDesc(rm, status, buff); // Get status description
		printf("viOpen: %s\r\n", buff);
		std::cin.get();
		return 0;
	}
	// Set timeout
	viSetAttribute(rsrc, VI_ATTR_TMO_VALUE, 5000);
	// Получаем информацию об устройстве
	status = viQueryf(rsrc, (ViString)"*IDN?\n", (ViString)"%T", buff);
	// ответ сохраняется в буфере, который выводим
	printf("IDN: %s\r\n", buff);
	// Send "Reset" command
	status = viPrintf(rsrc, (ViString)"*RST\r\n");
	// Send "Clear status" command 
	status = viPrintf(rsrc, (ViString)"*CLS\r\n");

	AskError(rsrc, status, buff);
	/*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);*/

	//// Delete all traces это надо
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DELete:ALL\n");
	//// Create S11 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S11\",S11\n"); // коэф отражения порт 1
	//// Create S22 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S22\",S22\n");  // коэф отражения порт 2
	//// Create S22 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S21\",S21\n"); // коэф усиления вход выход
	//// Create S22 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S12\",S12\n"); // коэф пропускания выход вход

	// хз надо ли (надо)
	//// Select trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S21\"\n");
	//// Smoothing
	status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:STATe ON\n");
	status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:APERture 3\n");
	////// Scale parameters
	//status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:PDIVision 15\n");
	//status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RLEVel -20\n");
	//status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RPOSition 6\n");

	AskError(rsrc, status, buff);
	/*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Тут сами матиметически настраиваем трассы
	// Может не надо 
	// Math traces
	// Create math trace
	//status = viPrintf(rsrc, (ViString)":CALCulate:MATH:CREate \"Math_New\"\n");
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);
	//// Select math trace
	//status = viPrintf(rsrc, (ViString)"CALCulate:PARameter:SELect \"Math_New\"\n");
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);
	//// Set math trace source 1
	//status = viPrintf(rsrc, (ViString)":CALCulate:MATH:SOURce1 \"Trc_S11\"\n");
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);
	//// Set math trace source 2
	//status = viPrintf(rsrc, (ViString)":CALCulate:MATH:SOURce2 \"Trc_S22\"\n");
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);
	//// Set math trace function
	//status = viPrintf(rsrc, (ViString)":CALCulate:MATH:FUNCtion \"|A|+|B|\"\n");
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);

	//ViUInt32 retCnt = 0; //?


	///////////////////////////////////////////////////////////////////////////////////////////// файл калибровки
	//// Load calibration data
	//status = viPrintf(rsrc, (ViString)":MMEMory:LOAD:CORRection \"E:\\MicranStart\\kal.r4mc\"\n");
	//// Set correction enabled
	//status = viPrintf(rsrc, (ViString)":SENSe:CORRection ON\n");


	// настройка параметров измерения
	// Set RBW
	status = viPrintf(rsrc, (ViString)":SENSe:BANDwidth %d\n", 10000); // полоса пропуска

	// Set power level value
	status = viPrintf(rsrc, (ViString)"SOURce:POWer -10\n"); // мощность

	// Set frequency range values
	double start = 4, stop = 13; // диапазон частот
	status = viPrintf(rsrc, (ViString)"SENSe:FREQuency:STARt %lfGHz;STOP %lfGHz\n", start, stop);

	// Set points count
	int points = 764; // количество точек
	status = viPrintf(rsrc, (ViString)"SENSe:SWEEp:POINts %d\n", points);

	AskError(rsrc, status, buff);
	// Check errors
	/*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);*/

	// аттенюатор скорее всего не надо
	//// Port 1 input attenuator value
	//int att = 20;
	//status = viPrintf(rsrc, (ViString)"SENSe:POWer:ATTenuator ARECeiver,%d\n", att);

	AskError(rsrc, status, buff);
	//// Check errors
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);


	/////////////////////////////////////////////////////////////////////////////////// сохранение профиля
	//// Save data
	//status = viPrintf(rsrc, (ViString)":CALCulate:DATA:SNP:SAVE \"E:\\MicranStart\\kalls.s2p\"\n");

	// Save profile 
	//status = viPrintf(rsrc, (ViString)":MMEMory:STORe:STATe \"d:\\calib.gpr\"\n");
	// Check errors
	/*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);*/


	//-------------------- синхронизация и запус измерений 
	// Sync gen parameters
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:PERiod 110us\n");
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:WIDTh 10us\n");

	AskError(rsrc, status, buff);
	//// Check errors
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);

	// Disable continous sweeping
	status = viPrintf(rsrc, (ViString)"INITiate:CONTinuous OFF\n");
	// Set frequency sweep mode
	status = viPrintf(rsrc, (ViString)":SENSe:SWEep:TYPE LINear\n");
	// Trigger RUN
	/*status = viPrintf(rsrc, (ViString)":INITiate:IMMediate\n"); */// эта строчка кода отвечает за измерение

	AskError(rsrc, status, buff);
	//// Check errors
	//status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);



	// ---------------------- тут какие-то маркеры (могут пригодится для измерения чего-то конкретного)
	//// Checking Markers
	//// Set diagram active
	//status = viPrintf(rsrc, (ViString)"DISPlay:WINDow:ACTivate\n");

	//AskError(rsrc, status, buff);
	////// Check errors
	////status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	////printf("Error: %s\r\n", buff);
	//// Turn on marker
	//status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:STATe ON\n");
	//// Check errors

	//AskError(rsrc, status, buff);
	///*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);*/
	//// Set marker on frequency

	//// Set marker search user domain
	//status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:RANGe 1\n"); // set 0 if full span needed

	//AskError(rsrc, status, buff);
	////// Check errors
	////status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	////printf("Error: %s\r\n", buff);
	//status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STARt 1GHz\n");

	//AskError(rsrc, status, buff);
	//// Check errors
	///*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);*/
	//status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STOP 2GHz\n");

	//AskError(rsrc, status, buff);
	//// Check errors
	///*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);*/
	//// Set marker on maximum
	//status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:SELect MAXimum\n");

	//AskError(rsrc, status, buff);
	//// Check errors
	///*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);*/



	// Wait for measurement complete
/*	status = viQueryf(rsrc, (ViString)"*OPC?\n", (ViString)"%T", buff);*/ // тут ждём, когда закончится измерение

	//----------------------- по идее тут заканчиваются измерения

	//// ..................это вывод в консоль, это нам не надо
	//if (status == VI_SUCCESS)
	//{
	//	printf("Data ready\r\n");
	//	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S11\"\n");
	//	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//	printf("Error: %s\r\n", buff);
	//	// Set data format		
	//	status = viPrintf(rsrc, (ViString)":FORMat REAL,64\n");
	//	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//	printf("Error: %s\r\n", buff);
	//	bool bSData = true;
	//	// Query data
	//	if (bSData)
	//		status = viPrintf(rsrc, (ViString)":CALCulate:DATA? SDATA\n");
	//	else
	//		status = viPrintf(rsrc, (ViString)":CALCulate:DATA? FDATA\n");

	//	// Parse IEEE 488.2 binary block	
	//	int bitSize = 0;
	//	char head[32];
	//	if ((status = viRead(rsrc, (ViPBuf)buff, 2, &retCnt)) < 0) // #n - number of bytes of size record
	//		return 0;
	//	buff[retCnt] = '\0';
	//	memcpy(&head[0], &buff, retCnt);
	//	bitSize = retCnt;
	//	int numBytes = atoi(buff + 1);
	//	if (buff[0] != '#' || numBytes == 0)
	//		return 0;

	//	if (viRead(rsrc, (ViPBuf)buff, numBytes, &retCnt) < 0) // read data length
	//		return 0;
	//	buff[retCnt] = '\0';
	//	memcpy(&head[0] + bitSize, &buff, retCnt);
	//	bitSize += retCnt;
	//	head[bitSize] = '\0';
	//	int dataSize = atoi(buff);
	//	if (dataSize == 0)
	//		return 0;

	//	double* data = new double[dataSize + 1];
	//	if (viRead(rsrc, (ViPBuf)data, dataSize + 1, &retCnt) < 0)
	//		return 0;

	//	// Writing data to memory trace
	//	if (!bSData) // supports only formatted data 
	//	{
	//		status = viPrintf(rsrc, (ViString)"*CLS\r\n");
	//		status = viPrintf(rsrc, (ViString)"MMEMory:CREate \"Mem1\",S11\n");
	//		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//		printf("Error: %s\r\n", buff);
	//		status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Mem1\"\n");
	//		status = viPrintf(rsrc, (ViString)":CALCulate:FORMat MLOGarithmic\n");
	//		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//		printf("Error: %s\r\n", buff);
	//		status = viPrintf(rsrc, (ViString)":CALCulate:DATA FMEM,%s", head);
	//		((ViPBuf)data)[retCnt - 1] = '\n';
	//		status = viWrite(rsrc, (ViPBuf)data, retCnt, &retCnt);
	//	}
	//	for (int i = 0; i < points; i++)
	//	{
	//		if (bSData)
	//			printf("%g MHz\tRe: %f\tIm: %f\r\n", start + i * (stop - start) / (points - 1), data[i * 2], data[i * 2 + 1]);
	//		else
	//			printf("%g MHz\tRe: %f\t\r\n", start + i * (stop - start) / (points - 1), data[i]);
	//	}
	//	delete[] data;
	//}
	//else
	//{
	//	printf("Data not ready. OPC failed\r\n");
	//	status = viPrintf(rsrc, (ViString)"abort\n");

	//	// Check errors
	//	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//	printf("Error: %s\r\n", buff);
	//}
	//// ..................конец вывода в консоль, это нам не надо 

	// Save data
	/*status = viPrintf(rsrc, (ViString)":CALCulate:DATA:SNP:SAVE \"E:\\MicranStart\\kalls_10.s2p\"\n");*/



	// ----------------------------------------------------- тестовый цикл

	for (int power = -20; power <= -15; power++) {
		char filename[64];

		// Установка мощности
		status = viPrintf(rsrc, (ViString)"SOURce:POWer  %d\n", power);

		//// Загрузка файла калибровки (подразумевается, что у всех разные названия)

		// Load calibration data
		status = viPrintf(rsrc, (ViString)":MMEMory:LOAD:CORRection \"E:\\MicranStart\\config\\%d.r4mc\"\n", power);
		// Set correction enabled
		status = viPrintf(rsrc, (ViString)":SENSe:CORRection ON\n");

		// Небольшая задержка (если нужно подождать загрузку калибровки и установку мощности)
		Sleep(500);  // Windows, в миллисекундах // #include <windows.h>


		// Trigger RUN
		status = viPrintf(rsrc, (ViString)":INITiate:IMMediate\n"); // эта строчка кода отвечает за измерение


		// Wait for measurement complete
		status = viQueryf(rsrc, (ViString)"*OPC?\n", (ViString)"%T", buff); // тут ждём, когда закончится измерение

		// Сохранение измерения в файл (подразумевается, что у всех разные названия)
		status = viPrintf(rsrc, (ViString)":CALCulate:DATA:SNP:SAVE \"E:\\MicranStart\\kalls\\kalls_%d.s2p\"\n", power);
	}


	// ----------------------------------------------------- тестовый цикл


	//// тут получаем значение с маркера
	//// Get marker value
	//status = viQueryf(rsrc, (ViString)"CALCulate:MARKer0:Y? \"Trc_S11\"\n", (ViString)"%T", buff);
	//printf("Marker #0 value: %s\r\n", buff);

	//AskError(rsrc, status, buff);
	//// Check errors
	///*status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	//printf("Error: %s\r\n", buff);*/

	//// Hide all markers
	//status = viPrintf(rsrc, (ViString)"CALCulate:MARKer:AOFF\n");

	// это убрать
	// Wait for user action
	/*_getch();
	// Clean up averaging
	status = viPrintf(rsrc, "SENSe:AVERage:CLEar\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Wait for user action
	std::cin.get();

	//// зарываем 
	// Close session
	status = viClose(rsrc);
	// Close resource manager
	viClose(rm);

	return 0;
}
