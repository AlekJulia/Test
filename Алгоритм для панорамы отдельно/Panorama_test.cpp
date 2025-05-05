#include <iostream>
#include <string>
//#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
//#include "visa.h"
#include "windows.h"
#include <cmath>  // для abs


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>

// подключение библиотеки
//#pragma comment(lib, "E:\\MicranStart\\SCPI_R4M\\_Libs\\mivisa32.lib")

using namespace std;


int main()
{
    double start_power = -20; 
    double stop_power = -15; 
    double main_freq = 10003931896; // частота на которой ищем разницу на 1 дб
    double amp_difference = 0.2; // разница между мощностями
    double tolerance = 0.05; // допустимое отклонение от 1 дБ (возможно не будет)

 
    string amp_diff_file; // найденный файл

    double amp_diff_power = 0;
    double best_diff = 1e5; // лучшая разница

    double start_amp = 0; // сохраняем амплитуду у первого файла
   

    // --- сначала находим файл с разницей ≈ 1 дБ или самое близкое значение
    for (int power = start_power; power <= stop_power; power++) {
        string filePath = "D:\\kalls\\kall" + to_string(power) + ".s2p";
        ifstream inputFile(filePath);

        if (inputFile.is_open()) {
            string line;
            while (getline(inputFile, line)) {
                if (line.empty() || line[0] == '!' || line[0] == '#')
                    continue;

                stringstream ss(line);
                double frequency, s11_re, s11_im, s21_re, s21_im;

                if (ss >> frequency >> s11_re >> s11_im >> s21_re >> s21_im) { // считываем даные со строки 
                    if (frequency == main_freq) { // сохраняем первую амплитуду
                        if (power == start_power)
                            start_amp = s21_re;
                        else { // находим разность между амплитудами, чтобы найти ближайшее значение с разницей в 1 дб
                            double diff = abs((start_amp - s21_re) - amp_difference);

                            if (diff <= best_diff) { // сохраняем файл с лучшей разницей
                                best_diff = diff;
                            } 
                            else { // если у следующего файла разница больше, то берём предидущий файл
                                amp_diff_file = "D:\\kalls\\kall" + to_string(power - 1) + ".s2p";
                                amp_diff_power = power - 1;
                                inputFile.close();
                                break;
                            }

                        }
                        
                    }
                }
            }

            inputFile.close();

        }
    }

    if (amp_diff_file.empty()) {
        std::cout << "Can not find file" << endl;
        return 0;
    }

    if (!amp_diff_file.empty()) {
        std::cout << "Nearest file: " << amp_diff_file << " (power = " << amp_diff_power << ", diff = " << best_diff << " db)" << endl;
    }

    // --- теперь читаем выбранный файл и записываем данные в новый файл
    ifstream selectedFile(amp_diff_file);
    //ofstream outputFile("D:\\kalls\\result.txt");
    ofstream outputFile("D:\\kalls\\result.csv"); // фалй для сохранения данных

    double freq_min = 4; // эти данные нужно взять из модуля для работы с панорамой
    double freq_max = 13;//


    outputFile << "Frequency_GHz"<<";"<<"Pin_dBm"<<";"<<"Pout_dBm"<<";" <<"S21_dB"<< endl;

    if (selectedFile.is_open()) {
        string line;
        while (getline(selectedFile, line)) {
            if (line.empty() || line[0] == '!' || line[0] == '#')
                continue;

            stringstream ss(line);
            double frequency, s11_re, s11_im, s21_re, s21_im;

            if (ss >> frequency >> s11_re >> s11_im >> s21_re >> s21_im) {
                
                for (int f = freq_min; f <= freq_max; f++) {

                    double frequency_GHz = f * 1e9;

                    if (frequency == frequency_GHz) {
                        double Pout = s21_re + amp_diff_power;

                        outputFile << fixed << setprecision(0) << f << ";";
                        outputFile << amp_diff_power << ";";
                        outputFile << fixed << setprecision(10) << Pout << ";";
                        outputFile << s21_re << endl;
                    }
                }
            }
        }

        selectedFile.close();
        outputFile.close();

        std::cout << "Result saved in D:\\kalls\\result.csv" << endl;
       // cout << "Result saved in D:\\kalls\\result.txt" << endl;
    }
    else {
        cerr << "Error: can not open result file" << endl;
    }

    return 0;
}


//int main()
//{
//	double start_power = -20;
//	double stop_power = -15;
//	double main_freq = 10003931896;
//	double amp_difference = 1;
//	double difference = 0;
//
//	double start_amp = 0;
//
//	for (int power = start_power; power <= stop_power; power++) {
//
//		// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ для поиска разницы мощности в 1 дбм
//		string filePath = "D:\\kalls\\kall" + to_string(power) + ".s2p";
//		ifstream inputFile(filePath);
//
//		
//
//		if (inputFile.is_open()) {
//			// тут делаем всякое
//			string line;
//			double fourthNumber = -1;
//
//			while (getline(inputFile, line)) {
//				if (line.empty() || line[0] == '!' || line[0] == '#') {
//					continue;
//				}
//
//				stringstream ss(line);
//				double frequency, s11_re, s11_im, s21_re, s21_im;
//
//				if (ss >> frequency >> s11_re >> s11_im >> s21_re >> s21_im) {
//
//					if (frequency == main_freq) {
//						if (power == start_power) {
//							start_amp = s21_re;
//						}
//						else {
//							cout << start_amp - s21_re << " difference" << endl;
//
//							if (start_amp - s21_re == amp_difference) {
//								break;
//							}
//							else if (start_amp - s21_re > amp_difference) { // если значение больше 1 дб ближе (открываем этот же файл)
//								if (abs(amp_difference - (start_amp - s21_re)) < abs(amp_difference - difference)) {
//									inputFile.close();
//									filePath = "D:\\kalls\\kall" + to_string(power) + ".s2p";
//								}
//								else { // если значение меньше 1 дб ближе (открываем предидущий файл)
//									inputFile.close();
//									filePath = "D:\\kalls\\kall" + to_string(power - 1) + ".s2p";
//								}
//							}
//
//							difference = start_amp - s21_re;
//						}
//
//						/*cout << "\n" << endl;
//						cout << fixed << frequency << " freq" << endl;
//						cout << fixed << s21_re << " amp" << endl;*/
//						break;
//					}
//				}
//			}
//
//			inputFile.close();
//		}
//		else {
//			cerr << "Error: file not open" << endl;
//		}
//
//
//
//
//	}
//
//	return 0;
//}
//
