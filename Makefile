.PHONY: clean All

All:
	@echo "----------Building project:[ Server - Release ]----------"
	@$(MAKE) -f  "Server.mk"
clean:
	@echo "----------Cleaning project:[ Server - Release ]----------"
	@$(MAKE) -f  "Server.mk" clean
