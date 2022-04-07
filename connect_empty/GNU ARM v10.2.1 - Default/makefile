################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include gecko_sdk_4.0.1/util/third_party/printf/src/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/printf/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/stack/config/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/poll/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/app-framework-common/subdir.mk
-include gecko_sdk_4.0.1/platform/service/token_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/system/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/sleeptimer/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/power_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/mpu/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/legacy_hal/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/iostream/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/hfxo_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/device_init/src/subdir.mk
-include gecko_sdk_4.0.1/platform/radio/rail_lib/plugin/pa-conversions/subdir.mk
-include gecko_sdk_4.0.1/platform/peripheral/src/subdir.mk
-include gecko_sdk_4.0.1/platform/emlib/src/subdir.mk
-include gecko_sdk_4.0.1/platform/emdrv/nvm3/src/subdir.mk
-include gecko_sdk_4.0.1/platform/driver/leddrv/src/subdir.mk
-include gecko_sdk_4.0.1/platform/common/toolchain/src/subdir.mk
-include gecko_sdk_4.0.1/platform/common/src/subdir.mk
-include gecko_sdk_4.0.1/platform/Device/SiliconLabs/EFR32FG23/Source/GCC/subdir.mk
-include gecko_sdk_4.0.1/platform/Device/SiliconLabs/EFR32FG23/Source/subdir.mk
-include gecko_sdk_4.0.1/app/common/util/app_log/subdir.mk
-include autogen/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: connect_empty.axf

# Tool invocations
connect_empty.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -T "C:\Users\█████████\Desktop\I2DS\connect_empty/autogen/linkerfile.ld" -Xlinker --gc-sections -Xlinker -Map="connect_empty.map" -mfpu=fpv5-sp-d16 -mfloat-abi=hard --specs=nano.specs -o connect_empty.axf -Wl,--start-group "./app_init.o" "./app_process.o" "./app_radio.o" "./main.o" "./autogen/app_framework_event_table.o" "./autogen/callback_dispatcher.o" "./autogen/rail_config.o" "./autogen/sl_device_init_clocks.o" "./autogen/sl_event_handler.o" "./autogen/sl_iostream_handles.o" "./autogen/sl_iostream_init_eusart_instances.o" "./autogen/sl_power_manager_handler.o" "./autogen/sl_simple_led_instances.o" "./gecko_sdk_4.0.1/app/common/util/app_log/app_log.o" "./gecko_sdk_4.0.1/platform/Device/SiliconLabs/EFR32FG23/Source/GCC/startup_efr32fg23.o" "./gecko_sdk_4.0.1/platform/Device/SiliconLabs/EFR32FG23/Source/system_efr32fg23.o" "./gecko_sdk_4.0.1/platform/common/src/sl_slist.o" "./gecko_sdk_4.0.1/platform/common/toolchain/src/sl_memory.o" "./gecko_sdk_4.0.1/platform/driver/leddrv/src/sl_led.o" "./gecko_sdk_4.0.1/platform/driver/leddrv/src/sl_simple_led.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_default_common_linker.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_hal_flash.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_lock.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_assert.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_burtc.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_cmu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_core.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_emu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_eusart.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_gpio.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_iadc.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_lesense.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_msc.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_prs.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_rmu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_se.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_system.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_usart.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_wdog.o" "./gecko_sdk_4.0.1/platform/peripheral/src/peripheral_sysrtc.o" "./gecko_sdk_4.0.1/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.o" "./gecko_sdk_4.0.1/platform/radio/rail_lib/plugin/pa-conversions/pa_curves_efr32.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_dcdc_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_emu_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_hfxo_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_nvic.o" "./gecko_sdk_4.0.1/platform/service/hfxo_manager/src/sl_hfxo_manager.o" "./gecko_sdk_4.0.1/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_eusart.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_retarget_stdio.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_stdlib_config.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_uart.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_usart.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/base-replacement.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/crc.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/diagnostic.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/ember-phy.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/faults.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/random.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/token_legacy.o" "./gecko_sdk_4.0.1/platform/service/mpu/src/sl_mpu.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager_debug.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager_hal_s2.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o" "./gecko_sdk_4.0.1/platform/service/system/src/sl_system_init.o" "./gecko_sdk_4.0.1/platform/service/system/src/sl_system_process_action.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_def.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_manager.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_manufacturing.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_common.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_common_cb.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_sleep.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_stack_cb.o" "./gecko_sdk_4.0.1/protocol/flex/poll/poll.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/ember-configuration.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/stack-callback-stubs.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/stack-stubs.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.o" "./gecko_sdk_4.0.1/util/third_party/printf/printf.o" "./gecko_sdk_4.0.1/util/third_party/printf/src/iostream_printf.o" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-stack-common-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//platform/emdrv/nvm3/lib/libnvm3_CM33_gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//platform/radio/rail_lib/autogen/librail_release/librail_efr32xg23_gcc_release.a" -lgcc -lc -lm -lnosys -Wl,--end-group -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: connect_empty.hex'
	arm-none-eabi-objcopy -O ihex "connect_empty.axf" "connect_empty.hex"
	@echo ' '

	@echo 'Building bin file: connect_empty.bin'
	arm-none-eabi-objcopy -O binary "connect_empty.axf" "connect_empty.bin"
	@echo ' '

	@echo 'Building s37 file: connect_empty.s37'
	arm-none-eabi-objcopy -O srec "connect_empty.axf" "connect_empty.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "connect_empty.axf" -A
	@echo ' '

# Other Targets
clean:
	-@$(foreach FILE, $(EXECUTABLES), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(OBJS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_DEPS), $(shell $(RM) $(FILE)) )
	-$(RM) connect_empty.axf
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
