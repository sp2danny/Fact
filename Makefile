.PHONY: clean All

All:
	@echo "----------Building project:[ mm - Release ]----------"
	@"$(MAKE)" -f  "mm.mk"
clean:
	@echo "----------Cleaning project:[ mm - Release ]----------"
	@"$(MAKE)" -f  "mm.mk" clean
