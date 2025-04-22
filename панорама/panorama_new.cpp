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

// диапазон мощности start_power, stop_power
// диапазон частот start, stop
// количество измерений points
void Measure(double start_power = -20, double stop_power = -15, double start = 4, double stop = 13, int points = 764) {
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


	AskError(rsrc, status, buff);

	// настройка параметров измерения
	// Set RBW
	status = viPrintf(rsrc, (ViString)":SENSe:BANDwidth %d\n", 10000); // полоса пропуска


	// Set frequency range values
	// диапазон частот
	status = viPrintf(rsrc, (ViString)"SENSe:FREQuency:STARt %lfGHz;STOP %lfGHz\n", start, stop);

	// Set points count
	// количество точек
	status = viPrintf(rsrc, (ViString)"SENSe:SWEEp:POINts %d\n", points);

	AskError(rsrc, status, buff);
	

	//-------------------- синхронизация и запус измерений 
	// Sync gen parameters
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:PERiod 110us\n");
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:WIDTh 10us\n");

	AskError(rsrc, status, buff);

	// Disable continous sweeping
	status = viPrintf(rsrc, (ViString)"INITiate:CONTinuous OFF\n");
	// Set frequency sweep mode
	status = viPrintf(rsrc, (ViString)":SENSe:SWEep:TYPE LINear\n");
	// Trigger RUN
	/*status = viPrintf(rsrc, (ViString)":INITiate:IMMediate\n"); */// эта строчка кода отвечает за измерение

	AskError(rsrc, status, buff);




	// ----------------------------------------------------- тестовый цикл

	for (int power = start_power; power <= stop_power; power++) {
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


	// Wait for user action
	std::cin.get();

	//// зарываем 
	// Close session
	status = viClose(rsrc);
	// Close resource manager
	viClose(rm);
}

int main()
{
	Measure();
	return 0;
}
