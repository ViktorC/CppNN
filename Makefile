MAKE := make -f Makefile
GCC_CXX := g++
CLANG_CXX := clang++
CXXFLAGS := -std=c++11 -fmessage-length=0 -ftemplate-backtrace-limit=0
# AVX instructions are problematic with GCC 64 bit on Windows due to its lack of support for 32 byte stack alignment.
GCC_CXXFLAGS := $(CXXFLAGS) -Wno-ignored-attributes -fopenmp
GCC_CUDA_CXXFLAGS := $(GCC_CXXFLAGS) -DCATTLE_USE_CUBLAS
# Clang does not actually utilize OpenMP on Windows; no libomp or libiomp5.
CLANG_CXXFLAGS := $(CXXFLAGS) -march=native
CLANG_CUDA_CXXFLAGS := $(CLANG_CXXFLAGS) -DCATTLE_USE_CUBLAS
RELEASE_OPT_FLAGS := -O3 -DNDEBUG
DEBUG_OPT_FLAGS := -O1 -Wa,-mbig-obj -g
GTEST_DIR := test/gtest
# For Clang on Windows, omp.h must be copied from GCC.
INCLUDES := -IC-ATTL3 -IEigen -I$(GTEST_DIR)/include -Itest/
CUDA_INCLUDES := -I"$(CUDA_INC_PATH)" $(INCLUDES)
LIBS := -lpthread -lgomp
CUDA_LIBS := $(LIBS) -L"$(CUDA_LIB_PATH)" -lcudart -lcublas
SOURCE_DIR := test
SOURCES := test.cpp
BUILD_DIR := build
TARGET_DIR := bin
TARGET_NAME := cattle_test.exe
GTEST_MAKE_PATH := $(GTEST_DIR)/make
TARGET := $(TARGET_DIR)/$(TARGET_NAME)
OBJECTS := $(BUILD_DIR)/$(SOURCES:%.cpp=%.o)
$(OBJECTS): $(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@cd $(GTEST_MAKE_PATH) && make gtest-all.o && cd $(CURDIR)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) $(INCLUDES) -c -o $@ $<
$(TARGET): $(OBJECTS)
	@cd $(GTEST_MAKE_PATH) && make gtest.a && cd $(CURDIR)
	@mkdir -p $(TARGET_DIR)
	# Link the gtest static library directly as it is built by default without the 'lib' prefix.
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -o $@ $? $(GTEST_MAKE_PATH)/gtest.a $(LIBS)
.PHONY: all clean
.DEFAULT_GOAL: all
all:
	$(MAKE) $(TARGET) \
		CXX='$(GCC_CXX)' \
		CXXFLAGS='$(GCC_CXXFLAGS)' \
		OPT_FLAGS='$(RELEASE_OPT_FLAGS)'
debug:
	$(MAKE) $(TARGET) \
		CXX='$(GCC_CXX)' \
		CXXFLAGS='$(GCC_CXXFLAGS)' \
		OPT_FLAGS='$(DEBUG_OPT_FLAGS)'
clang_all:
	$(MAKE) $(TARGET) \
		CXX='$(CLANG_CXX)' \
		CXXFLAGS='$(CLANG_CXXFLAGS)' \
		OPT_FLAGS='$(RELEASE_OPT_FLAGS)'
clang_debug:
	$(MAKE) $(TARGET) \
		CXX='$(CLANG_CXX)' \
		CXXFLAGS='$(CLANG_CXXFLAGS)' \
		OPT_FLAGS='$(DEBUG_OPT_FLAGS)'
cuda_all:
	$(MAKE) $(TARGET) \
		CXX='$(GCC_CXX)' \
		CXXFLAGS='$(GCC_CUDA_CXXFLAGS)' \
		OPT_FLAGS='$(RELEASE_OPT_FLAGS)' \
		INCLUDES='$(CUDA_INCLUDES)' \
		LIBS='$(CUDA_LIBS)'
cuda_debug:
	$(MAKE) $(TARGET) \
		CXX='$(GCC_CXX)' \
		CXXFLAGS='$(GCC_CUDA_CXXFLAGS)' \
		OPT_FLAGS='$(DEBUG_OPT_FLAGS)' \
		INCLUDES='$(CUDA_INCLUDES)' \
		LIBS='$(CUDA_LIBS)'
clang_cuda_all:
	$(MAKE) $(TARGET) \
		CXX='$(CLANG_CXX)' \
		CXXFLAGS='$(CLANG_CUDA_CXXFLAGS)' \
		OPT_FLAGS='$(RELEASE_OPT_FLAGS)' \
		INCLUDES='$(CUDA_INCLUDES)' \
		LIBS='$(CUDA_LIBS)'
clang_cuda_debug:
	$(MAKE) $(TARGET) \
		CXX='$(CLANG_CXX)' \
		CXXFLAGS='$(CLANG_CUDA_CXXFLAGS)' \
		OPT_FLAGS='$(DEBUG_OPT_FLAGS)' \
		INCLUDES='$(CUDA_INCLUDES)' \
		LIBS='$(CUDA_LIBS)'
check:
	@bin/cattle_test.exe
clean:
	$(RM) $(OBJECTS) $(TARGET)
		@cd $(GTEST_MAKE_PATH) && make clean && cd $(CURDIR)
.depend:
	$(CC) -MM $(CFLAGS) $(SOURCES) > $@
include .depend