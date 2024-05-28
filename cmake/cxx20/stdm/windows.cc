module;

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif

export module stdm;

export {
	auto setConsoleOutputCP = SetConsoleOutputCP;
}

export struct IPlayer
{
	virtual void SendMsg() = 0;
};
