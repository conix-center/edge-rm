# Include the nanopb provided Makefile rules
include ../../support/nanopb/extra/nanopb.mk

# Build rule for the protocol
all: ../../../proto/messages.proto
	$(PROTOC) $(PROTOC_OPTS) --proto_path=../../../proto --nanopb_out=. ../../../proto/messages.proto

