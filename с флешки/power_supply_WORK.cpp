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

void add_voltage_difference_if_needed (ViSession vi,  float limitU, bool output ){
	char buf[256] = { 0 };
	char command[256] = { 0 };
	long ErrorStatus;
	float measured_voltage = 0;

	if (output == 0) {
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
	}
	else {
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	}
	
	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	measured_voltage = atof(buf);

	if (measured_voltage < limitU) {

		limitU += limitU - measured_voltage;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);

	}
	else if (measured_voltage > limitU) {
		
		limitU -= measured_voltage - limitU;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);

	}

	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	measured_voltage = atof(buf);
	printf("Voltage = %f V\n\n", measured_voltage);
}

int main()
{
	ViSession defaultRM, vi;
	char buf[256] = { 0 };
	long ErrorStatus;

	float I_offset = 0.36; // A
	float found_U_offset = NAN; // found offset voltage U 

	//output 1
	float limitI_offset = 20; //mA
	float limitU_offset = 1; //V
	float limitU_offset_changed = limitU_offset;
	//output 2
	float limitI_supply = 500; //mA
	//float limitU_supply = 6.00; //V
	
	float measured_current = 0;
	float measured_voltage = 0;
	char command[256];

	float step = 0.1;
	bool stepChanged = false;

	float limitU_supply[] = { 6, 7, 8 };
	

	viOpenDefaultRM(&defaultRM);

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);



	ErrorStatus = viPrintf(vi, (ViPRsrc)"*RST\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*IDN?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Instrument identification string: %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
	
	snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	
	

	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);



	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	snprintf(command, sizeof(command), "VOLT 0\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);


	ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");
	std::this_thread::sleep_for(std::chrono::seconds(1));

	for (int i = 0; i < 3; i++) {

		printf("======== VOLTAGE SUPPLY = %.0f V ========\n", limitU_supply[i]);

		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_supply[i]);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);

		add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);

		add_voltage_difference_if_needed(vi, limitU_supply[i], 1);


		while (1) {

			std::this_thread::sleep_for(std::chrono::seconds(2));

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
			ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
			ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);

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
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset_changed);
			ErrorStatus = viPrintf(vi, (ViPRsrc)command);

			add_voltage_difference_if_needed(vi, limitU_offset_changed, 0);


		}
		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		limitU_offset_changed = limitU_offset;
		step = 0.1;
		stepChanged = false;
	}

	viClose(vi);
	viClose(defaultRM);
	return 0;
}
