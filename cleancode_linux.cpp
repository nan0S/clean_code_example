#include <time.h>

typedef timespec timestamp;

void BeginTimeMeasurement(timestamp *BeginTs)
{
   clock_gettime(CLOCK_MONOTONIC_RAW, BeginTs);
}

u64 EndTimeMeasurement(timestamp BeginTs)
{
   timestamp EndTs;
   clock_gettime(CLOCK_MONOTONIC_RAW, &EndTs);
   
   u64 DiffSec = EndTs.tv_sec - BeginTs.tv_sec;
   u64 DiffNsec;
   if (EndTs.tv_nsec >= BeginTs.tv_nsec)
   {
      DiffNsec = EndTs.tv_nsec - BeginTs.tv_nsec;
   }
   else
   {
      DiffSec -= 1;
      DiffNsec = (1000000000 + EndTs.tv_nsec) - BeginTs.tv_nsec;
   }
   DiffNsec += DiffSec * 1000000000;
   
   return DiffNSec;
}
