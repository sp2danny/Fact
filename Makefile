.PHONY: clean All

All:
	@echo "----------Building project:[ mmhp - Release ]----------"
	@"$(MAKE)" -f  "mmhp.mk"
clean:
	@echo "----------Cleaning project:[ mmhp - Release ]----------"
	@"$(MAKE)" -f  "mmhp.mk" clean
