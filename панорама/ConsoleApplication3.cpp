// PanoramaCppExample.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
//#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include "visa.h"
#include "windows.h"

#pragma comment(lib, "E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")
//#pragma comment(linker, "/include:E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")

int main()
{
	std::cout << "Hello World!\n";
	ViSession rm, rsrc;
	ViStatus status;
	status = viOpenDefaultRM(&rm);
	char buff[128];
	//char cmd_str[128];
	// Open conection
	//strcpy(cmd_str, "TCPIP::localhost::8888::SOCKET::VNA");
	status = viOpen(rm, (ViString)"TCPIP::169.254.0.254::8888::SOCKET::VNA", VI_EXCLUSIVE_LOCK, 10000, &rsrc); // directly (via MiVISA only) without Micran Instrument Connector	
	//status = viOpen(rm, "TCPIP::localhost::5025::SOCKET", VI_EXCLUSIVE_LOCK, 10000, &rsrc); // via arbitrary VISA library using Micran Instrument Connector

	if (status != VI_SUCCESS)
	{
		viStatusDesc(rm, status, buff); // Get status description
		printf("viOpen: %s\r\n", buff);
		std::cin.get();
		return 0;
	}
	// Set timeout
	viSetAttribute(rsrc, VI_ATTR_TMO_VALUE, 5000);
	// Send parameters
	// Device ID
	//strcpy(cmd_str, "*IDN?\n");
	status = viQueryf(rsrc, (ViString)"*IDN?\n", (ViString)"%T", buff);
	printf("IDN: %s\r\n", buff);
	// Send "Reset" command
	status = viPrintf(rsrc, (ViString)"*RST\r\n");
	// Send "Clear status" command
	status = viPrintf(rsrc, (ViString)"*CLS\r\n");
	// Get options
	//status = viQueryf(rsrc, "SYSTem:MCLass:CATalog?\n", "%T", buff);
	//printf("Options: %s\r\n", buff);
	//status = viPrintf(rsrc, "SYSTem:PRESet\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Delete all traces
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DELete:ALL\n");
	// Create S11 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S11\",S11\n");
	// Create S22 parameter trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:DEFine \"Trc_S22\",S22\n");
	// Select trace
	status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S11\"\n");
	// Smoothing
	status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:STATe ON\n");
	status = viPrintf(rsrc, (ViString)":CALCulate:SMOothing:APERture 3\n");
	// Scale parameters
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:PDIVision 15\n");
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RLEVel -20\n");
	status = viPrintf(rsrc, (ViString)":DISPlay:WINDow1:TRACe1:Y:SCALe:RPOSition 6\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Math traces
	// Create math trace
	status = viPrintf(rsrc, (ViString)":CALCulate:MATH:CREate \"Math_New\"\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Select math trace
	status = viPrintf(rsrc, (ViString)"CALCulate:PARameter:SELect \"Math_New\"\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set math trace source 1
	status = viPrintf(rsrc, (ViString)":CALCulate:MATH:SOURce1 \"Trc_S11\"\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set math trace source 2
	status = viPrintf(rsrc, (ViString)":CALCulate:MATH:SOURce2 \"Trc_S22\"\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set math trace function
	status = viPrintf(rsrc, (ViString)":CALCulate:MATH:FUNCtion \"|A|+|B|\"\n");
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	ViUInt32 retCnt = 0;
	/*status = viPrintf(rsrc, "SOURce:ROSCillator:SOURce EXTernal\n");
	status = viPrintf(rsrc, "SOURce:ROSCillator:SOURce?\n");
	status = viRead(rsrc, (ViPBuf)buff, 10, &retCnt);
	printf("ExtRefGen: %s\r\n", buff);*/

	// Port 1 output attenuation
	/*int attport1 = 10;
	status = viPrintf(rsrc, "SOURce:POWer1:ATTenuation %d\n", attport1);
	status = viPrintf(rsrc, "SOURce:POWer1:ATTenuation?\n");
	status = viRead(rsrc, (ViPBuf)buff, 10, &retCnt);
	buff[retCnt] = '\0';
	printf("AttPort1: %s\r\n", buff);
	// Port 2 output attenuation
	int attport2 = 15;
	status = viPrintf(rsrc, "SOURce:POWer2:ATTenuation %d\n", attport2);
	status = viPrintf(rsrc, "SOURce:POWer2:ATTenuation?\n");
	status = viRead(rsrc, (ViPBuf)buff, 10, &retCnt);
	buff[retCnt] = '\0';
	printf("AttPort2: %s\r\n", buff);*/

	///////////////////////////////////////////////////////////////////////////////////////////// файл калибровки
	// Load calibration data
	status = viPrintf(rsrc, (ViString)":MMEMory:LOAD:CORRection \"E:\\MicranStart\\kal2.r4mc\"\n");
	// Set correction enabled
	status = viPrintf(rsrc, (ViString)":SENSe:CORRection ON\n");
	
	// Set RBW
	status = viPrintf(rsrc, (ViString)":SENSe:BANDwidth %d\n", 10000);
	// Set power level value
	status = viPrintf(rsrc, (ViString)"SOURce:POWer -10\n");

	// Sweep on frequency list
	/*status = viPrintf(rsrc, "SENS:FREQ:MODE LIST\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);

	status = viPrintf(rsrc, "SENS:LIST:FREQ 10,200,3000\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	/*unsigned int i = 0;
	status = viQueryf(rsrc, "SENS:LIST:FREQ:POIN?\n", "%T", buff);
	printf("SENS:LIST:FREQ:POIN: %s\r\n", buff);
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Set frequency range values
	double start = 40, stop = 20000;
	status = viPrintf(rsrc, (ViString)"SENSe:FREQuency:STARt %lfMHz;STOP %lfMHz\n", start, stop);
	// Set points count
	int points = 201;
	status = viPrintf(rsrc, (ViString)"SENSe:SWEEp:POINts %d\n", points);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Port 1 input attenuator value
	int att = 20;
	status = viPrintf(rsrc, (ViString)"SENSe:POWer:ATTenuator ARECeiver,%d\n", att);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Get max attenuator value
	/*status = viQueryf(rsrc, "SENSe:POWer:ATTenuator? ARECeiver,MAX\n", "%T", buff);
	printf("Max att: %s dB\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	/////////////////////////////////////////////////////////////////////////////////// сохранение профиля
	//// Save data
	//status = viPrintf(rsrc, (ViString)":CALCulate:DATA:SNP:SAVE \"E:\\MicranStart\\kalls.s2p\"\n");

	// Save profile 
	//status = viPrintf(rsrc, (ViString)":MMEMory:STORe:STATe \"d:\\calib.gpr\"\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString) "%T", buff);
	printf("Error: %s\r\n", buff);	

	// Set external trigger, trig point measure
	/*status = viPrintf(rsrc, ":TRIGger:SEQuence:SOURce EXT\n");
	status = viPrintf(rsrc, ":SENSe:SWEep:TRIGger:MODE POINt\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);

	// Enable SYNC OUTPUT, translate sync gen, pulse width 1us
	status = viPrintf(rsrc, ":TRIGger:AUXiliary ON\n");
	status = viPrintf(rsrc, ":TRIGger:AUXiliary:INTerval PULSe\n");
	status = viPrintf(rsrc, ":TRIGger:AUXiliary:DURation 1us\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Configure pulse profile measurement
	/*status = viPrintf(rsrc, ":SENSe:SWEep:PULSe:MODE PROFile\n"); // mode
	status = viPrintf(rsrc, ":SENSe:IF:GATE:STATe ON\n"); // gate
	double width = 0.5;
	status = viPrintf(rsrc, ":SENSe:IF:GATE:WIDTh %lfUS\n", width); // gate width
	double delay = 10;
	status = viPrintf(rsrc, ":SENSe:IF:GATE:DELay %lfUS\n", delay); // gate delay (start)
	double inc = 1;
	status = viPrintf(rsrc, ":SENSe:IF:GATE:DINCrement %lfUS\n", inc); // gate increment
	points = 51;
	status = viPrintf(rsrc, ":SENSe:IF:GATE:POINts %d\n", points); // point count (set after increment only!)
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Set average
	/*unsigned int i = 1;
	status = viPrintf(rsrc, "SENSe:AVERage:COUNt %d\n", i);
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);

	// Get average value
	status = viQueryf(rsrc, "SENSe:AVERage:COUNt?\n", "%T", buff);
	printf("SENSe:AVERage:COUNt: %s\r\n", buff);
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Disable average
	/*status = viPrintf(rsrc, "SENSe:AVERage:STATe OFF\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Sync gen parameters
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:PERiod 110us\n");
	status = viPrintf(rsrc, (ViString)"SENSe:PULSe1:WIDTh 10us\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Disable continous sweeping
	status = viPrintf(rsrc, (ViString)"INITiate:CONTinuous OFF\n");
	// Set frequency sweep mode
	status = viPrintf(rsrc, (ViString)":SENSe:SWEep:TYPE LINear\n");
	// Trigger RUN
	status = viPrintf(rsrc, (ViString)":INITiate:IMMediate\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// Checking Markers
	// Set diagram active
	status = viPrintf(rsrc, (ViString)"DISPlay:WINDow:ACTivate\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Turn on marker
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:STATe ON\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set marker on frequency
	//status = viPrintf(rsrc, ":CALCulate:MARKer0:X 3GHz\n");
	// Set marker search user domain
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:RANGe 1\n"); // set 0 if full span needed
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STARt 1GHz\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:DOMain:USER:STOP 2GHz\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Set marker on maximum
	status = viPrintf(rsrc, (ViString)":CALCulate:MARKer0:FUNCtion:SELect MAXimum\n");
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);

	// TimeDomain functions
	/*
	// Enable TimeDomain
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:STATe ON\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Check TimeDomain enabled
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:STATe?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:STATe? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Set TimeDomain stimulus type
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:UNIT TIME\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get TimeDomain stimulus type
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:UNIT?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:UNIT? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Set time range values
	start=-3.8e-9, stop=2.4e-9;
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:STARt %.12lf\n", start);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get time stimulus start
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:STARt?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:STARt? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:STOP %.12lf\n", stop);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get time stimulus stop
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:STOP?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:STOP? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Set TimeDomain window type
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:WINDow NUTTall\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get window type
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:WINDow?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:WINDow? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Set TimeDomain conversion type
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:TYPE LPASs\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get conversion type
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:TYPE?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:TYPE? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Calc TimeDomain low pass params
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:LPFRequency\n");
	// Set TimeDomain stimulus type
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:STIMulus STEP\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get stimulus type
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:STIMulus?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:STIMulus? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Set up TimeDomain AutoZero mode
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:LPASs:DCSParam:EXTRapolate\n");
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	double temp = 200;
	// Set TimeDomain ResponseZero value
	status = viPrintf(rsrc, "CALCulate:TRANsform:TIME:LPASs:DCSParam %lf\n", temp);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
	// Get ResponseZero value
	viQueryf(rsrc, "CALCulate:TRANsform:TIME:LPASs:DCSParam?\r\n", "%T", buff);
	printf("CALCulate:TRANsform:TIME:LPASs:DCSParam? : %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);
*/

// Gating functions
/*
// Select trace first
status = viPrintf(rsrc, ":CALCulate:PARameter:SELect \"Trc_S11\"\n");
// Enable Gating
status = viPrintf(rsrc, "CALCulate:FILTer:TIME:STATe ON\n");
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Check Gating enabled
viQueryf(rsrc, "CALCulate:FILTer:GATE:TIME:STATe?\r\n", "%T", buff);
printf("CALCulate:FILTer:GATE:TIME:STATe? : %s\r\n", buff);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Set Gating type
status = viPrintf(rsrc, "CALCulate:FILTer:GATE:TIME:TYPE NOTCh\n");
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Get Gating type
viQueryf(rsrc, "CALCulate:FILTer:GATE:TIME:TYPE?\r\n", "%T", buff);
printf("CALCulate:FILTer:GATE:TIME:TYPE? : %s\r\n", buff);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Set time range values for Gating
double center=-3.8e-9, span=2.4e-9;
status = viPrintf(rsrc, "CALCulate:FILTer:GATE:TIME:CENTer %.12lf\n", center);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Get time stimulus center
viQueryf(rsrc, "CALCulate:FILTer:GATE:TIME:CENTer?\r\n", "%T", buff);
printf("CALCulate:FILTer:GATE:TIME:CENTer? : %s\r\n", buff);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
status = viPrintf(rsrc, "CALCulate:FILTer:GATE:TIME:SPAN %.12lf\n", span);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Get time stimulus span
viQueryf(rsrc, "CALCulate:FILTer:GATE:TIME:SPAN?\r\n", "%T", buff);
printf("CALCulate:FILTer:GATE:TIME:SPAN? : %s\r\n", buff);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Set Gating BetaKaiser window value
unsigned int bkai = 5;
status = viPrintf(rsrc, "CALCulate:FILTer:GATE:TIME:SHAPe:BKAIser %d\n", bkai);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
// Get Gating BetaKaiser window value
viQueryf(rsrc, "CALCulate:FILTer:GATE:TIME:SHAPe:BKAIser?\r\n", "%T", buff);
printf("CALCulate:FILTer:GATE:TIME:SHAPe:BKAIser? : %s\r\n", buff);
// Check errors
status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
printf("Error: %s\r\n", buff);
*/
// Wait for measurement complete
	status = viQueryf(rsrc, (ViString)"*OPC?\n", (ViString)"%T", buff);


	if (status == VI_SUCCESS)
	{
		printf("Data ready\r\n");
		status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Trc_S11\"\n");
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
		// Set data format		
		status = viPrintf(rsrc, (ViString)":FORMat REAL,64\n");
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
		bool bSData = true;
		// Query data
		if (bSData)
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA? SDATA\n");
		else
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA? FDATA\n");

		// Parse IEEE 488.2 binary block	
		int bitSize = 0;
		char head[32];
		if ((status = viRead(rsrc, (ViPBuf)buff, 2, &retCnt)) < 0) // #n - number of bytes of size record
			return 0;
		buff[retCnt] = '\0';
		memcpy(&head[0], &buff, retCnt);
		bitSize = retCnt;
		int numBytes = atoi(buff + 1);
		if (buff[0] != '#' || numBytes == 0)
			return 0;

		if (viRead(rsrc, (ViPBuf)buff, numBytes, &retCnt) < 0) // read data length
			return 0;
		buff[retCnt] = '\0';
		memcpy(&head[0] + bitSize, &buff, retCnt);
		bitSize += retCnt;
		head[bitSize] = '\0';
		int dataSize = atoi(buff);
		if (dataSize == 0)
			return 0;

		double* data = new double[dataSize + 1];
		if (viRead(rsrc, (ViPBuf)data, dataSize + 1, &retCnt) < 0)
			return 0;

		// Writing data to memory trace
		if (!bSData) // supports only formatted data 
		{
			status = viPrintf(rsrc, (ViString)"*CLS\r\n");
			status = viPrintf(rsrc, (ViString)"MMEMory:CREate \"Mem1\",S11\n");
			status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
			printf("Error: %s\r\n", buff);
			status = viPrintf(rsrc, (ViString)":CALCulate:PARameter:SELect \"Mem1\"\n");
			status = viPrintf(rsrc, (ViString)":CALCulate:FORMat MLOGarithmic\n");
			status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
			printf("Error: %s\r\n", buff);
			status = viPrintf(rsrc, (ViString)":CALCulate:DATA FMEM,%s", head);
			((ViPBuf)data)[retCnt - 1] = '\n';
			status = viWrite(rsrc, (ViPBuf)data, retCnt, &retCnt);
		}
		for (int i = 0; i < points; i++)
		{
			if (bSData)
				printf("%g MHz\tRe: %f\tIm: %f\r\n", start + i * (stop - start) / (points - 1), data[i * 2], data[i * 2 + 1]);
			else
				printf("%g MHz\tRe: %f\t\r\n", start + i * (stop - start) / (points - 1), data[i]);
		}
		delete[] data;
	}
	else
	{
		printf("Data not ready. OPC failed\r\n");
		status = viPrintf(rsrc, (ViString)"abort\n");

		// Check errors
		status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
		printf("Error: %s\r\n", buff);
	}


	// Save data
	status = viPrintf(rsrc, (ViString)":CALCulate:DATA:SNP:SAVE \"E:\\MicranStart\\kalls2.s2p\"\n");

	// Get marker value
	status = viQueryf(rsrc, (ViString)"CALCulate:MARKer0:Y? \"Trc_S11\"\n", (ViString)"%T", buff);
	printf("Marker #0 value: %s\r\n", buff);
	// Check errors
	status = viQueryf(rsrc, (ViString)"SYSTem:ERRor?\n", (ViString)"%T", buff);
	printf("Error: %s\r\n", buff);
	// Hide all markers
	status = viPrintf(rsrc, (ViString)"CALCulate:MARKer:AOFF\n");
	// Wait for user action
	/*_getch();
	// Clean up averaging
	status = viPrintf(rsrc, "SENSe:AVERage:CLEar\n");
	status = viQueryf(rsrc, "SYSTem:ERRor?\n", "%T", buff);
	printf("Error: %s\r\n", buff);*/

	// Wait for user action
	std::cin.get();

	// Stop measurement for automatic mode
	//status = viPrintf(rsrc, ":ABORt\n");
	// Close session
	status = viClose(rsrc);
	// Close resource manager
	viClose(rm);

	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
