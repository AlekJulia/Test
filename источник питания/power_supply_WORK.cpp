#include "visa.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <iostream>

void checkError(ViStatus errorStatus, ViSession viSession, const char* operation) {
    if (errorStatus < VI_SUCCESS) {
        char errMessage[VI_MAX_ERR_MSG];
        ViStatus descStatus = viStatusDesc(viSession, errorStatus, errMessage);
        //viGetErrorDesc(viSession, errorStatus, errMessage);
        //printf("Voltage = %f V\n\n", measured_voltage);
        printf("Error during operation %s:\n", operation);
        if (descStatus == VI_SUCCESS) {
            printf("Error: %d\t%s\n", errorStatus, errMessage);
        } else {
            printf("Error: %d\t(Unable to retrieve error description)\n", errorStatus);
        }
        //printf("Error: %d\t%s\n", errorStatus, errMessage);
        exit(EXIT_FAILURE);
    }
}

void add_voltage_difference_if_needed(ViSession vi,  float limitU, bool output) {
	char buf[256] = { 0 };
	char command[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;
	//float measured_voltage = 0;

	if (output == 0) {
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
		checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");
	} else {
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");
	}
	
	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	checkError(ErrorStatus, vi, "viPrintf (MEAS:VOLT?)");
	
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	checkError(ErrorStatus, vi, "viScanf (MEAS:VOLT?)");
	
	float measured_voltage = atof(buf);

	if (measured_voltage < limitU) {
		limitU += limitU - measured_voltage;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		checkError(ErrorStatus, vi, "viPrintf (VOLT)");
	} else if (measured_voltage > limitU) {
		limitU -= measured_voltage - limitU;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		checkError(ErrorStatus, vi, "viPrintf (VOLT)");
	}

	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	checkError(ErrorStatus, vi, "viPrintf (MEAS:VOLT?)");
	
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	checkError(ErrorStatus, vi, "viScanf (MEAS:VOLT?)");
	
	measured_voltage = atof(buf);
	printf("Voltage = %f V\n\n", measured_voltage);
}

void processVoltageRange(ViSession vi, float limitU_supply_min, float limitU_supply_max, float limitU_supply_step, float limitU_offset, float I_offset) {
    char buf[256] = { 0 };
	char command[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;
    float measured_current = 0;
    float found_U_offset = NAN;
    float limitU_offset_changed = limitU_offset;
    float step = 0.1;
    bool stepChanged = false;

    for (float current_limitU_supply = limitU_supply_min; current_limitU_supply <= limitU_supply_max; current_limitU_supply += limitU_supply_step) {
        printf("======== VOLTAGE SUPPLY = %.2f V ========\n", current_limitU_supply);

        ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
        checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");

        snprintf(command, sizeof(command), "VOLT %.2f\n", current_limitU_supply);
        ErrorStatus = viPrintf(vi, (ViPRsrc)command);
        checkError(ErrorStatus, vi, "viPrintf (VOLT)");

        add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);
        add_voltage_difference_if_needed(vi, current_limitU_supply, 1);

        while (1) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
            checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");

            ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
            checkError(ErrorStatus, vi, "viPrintf (MEAS:CURR?)");

            ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
            checkError(ErrorStatus, vi, "viScanf (MEAS:CURR?)");

            measured_current = atof(buf);
            printf("Current = %f A\n", measured_current);

            if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
                found_U_offset = limitU_offset_changed;
                printf("Last current = %f A\n", measured_current);
                printf("Found offset voltage = %f V\n\n", found_U_offset);
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));

            if ((!stepChanged) && (measured_current > (I_offset + 0.01))) {
                limitU_offset_changed += step;
                step = 0.01;
                stepChanged = true;
            }

            if (limitU_offset == 0.4) {
                printf("Last current = %f out of 0.4 voltage\n", measured_current);
                break;
            }

            limitU_offset_changed -= step;

            ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
            checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");

            snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed);
            ErrorStatus = viPrintf(vi, (ViPRsrc)command);
            checkError(ErrorStatus, vi, "viPrintf (VOLT)");

            add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);
        }

        ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
        checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");

        snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
        ErrorStatus = viPrintf(vi, (ViPRsrc)command);
        checkError(ErrorStatus, vi, "viPrintf (VOLT)");

        limitU_offset_changed = limitU_offset;
        step = 0.1;
        stepChanged = false;
    }
}

int main() {
	ViSession defaultRM = VI_NULL, vi = VI_NULL;
	char buf[256] = { 0 };
	ViStatus ErrorStatus = VI_SUCCESS;

	float I_offset = 0.36; // A
	float found_U_offset = NAN; // found offset voltage U 

	//output 1
	float limitI_offset = 20; //mA
	float limitU_offset = 1; //V
	//output 2
	float limitI_supply = 500; //mA
	//float limitU_supply = 6.00; //V
	
	float measured_current = 0;
	float measured_voltage = 0;
	char command[256];

	float step = 0.1;
	bool stepChanged = false;

	//float limitU_supply[] = { 6, 7, 8 };
	float limitU_supply_min = 6.0;      // Минимальное значение напряжения
    float limitU_supply_max = 8.0;      // Максимальное значение напряжения
    float limitU_supply_step = 1.0;     // Шаг изменения напряжения
	
	ErrorStatus = viOpenDefaultRM(&defaultRM);
	checkError(ErrorStatus, VI_NULL, "viOpenDefaultRM");

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);
	checkError(ErrorStatus, defaultRM, "viOpen");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"*RST\n");
	checkError(ErrorStatus, vi, "viPrintf (*RST)");
	
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*IDN?\n");
	checkError(ErrorStatus, vi, "viPrintf (*IDN?)");
	
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	checkError(ErrorStatus, vi, "viScanf");
	
	printf("Instrument identification string: %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
	checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");
	
	snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	checkError(ErrorStatus, vi, "viPrintf (VOLT)");

	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	checkError(ErrorStatus, vi, "viPrintf (CURR mA)");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");
	
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	checkError(ErrorStatus, vi, "viPrintf (CURR mA)");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");
	
	snprintf(command, sizeof(command), "VOLT 0\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	checkError(ErrorStatus, vi, "viPrintf (VOLT 0)");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");
	checkError(ErrorStatus, vi, "viPrintf (OUTP ON)");
	
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	processVoltageRange(vi, limitU_supply_min, limitU_supply_max, limitU_supply_step, limitU_offset, I_offset);

	//for (int i = 0; i < 3; i++) {
	/*for (float current_limitU_supply = limitU_supply_min; current_limitU_supply <= limitU_supply_max; current_limitU_supply += limitU_supply_step) {
		printf("======== VOLTAGE SUPPLY = %.0f V ========\n", current_limitU_supply);

		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");
		
		snprintf(command, sizeof(command), "VOLT %.2f\n", current_limitU_supply);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		checkError(ErrorStatus, vi, "viPrintf (VOLT)");

		add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);
		add_voltage_difference_if_needed(vi, current_limitU_supply, 1);

		while (1) {
			std::this_thread::sleep_for(std::chrono::seconds(2));

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
			checkError(ErrorStatus, vi, "viPrintf (inst:sel out2)");
			
			ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
			checkError(ErrorStatus, vi, "viPrintf (MEAS:CURR?)");
			
			ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
			checkError(ErrorStatus, vi, "viScanf (MEAS:CURR?)");

			measured_current = atof(buf);
			printf("Current = %f A\n", measured_current);
			if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
				found_U_offset = limitU_offset_changed;
				printf("Last current = %f A\n", measured_current);
				printf("Found offset voltage = %f V\n\n", found_U_offset);
				break;
			}

			std::this_thread::sleep_for(std::chrono::seconds(2));

			if ((!stepChanged) && (measured_current > (I_offset + 0.01))) {
				limitU_offset_changed += step;
				step = 0.01;
				stepChanged = true;
			}
			if (limitU_offset == 0.4) {
				printf("Last current = %f out of 0.4 voltage\n", measured_current);
				break;
			}

			limitU_offset_changed -= step; //0.1

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
			checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");
			
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed);
			ErrorStatus = viPrintf(vi, (ViPRsrc)command);
			checkError(ErrorStatus, vi, "viPrintf (VOLT)");

			add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);
		}
		
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
		checkError(ErrorStatus, vi, "viPrintf (inst:sel out1)");
		
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		checkError(ErrorStatus, vi, "viPrintf (VOLT)");
		
		limitU_offset_changed = limitU_offset;
		step = 0.1;
		stepChanged = false;
	}*/
	
	if (vi != VI_NULL) {
        viClose(vi);
    }
    if (defaultRM != VI_NULL) {
        viClose(defaultRM);
    }
	//viClose(vi);
	//viClose(defaultRM);
	return 0;
}
