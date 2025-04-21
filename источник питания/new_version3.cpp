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

void checkError(ViStatus errorStatus, ViSession viSession, const char* operation) {
	if (errorStatus < VI_SUCCESS) {
		char errMessage[256];
		ViStatus descStatus = viStatusDesc(viSession, errorStatus, errMessage);
		if (descStatus == VI_SUCCESS) {
			printf("Error during operation %s: %d\t%s\n", operation, errorStatus, errMessage);
		} else {
			printf("Error: %d\t(Unable to retrieve error description)\n", errorStatus);
		}
		exit(EXIT_FAILURE);
	}
}

void sendCommand(ViSession instrument, const char* command) {
	ViStatus status = viPrintf(instrument, (ViPRsrc)command);
	checkError(status, instrument, "viPrintf");
}

std::string readResponse(ViSession instrument) {
	ViChar buffer[256];
	ViStatus status = viScanf(instrument, (ViPRsrc)"%t", &buffer);
	checkError(status, instrument, "viScanf");
	return std::string(buffer);
}

float getVoltage(ViSession instrument, bool output) {
	if (output == 0) {
		sendCommand(instrument, "inst:sel out1\n");
	} else if (output == 1) {
		sendCommand(instrument, "inst:sel out2\n");
	}
	sendCommand(instrument, "MEAS:VOLT?\n");
	std::string response = readResponse(instrument);
	try {
		return std::stof(response);
	} catch (const std::invalid_argument& e) {
		printf("Error converting voltage response to float: %s\n", e.what());
		return NAN;
	}
}

float getCurrent(ViSession instrument, bool output) {
	if (output == 0) {
		sendCommand(instrument, "inst:sel out1\n");
	} else if (output == 1) {
		sendCommand(instrument, "inst:sel out2\n");
	}
	sendCommand(instrument, "MEAS:CURR?\n");
	std::string response = readResponse(instrument);
	try {
		return std::stof(response);
	} catch (const std::invalid_argument& e) {
		printf("Error converting current response to float: %s\n", e.what());
		return NAN;
	}
}

void addVoltageDifference(ViSession vi, float limitU, bool output) {
	char command[256] = { 0 };
	float measured_voltage = getVoltage(vi, output);
	if (measured_voltage < limitU) {
		limitU += limitU - measured_voltage;
		sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", limitU));
	} else if (measured_voltage > limitU) {
		limitU -= measured_voltage - limitU;
		sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", limitU));
	}
	measured_voltage = getVoltage(vi, output);
	printf("Voltage = %f V\n", measured_voltage);
}

void emergencyShutdown(ViSession vi, const char* message) {
	printf("EMERGENCY: %s\n", message);
	sendCommand(instrument, "inst:sel out2\n");
	sendCommand(instrument, "VOLT 0\n");
	sendCommand(instrument, "OUTP OFF\n");
	if (vi != VI_NULL) {
		viClose(vi);
	}
	exit(EXIT_FAILURE);
}

float solveParabolaEquation(float a, float b, float c, float target) {
	float discriminant = b * b - 4 * a * (c - target);
	if (std::isnan(a) || std::isnan(b) || std::isnan(c)) { // Коэффициенты не определены
		printf("Error: coefficients of the parabola are NAN\n");
		return NAN;
	}
	if (discriminant < 0) { // Нет реальных решений
		printf("Error: no real solutions for the parabola equation\n");
		return NAN;  
	} else {
		float x1 = (-b + sqrt(discriminant)) / (2 * a);
		float x2 = (-b - sqrt(discriminant)) / (2 * a);

		if (x1 > 0) {
			return x1;
		} else if (x2 > 0) {
			return x2;
		}
	}
}

void updateParabolaCoefficients(const float& new_x,            // Новое значение x
								const float& new_y,            // Новое значение y
								int& n,                        // Количество точек
								double& sum_x,                 // Сумма x
								double& sum_y,                 // Сумма y
								double& sum_x2,                // Сумма x^2
								double& sum_x3,                // Сумма x^3
								double& sum_x4,                // Сумма x^4
								double& sum_xy,                // Сумма x*y
								double& sum_x2y,               // Сумма x^2*y
								float& a,                      // Коэффициент a
								float& b,                      // Коэффициент b
								float& c) {                    // Коэффициент c
	sum_x += new_x;
	sum_y += new_y;
	sum_x2 += new_x * new_x;
	sum_x3 += new_x * new_x * new_x;
	sum_x4 += new_x * new_x * new_x * new_x;
	sum_xy += new_x * new_y;
	sum_x2y += new_x * new_x * new_y;
	n++;
	if (n >= 3) {
		double det = n * (sum_x2 * sum_x4 - sum_x3 * sum_x3) - sum_x * (sum_x * sum_x4 - sum_x3 * sum_x2) + sum_x2 * (sum_x * sum_x3 - sum_x2 * sum_x2);
		if (det != 0) {
			a = (float)((sum_y * (sum_x2 * sum_x4 - sum_x3 * sum_x3) - sum_xy * (sum_x * sum_x4 - sum_x3 * sum_x2) + sum_x2y * (sum_x * sum_x3 - sum_x2 * sum_x2)) / det);
			b = (float)((n * (sum_xy * sum_x4 - sum_x3 * sum_x2y) - sum_x * (sum_y * sum_x4 - sum_x2y * sum_x2) + sum_x2 * (sum_y * sum_x3 - sum_xy * sum_x2)) / det);
			c = (float)((n * (sum_x2 * sum_x2y - sum_xy * sum_x3) - sum_x * (sum_x * sum_x2y - sum_xy * sum_x2) + sum_x2 * (sum_x * sum_xy - sum_y * sum_x2)) / det);
		} else {
			a = NAN;
			b = NAN;
			c = NAN;
		}
	} else {
		a = NAN;
		b = NAN;
		c = NAN;
	}
}

void processVoltageRange(ViSession vi, float limitU_supply_min, float limitU_supply_max, float limitU_supply_step, float limitU_offset, float I_offset) {
	int n = 0;
	double sum_x = 0, sum_y = 0, sum_x2 = 0, sum_x3 = 0, sum_x4 = 0, sum_xy = 0, sum_x2y = 0;
	float a = NAN, b = NAN, c = NAN;
	char buf[256] = { 0 };
	char command[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;
	float measured_current = 0;
	float measured_voltage = 0;
	float found_U_offset = NAN;
	float limitU_offset_changed = limitU_offset;
	float step = 0.1;

	bool stepChanged = false;

	int oscillationCount = 0; // Счетчик переходов через диапазон
	const int maxOscillations = 3; // Максимальное количество переходов

	const float criticalCurrentOffset = 0.015; // Аварийная граница значения тока смещения
	const float criticalCurrentSupply = 0.45; // Аварийная граница значения тока питания


	for (float current_limitU_supply = limitU_supply_min; current_limitU_supply <= limitU_supply_max; current_limitU_supply += limitU_supply_step) {
		n = 0;
		sum_x = 0;
		sum_y = 0;
		sum_x2 = 0;
		sum_x3 = 0;
		sum_x4 = 0;
		sum_xy = 0;
		sum_x2y = 0;
		a = NAN;
		b = NAN;
		c = NAN;

		printf("======== VOLTAGE SUPPLY = %.2f V ========\n", current_limitU_supply);

		sendCommand(vi, "inst:sel out2\n");
		sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", current_limitU_supply));

		printf("Voltage at 1 output\n");
		addVoltageDifference(vi, limitU_offset_changed, 0);
		printf("Voltage at 2 output\n");
		addVoltageDifference(vi, current_limitU_supply, 1);

		bool aboveIOffset = false; // Флаг, показывающий, было ли значение выше I_offset

		while (1) {
			std::this_thread::sleep_for(std::chrono::seconds(2));

			//sendCommand(vi, "inst:sel out1\n");
			measured_current = getCurrent(vi, 0);

			if (measured_current > criticalCurrentOffset) {
				emergencyShutdown(vi, "Excess offset current");
			}
			
			measured_voltage = getVoltage(vi, 0);

			//sendCommand(vi, "inst:sel out2\n");
			measured_current = getCurrent(vi, 1);
			printf("Current supply = %f A\n\n", measured_current);

			/*if (aboveIOffset && (measured_current < I_offset - 0.01)) {
				oscillationCount++;
			}
			if (oscillationCount >= maxOscillations) {
				found_U_offset = limitU_offset_changed;
				printf("The maximum number of range hops has been exceeded\n");
				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", found_U_offset);
				break;
			}*/

			if (measured_current > criticalCurrentSupply || measured_current <= 0.01) {
				emergencyShutdown(vi, "Excess supply current");
			} 
			if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
				found_U_offset = limitU_offset_changed;
				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", found_U_offset);
				break;
			} 

			updateParabolaCoefficients(measured_voltage, measured_current, n, sum_x, sum_y, sum_x2, sum_x3, sum_x4, sum_xy, sum_x2y, a, b, c);
			printf("Parabola with a = %f, b = %f, c = %f\n", a, b, c);

			if ((!stepChanged) && (measured_current > I_offset + 0.01)) {
				printf("FINAL parabola with a = %f, b = %f, c = %f\n", a, b, c);
				float solutionU = solveParabolaEquation(a, b, c, I_offset);

				sendCommand(vi, "inst:sel out1\n");
				sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", solutionU)));
				addVoltageDifference(vi, solutionU, 0);
				measured_voltage = getVoltage(vi, 0);

				measured_current = getCurrent(vi, 1);

				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", measured_voltage);

				//limitU_offset_changed += step;
				//step = 0.01;
				//stepChanged = true;
			}

			//aboveIOffset = (measured_current > I_offset + 0.01);

			limitU_offset_changed -= step;
			if (limitU_offset_changed > 0.4) {
				printf("Last current = %f out of 0.4 voltage\n", measured_current);
				break;
			}

			sendCommand(vi, "inst:sel out1\n");
			sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed)));
			addVoltageDifference(vi, limitU_offset_changed, 0);
		}

		sendCommand(vi, "inst:sel out1\n");
		sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset)));
		addVoltageDifference(vi, limitU_offset, 0);

		limitU_offset_changed = limitU_offset;
		step = 0.1;
		stepChanged = false;
	}
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
	sendCommand(vi, snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset));
	sendCommand(vi, snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset));

	sendCommand(vi, "inst:sel out2\n");
	sendCommand(vi, snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply));
	sendCommand(vi, "VOLT 0\n");

	sendCommand(vi, "OUTP ON\n");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	processVoltageRange(vi, limitU_supply_min, limitU_supply_max, limitU_supply_step, limitU_offset, I_offset);

	if (vi != VI_NULL) {
		viClose(vi);
	}
	if (defaultRM != VI_NULL) {
		viClose(defaultRM);
	}
	
	return 0;
}
