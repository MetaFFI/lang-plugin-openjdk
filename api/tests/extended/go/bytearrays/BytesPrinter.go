package bytesprinter


func PrintBytesArrays(bytesArrays [][]byte){
	for i, bytes := range bytesArrays {
		println("bytesArrays[", i, "] = ", bytes)
	}

	return bytesArrays
}