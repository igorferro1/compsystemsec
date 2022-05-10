fuzzer: help.c
ifdef extractor_path
	gcc help.c -o fuzzer
	./fuzzer $(extractor_path)
else
	@echo You have to pass the extractor path as a parameter:
	@echo extractor_path=path/to/extractor
endif