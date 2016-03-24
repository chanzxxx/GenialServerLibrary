#pragma once
#pragma comment(lib, "winmm.lib")

#include "../Network/GNetworkGlobal.h"
#include "GRWLock.h"
#include <time.h>
#include <functional>
#include <map>

class GTimerManager;


class GTimer
{
	typedef bool(*TickEvent)(GTimer&);
public:
	inline void setInterval(uint64 Interval);
	inline void setData(uint64 Data);
	inline void setSecondData(uint64 Data);

	inline void setTickEvent(TickEvent F);
	void Activate();
	void Deactivate();
	inline uint64 getKey();
	inline uint64 getData();
	inline uint64 getSecondData();
	inline uint64 getInterval();
	~GTimer();

private:
	GTimer(uint64 Key, uint64 Data, uint64 Interval, TickEvent TickLambda);
	inline GTimer(uint64 Key);
	bool Proc(uint64 NowTime);
	uint64 mKey;
	uint64 mData;
	uint64 mData2;
	uint64 mNextTickTime;
	uint64 mInterval;
	bool bActive;
	TickEvent Lambda;

	friend GTimerManager;
};

inline GTimer::GTimer(uint64 Key) : mKey(Key), bActive(false) { }
inline void GTimer::setInterval(uint64 Interval) { mInterval = Interval; }
inline void GTimer::setData(uint64 Data) { mData = Data; }
inline void GTimer::setSecondData(uint64 Data) { mData2 = Data; }
inline uint64 GTimer::getSecondData() {	return mData2; }
inline uint64 GTimer::getKey() { return mKey; }
inline uint64 GTimer::getData() { return mData; }
inline uint64 GTimer::getInterval() { return mInterval; }
inline void GTimer::setTickEvent(GTimer::TickEvent F) { Lambda = F; }

class GTimerManager
{
public:
	inline static GTimerManager* CreateInstance();
	inline static GTimerManager* getInstance();
	inline static uint64 getNowTime();

	GTimer *CreateActiveTimer(uint64 Data, uint64 Interval, GTimer::TickEvent TickFunc)
	{
		GTimer *Timer = new GTimer(KeyIncrement++, Data, Interval, TickFunc);
		AddTimer(*Timer);
		return Timer;
	}

	GTimer *CreateTimer()
	{
		GTimer *Timer = new GTimer(KeyIncrement++);
		AddTimer(*Timer);
		return Timer;
	}

	GTimer *CreateTimer_InTickEvent(uint64 Data, uint64 Interval, GTimer::TickEvent TickFunc)
	{
		GTimer *Timer = new GTimer(KeyIncrement++, Data, Interval, TickFunc);
		AddTimer_InTickFunc(*Timer);
		return Timer;
	}

	void DeleteTimer(GTimer& Timer)
	{
		TimerMap.erase(Timer.mKey);
		delete &Timer;
	}

	void DeleteTimer(uint64 Key)
	{
		TimerMapLock.BeginWriteLock();
		std::map<uint64, GTimer*>::iterator it = TimerMap.find(Key);
		delete it->second;
		TimerMap.erase(it);
		TimerMapLock.EndWriteLock();
	}

	void Proc()
	{
		uint64 nowTime = getNowTime();
		TimerMapLock.BeginWriteLock();
		for (std::map<uint64, GTimer*>::iterator it = TimerMap.begin(); it != TimerMap.end(); ++it)
		{
			GTimer* Timer = dynamic_cast<GTimer*>(it->second);
			if (Timer->bActive == false) continue;
			if (Timer->Proc(nowTime) == false)
				_DeleteTimer(it);
		}
		TimerMapLock.EndWriteLock();
	}

	void AddTimer(GTimer& Timer)
	{
		// Tick Event 가 아닌 다른 로직에서 타이머를 추가할 경우,
		// 메인 스레드에서 Timer 작업중일 수가 있기 때문에 Lock을 걸어줍니다.
		TimerMapLock.BeginWriteLock();
		TimerMap[Timer.mKey] = &Timer;
		TimerMapLock.EndWriteLock();
	}

	void AddTimer_InTickFunc(GTimer& Timer)
	{
		// Tick Event로 호출된 함수 내에서 Timer를 추가하는 작업을 할 경우, Proc 함수에서 락을 걸어줬기 때문에 또 락을 걸면 데드락에 걸립니다.
		// 따라서, 이 함수를 사용하여 락이 또 걸리지 않도록 합니다.
		TimerMap[Timer.mKey] = &Timer;
	}
	
	void Shutdown()
	{
		for (std::map<uint64, GTimer*>::iterator it = TimerMap.begin(); it != TimerMap.end(); ++it)
		{
			GTimer* Timer = dynamic_cast<GTimer*>(it->second);
			TimerMap.erase(it);
			DeleteTimer(*Timer);
		}

		delete this;
	}


private:
	GTimerManager()
		: KeyIncrement(1)
	{

	}
	~GTimerManager()
	{
	}

	void _DeleteTimer(std::map<uint64, GTimer*>::iterator &it)
	{
		TimerMap.erase(it);
		delete it->second;
	}

	std::map<uint64, GTimer*> TimerMap;
	static GTimerManager* Singleton;
	uint64 KeyIncrement;
	GRWLock TimerMapLock;

};

inline GTimerManager* GTimerManager::CreateInstance()
{
	return Singleton = new GTimerManager;
}

inline GTimerManager* GTimerManager::getInstance()
{
	return Singleton;
}

inline uint64 GTimerManager::getNowTime()
{
	return timeGetTime();
}