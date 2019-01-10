.PHONY: clean All

All:
	@echo "----------Building project:[ Server - Debug ]----------"
	@$(MAKE) -f  "Server.mk"
clean:
	@echo "----------Cleaning project:[ Server - Debug ]----------"
	@$(MAKE) -f  "Server.mk" clean
