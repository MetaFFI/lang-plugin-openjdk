module github.com/MetaFFI/lang-plugin-openjdk/compiler

go 1.21.4

toolchain go1.22.0

require (
	github.com/Masterminds/sprig/v3 v3.2.3
	github.com/MetaFFI/lang-plugin-openjdk/idl v0.0.0-20231116161737-5ab7c32c5996
	github.com/MetaFFI/plugin-sdk v0.0.0-20240418113454-40cb0644f6c7
	golang.org/x/text v0.14.0
)

require (
	dario.cat/mergo v1.0.0 // indirect
	github.com/Masterminds/goutils v1.1.1 // indirect
	github.com/Masterminds/semver/v3 v3.2.1 // indirect
	github.com/google/uuid v1.6.0 // indirect
	github.com/huandu/xstrings v1.4.0 // indirect
	github.com/imdario/mergo v0.3.16 // indirect
	github.com/mitchellh/copystructure v1.2.0 // indirect
	github.com/mitchellh/reflectwalk v1.0.2 // indirect
	github.com/shopspring/decimal v1.4.0 // indirect
	github.com/spf13/cast v1.6.0 // indirect
	golang.org/x/crypto v0.22.0 // indirect
)

replace github.com/MetaFFI/plugin-sdk => ../plugin-sdk

replace github.com/MetaFFI/lang-plugin-openjdk/idl => ../idl
