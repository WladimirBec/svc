BLDD := $(abspath ./bld)

CC     ?= clang
CFLAGS := -Wall -Wextra -Werror \
		  -D_POSIX_C_SOURCE=202405L -D_XOPEN_SOURCE=800 -D_GNU_SOURCE \
		  -std=c99 \
		  $(if $(filter 1,$(DEBUG)),-O0 -g -mno-avx -mno-avx512f,-O3 -flto -pipe -march=native -static)
SRCS   := $(wildcard *.c)
OBJS   := $(patsubst %.c,$(BLDD)/%.o,$(SRCS))

$(BLDD)/svc: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BLDD):
	@ mkdir $@

$(OBJS): $(BLDD)/%.o: %.c | $(BLDD)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -fr $(BLDD)
	rm -f compile_commands.json

compdb:
	@ ( \
		echo "["; \
		for src in $(SRCS); do \
			echo "  {"; \
			echo "    \"directory\": \"$(BLDD)\","; \
			echo "    \"file\": \"../$$src\","; \
			echo "    \"command\": \"$(CC) $(CFLAGS) -c $$src\""; \
			echo "  },"; \
		done; \
		echo "]"; \
	) > ./compile_commands.json

.PHONY: clean compdb
