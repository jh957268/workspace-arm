################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CrcUtils.cpp \
../MsgUtils.cpp \
../iReader.cpp \
../iReaderapi.cpp \
../muxClient.cpp \
../muxdev.cpp \
../muxserial.cpp 

OBJS += \
./CrcUtils.o \
./MsgUtils.o \
./iReader.o \
./iReaderapi.o \
./muxClient.o \
./muxdev.o \
./muxserial.o 

CPP_DEPS += \
./CrcUtils.d \
./MsgUtils.d \
./iReader.d \
./iReaderapi.d \
./muxClient.d \
./muxdev.d \
./muxserial.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -I/home/joohong/workspace-arm/llrpreader/wrapper/inc -I/home/joohong/workspace-arm/llrpreader/llrp-new -O0 -g -Wall -c -fmessage-length=0 -Wno-write-strings -Wno-misleading-indentation -Wno-narrowing -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


