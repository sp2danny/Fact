.PHONY: clean All

All:
	@echo "----------Building project:[ Client - Debug ]----------"
	@$(MAKE) -f  "Client.mk"
clean:
	@echo "----------Cleaning project:[ Client - Debug ]----------"
	@$(MAKE) -f  "Client.mk" clean
