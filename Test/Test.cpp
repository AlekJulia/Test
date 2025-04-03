#include "visa.h"
#include <stdio.h>
#include <chrono>
#include <thread>
#include <cstdlib>

int main() 
{
	ViSession defaultRM, vi;
	char buf[256] = { 0 };
	long ErrorStatus;
	float limit_current = 0.2;
	float number;
	//ViStatus status;
	//status = viOpenDefaultRM(&defaultRM);

	viOpenDefaultRM(&defaultRM);

	ErrorStatus = viOpen(defaultRM, (ViPRsrc)"GPIB0::6::INSTR", VI_NULL, VI_NULL, &vi);
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*RST\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"*IDN?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Instrument identification string: %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out1\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 1\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR 200 mA\n");



	ErrorStatus = viPrintf(vi, (ViPRsrc)"inst:sel out2\n");
	//ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 5\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR 100 mA\n");


	ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");




	/*ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 1\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR 200 mA\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"OUTP ON\n");

	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Current = %s\n", buf);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 2\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Voltage = %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Current = %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("CurrentT = %s\n", buf);

	number = atof(buf);
	if (number > limit_current) printf("LIMIT\n");
	std::this_thread::sleep_for(std::chrono::seconds(5));

	ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT 5\n");
	ErrorStatus = viPrintf(vi, (ViPRsrc)"VOLT?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Voltage = %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"CURR?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("Current = %s\n", buf);

	ErrorStatus = viPrintf(vi, (ViPRsrc)"MEAS:CURR?\n");
	ErrorStatus = viScanf(vi, (ViPRsrc)"%t", &buf);
	printf("CurrentT = %s\n", buf);

	number = atof(buf);
	if (number > limit_current) printf("LIMIT\n");*/

	viClose(vi);
	viClose(defaultRM);
	return 0;
}
