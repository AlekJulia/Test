#include "visa.h"
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cmath>

//void checkError(long errorStatus) {
//	if (errorStatus < VI_SUCCESS) {
//		char errorMessage[256];
//		viStatusDesc(errorStatus, errorMessage);
//		fprintf(stderr, "Error: %s\n", errorMessage);
//		exit(EXIT_FAILURE);
//	}
//}

//надо как-то error статус добавить и выводить потом ошибки
float measure_voltage(ViSession vi, char* buf) {
    viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
    viScanf(vi, (ViPRsrc)"%t", buf);
    return atof(buf);
}

float measure_current(ViSession vi, char* buf) {
    viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
    viScanf(vi, (ViPRsrc)"%t", buf);
    return atof(buf);
}

float add_voltage_if_needed(ViSession vi, float voltage_limit, float measured_voltage, char* command_buf, size_t buf_size) {
    if (measured_voltage < voltage_limit) {
        voltage_limit += voltage_limit - measured_voltage;
        snprintf(command_buf, buf_size, "VOLT %.2f\n", voltage_limit);
        viPrintf(vi, (ViPRsrc)command_buf);
    }
    return voltage_limit;
}


int main()
{
	ViSession defaultRM, vi;
	char buf[256] = { 0 };
	long ErrorStatus;
	char command[256];


	float I_offset = 0.36; // A
	
	float limitI_offset = 20; //mA (output 1)
	float limitI_supply = 500; //mA (output 2)

	float found_U_offsets[3] = { NAN, NAN, NAN }; // (output 1)
	float limitU_supplies[] = { 6, 7, 8 }; //V (output 2)

	double step = 0.1;
    

	viOpenDefaultRM(&defaultRM);

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);

	viSetAttribute(vi, VI_ATTR_TMO_VALUE, 2000);// set timeout
	//checkError(ErrorStatus);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"*RST\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*IDN?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Instrument identification string: %s\n", buf);
	
	for (int i=0; i<3; i++) {
		float limitU_supply = limitU_supplies[i];
		float limitU_offset = 1; //V
		float found_U_offset = NAN;
        float measured_current = 0;
        float measured_voltage = 0;
		double step_local = step;
		bool stepChanged = false;

		printf("===== Starting for limitU_supply = %.2f V =====\n", limitU_supply);

		// Output 1 - set I + U
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		// add value if U1 need
		measured_voltage = measure_voltage(vi, buf);
    	limitU_offset = add_voltage_if_needed(vi, limitU_offset, measured_voltage, command, sizeof(command));
		

		snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);

		// Output 2 - set I
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);

		// Outputs ON
		ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");

		// Output 2 - set U
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_supply);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		// add value if U2 need
		measured_voltage = measure_voltage(vi, buf);
    	limitU_supply = add_voltage_if_needed(vi, limitU_supply, measured_voltage, command, sizeof(command));
		

		while (1) {
			
			std::this_thread::sleep_for(std::chrono::seconds(2));

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
			measured_current = measure_current(vi, buf);
        	printf("Current = %s\n", buf);

			measured_current = atof(buf);
			if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
				found_U_offset = limitU_offset;
				printf("MATCH FOUND: I = %f A\n", measured_current);
				printf("FOUND OFFSET VOLTAGE =  %f\n", found_U_offset);
				break;
			}
			else if ((stepChanged)&& (measured_current > I_offset + 0.01)) {
				found_U_offset = limitU_offset;
				printf("The value of voltage is out of range 350-370 mA. The nearest greater value was found\n");
				printf("I = %f A\n", measured_current);
				printf("FOUND OFFSET VOLTAGE = %f\n", found_U_offset);
				break;
			}
			
			std::this_thread::sleep_for(std::chrono::seconds(2));

			limitU_offset -= step_local; //0.1;

			if ((!stepChanged) && (measured_current > (I_offset + 0.01))) {
				limitU_offset += step_local;
				step_local = 0.01;
				stepChanged = true;
			}
			
			if (limitU_offset <= 0.4) {
				printf("Reached voltage floor. I = %f A\n\n", measured_current);
				break;
			}

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
			ErrorStatus = viPrintf(vi, (ViPRsrc)command);
			
			measured_voltage = measure_voltage(vi, buf);
        	limitU_offset = add_voltage_if_needed(vi, limitU_offset, measured_voltage, command, sizeof(command));
			
		}

		found_U_offsets[i] = found_U_offset;
	}
	
	// Show results
    printf("======== Final Results ========\n");
    for (int i = 0; i < 3; ++i) {
        printf("limitU_supply = %.2f V -> found_U_offset = %.3f V\n", limitU_supplies[i], found_U_offsets[i]);
    }

	viClose(vi);
	viClose(defaultRM);
	return 0;
}
