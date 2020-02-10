################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../iReader.cpp \
../iReaderapi.cpp \
../main.cpp \
../muxClient.cpp 

OBJS += \
./iReader.o \
./iReaderapi.o \
./main.o \
./muxClient.o 

CPP_DEPS += \
./iReader.d \
./iReaderapi.d \
./main.d \
./muxClient.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -O0 -g3 -Wall -c -fmessage-length=0  -Wno-narrowing -Wno-misleading-indentation -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


