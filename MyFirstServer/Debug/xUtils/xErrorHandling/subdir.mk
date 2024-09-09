################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../xUtils/xErrorHandling/xErrorHandling.c 

C_DEPS += \
./xUtils/xErrorHandling/xErrorHandling.d 

OBJS += \
./xUtils/xErrorHandling/xErrorHandling.o 


# Each subdirectory must supply rules for building sources it contributes
xUtils/xErrorHandling/%.o: ../xUtils/xErrorHandling/%.c xUtils/xErrorHandling/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -I../xUtils -I../xUtils/xLogging -I../xUtils/xErrorHandling -I/usr/include/libxml2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-xUtils-2f-xErrorHandling

clean-xUtils-2f-xErrorHandling:
	-$(RM) ./xUtils/xErrorHandling/xErrorHandling.d ./xUtils/xErrorHandling/xErrorHandling.o

.PHONY: clean-xUtils-2f-xErrorHandling

