################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include gecko_sdk_4.0.1/util/third_party/printf/src/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/printf/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_mbedtls_support/src/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/subdir.mk
-include gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/subdir.mk
-include gecko_sdk_4.0.1/util/silicon_labs/silabs_core/memory_manager/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/stack/config/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/poll/subdir.mk
-include gecko_sdk_4.0.1/protocol/flex/app-framework-common/subdir.mk
-include gecko_sdk_4.0.1/platform/service/udelay/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/token_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/system/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/sleeptimer/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/power_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/mpu/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/legacy_hal/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/iostream/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/hfxo_manager/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/device_init/src/subdir.mk
-include gecko_sdk_4.0.1/platform/service/cli/src/subdir.mk
-include gecko_sdk_4.0.1/platform/radio/rail_lib/plugin/pa-conversions/subdir.mk
-include gecko_sdk_4.0.1/platform/peripheral/src/subdir.mk
-include gecko_sdk_4.0.1/platform/emlib/src/subdir.mk
-include gecko_sdk_4.0.1/platform/emdrv/nvm3/src/subdir.mk
-include gecko_sdk_4.0.1/platform/emdrv/gpiointerrupt/src/subdir.mk
-include gecko_sdk_4.0.1/platform/driver/leddrv/src/subdir.mk
-include gecko_sdk_4.0.1/platform/driver/i2cspm/src/subdir.mk
-include gecko_sdk_4.0.1/platform/driver/button/src/subdir.mk
-include gecko_sdk_4.0.1/platform/common/toolchain/src/subdir.mk
-include gecko_sdk_4.0.1/platform/common/src/subdir.mk
-include gecko_sdk_4.0.1/platform/Device/SiliconLabs/ZGM23/Source/GCC/subdir.mk
-include gecko_sdk_4.0.1/platform/Device/SiliconLabs/ZGM23/Source/subdir.mk
-include gecko_sdk_4.0.1/hardware/driver/si70xx/src/subdir.mk
-include gecko_sdk_4.0.1/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/subdir.mk
-include gecko_sdk_4.0.1/hardware/board/src/subdir.mk
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
all: sensor.axf

# Tool invocations
sensor.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -T "C:\Users\█████████\SimplicityStudio\v5_workspace\sensor/autogen/linkerfile.ld" -Xlinker --gc-sections -Xlinker -Map="sensor.map" -mfpu=fpv5-sp-d16 -mfloat-abi=hard --specs=nano.specs -o sensor.axf -Wl,--start-group "./app_cli.o" "./app_init.o" "./app_process.o" "./main.o" "./autogen/app_framework_event_table.o" "./autogen/callback_dispatcher.o" "./autogen/sl_board_default_init.o" "./autogen/sl_cli_command_table.o" "./autogen/sl_cli_instances.o" "./autogen/sl_device_init_clocks.o" "./autogen/sl_event_handler.o" "./autogen/sl_i2cspm_init.o" "./autogen/sl_iostream_handles.o" "./autogen/sl_iostream_init_eusart_instances.o" "./autogen/sl_power_manager_handler.o" "./autogen/sl_simple_button_instances.o" "./autogen/sl_simple_led_instances.o" "./autogen/sli_cli_hooks.o" "./gecko_sdk_4.0.1/app/common/util/app_log/app_log.o" "./gecko_sdk_4.0.1/hardware/board/src/sl_board_control_gpio.o" "./gecko_sdk_4.0.1/hardware/board/src/sl_board_init.o" "./gecko_sdk_4.0.1/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.o" "./gecko_sdk_4.0.1/hardware/driver/si70xx/src/sl_si70xx.o" "./gecko_sdk_4.0.1/platform/Device/SiliconLabs/ZGM23/Source/GCC/startup_zgm23.o" "./gecko_sdk_4.0.1/platform/Device/SiliconLabs/ZGM23/Source/system_zgm23.o" "./gecko_sdk_4.0.1/platform/common/src/sl_slist.o" "./gecko_sdk_4.0.1/platform/common/src/sl_string.o" "./gecko_sdk_4.0.1/platform/common/toolchain/src/sl_memory.o" "./gecko_sdk_4.0.1/platform/driver/button/src/sl_button.o" "./gecko_sdk_4.0.1/platform/driver/button/src/sl_simple_button.o" "./gecko_sdk_4.0.1/platform/driver/i2cspm/src/sl_i2cspm.o" "./gecko_sdk_4.0.1/platform/driver/leddrv/src/sl_led.o" "./gecko_sdk_4.0.1/platform/driver/leddrv/src/sl_simple_led.o" "./gecko_sdk_4.0.1/platform/emdrv/gpiointerrupt/src/gpiointerrupt.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_default_common_linker.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_hal_flash.o" "./gecko_sdk_4.0.1/platform/emdrv/nvm3/src/nvm3_lock.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_assert.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_burtc.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_cmu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_core.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_emu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_eusart.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_gpio.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_i2c.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_msc.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_prs.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_rmu.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_se.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_system.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_usart.o" "./gecko_sdk_4.0.1/platform/emlib/src/em_wdog.o" "./gecko_sdk_4.0.1/platform/peripheral/src/peripheral_sysrtc.o" "./gecko_sdk_4.0.1/platform/radio/rail_lib/plugin/pa-conversions/pa_conversions_efr32.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli_arguments.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli_command.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli_input.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli_io.o" "./gecko_sdk_4.0.1/platform/service/cli/src/sl_cli_tokenize.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_dcdc_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_emu_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_hfxo_s2.o" "./gecko_sdk_4.0.1/platform/service/device_init/src/sl_device_init_nvic.o" "./gecko_sdk_4.0.1/platform/service/hfxo_manager/src/sl_hfxo_manager.o" "./gecko_sdk_4.0.1/platform/service/hfxo_manager/src/sl_hfxo_manager_hal_s2.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_eusart.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_retarget_stdio.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_stdlib_config.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_uart.o" "./gecko_sdk_4.0.1/platform/service/iostream/src/sl_iostream_usart.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/base-replacement.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/crc.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/diagnostic.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/ember-phy.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/faults.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/random.o" "./gecko_sdk_4.0.1/platform/service/legacy_hal/src/token_legacy.o" "./gecko_sdk_4.0.1/platform/service/mpu/src/sl_mpu.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager_debug.o" "./gecko_sdk_4.0.1/platform/service/power_manager/src/sl_power_manager_hal_s2.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.o" "./gecko_sdk_4.0.1/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.o" "./gecko_sdk_4.0.1/platform/service/system/src/sl_system_init.o" "./gecko_sdk_4.0.1/platform/service/system/src/sl_system_process_action.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_def.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_manager.o" "./gecko_sdk_4.0.1/platform/service/token_manager/src/sl_token_manufacturing.o" "./gecko_sdk_4.0.1/platform/service/udelay/src/sl_udelay.o" "./gecko_sdk_4.0.1/platform/service/udelay/src/sl_udelay_armv6m_gcc.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_common.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_common_cb.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_sleep.o" "./gecko_sdk_4.0.1/protocol/flex/app-framework-common/app_framework_stack_cb.o" "./gecko_sdk_4.0.1/protocol/flex/poll/poll.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/ember-configuration.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/stack-callback-stubs.o" "./gecko_sdk_4.0.1/protocol/flex/stack/config/stack-stubs.o" "./gecko_sdk_4.0.1/util/silicon_labs/silabs_core/memory_manager/sl_malloc.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/aes.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/ccm.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/cipher_wrap.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/platform.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/platform_util.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/psa_crypto_client.o" "./gecko_sdk_4.0.1/util/third_party/crypto/mbedtls/library/threading.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_attestation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_entropy.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_hash.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_derivation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_key_handling.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_signature.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/se_manager/src/sl_se_manager_util.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_mbedtls_support/src/mbedtls_ccm.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_mbedtls_support/src/se_aes.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_mbedtls_support/src/sl_mbedtls.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_common.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_psa_driver_init.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_aead.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_key_management.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_mac.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_driver_signature.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.o" "./gecko_sdk_4.0.1/util/third_party/crypto/sl_component/sl_psa_driver/src/sli_se_version_dependencies.o" "./gecko_sdk_4.0.1/util/third_party/printf/printf.o" "./gecko_sdk_4.0.1/util/third_party/printf/src/iostream_printf.o" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-aes-security-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-frequency-hopping-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-packet-queue-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-parent-support-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-stack-common-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//protocol/flex/libs/libconnect-stack-counters-library-efr32xg2x-gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//platform/emdrv/nvm3/lib/libnvm3_CM33_gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//platform/radio/rail_lib/autogen/librail_release/librail_config_zgm230sb27hgn_gcc.a" "C:/Users/█████████/SimplicityStudio/SDKs/gecko_sdk//platform/radio/rail_lib/autogen/librail_release/librail_module_efr32xg23_gcc_release.a" -lgcc -lc -lm -lnosys -Wl,--end-group -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: sensor.hex'
	arm-none-eabi-objcopy -O ihex "sensor.axf" "sensor.hex"
	@echo ' '

	@echo 'Building bin file: sensor.bin'
	arm-none-eabi-objcopy -O binary "sensor.axf" "sensor.bin"
	@echo ' '

	@echo 'Building s37 file: sensor.s37'
	arm-none-eabi-objcopy -O srec "sensor.axf" "sensor.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "sensor.axf" -A
	@echo ' '

# Other Targets
clean:
	-@$(foreach FILE, $(EXECUTABLES), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(OBJS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_DEPS), $(shell $(RM) $(FILE)) )
	-$(RM) sensor.axf
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
