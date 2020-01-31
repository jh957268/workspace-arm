################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CWatchDogFeeder.cpp \
../cli.cpp \
../gpio.cpp \
../iAgent.cpp \
../iAgent_executor.cpp \
../llrp.cpp \
../llrp_MntAgent.cpp \
../llrp_MntServer.cpp \
../llrp_MsgProcessor.cpp \
../llrp_ROSpecExecutor.cpp 

OBJS += \
./CWatchDogFeeder.o \
./cli.o \
./gpio.o \
./iAgent.o \
./iAgent_executor.o \
./llrp.o \
./llrp_MntAgent.o \
./llrp_MntServer.o \
./llrp_MsgProcessor.o \
./llrp_ROSpecExecutor.o 

CPP_DEPS += \
./CWatchDogFeeder.d \
./cli.d \
./gpio.d \
./iAgent.d \
./iAgent_executor.d \
./llrp.d \
./llrp_MntAgent.d \
./llrp_MntServer.d \
./llrp_MsgProcessor.d \
./llrp_ROSpecExecutor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -I/home/joohong/workspace-arm/llrpreader/wrapper/inc -I/home/joohong/workspace-arm/llrpreader/iReader -I/home/joohong/workspace-arm/LTK/LTKCPP/Library -O0 -g -Wall -c -fmessage-length=0 -Wno-write-strings -Wno-literal-suffix -Wno-pointer-arith -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


