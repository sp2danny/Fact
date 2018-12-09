.PHONY: clean All

All:
	@echo "----------Building project:[ factmm - Release ]----------"
	@"$(MAKE)" -f  "factmm.mk"
clean:
	@echo "----------Cleaning project:[ factmm - Release ]----------"
	@"$(MAKE)" -f  "factmm.mk" clean
