.PHONY: clean All

All:
	@echo "----------Building project:[ Fact - Release ]----------"
	@$(MAKE) -f  "Fact.mk"
clean:
	@echo "----------Cleaning project:[ Fact - Release ]----------"
	@$(MAKE) -f  "Fact.mk" clean
