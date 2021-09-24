CXX ?= g++

# path #
SRC_PATH = source
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin

# extensions #
SRC_EXT = cpp

.PHONY: default
default: dirs
	@echo "Compiling: replaceRadioMetadata"
	$(CXX) $(SRC_PATH)/replaceRadioMetadata.$(SRC_EXT) -o $(BIN_PATH)/replaceRadioMetadata
	@echo "Compiling: copyJPG"
	$(CXX) $(SRC_PATH)/copyJPG.$(SRC_EXT) -o $(BIN_PATH)/copyJPG

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)
