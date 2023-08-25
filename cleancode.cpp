#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef float f32;
typedef uint32_t u32;
typedef uint64_t u64;

#define Pi32 3.14159265359f
#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

//- OS recognition
#if defined(_WIN32)
#define OS_WINDOWS 1
#elif defined (__gnu_linux__)
#define OS_LINUX 1
#elif defined (__APPLE__) && defined(__MACM__)
#define OS_MAC 1
#else
#error missing OS detection
#endif)

//- High precision OS measurement implementations
#if OS_WINDOWS
#include "cleancode_windows.cpp"

#include <xmmintrin.h>
#include <immintrin.h>

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

__m128 GetCornerAreaTableSIMD(shape_union *BaseShape)
{
   __m128 Multiplier = _mm_set_ps(CTable[BaseShape->Type],
                                  CTable[(BaseShape + 1)->Type],
                                  CTable[(BaseShape + 2)->Type],
                                  CTable[(BaseShape + 3)->Type]);
   
   __m128 Width = _mm_set_ps(BaseShape->Width,
                             (BaseShape + 1)->Width,
                             (BaseShape + 2)->Width,
                             (BaseShape + 3)->Width);
   
   __m128 Height = _mm_set_ps(BaseShape->Height,
                              (BaseShape + 1)->Height,
                              (BaseShape + 2)->Height,
                              (BaseShape + 3)->Height);
   
   __m128 Result = _mm_mul_ps(Multiplier, _mm_mul_ps(Width, Height));
   
   return Result;
}

f32 SumUpSIMDVector(__m128 V)
{
   f32 C[4];
   _mm_storeu_ps(C, V);
   f32 Result = (C[0] + C[1]) + (C[2] + C[3]);
   
   return Result;
}

f32 CornerAreaTableSIMD(u32 ShapeCount, shape_union *Shapes)
{
   __m128 Accum = _mm_set1_ps(0.0f);
   
   u32 Count = ShapeCount/4;
   while (Count--)
   {
      Accum = _mm_add_ps(Accum, GetCornerAreaTableSIMD(Shapes));
      Shapes += 4;
   }
   
   f32 Result = SumUpSIMDVector(Accum);
   
   return Result;
}

f32 CornerAreaTableSIMD2(u32 ShapeCount, shape_union *Shapes)
{
   __m128 Accum0 = _mm_set1_ps(0.0f);
   __m128 Accum1 = _mm_set1_ps(0.0f);
   
   u32 Count = ShapeCount/8;
   while (Count--)
   {
      __m128 Current0 = GetCornerAreaTableSIMD(Shapes);
      __m128 Current1 = GetCornerAreaTableSIMD(Shapes + 4);
      
      Accum0 = _mm_add_ps(Accum0, Current0);
      Accum1 = _mm_add_ps(Accum1, Current1);
      
      Shapes += 8;
   }
   
   f32 Result0 = SumUpSIMDVector(Accum0);
   f32 Result1 = SumUpSIMDVector(Accum1);
   f32 Result = Result0 + Result1;
   
   return Result;
}

f32 CornerAreaTableSIMD4(u32 ShapeCount, shape_union *Shapes)
{
   __m128 Accum0 = _mm_set1_ps(0.0f);
   __m128 Accum1 = _mm_set1_ps(0.0f);
   __m128 Accum2 = _mm_set1_ps(0.0f);
   __m128 Accum3 = _mm_set1_ps(0.0f);
   
   u32 Count = ShapeCount/16;
   while (Count--)
   {
      __m128 Current0 = GetCornerAreaTableSIMD(Shapes);
      __m128 Current1 = GetCornerAreaTableSIMD(Shapes + 4);
      __m128 Current2 = GetCornerAreaTableSIMD(Shapes + 8);
      __m128 Current3 = GetCornerAreaTableSIMD(Shapes + 12);
      
      Accum0 = _mm_add_ps(Accum0, Current0);
      Accum1 = _mm_add_ps(Accum1, Current1);
      Accum2 = _mm_add_ps(Accum2, Current2);
      Accum3 = _mm_add_ps(Accum3, Current3);
      
      Shapes += 16;
   }
   
   f32 Result0 = SumUpSIMDVector(Accum0);
   f32 Result1 = SumUpSIMDVector(Accum1);
   f32 Result2 = SumUpSIMDVector(Accum2);
   f32 Result3 = SumUpSIMDVector(Accum3);
   f32 Result = (Result0 + Result1) + (Result2 + Result3);
   
   return Result;
}

__m256 GetCornerAreaTableSIMD256(shape_union *BaseShape)
{
   __m256 Multiplier = _mm256_set_ps(CTable[BaseShape->Type],
                                     CTable[(BaseShape + 1)->Type],
                                     CTable[(BaseShape + 2)->Type],
                                     CTable[(BaseShape + 3)->Type],
                                     CTable[(BaseShape + 4)->Type],
                                     CTable[(BaseShape + 5)->Type],
                                     CTable[(BaseShape + 6)->Type],
                                     CTable[(BaseShape + 7)->Type]);
   
   __m256 Width = _mm256_set_ps(BaseShape->Width,
                                (BaseShape + 1)->Width,
                                (BaseShape + 2)->Width,
                                (BaseShape + 3)->Width,
                                (BaseShape + 4)->Width,
                                (BaseShape + 5)->Width,
                                (BaseShape + 6)->Width,
                                (BaseShape + 7)->Width);
   
   __m256 Height = _mm256_set_ps(BaseShape->Height,
                                 (BaseShape + 1)->Height,
                                 (BaseShape + 2)->Height,
                                 (BaseShape + 3)->Height,
                                 (BaseShape + 4)->Height,
                                 (BaseShape + 5)->Height,
                                 (BaseShape + 6)->Height,
                                 (BaseShape + 7)->Height);
   
   __m256 Result = _mm256_mul_ps(Multiplier, _mm256_mul_ps(Width, Height));
   
   return Result;
}

f32 SumUpSIMD256Vector(__m256 V)
{
   f32 C[8];
   _mm256_storeu_ps(C, V);
   f32 Result = ((C[0] + C[1]) + (C[2] + C[3])) + ((C[4] + C[5]) + (C[6] + C[7]));
   
   return Result;
}

f32 CornerAreaTableSIMD256(u32 ShapeCount, shape_union *Shapes)
{
   __m256 Accum = _mm256_set1_ps(0.0f);
   
   u32 Count = ShapeCount/8;
   while (Count--)
   {
      Accum = _mm256_add_ps(Accum, GetCornerAreaTableSIMD256(Shapes));
      Shapes += 8;
   }
   
   f32 Result = SumUpSIMD256Vector(Accum);
   
   return Result;
}

f32 CornerAreaTableSIMD256_2(u32 ShapeCount, shape_union *Shapes)
{
   __m256 Accum0 = _mm256_set1_ps(0.0f);
   __m256 Accum1 = _mm256_set1_ps(0.0f);
   
   u32 Count = ShapeCount/16;
   while (Count--)
   {
      __m256 Current0 = GetCornerAreaTableSIMD256(Shapes);
      __m256 Current1 = GetCornerAreaTableSIMD256(Shapes + 8);
      
      Accum0 = _mm256_add_ps(Accum0, Current0);
      Accum1 = _mm256_add_ps(Accum1, Current1);
      
      Shapes += 16;
   }
   
   f32 Result0 = SumUpSIMD256Vector(Accum0);
   f32 Result1 = SumUpSIMD256Vector(Accum1);
   f32 Result = Result0 + Result1;
   
   return Result;
}

f32 CornerAreaTableSIMD256_4(u32 ShapeCount, shape_union *Shapes)
{
   __m256 Accum0 = _mm256_set1_ps(0.0f);
   __m256 Accum1 = _mm256_set1_ps(0.0f);
   __m256 Accum2 = _mm256_set1_ps(0.0f);
   __m256 Accum3 = _mm256_set1_ps(0.0f);
   
   u32 Count = ShapeCount/32;
   while (Count--)
   {
      __m256 Current0 = GetCornerAreaTableSIMD256(Shapes);
      __m256 Current1 = GetCornerAreaTableSIMD256(Shapes + 8);
      __m256 Current2 = GetCornerAreaTableSIMD256(Shapes + 16);
      __m256 Current3 = GetCornerAreaTableSIMD256(Shapes + 24);
      
      Accum0 = _mm256_add_ps(Accum0, Current0);
      Accum1 = _mm256_add_ps(Accum1, Current1);
      Accum2 = _mm256_add_ps(Accum2, Current2);
      Accum3 = _mm256_add_ps(Accum3, Current3);
      
      Shapes += 32;
   }
   
   f32 Result0 = SumUpSIMD256Vector(Accum0);
   f32 Result1 = SumUpSIMD256Vector(Accum1);
   f32 Result2 = SumUpSIMD256Vector(Accum2);
   f32 Result3 = SumUpSIMD256Vector(Accum3);
   f32 Result = (Result0 + Result1) + (Result2 + Result3);
   
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
   
   printf("%30s(%d): ", "CornerAreaVTBL", ShapeCount); fflush(stdout);
   f32 MeasurementVTBL = MeasureVTBL(&CornerAreaVTBL, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementVTBL);
   
   printf("%30s(%d): ", "CornerAreaVTBL4", ShapeCount); fflush(stdout);
   f32 MeasurementVTBL4 = MeasureVTBL(&CornerAreaVTBL4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementVTBL4);
   
   printf("%30s(%d): ", "CornerAreaSwitch", ShapeCount); fflush(stdout);
   f32 MeasurementSwitch = MeasureUnion(&CornerAreaSwitch, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSwitch);
   
   printf("%30s(%d): ", "CornerAreaSwitch4", ShapeCount); fflush(stdout);
   f32 MeasurementSwitch4 = MeasureUnion(&CornerAreaSwitch4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSwitch4);
   
   printf("%30s(%d): ", "CornerAreaTable", ShapeCount); fflush(stdout);
   f32 MeasurementTable = MeasureUnion(&CornerAreaTable, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementTable);
   
   printf("%30s(%d): ", "CornerAreaTable4", ShapeCount); fflush(stdout);
   f32 MeasurementTable4 = MeasureUnion(&CornerAreaTable4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementTable4);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD = MeasureUnion(&CornerAreaTableSIMD, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD2", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD2 = MeasureUnion(&CornerAreaTableSIMD2, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD2);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD4", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD4 = MeasureUnion(&CornerAreaTableSIMD4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD4);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD256", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD256 = MeasureUnion(&CornerAreaTableSIMD256, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD256);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD256_2", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD256_2 = MeasureUnion(&CornerAreaTableSIMD256_2, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD256_2);
   
   printf("%30s(%d): ", "CornerAreaTableSIMD256_4", ShapeCount); fflush(stdout);
   f32 MeasurementSIMD256_4 = MeasureUnion(&CornerAreaTableSIMD256_4, ShapeCount, MeasurementsPerTest, RepeatCount);
   printf("%f ns/shape\n", MeasurementSIMD256_4);
   
   printf("\n");
   
   f32 SpeedupVTBL = MeasurementVTBL / MeasurementVTBL;
   f32 SpeedupVTBL4 = MeasurementVTBL / MeasurementVTBL4;
   f32 SpeedupSwitch = MeasurementVTBL / MeasurementSwitch;
   f32 SpeedupSwitch4 = MeasurementVTBL / MeasurementSwitch4;
   f32 SpeedupTable = MeasurementVTBL / MeasurementTable;
   f32 SpeedupTable4 = MeasurementVTBL / MeasurementTable4;
   f32 SpeedupSIMD = MeasurementVTBL / MeasurementSIMD;
   f32 SpeedupSIMD2 = MeasurementVTBL / MeasurementSIMD2;
   f32 SpeedupSIMD4 = MeasurementVTBL / MeasurementSIMD4;
   f32 SpeedupSIMD256 = MeasurementVTBL / MeasurementSIMD256;
   f32 SpeedupSIMD256_2 = MeasurementVTBL / MeasurementSIMD256_2;
   f32 SpeedupSIMD256_4 = MeasurementVTBL / MeasurementSIMD256_4;
   
   printf("%30s: %fx\n", "CornerAreaVTBL", SpeedupVTBL);
   printf("%30s: %fx\n", "CornerAreaVTBL4", SpeedupVTBL4);
   printf("%30s: %fx\n", "CornerAreaSwitch", SpeedupSwitch);
   printf("%30s: %fx\n", "CornerAreaSwitch4", SpeedupSwitch4);
   printf("%30s: %fx\n", "CornerAreaTable", SpeedupTable);
   printf("%30s: %fx\n", "CornerAreaTable4", SpeedupTable4);
   printf("%30s: %fx\n", "CornerAreaTableSIMD", SpeedupSIMD);
   printf("%30s: %fx\n", "CornerAreaTableSIMD2", SpeedupSIMD2);
   printf("%30s: %fx\n", "CornerAreaTableSIMD4", SpeedupSIMD4);
   printf("%30s: %fx\n", "CornerAreaTableSIMD256", SpeedupSIMD256);
   printf("%30s: %fx\n", "CornerAreaTableSIMD256_2", SpeedupSIMD256_2);
   printf("%30s: %fx\n", "CornerAreaTableSIMD256_4", SpeedupSIMD256_4);
   
   printf("\n");
}

int main()
{
   srand(123123210);
   
#if 1
   
   Measure(1);
   Measure(100);
   
#else
   
   u32 ShapeCount = 256;
   shape_union *Shapes = (shape_union *)malloc(ShapeCount * sizeof(Shapes[0]));
   
   shape_union ShapeTemplates[4];
   
   ShapeTemplates[0].Type = Shape_Square;
   ShapeTemplates[0].Width = ShapeTemplates[0].Height = 2.0f;
   
   ShapeTemplates[1].Type = Shape_Rectangle;
   ShapeTemplates[1].Width = 3.0f;
   ShapeTemplates[1].Height = 2.0f;
   
   ShapeTemplates[2].Type = Shape_Triangle;
   ShapeTemplates[2].Width = 1.0f;
   ShapeTemplates[2].Height = 2.0f;
   
   ShapeTemplates[3].Type = Shape_Circle;
   ShapeTemplates[3].Width = 1.0f;
   ShapeTemplates[3].Height = 1.0f;
   
   for (u32 ShapeIndex = 0;
        ShapeIndex < ShapeCount;
        ++ShapeIndex)
   {
      Shapes[ShapeIndex] = ShapeTemplates[ShapeIndex % ArrayCount(ShapeTemplates)];
   }
   
   f32 Table = CornerAreaTable(ShapeCount, Shapes);
   f32 Switch = CornerAreaSwitch(ShapeCount, Shapes);
   f32 SIMD = CornerAreaTableSIMD(ShapeCount, Shapes);
   f32 SIMD4 = CornerAreaTableSIMD4(ShapeCount, Shapes);
   
   printf("Table = %f\n", Table);
   printf("Switch = %f\n", Switch);
   printf("SIMD  = %f\n", SIMD );
   printf("SIMD4 = %f\n", SIMD4);
   
#endif
   
   return 0;
}