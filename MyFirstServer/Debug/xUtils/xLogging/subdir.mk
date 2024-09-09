################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../xUtils/xLogging/xLogging.c 

C_DEPS += \
./xUtils/xLogging/xLogging.d 

OBJS += \
./xUtils/xLogging/xLogging.o 


# Each subdirectory must supply rules for building sources it contributes
xUtils/xLogging/%.o: ../xUtils/xLogging/%.c xUtils/xLogging/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -I"E:\GIT\OrangePI_projects\MyFirstServer\xUtils" -I"E:\GIT\OrangePI_projects\MyFirstServer\xUtils\xLogging" -I"E:\GIT\OrangePI_projects\MyFirstServer\xUtils\xErrorHandling" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-xUtils-2f-xLogging

clean-xUtils-2f-xLogging:
	-$(RM) ./xUtils/xLogging/xLogging.d ./xUtils/xLogging/xLogging.o

.PHONY: clean-xUtils-2f-xLogging

