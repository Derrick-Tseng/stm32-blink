TARGET  = firmware
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CPU     = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS  = $(CPU) -DSTM32F411xE -Iinc -Wall -Wextra -O0 -g3 -ffunction-sections -fdata-sections -DSTM32F411xE
ASFLAGS = $(CPU) -g3
LDFLAGS = $(CPU) -TSTM32F411RE.ld --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=$(TARGET).map

OBJS = build/main.o build/ring.o build/system_stm32f4xx.o build/startup_stm32f411xe.o

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.s | build
	$(CC) $(ASFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) STM32F411RE.ld
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x8000000

clean:
	rm -rf build $(TARGET).elf $(TARGET).bin $(TARGET).map

.PHONY: all flash clean
