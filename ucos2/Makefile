include $(ECOS_PRODUCT_DIR)/config.mk
EXECUTABLE   := UCOS210.elf
LIBS         := lib/libc.a


INCLUDEDIR 	:=  -I$(shell pwd)/include/lib  -I./include/app -I./include/source -I./
LINKERSCRIPT := map.lds


CFLAGS 		:= -Wall -O2 -fno-builtin
CPPFLAGS   	:= -nostdinc $(INCLUDEDIR)
LDFLAGS:= -lgcc -L $(ECOS_PRODUCT_DIR)/toolschain/4.5.1/lib/gcc/arm-none-linux-gnueabi/4.5.1



export 	CC AR LD OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS #这是为了 给 make lib/libc.a 外部输出的 这里不用

SOURCE := \
		arm/Os_cpu_a.S \
		arm/Os_cpu_c.c\
		source/ucos_ii.c \
		app/start.S \
		app/main.c \
		app/led.c \
		app/uart.c \
		app/exception.c \
		app/timer.c \
		app/clock.c \
		app/target.c \
		app/font_libs.c \
		app/lcd.c \
		app/key.c \
		app/rtc.c \
		app/ugui.c

objs := \
	$(patsubst %.c, %.o, $(filter %.c, $(SOURCE))) \
	$(patsubst %.S, %.o, $(filter %.S, $(SOURCE))) \
	$(LIBS)

all:$(EXECUTABLE)
	${OBJCOPY} -O binary -S $(EXECUTABLE) $(patsubst %.elf,%.bin,$(EXECUTABLE))
	${OBJDUMP} -D UCOS210.elf >$(patsubst %.elf,%.dis,$(EXECUTABLE))

$(EXECUTABLE): $(objs)
	${LD} -T$(LINKERSCRIPT) -o $(EXECUTABLE) $^ $(LDFLAGS)
.PHONY : lib/libc.a
lib/libc.a:
	cd lib; make; cd ..
	
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	make  clean -C lib
	rm -rf *.bin *.elf *.dis  *~  $(objs) 
