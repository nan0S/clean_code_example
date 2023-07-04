#include <windows.h>

typedef LARGE_INTEGER timestamp;

void BeginTimeMeasurement(timestamp *BeginTs)
{
   QueryPerformanceCounter(BeginTs);
}

u64 EndTimeMeasurement(timestamp BeginTs)
{
   timestamp EndTs;
   QueryPerformanceCounter(&EndTs);
   
   LARGE_INTEGER Frequency;
   QueryPerformanceFrequency(&Frequency);
   
   u64 Diff = EndTs.QuadPart - BeginTs.QuadPart;
   u64 DiffNSec = Diff / (Frequency.QuadPart / 1000000000.0);
   
   return DiffNSec;
}
