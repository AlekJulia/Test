#include "visa.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream> 
#include <iomanip>

// Функция проверки ошибок выполнения VISA-функций
void checkError(ViStatus errorStatus, ViSession viSession, const char* operation) {
	if (errorStatus < VI_SUCCESS) {
		char errMessage[256];
		ViStatus descStatus = viStatusDesc(viSession, errorStatus, errMessage);
		if (descStatus == VI_SUCCESS) {
			printf("Error during operation %s: %d\t%s\n", operation, errorStatus, errMessage);
		}
		else {
			printf("Error: %d\t(Unable to retrieve error description)\n", errorStatus);
		}
		exit(EXIT_FAILURE);// Завершить программу
	}
}

// Функция отправки команды в прибор
void sendCommand(ViSession instrument, const char* command) {
	ViStatus status = viPrintf(instrument, (ViPRsrc)command); //Отправка
	checkError(status, instrument, "viPrintf"); //Проверка ошибки
}

// Функция чтения ответа от прибора
std::string readResponse(ViSession instrument) {
	ViChar buffer[256] = { 0 };
	ViStatus status = viScanf(instrument, (ViPRsrc)"%t", &buffer);// Чтение ответа
	checkError(status, instrument, "viScanf");// Проверка ошибки
	return std::string(buffer); // Возврат прочитанной строки
}

// Функция измерения напряжения на выходе прибора
float getVoltage(ViSession instrument, bool output) {
	if (output == 0) {
		sendCommand(instrument, "inst:sel out1\n");
	}
	else if (output == 1) {
		sendCommand(instrument, "inst:sel out2\n");
	}
	sendCommand(instrument, "MEAS:VOLT?\n");
	std::string response = readResponse(instrument);
	try {
		return std::stof(response);// Преобразование ответа в float
	}
	catch (const std::invalid_argument& e) {
		printf("Error converting voltage response to float: %s\n", e.what());
		return NAN;
	}
}

// Функция измерения тока на выходе прибора
float getCurrent(ViSession instrument, bool output) {
	if (output == 0) {
		sendCommand(instrument, "inst:sel out1\n");
	}
	else if (output == 1) {
		sendCommand(instrument, "inst:sel out2\n");
	}
	sendCommand(instrument, "MEAS:CURR?\n");
	std::string response = readResponse(instrument);
	try {
		return std::stof(response);// Преобразование ответа в float
	}
	catch (const std::invalid_argument& e) {
		printf("Error converting current response to float: %s\n", e.what());
		return NAN;
	}
}

// Функция подстройки напряжения до заданного уровня
void addVoltageDifference(ViSession vi, float limitU, bool output) {
	char command[256] = { 0 };
	float measured_voltage = getVoltage(vi, output);// Измерить текущее напряжение
	if (measured_voltage < limitU) {// Если меньше желаемого
		limitU += limitU - measured_voltage;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU); //Устанавливаем новое напряжение
		sendCommand(vi, command);
	}
	else if (measured_voltage > limitU) {// Если больше желаемого
		limitU -= measured_voltage - limitU;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU);
		sendCommand(vi, command);
	}
	measured_voltage = getVoltage(vi, output);// Повторно измеряем напряжение 
	printf("Voltage = %f V\n", measured_voltage); // Выводим значение
}

// Аварийное отключение прибора при ошибке
void emergencyShutdown(ViSession vi, const char* message) {
	printf("EMERGENCY: %s\n", message);
	sendCommand(vi, "inst:sel out2\n");
	sendCommand(vi, "VOLT 0\n");
	sendCommand(vi, "OUTP OFF\n");// Выключение выходов
	if (vi != VI_NULL) {
		viClose(vi); // Закрытие сессии VISA
	}
	exit(EXIT_FAILURE);
}


// Функция выполнения измерений в диапазоне напряжений
void processVoltageRange(ViSession vi, float limitU_supply_min, float limitU_supply_max, float limitU_supply_step, float limitU_offset, float I_offset, const std::string& filePath, const std::string& fileName) {
	char buf[256] = { 0 };
	char command[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;
	float measured_current = 0;
	float found_U_offset = NAN;
	float limitU_offset_changed = limitU_offset;
	float step = 0.1;// Начальный шаг изменения напряжения. Потом становится = 0.01

	bool stepChanged = false;

	int oscillationCount = 0; // Счетчик переходов через диапазон
	const int maxOscillations = 3; // Максимальное количество переходов

	const float criticalCurrentOffset = 0.015; // Аварийная граница значения тока смещения
	const float criticalCurrentSupply = 0.45; // Аварийная граница значения тока питания


	// Цикл по диапазону напряжений питания
	for (float current_limitU_supply = limitU_supply_min; current_limitU_supply <= limitU_supply_max; current_limitU_supply += limitU_supply_step) {
		printf("======== VOLTAGE SUPPLY = %.2f V ========\n", current_limitU_supply);
		
		// ==== Здесь создаём уникальное имя файла для каждого напряжения ====
        std::ostringstream oss;
        oss << filePath << "/" << fileNamePrefix << "_" << std::fixed << std::setprecision(2) << current_limitU_supply << "V.txt";
        std::string fullPath = oss.str();
        std::ofstream outputFile(fullPath);
        if (!outputFile.is_open()) {
            printf("Error opening file: %s\n", fullPath.c_str());
            continue;
        }
        outputFile << std::fixed << std::setprecision(6);
		outputFile << "Voltage (V),Current (A)\n";

		// ==========================================================

		sendCommand(vi, "inst:sel out2\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", current_limitU_supply);
		sendCommand(vi, command);

		printf("Voltage at 1 output\n");
		addVoltageDifference(vi, limitU_offset_changed, 0);// Коррекция напряжения 1
		outputFile << limitU_offset_changed << ",";

		printf("Voltage at 2 output\n");
		addVoltageDifference(vi, current_limitU_supply, 1);

		bool aboveIOffset = false; // Флаг, показывающий, было ли значение выше I_offset

		// oscillationCount = 0; // Обнуляем счётчик при переходе на новое напряжение
        // limitU_offset_changed = limitU_offset;
        // step = 0.1;
        // stepChanged = false;
		while (1) {
			std::this_thread::sleep_for(std::chrono::seconds(2));

			sendCommand(vi, "inst:sel out1\n");
			measured_current = getCurrent(vi, 0);

			if (measured_current > criticalCurrentOffset) {
				emergencyShutdown(vi, "Excess offset current");
			}

			sendCommand(vi, "inst:sel out2\n");
			measured_current = getCurrent(vi, 1);
			printf("Current supply = %f A\n\n", measured_current);
			outputFile << measured_current << "\n";

			if (aboveIOffset && (measured_current < I_offset - 0.01)) {
				oscillationCount++;
			}
			if (oscillationCount >= maxOscillations) {
				found_U_offset = limitU_offset_changed;
				printf("The maximum number of range hops has been exceeded\n");
				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", found_U_offset);
				break;
			}

			if (measured_current > criticalCurrentSupply || measured_current <= 0.01) {
				emergencyShutdown(vi, "Excess supply current");
			}
			else if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
				found_U_offset = limitU_offset_changed;
				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", found_U_offset);
				break;
			}
			else if ((!stepChanged) && (measured_current > I_offset + 0.01)) {
				limitU_offset_changed += step;
				step = 0.01;
				stepChanged = true;
			}

			aboveIOffset = (measured_current > I_offset + 0.01);

			limitU_offset_changed -= step;
			if (limitU_offset_changed < 0.4) {
				printf("Last current = %f out of 0.4 voltage\n", measured_current);
				break;
			}

			sendCommand(vi, "inst:sel out1\n");
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed);
			sendCommand(vi, command);
			addVoltageDifference(vi, limitU_offset_changed, 0);
			outputFile << limitU_offset_changed << ",";
		}

		sendCommand(vi, "inst:sel out1\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		sendCommand(vi, command);
		addVoltageDifference(vi, limitU_offset, 0);

		limitU_offset_changed = limitU_offset;
		step = 0.1;
		stepChanged = false;
	}
	outputFile.close();
	//printf("Data saved to %s\n", fullPath);
}

int main() {
	ViSession defaultRM = VI_NULL, vi = VI_NULL;
	//char buf[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;

	float I_offset = 0.36; //A
	float found_U_offset = NAN;

	//output 1
	float limitI_offset = 20; //mA
	float limitU_offset = 1; //V
	//output 2
	float limitI_supply = 500; //mA

	float measured_current = 0;
	float measured_voltage = 0;
	char command[256];

	float step = 0.1;
	bool stepChanged = false;

	float limitU_supply_min = 6.0;      // Минимальное значение напряжения
	float limitU_supply_max = 8.0;      // Максимальное значение напряжения
	float limitU_supply_step = 1.0;     // Шаг изменения напряжения

	ErrorStatus = viOpenDefaultRM(&defaultRM);
	checkError(ErrorStatus, VI_NULL, "viOpenDefaultRM");

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);
	checkError(ErrorStatus, defaultRM, "viOpen");

	sendCommand(vi, "*RST\n");
	sendCommand(vi, "*IDN?\n");
	std::string buf = readResponse(vi);
	printf("Instrument identification string: %s\n", buf.c_str());
	//printf("Instrument identification string: %s\n", buf);

	sendCommand(vi, "inst:sel out1\n");
	snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
	sendCommand(vi, command);
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset);
	sendCommand(vi, command);

	sendCommand(vi, "inst:sel out2\n");
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply);
	sendCommand(vi, command);
	sendCommand(vi, "VOLT 0\n");

	sendCommand(vi, "OUTP ON\n");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::string filePath = "E:/MicranStart/vach"; // "C:/Users/MyUser/Documents"
	std::string fileName = "data.txt";
	processVoltageRange(vi, limitU_supply_min, limitU_supply_max, limitU_supply_step, limitU_offset, I_offset, filePath, fileName);

	if (vi != VI_NULL) {
		viClose(vi);
	}
	if (defaultRM != VI_NULL) {
		viClose(defaultRM);
	}

	return 0;
}
