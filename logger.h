#ifndef ASLB_LOGGER_H
#define ASLB_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <sys/stat.h>

using namespace std;

namespace logger {

	const std::string logFilePath = "aslb.log";
	const size_t maxLogFileSize = 1024 * 1024 * 10; // 10 MB

	void checkAndTruncateLogFile() {
		struct stat fileStat;
		if (stat(logFilePath.c_str(), &fileStat) == 0) 
		{
			if (fileStat.st_size > maxLogFileSize) 
			{
				std::ofstream logFile(logFilePath, std::ios::trunc); // Open file in truncation mode
				if (logFile.is_open()) 
				{
					logFile.close();
					std::cout << "Log file truncated successfully." << std::endl;
				}
				else {
					std::cerr << "Failed to open log file for truncation." << std::endl;
				}
			}
		}
		else {
			std::cerr << "Failed to get file status." << std::endl;
		}
	}

	// Function to get the current time as a string
	std::string getCurrentTime() {
		std::time_t now = std::time(nullptr);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		return std::string(buf);
	}

	// Variadic template function to log multiple arguments
	template <typename... Args> void ltf(Args... args) 
	{
		checkAndTruncateLogFile();

		std::ofstream logFile;
		logFile.open("aslb.log", std::ios_base::app); // Open in append mode

		if (logFile.is_open()) {

			std::stringstream ss;

			ss << "[" << getCurrentTime() << "]" << " ~ ";
			// Fold expression to handle variadic arguments
			(ss << ... << args);  

		#ifdef DEV_MODE
			cout << ss.str() << endl;
		#endif // DEV_MODE

			logFile << ss.str() << std::endl;
			logFile.close();
		}
		else {
			std::cerr << "Unable to open log file" << std::endl;
		}
	}
}

#endif // !ASLB_LOGGER_H