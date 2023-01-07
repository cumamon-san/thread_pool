#ifndef LOG_H
#define LOG_H

#include <iostream>

#define ERROR(X) do { std::cerr << "ERROR: " << __func__ << ": " << X << std::endl; } while(0)
#define PRINT(X) do { std::cout << X << std::endl; } while(0)
#define DEBUG(X) do { std::cout << "DEBUG: " << X << std::endl; } while(0)
#define DEBUG_VAR(X) DEBUG(#X " = " << X)

#endif // LOG_H
