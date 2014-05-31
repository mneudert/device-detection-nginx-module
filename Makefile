default:
	@echo "use 'make lint' to run lint!"

lint:
	@cpplint \
		--root="src" \
		--extensions=c,h \
		--filter="-legal/copyright,-readability/casting" \
		src/*.h src/*.c

.PHONY: default
.DEFAULT_GOAL := default
