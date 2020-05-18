################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CAntenna.cpp \
../CSqlite.cpp \
../CWatchDogFeeder.cpp \
../cli.cpp \
../gpio.cpp \
../iAgent.cpp \
../iAgent_executor.cpp \
../llrp.cpp \
../llrp_MntAgent.cpp \
../llrp_MntServer.cpp \
../llrp_MsgProcessor.cpp \
../llrp_ROSpecExecutor.cpp \
../pwm.cpp 

OBJS += \
./CAntenna.o \
./CSqlite.o \
./CWatchDogFeeder.o \
./cli.o \
./gpio.o \
./iAgent.o \
./iAgent_executor.o \
./llrp.o \
./llrp_MntAgent.o \
./llrp_MntServer.o \
./llrp_MsgProcessor.o \
./llrp_ROSpecExecutor.o \
./pwm.o 

CPP_DEPS += \
./CAntenna.d \
./CSqlite.d \
./CWatchDogFeeder.d \
./cli.d \
./gpio.d \
./iAgent.d \
./iAgent_executor.d \
./llrp.d \
./llrp_MntAgent.d \
./llrp_MntServer.d \
./llrp_MsgProcessor.d \
./llrp_ROSpecExecutor.d \
./pwm.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -I/home/joohong/workspace-arm/llrpreader/wrapper/inc -I/home/joohong/workspace-arm/llrpreader/iReader -I/home/joohong/workspace-arm/LTK/LTKCPP/Library -I/home/joohong/workspace-arm/install/include -O0 -g -Wall -c -fmessage-length=0 -Wno-write-strings -Wno-literal-suffix -Wno-pointer-arith -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


