#include "GTimer.h"

GTimerManager* GTimerManager::Singleton = NULL;

GTimer::GTimer(uint64 Key, uint64 Data, uint64 Interval, TickEvent TimerFunc)
	: mKey(Key), mData(Data), mInterval(Interval), Lambda(TimerFunc)
{
	mNextTickTime = GTimerManager::getNowTime() + Interval;
	bActive = true;
}


bool GTimer::Proc(uint64 NowTime)
{
	int64 Diff = mNextTickTime - NowTime;
	if (Diff <= 0)
	{
		mNextTickTime = NowTime + mInterval + Diff; // 오차보정
		return Lambda(*this);
	}
	return true;
}


GTimer::~GTimer()
{

}

void GTimer::Activate()
{
	mNextTickTime = GTimerManager::getNowTime() + mInterval;
	bActive = true;
}

void GTimer::Deactivate()
{
	bActive = false;
}