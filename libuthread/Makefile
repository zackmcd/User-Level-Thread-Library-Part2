lib     := libuthread.a
objs    := sem.o tps.o queue.o thread.o
del     := sem.o tps.o

CC      := gcc
CFLAGS  := -Wall -Werror

AA      := ar
AFLAGS  := -crs

ifneq ($(V),1)
Q = @
endif

all : $(lib)

# Dep tracking *must* be below the 'all' rule
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

libuthread.a : $(objs)
	@echo "CC $@"
	$(Q)$(AA) $(AFLAGS) $(lib) $(objs)

%.o : %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean :
	@echo "clean"
	$(Q)rm -f $(lib) $(del) $(deps)
