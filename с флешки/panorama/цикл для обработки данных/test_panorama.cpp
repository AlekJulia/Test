#include <iostream>
#include <string>
//#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "visa.h"
#include "windows.h"

// подключение библиотеки
#pragma comment(lib, "E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")

using namespace std;

int main()
{
	double start_power = -20;
	double stop_power = -15;
	double main_freq = 1e+10;

	for (int power = start_power; power <= stop_power; power++) {
		
		// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ для поиска разницы мощности в 1 дбм
		string filePath = "E:\\MicranStart\\kalls\\kalls_" + to_string(power) + ".s2p";
		ifstream inputFile(filePath);
		
		double start_amp = 0;

		if (inputFile.is_open()) {
			// тут делаем всякое
			string line;
			double fourthNumber = -1;

			while (getline(inputFile, line)) {
				if(line.empty() || line[0] == '!' || line[0] == '#'){
					continue;
				}
				
				stringstream ss(line);
				double frequency, s11_re, s11_im, s21_re, s21_im;

				if (ss >> frequency >> s11_re >> s11_im >> s21_re >> s21_im) {

					if (frequency == main_freq) {
						/*if (power = start_power) {
							start_amp = s21_re;
						}
						else {
							cout << start_amp - s21_re << "difference" << endl;
						}*/

						cout << fixed << frequency<< " freq" << endl;
						cout << fixed << s21_re << " amp" << endl;
						break;
					}
				}
			}

			inputFile.close();
		}
		else{
			cerr << "Error: file not open" << endl;
		}




	}

	return 0;
}

