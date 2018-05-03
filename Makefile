objs = efsfuse.o file_table.o free_space_table.o fs_operations.o util.o

CFLAGS += -D_FILE_OFFSET_BITS=64 -lfuse3 -pthread

all: $(addprefix src/, $(objs))
	gcc $(CFLAGS) $(addprefix src/, $(objs)) -o efsfuse

.PHONY: docs
docs:
	doxygen Doxyfile

.PHONY: clean
clean:
	rm -f $(addprefix src/, $(objs))
	rm -f efsfuse
