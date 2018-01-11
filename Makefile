.PHONY: clean All

All:
	@echo "----------Building project:[ BranchPrediction - Debug ]----------"
	@cd "BranchPrediction" && "$(MAKE)" -f  "BranchPrediction.mk"
clean:
	@echo "----------Cleaning project:[ BranchPrediction - Debug ]----------"
	@cd "BranchPrediction" && "$(MAKE)" -f  "BranchPrediction.mk" clean
