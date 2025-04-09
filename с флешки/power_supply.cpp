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
	//output 2
	float limitI_supply = 500; //mA
	float limitU_supply = 6; //V
	//float number;
	float measured_current = 0;
	float measured_voltage = 0;
	char command[256];
	
	double step = 0.1;
    bool stepChanged = false;
	//ViStatus status;
	//status = viOpenDefaultRM(&defaultRM);

	viOpenDefaultRM(&defaultRM);

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);

	viSetAttribute(vi, VI_ATTR_TMO_VALUE, 2000);// set timeout
	//checkError(ErrorStatus);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"*RST\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*IDN?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Instrument identification string: %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
	//ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 1\n");
	//ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR 200 mA\n");
	snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	measured_voltage = atof(buf);
	if (measured_voltage < limitU_offset) {
		limitU_offset += limitU_offset - measured_voltage;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	}
	
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_offset);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	//ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 5\n");
	//ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR 100 mA\n");
	snprintf(command, sizeof(command), "CURR %.2f mA\n", limitI_supply);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_supply);
	ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	measured_voltage = atof(buf);
	if (measured_voltage < limitU_supply) {
		limitU_supply += limitU_supply - measured_voltage;
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_supply);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
	}

	while (1) {
		
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
		ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
		ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
		printf("Current = %s\n", buf);
		measured_current = atof(buf);
		if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
			found_U_offset = limitU_offset;
			printf("Last current = %f\n", measured_current);
			printf("Found offset voltage = %f\n", found_U_offset);
			break;
		}
		else if ((stepChanged)&& (measured_current > I_offset + 0.01)) {
			found_U_offset = limitU_offset;
			printf("The value of voltage is out of range 350-370 mA. The nearest greater value was found");
			printf("Last current = %f\n", measured_current);
			printf("Found offset voltage = %f\n", found_U_offset);
			break;
		}
		
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		limitU_offset -= step; //0.1;

		if ((!stepChanged) && (limitU_offset > (I_offset + 0.01))) {
            limitU_offset += step;
			step = 0.01;
            stepChanged = true;
		}
		
		if (limitU_offset == 0.4) {
			printf("Last current = %f out of 0.4 voltage\n", measured_current);
			break;
		}

		ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
		snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
		ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		
		ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
		ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
		measured_voltage = atof(buf);
		if (measured_voltage < limitU_offset) {
			limitU_offset += limitU_offset - measured_voltage;
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
			ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		}
		
	}

	viClose(vi);
	viClose(defaultRM);
	return 0;
}
