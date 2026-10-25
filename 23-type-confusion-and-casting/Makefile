CXX = g++
CLANGXX = clang++
CXXFLAGS = -Wall -Wextra -g -O0

all: type_confusion_lab type_confusion_ubsan

type_confusion_lab: type_confusion_lab.cpp
	$(CXX) $(CXXFLAGS) type_confusion_lab.cpp -o type_confusion_lab

type_confusion_ubsan: type_confusion_lab.cpp
	$(CLANGXX) $(CXXFLAGS) -fsanitize=undefined -fno-sanitize-recover=all type_confusion_lab.cpp -o type_confusion_ubsan

clean:
	rm -f type_confusion_lab type_confusion_ubsan

.PHONY: all clean
