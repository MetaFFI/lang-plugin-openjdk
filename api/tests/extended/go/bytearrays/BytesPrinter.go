package bytesprinter

import "fmt"

func PrintBytesArrays(bytesArrays [][]byte) [][]byte{
	for _, bytes := range bytesArrays {
		fmt.Printf("%v\n", bytes)
	}

	return bytesArrays
}
