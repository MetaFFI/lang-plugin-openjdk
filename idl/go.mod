module github.com/MetaFFI/lang-plugin-openjdk/idl

go 1.21.4

toolchain go1.22.0

require (
	github.com/MetaFFI/lang-plugin-go/api v0.0.0-20240222071555-490e2d6f3fed
	github.com/MetaFFI/lang-plugin-go/go-runtime v0.0.0-20240222071555-490e2d6f3fed
	github.com/MetaFFI/plugin-sdk v0.0.0-20240222071543-b9a6812a8106
)


replace github.com/MetaFFI/plugin-sdk => ../plugin-sdk
