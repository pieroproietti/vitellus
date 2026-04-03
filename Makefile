# artisan/Makefile

# Directories
OA_DIR = oa
COA_DIR = coa

# Binaries
OA_BIN = $(OA_DIR)/oa
COA_BIN = $(COA_DIR)/coa

all: build_oa build_coa
	@echo "--------------------------------------"
	@echo "Hatching completed successfully! 🐣"
	@echo "Workhorse (C):  ./$(OA_BIN)"
	@echo "Brain (Go):   ./$(COA_BIN)"
	@echo "--------------------------------------"

build_oa:
	@echo "  MAKING oa..."
	@$(MAKE) -C $(OA_DIR)

build_coa:
	@echo "  MAKING coa..."
	@cd $(COA_DIR) && go build -o coa ./src/*.go

clean:
	@echo "  Pulizia in corso..."
	@$(MAKE) -C $(OA_DIR) clean
	@rm -f $(COA_BIN)
	@rm -f $(COA_DIR)/plan_coa_tmp.json

.PHONY: all build_oa build_coa clean