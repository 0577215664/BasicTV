BUILD := basictv
CXXFLAGS += -Wall -Wextra -std=c++14 -Wno-unused-function -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wswitch-default -Wunreachable-code -I/usr/include/opus -I/usr/local/include/opus

cxxflags.fast := ${CXXFLAGS} -Ofast -march=native
cxxflags.basictv := ${CXXFLAGS} -O0 -g

CXXFLAGS := ${cxxflags.${BUILD}}

LDLIBS = -lcurl -lSDL2_net -lSDL2 -lz -lcrypto -lopus -lzstd -lopusfile -rdynamic -pthread -lportaudio
CPPFLAGS = -DDEBUG

SRC = $(shell find . -name '*.cpp')
DEP = .depend

all: basictv

%.o: %.cpp
	@mkdir -p $(DEP)/$(@D)
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MF $(DEP)/$*.d -c $< -o $@
	@echo -e "CXX\t$<"

basictv: $(SRC:.cpp=.o)
	@echo CXXFLAGS=${CXXFLAGS}
	@$(CXX) $^ -o $@ $(LDLIBS)
	@echo -e "LD\t$@"

fast: basictv

clean:
	$(RM) $(SRC:.cpp=.o) basictv
	$(RM) -r $(DEP)

.PHONY: all clean

.PRECIOUS: $(DEP)/%.d

-include $(addprefix $(DEP)/, $(SRC:.cpp=.d))
