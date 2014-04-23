.PHONY: clean All

All:
	@echo "----------Building project:[ plex1 - Debug ]----------"
	@$(MAKE) -f  "plex1.mk"
clean:
	@echo "----------Cleaning project:[ plex1 - Debug ]----------"
	@$(MAKE) -f  "plex1.mk" clean
