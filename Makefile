.PHONY: clean All

All:
	@echo "----------Building project:[ Client - Release ]----------"
	@"$(MAKE)" -f  "Client.mk"
clean:
	@echo "----------Cleaning project:[ Client - Release ]----------"
	@"$(MAKE)" -f  "Client.mk" clean
