package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"testing"

	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

const idlHost = `{"idl_filename": "test","idl_extension": ".json","idl_filename_with_extension": "test.json", "target_language": "test", "idl_full_path": "","modules": [{"name": "TestModule","comment": "Comments for TestModule\n","tags": null,"functions": [{"name": "f1","comment": "F1 comment\nparam1 comment\n","function_path": {"module": "$PWD/temp","package": "GoFuncs","function": "f1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{"name": "p1","type": "float64","comment": "= 3.141592","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p2","type": "float32","comment": "= 2.71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p3","type": "int8","comment": "= -10","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p4","type": "int16","comment": "= -20","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p5","type": "int32","comment": "= -30","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p6","type": "int64","comment": "= -40","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p7","type": "uint8","comment": "= 50","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p8","type": "uint16","comment": "= 60","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p9","type": "uint32","comment": "= 70","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p10","type": "uint64","comment": "= 80","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p12","type": "string8","comment": "= This is an input","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p13","type": "string8","comment": "= {element one, element two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "p14","type": "uint8","comment": "= {2, 4, 6, 8, 10}","tags": null,"dimensions": 1,"pass_method": ""}],"return_values": [{"name": "r1","type": "float64","comment": "= 0.57721","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r2","type": "float32","comment": "= 3.359","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r3","type": "int8","comment": "= -11","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r4","type": "int16","comment": "= -21","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r5","type": "int32","comment": "= -31","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r6","type": "int64","comment": "= -41","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r7","type": "uint8","comment": "= 51","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r8","type": "uint16","comment": "= 61","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r9","type": "uint32","comment": "= 71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r10","type": "uint64","comment": "= 81","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r12","type": "string8","comment": "= This is an output","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r13","type": "string8","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "r14","type": "uint8","comment": "= {20, 40, 60, 80, 100}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]}`
const javaHostTest = `
import metaffi_host.*;
public class HostTest {
    public static void main(String[] args) {
        try { /* This function expects the parameters (in that order):     func_id = 0     double = 3.141592     float = 2.71f     int8 = -10     int16 = -20     int32 = -30     int64 = -40     uint8 = 50     uint16 = 60     uint32 = 70     uint64 = 80     bool = 1     string = "This is an input"     string[] = ["element one", "element two"]     bytes = [2, 4, 6, 8, 10] */
            TestModule.load("TestModule");
            var res = TestModule.f1(3.141592, 2.71f, (byte) - 10, (short) - 20, -30, -40L, (byte) 50, (short) 60, 70, 80L, true, "This is an input", new String[] {
                "element one",
                "element two"
            }, new byte[] {
                2,
                4,
                6,
                8,
                10
            }); /* This function returns:     double = 0.57721     float = 3.359f     int8 = -11     int16 = -21     int32 = -31     int64 = -41     uint8 = 51     uint16 = 61     uint32 = 71     uint64 = 81     bool = 1     string = "This is an output"     string[] = ["return one", "return two"]     bytes = [20, 40, 60, 80, 100] */
            if (res.r1 != 0.57721) {
                System.out.println("r1 is incorrect");
                System.exit(1);
            }
            if (res.r2 != 3.359f) {
                System.out.println("r2 is incorrect");
                System.exit(1);
            }
            if (res.r3 != -11) {
                System.out.println("r3 is incorrect");
                System.exit(1);
            }
            if (res.r4 != -21) {
                System.out.println("r4 is incorrect");
                System.exit(1);
            }
            if (res.r5 != -31) {
                System.out.println("r5 is incorrect");
                System.exit(1);
            }
            if (res.r6 != -41) {
                System.out.println("r6 is incorrect");
                System.exit(1);
            }
            if (res.r7 != 51) {
                System.out.println("r7 is incorrect");
                System.exit(1);
            }
            if (res.r8 != 61) {
                System.out.println("r8 is incorrect");
                System.exit(1);
            }
            if (res.r9 != 71) {
                System.out.println("r9 is incorrect");
                System.exit(1);
            }
            if (res.r10 != 81) {
                System.out.println("r10 is incorrect");
                System.exit(1);
            }
            if (!res.r11) {
                System.out.println("r11 is incorrect");
                System.exit(1);
            }
            if (!res.r12.equals("This is an output")) {
                System.out.println("r12 is incorrect");
                System.exit(1);
            }
            if (res.r13.length != 2) {
                System.out.println("r13 size is incorrect");
                System.exit(1);
            }
            if (!res.r13[0].equals("return one")) {
                System.out.printf("r13[0] is incorrect. r13[0]=%s\n", res.r13[0]);
                System.exit(1);
            }
            if (!res.r13[1].equals("return two")) {
                System.out.println("r13[1] is incorrect");
                System.exit(1);
            }
            if (res.r14.length != 5) {
                System.out.println("r14 size is incorrect");
                System.exit(1);
            }
            if (res.r14[0] != 20) {
                System.out.println("r14[0] is incorrect");
                System.exit(1);
            }
            if (res.r14[1] != 40) {
                System.out.println("r14[1] is incorrect");
                System.exit(1);
            }
            if (res.r14[2] != 60) {
                System.out.println("r14[2] is incorrect");
                System.exit(1);
            }
            if (res.r14[3] != 80) {
                System.out.println("r14[3] is incorrect");
                System.exit(1);
            }
            if (res.r14[4] != 100) {
                System.out.println("r14[4] is incorrect");
                System.exit(1);
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
`

// --------------------------------------------------------------------
func TestJavaHost(t *testing.T) {

	// 1. Compile IDL to create Java host code
	def, err := IDL.NewIDLDefinitionFromJSON(idlHost)
	if err != nil {
		t.Fatal(err)
	}

	_ = os.RemoveAll("temp_host")

	err = os.Mkdir("temp_host", 0700)
	if err != nil {
		t.Fatal(err)
		return
	}

	defer func() {
		err = os.RemoveAll("temp_host")
		if err != nil {
			t.Fatal(err)
			return
		}
	}()

	cmp := NewHostCompiler()
	err = cmp.Compile(def, "temp_host", "HostTest", nil)
	if err != nil {
		t.Fatal(err)
		return
	}

	_ = os.Chdir("./temp_host")

	// 2. Use MetaFFI code to call from Java to "test function" in "xllr.test"
	_ = ioutil.WriteFile("HostTest.java", []byte(javaHostTest), 0600)

	var sep string
	if runtime.GOOS == "windows" {
		sep = ";"
	} else {
		sep = ":"
	}

	cmd := exec.Command("javac", "-cp", fmt.Sprintf(".%vHostTest_MetaFFIHost.jar%v"+os.Getenv("METAFFI_HOME")+"/openjdk/xllr.openjdk.bridge.jar", sep, sep), "HostTest.java")
	wd, _ := os.Getwd()
	cmd.Dir = wd
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("%v\n", strings.Join(cmd.Args, " "))
	err = cmd.Run()
	if err != nil {
		t.Fatalf("Failed compiling javac test host: %v", err)
	}

	cmd = exec.Command("java", "-cp", fmt.Sprintf(".%vHostTest_MetaFFIHost.jar%v"+os.Getenv("METAFFI_HOME")+"/openjdk/xllr.openjdk.bridge.jar", sep, sep), "HostTest")
	cmd.Dir = wd
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	fmt.Printf("%v\n", strings.Join(cmd.Args, " "))
	err = cmd.Run()

	if err != nil {
		t.Fatalf("Failed running java test host: %v", err)
	}

}

//--------------------------------------------------------------------
