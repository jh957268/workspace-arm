################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CCLI.cpp \
../iReader.cpp \
../iReaderapi.cpp \
../main.cpp \
../muxClient.cpp \
../post_var.cpp 

OBJS += \
./CCLI.o \
./iReader.o \
./iReaderapi.o \
./main.o \
./muxClient.o \
./post_var.o 

CPP_DEPS += \
./CCLI.d \
./iReader.d \
./iReaderapi.d \
./main.d \
./muxClient.d \
./post_var.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	aarch64-linux-gnu-g++ -O0 -g3 -Wall -c -fmessage-length=0  -Wno-narrowing -Wno-misleading-indentation -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


