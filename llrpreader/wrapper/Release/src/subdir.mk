################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/OwErrand.cpp \
../src/OwMutex.cpp \
../src/OwQueue.cpp \
../src/OwSemaphore.cpp \
../src/OwSystemTime.cpp \
../src/OwTask.cpp \
../src/OwTimer.cpp \
../src/debug_print.cpp 

OBJS += \
./src/OwErrand.o \
./src/OwMutex.o \
./src/OwQueue.o \
./src/OwSemaphore.o \
./src/OwSystemTime.o \
./src/OwTask.o \
./src/OwTimer.o \
./src/debug_print.o 

CPP_DEPS += \
./src/OwErrand.d \
./src/OwMutex.d \
./src/OwQueue.d \
./src/OwSemaphore.d \
./src/OwSystemTime.d \
./src/OwTask.d \
./src/OwTimer.d \
./src/debug_print.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -I/home/joohong/workspace-arm/llrpreader/wrapper/inc -O0 -g -Wall -c -fmessage-length=0 -Wno-literal-suffix -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


