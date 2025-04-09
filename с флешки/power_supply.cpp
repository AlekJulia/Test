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
		// add value if U need
		ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
		ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
		measured_voltage = atof(buf);
		if (measured_voltage < limitU_supply) {
			limitU_supply += limitU_supply - measured_voltage;
			snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_supply);
			ErrorStatus = viPrintf(vi, (ViPRsrc)command);
		}

		while (1) {
			
			std::this_thread::sleep_for(std::chrono::seconds(2));

			ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
			ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
			ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
			printf("Current = %s\n", buf);
			measured_current = atof(buf);
			if ((measured_current <= I_offset + 0.01) && (measured_current >= I_offset - 0.01)) {
				found_U_offset = limitU_offset;
				printf("Match found: I = %f A\n", measured_current);
				printf("Found offset voltage = %f\n", found_U_offset);
				break;
			}
			else if ((stepChanged)&& (measured_current > (I_offset + 0.01))) {
				found_U_offset = limitU_offset;
				printf("The value of voltage is out of range 350-370 mA. The nearest greater value was found\n");
				printf("I = %f A\n", measured_current);
				printf("Found offset voltage = %f\n", found_U_offset);
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
			
			ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:VOLT?\n");
			ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
			measured_voltage = atof(buf);
			if (measured_voltage < limitU_offset) {
				limitU_offset += limitU_offset - measured_voltage;
				snprintf(command, sizeof(command), "VOLT %.2f\n", limitU_offset);
				ErrorStatus = viPrintf(vi, (ViPRsrc)command);
			}
			
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
