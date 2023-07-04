#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef float f32;
typedef uint32_t u32;
typedef uint64_t u64;

#define Pi32 3.14159265359f

//- OS recognition
#if defined(_WIN32)
#define OS_WINDOWS 1
#elif defined (__gnu_linux__)
#define OS_LINUX 1
#elif defined (__APPLE__) && defined(__MACM__)
#define OS_MAC 1
#else
#error missing OS detection
#endif

//- High precision OS measurement implementations
#if OS_WINDOWS
#include "cleancode_windows.cpp"
#elif OS_LINUX
#include "cleancode_linux.cpp"
#else
#error missing OS implementation
#endif

class shape_base
{
   public:

   shape_base() {}
   virtual f32 Area() = 0;
   virtual u32 CornerCount() = 0;
};

class square : public shape_base
{
   public:

   square(f32 Side) : Side(Side) {}
   virtual f32 Area() { return Side*Side; }
   virtual u32 CornerCount() { return 4; }

   private:

   f32 Side;
};

class rectangle : public shape_base
{
   public:

   rectangle(f32 Width, f32 Height) : Width(Width), Height(Height) {}
   virtual f32 Area() { return Width*Height; }
   virtual u32 CornerCount() { return 4; }

   private:

   f32 Width;
   f32 Height;
};

class triangle : public shape_base
{
   public:

   triangle(f32 Base, f32 Height) : Base(Base), Height(Height) {}
   virtual f32 Area() { return 0.5f*Base*Height; }
   virtual u32 CornerCount() { return 3; }

   private:

   f32 Base;
   f32 Height;
};

class circle : public shape_base
{
   public:

   circle(f32 Radius) : Radius(Radius) {}
   virtual f32 Area() { return Pi32*Radius*Radius; }
   virtual u32 CornerCount() { return 0; }

   private:

   f32 Radius;
};

f32 CornerAreaVTBL(u32 ShapeCount, shape_base **Shapes)
{
   f32 Accum = 0.0f;
   for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
   {
      Accum += (1.0f / (1.0f + Shapes[ShapeIndex]->CornerCount())) * Shapes[ShapeIndex]->Area();
   }

   return Accum;
}

f32 CornerAreaVTBL4(u32 ShapeCount, shape_base **Shapes)
{
   f32 Accum0 = 0.0f;
   f32 Accum1 = 0.0f;
   f32 Accum2 = 0.0f;
   f32 Accum3 = 0.0f;

   u32 Count = ShapeCount/4;
   while (Count--)
   {
      Accum0 += (1.0f / (1.0f + Shapes[0]->CornerCount())) * Shapes[0]->Area();
      Accum1 += (1.0f / (1.0f + Shapes[1]->CornerCount())) * Shapes[1]->Area();
      Accum2 += (1.0f / (1.0f + Shapes[2]->CornerCount())) * Shapes[2]->Area();
      Accum3 += (1.0f / (1.0f + Shapes[3]->CornerCount())) * Shapes[3]->Area();

      Shapes += 4;
   }

   f32 Result = Accum0 + Accum1 + Accum2 + Accum3;
   return Result;
}

enum shape_type : u32
{
   Shape_Square,
   Shape_Rectangle,
   Shape_Triangle,
   Shape_Circle,

   Shape_Count,
};

struct shape_union
{
   shape_type Type;
   f32 Width;
   f32 Height;
};

f32 GetAreaSwitch(shape_union Shape)
{
   f32 Result = 0.0f;

   switch (Shape.Type)
   {
      case Shape_Square: { Result = Shape.Width*Shape.Width; } break;
      case Shape_Rectangle: { Result = Shape.Width*Shape.Height; } break;
      case Shape_Triangle: { Result = 0.5f*Shape.Width*Shape.Height; } break;
      case Shape_Circle: { Result = Pi32*Shape.Width*Shape.Width; } break;

      case Shape_Count: {} break;
   }

   return Result;
}

u32 GetCornerCountSwitch(shape_union Shape)
{
   u32 Result = 0;

   switch (Shape.Type)
   {
      case Shape_Square: { Result = 4; } break;
      case Shape_Rectangle: { Result = 4; } break;
      case Shape_Triangle: { Result = 3; } break;
      case Shape_Circle: { Result = 0; } break;

      case Shape_Count: {} break;
   }

   return Result;
}

f32 const CTable[Shape_Count] = { 1.0f / (1.0f + 4.0f), 1.0f / (1.0f + 4.0f), 0.5f / (1.0f + 3.0f), Pi32 };
f32 GetCornerAreaTable(shape_union Shape)
{
   f32 Result = CTable[Shape.Type]*Shape.Width*Shape.Height;
   return Result;
}

f32 CornerAreaSwitch(u32 ShapeCount, shape_union *Shapes)
{
   f32 Accum = 0.0f;

   for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
   {
      Accum += (1.0f / (1.0f + GetCornerCountSwitch(Shapes[ShapeIndex]))) * GetAreaSwitch(Shapes[ShapeIndex]);
   }

   return Accum;
}

f32 CornerAreaSwitch4(u32 ShapeCount, shape_union *Shapes)
{
   f32 Accum0 = 0.0f;
   f32 Accum1 = 0.0f;
   f32 Accum2 = 0.0f;
   f32 Accum3 = 0.0f;

   u32 Count = ShapeCount/4;
   while (Count--)
   {
      Accum0 += (1.0f / (1.0f + GetCornerCountSwitch(Shapes[0]))) * GetAreaSwitch(Shapes[0]);
      Accum1 += (1.0f / (1.0f + GetCornerCountSwitch(Shapes[1]))) * GetAreaSwitch(Shapes[1]);
      Accum2 += (1.0f / (1.0f + GetCornerCountSwitch(Shapes[2]))) * GetAreaSwitch(Shapes[2]);
      Accum3 += (1.0f / (1.0f + GetCornerCountSwitch(Shapes[3]))) * GetAreaSwitch(Shapes[3]);

      Shapes += 4;
   }

   f32 Result = Accum0 + Accum1 + Accum2 + Accum3;
   return Result;
}

f32 CornerAreaTable(u32 ShapeCount, shape_union *Shapes)
{
   f32 Accum = 0.0f;

   for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
   {
      Accum += GetCornerAreaTable(Shapes[ShapeIndex]);
   }

   return Accum;
}

f32 CornerAreaTable4(u32 ShapeCount, shape_union *Shapes)
{
   f32 Accum0 = 0.0f;
   f32 Accum1 = 0.0f;
   f32 Accum2 = 0.0f;
   f32 Accum3 = 0.0f;

   u32 Count = ShapeCount/4;
   while (Count--)
   {
      Accum0 += GetCornerAreaTable(Shapes[0]);
      Accum1 += GetCornerAreaTable(Shapes[1]);
      Accum2 += GetCornerAreaTable(Shapes[2]);
      Accum3 += GetCornerAreaTable(Shapes[3]);

      Shapes += 4;
   }

   f32 Result = Accum0 + Accum1 + Accum2 + Accum3;
   return Result;
}

f32 MeasureVTBL(f32 (*Function)(u32, shape_base **),
                u32 ShapeCount, u32 MeasurementsPerTest, u32 RepeatCount)
{
   f32 BestMeasurement = INFINITY;
   for (u32 MeasurementIndex = 0; MeasurementIndex < MeasurementsPerTest; ++MeasurementIndex)
   {
      shape_base **Shapes = (shape_base **)malloc(ShapeCount * sizeof(*Shapes));
      for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
      {
         u32 ShapeType = rand() % 4;
         switch (ShapeType)
         {
            case 0: { Shapes[ShapeIndex] = new square(rand()); } break;
            case 1: { Shapes[ShapeIndex] = new rectangle(rand(), rand()); } break;
            case 2: { Shapes[ShapeIndex] = new triangle(rand(), rand()); } break;
            case 3: { Shapes[ShapeIndex] = new circle(rand()); } break;

            default: { assert(false); } break;
         }
      }

      timestamp BeginTs;
      BeginTimeMeasurement(&BeginTs);

      for (u32 RepeatIndex = 0; RepeatIndex < RepeatCount; ++RepeatIndex)
      {
         f32 TotalArea = Function(ShapeCount, Shapes);
      }

      u64 MeasurementNSec = EndTimeMeasurement(BeginTs);

      f32 Measurement = (f32)MeasurementNSec / (RepeatCount * ShapeCount);
      if (Measurement < BestMeasurement)
      {
         BestMeasurement = Measurement;
      }

      for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
      {
         delete Shapes[ShapeIndex];
      }
      free(Shapes);
   }

   return BestMeasurement;
}

volatile f32 AntiUnusedThrowAwayRegister;

f32 MeasureUnion(f32 (*Function)(u32, shape_union *),
                 u32 ShapeCount, u32 MeasurementsPerTest, u32 RepeatCount)
{
   f32 BestMeasurement = INFINITY;
   f32 TotalAreaAccum = 0.0f;
   for (u32 MeasurementIndex = 0; MeasurementIndex < MeasurementsPerTest; ++MeasurementIndex)
   {
      shape_union *Shapes = (shape_union *)malloc(ShapeCount * sizeof(*Shapes));
      for (u32 ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
      {
         shape_union Shape;
         Shape.Type = (shape_type)(rand() % 4);
         Shape.Width = rand();
         Shape.Height = rand();
         switch (Shape.Type)
         {
            case Shape_Square:
            case Shape_Circle: { Shape.Height = Shape.Width; } break;
         }

         Shapes[ShapeIndex] = Shape;
      }

      timestamp BeginTs;
      BeginTimeMeasurement(&BeginTs);

      for (u32 RepeatIndex = 0; RepeatIndex < RepeatCount; ++RepeatIndex)
      {
         f32 TotalArea = Function(ShapeCount, Shapes);
         TotalAreaAccum += TotalArea;
      }

      u64 MeasurementNSec = EndTimeMeasurement(BeginTs);

      f32 Measurement = (f32)MeasurementNSec / (RepeatCount * ShapeCount);
      if (Measurement < BestMeasurement)
      {
         BestMeasurement = Measurement;
      }

      free(Shapes);
   }

   AntiUnusedThrowAwayRegister += TotalAreaAccum;

   return BestMeasurement;
}

void Measure(u32 RepeatCount)
{
   printf("Repeat Count: %d\n", RepeatCount);

   printf("\n");

   u32 ShapeCount = 1048576;
   u32 MeasurementsPerTest = 10;

   printf("%20s(%d): ", "CornerAreaVTBL", ShapeCount); fflush(stdout);
   f32 MeasurementVTBL = MeasureVTBL(&CornerAreaVTBL, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementVTBL);

   printf("%20s(%d): ", "CornerAreaVTBL4", ShapeCount); fflush(stdout);
   f32 MeasurementVTBL4 = MeasureVTBL(&CornerAreaVTBL4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementVTBL4);

   printf("%20s(%d): ", "CornerAreaSwitch", ShapeCount); fflush(stdout);
   f32 MeasurementSwitch = MeasureUnion(&CornerAreaSwitch, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSwitch);

   printf("%20s(%d): ", "CornerAreaSwitch4", ShapeCount); fflush(stdout);
   f32 MeasurementSwitch4 = MeasureUnion(&CornerAreaSwitch4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSwitch4);

   printf("%20s(%d): ", "CornerAreaTable", ShapeCount); fflush(stdout);
   f32 MeasurementTable = MeasureUnion(&CornerAreaTable, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementTable);

   printf("%20s(%d): ", "CornerAreaTable4", ShapeCount); fflush(stdout);
   f32 MeasurementTable4 = MeasureUnion(&CornerAreaTable4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementTable4);

   printf("\n");

   f32 SpeedupVTBL = MeasurementVTBL / MeasurementVTBL;
   f32 SpeedupVTBL4 = MeasurementVTBL / MeasurementVTBL4;
   f32 SpeedupSwitch = MeasurementVTBL / MeasurementSwitch;
   f32 SpeedupSwitch4 = MeasurementVTBL / MeasurementSwitch4;
   f32 SpeedupTable = MeasurementVTBL / MeasurementTable;
   f32 SpeedupTable4 = MeasurementVTBL / MeasurementTable4;

   printf("%20s: %fx\n", "CornerAreaVTBL", SpeedupVTBL);
   printf("%20s: %fx\n", "CornerAreaVTBL4", SpeedupVTBL4);
   printf("%20s: %fx\n", "CornerAreaSwitch", SpeedupSwitch);
   printf("%20s: %fx\n", "CornerAreaSwitch4", SpeedupSwitch4);
   printf("%20s: %fx\n", "CornerAreaTable", SpeedupTable);
   printf("%20s: %fx\n", "CornerAreaTable4", SpeedupTable4);

   printf("\n");
}

int main()
{
   srand(123123210);

   Measure(1);
   Measure(1000);

   return 0;
}
