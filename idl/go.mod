module github.com/MetaFFI/lang-plugin-openjdk/idl

go 1.21.4

toolchain go1.22.0

require (
	github.com/MetaFFI/lang-plugin-go/api v0.0.0-20240308210859-fe74e9c86f50
	github.com/MetaFFI/lang-plugin-go/go-runtime v0.0.0-20240308210859-fe74e9c86f50
	github.com/MetaFFI/plugin-sdk v0.0.0-20240314145634-aa0d103e53e7
)

replace github.com/MetaFFI/plugin-sdk => ../plugin-sdk
