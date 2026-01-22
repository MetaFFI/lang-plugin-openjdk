module github.com/MetaFFI/lang-plugin-jvm/compiler

go 1.23.0

toolchain go1.24.4

require (
	github.com/MetaFFI/sdk/compiler/go v0.0.0
	github.com/MetaFFI/sdk/idl_entities/go v0.0.0
	golang.org/x/text v0.24.0
)

replace github.com/MetaFFI/sdk/compiler/go => ../../sdk/compiler/go

replace github.com/MetaFFI/sdk/idl_entities/go => ../../sdk/idl_entities/go
