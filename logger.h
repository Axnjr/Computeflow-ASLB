#ifndef ASLB_LOGGER_H
#define ASLB_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>

using namespace std;

namespace logger {

	//void write_log(const string& message) {
	//    ofstream logFile;
	//    logFile.open("load_balancer.log", std::ios_base::app); // Open in append mode
	//    if (logFile.is_open())
	//    {
	//        logFile << "[" << getCurrentTime() << "]" << " ~ " << message << endl;
	//        logFile.close();
	//    }
	//    else {
	//        cerr << "Unable to open log file \n";
	//    }
	//}

	// Function to get the current time as a string
	std::string getCurrentTime() {
		std::time_t now = std::time(nullptr);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		return std::string(buf);
	}

	// Variadic template function to log multiple arguments
	template <typename... Args>
	void ltf(Args... args) {
		std::ofstream logFile;
		logFile.open("aslb.log", std::ios_base::app); // Open in append mode

		if (logFile.is_open()) {
			std::stringstream ss;
			ss << getCurrentTime() << " - ";
			(ss << ... << args);  // Fold expression to handle variadic arguments
			logFile << ss.str() << std::endl;
			logFile.close();
		}
		else {
			std::cerr << "Unable to open log file" << std::endl;
		}
	}
}

#endif // !ASLB_LOGGER_H