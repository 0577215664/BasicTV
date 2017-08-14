CXXFLAGS += -Wall -Werror -Wextra -std=c++14 -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wswitch-default -Wunreachable-code `pkg-config portaudio-2.0 opus opusfile SDL2_net sdl2 libcurl zlib openssl libzstd ogg --cflags`
LDLIBS += `pkg-config portaudio-2.0 opus opusfile SDL2_net sdl2 libcurl zlib openssl libzstd ogg --libs` -rdynamic -pthread
CPPFLAGS = -DDEBUG

SRC = $(shell find . -name '*.cpp')
DEP = .depend

all: debug

%.o: %.cpp
	@mkdir -p $(DEP)/$(@D)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MF $(DEP)/$*.d -c $< -o $@
	@echo -e "CXX\t$<"

basictv: $(SRC:.cpp=.o)
	@$(CXX) $^ -o $@ $(LDLIBS)
	@echo -e "LD\t$@"

debug: CXXFLAGS += -O0 -g
fast: CXXFLAGS += -Ofast -march=native

debug: basictv
fast: basictv

clean:
	$(RM) $(SRC:.cpp=.o) basictv
	$(RM) -r $(DEP)

.PHONY: all clean

.PRECIOUS: $(DEP)/%.d

-include $(addprefix $(DEP)/, $(SRC:.cpp=.d))
