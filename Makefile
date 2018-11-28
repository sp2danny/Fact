.PHONY: clean All

All:
	@echo "----------Building project:[ Fact - Debug ]----------"
	@$(MAKE) -f  "Fact.mk"
clean:
	@echo "----------Cleaning project:[ Fact - Debug ]----------"
	@$(MAKE) -f  "Fact.mk" clean
