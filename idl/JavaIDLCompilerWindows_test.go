//go:build windows
// +build windows

package main

import (
	"io/ioutil"
	"os"
	"testing"
)

var src string = `
package sanity;

import java.util.HashMap;

// This is the Main class
public class TestClass
{
	public TestClass(){}

	// prints hello world
	public static void HelloWorld(){
		System.out.println("Hello World, from Java");
	}

	public static void ReturnsAnError() throws Exception{
		throw new Exception("Error");
	}

	public static double div_integers(int x, int y){
		return x / y;
	}

	public static String JoinStrings(String[] arr)
	{
		return String.join(",", arr);
	}

	public static int FiveSeconds = 5;
	public static void WaitABit(int secs) throws InterruptedException
	{
		Thread.sleep(secs*1000);
	}

	public static class TestMap
	{
		public String Name;
		public HashMap<String, Object> dict = new HashMap<String, Object>();

		public TestMap(){}

		public void set(String k, Object v){
			this.dict.put(k, v);
		}

		public Object get(String k){
			return this.dict.get(k);
		}

		public boolean contains(String k){
			return this.dict.containsKey(k);
		}
	}
}
`

func TestGoIDLCompiler_Compile(t *testing.T) {
	ioutil.WriteFile("TestClass.java", []byte(src), 0600)
	defer os.Remove("TestClass.java")

	comp := NewJavaIDLCompiler()

	idl, _, err := comp.ParseIDL(src, "TestClass.java")
	if err != nil {
		t.Fatal(err)
	}

	resjson, err := idl.ToJSON()
	if err != nil {
		t.Fatal(err)
	}

	println(resjson)
}
