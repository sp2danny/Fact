.PHONY: clean All

All:
	@echo "----------Building project:[ mmhp - Debug ]----------"
	@$(MAKE) -f  "mmhp.mk"
clean:
	@echo "----------Cleaning project:[ mmhp - Debug ]----------"
	@$(MAKE) -f  "mmhp.mk" clean
