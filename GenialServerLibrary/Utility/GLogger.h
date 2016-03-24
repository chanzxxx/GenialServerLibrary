#pragma once
#include "GRWLock.h"

#define GLOG_CONSOLE
//#define GLOG_FILE


class GLogger
{
public:

	GLogger()
	{
	}

	~GLogger()
	{
	}


	static void Log(const char *Module, const char *LogMsg ...)
	{
#ifdef GLOG_CONSOLE
		ConsoleLock.BeginWriteLock();
		printf("[%s] ", Module);
		va_list Args;
		va_start(Args, LogMsg);
		vprintf(LogMsg, Args);
		va_end(Args);
		puts("");
		ConsoleLock.EndWriteLock();
#endif
#ifdef GLOG_FILE

		puts(Module);
		va_list Args;
		va_start(Args, LogMsg);
		vprintf(LogMsg, Args);
		va_end(Args);
#endif
	}

	
private:
	static GRWLock ConsoleLock;
	//static FILE *FilePointer;
	//static GRWLock ConsoleLock;

};

