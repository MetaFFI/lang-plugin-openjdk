package main

import (
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"os"
	"testing"
)

const idl_guest = `{"idl_filename": "test","idl_extension": ".json","idl_filename_with_extension": "test.json", "target_language": "test", "idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": null,"functions": [{"name": "f1","comment": "F1 comment\nparam1 comment\n","tags": null,"function_path": {"module": "GuestCode", "package":"GuestCode","function": "f1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{"name": "p1","type": "float64","comment": "= 3.141592","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p2","type": "float32","comment": "= 2.71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p3","type": "int8","comment": "= -10","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p4","type": "int16","comment": "= -20","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p5","type": "int32","comment": "= -30","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p6","type": "int64","comment": "= -40","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p7","type": "uint8","comment": "= 50","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p8","type": "uint16","comment": "= 60","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p9","type": "uint32","comment": "= 70","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p10","type": "uint64","comment": "= 80","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p12","type": "string8","comment": "= This is an input","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p13","type": "string8","comment": "= {element one, element two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "p14","type": "uint8","comment": "= {2, 4, 6, 8, 10}","tags": null,"dimensions": 1,"pass_method": ""}],"return_values": [{"name": "r1","type": "float64","comment": "= 0.57721","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r2","type": "float32","comment": "= 3.359","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r3","type": "int8","comment": "= -11","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r4","type": "int16","comment": "= -21","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r5","type": "int32","comment": "= -31","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r6","type": "int64","comment": "= -41","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r7","type": "uint8","comment": "= 51","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r8","type": "uint16","comment": "= 61","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r9","type": "uint32","comment": "= 71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r10","type": "uint64","comment": "= 81","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r12","type": "string8","comment": "= This is an output","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r13","type": "string8","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "r14","type": "uint8","comment": "= {20, 40, 60, 80, 100}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]}`

const GuestCode = `

public class GuestCode
{
	public static class f1result
	{
		public double r1 = 0.57721;
	    public float r2 = 3.359f;
	
	    public byte r3 = -11;
	    public short r4 = -21;
	    public int r5 = -31;
	    public long r6 = -41;
	
		public byte r7 = 51;
	    public short r8 = 61;
	    public int r9 = 71;
	    public long r10 = 81;
		
	    public boolean r11 = true;
	
	    public string r12 = "This is an output";
	    public string[] r13 = new string[]{"return one", "return two"};
	
	    public byte[] r14 = new byte[]{(byte)20, (byte)40, (byte)60, (byte)80, (byte)100};
	}

	public static f1result f1(double p1, float p2, byte p3, short p4, int p5, long p6, byte p7, short p8, int p9, long p10, boolean p11, String p12, String[] p13, byte[] p14)
	{
		/*
			This function expects the parameters (in that order):
			double = 3.141592
		    float = 2.71f
		
		    int8 = -10
		    int16 = -20
		    int32 = -30
		    int64 = -40
	
		    uint8 = 50
		    uint16 = 60
		    uint32 = 70
		    uint64 = 80
	
		    bool = 1
	
		    string = "This is an input"
		    string[] = {"element one", "element two"}
	
		    bytes = {2, 4, 6, 8, 10}
		*/
	
	
		System.out.println("Hello from Java F1");
	
		if(p1 != 3.141592):
			raise RuntimeError("p1 != 3.141592. p1 = "+str(p1))
		
	
		if( p2 != 2.71f )
			raise RuntimeError("p2 - 2.71 > 0.000001. p2 = "+str(p2))
	
		if(p3 != -10)
			raise RuntimeError("p3 != -10. p3 = "+str(p3))
		
	
		if(p4 != -20)
			raise RuntimeError("p4 != -20")
		
	
		if(p5 != -30)
			raise RuntimeError("p5 != -30")
		
	
		if(p6 != -40)
			raise RuntimeError("p6 != -40")
		
	
		if(p7 != 50)
			raise RuntimeError("p7 != 50")
		
	
		if(p8 != 60)
			raise RuntimeError("p8 != 60")
		
	
		if(p9 != 70)
			raise RuntimeError("p9 != 70")
		
	
		if(p10 != 80)
			raise RuntimeError("p10 != 80")
		
	
		if(!p11)
			raise RuntimeError("p11 == false")
		
	
		if(!p12.equals("This is an input"))
			raise RuntimeError("p12 != \"This is an input\"")
		
	
		if(p13.length != 2)
			raise RuntimeError("len(p13) != 2")
	
	
		if(!p13[0].equals("element one"))
			raise RuntimeError("p13[0] != \"element one\"")
	
	
		if(!p13[1].equals("element two"))
			raise RuntimeError("p13[1] != \"element two\"")
		
	
		if(len(p14) != 5)
			raise RuntimeError("len(p14) != 5")
	
		if(p14[0] != 2 or p14[1] != 4 or p14[2] != 6 or p14[3] != 8 or p14[4] != 10)
			raise RuntimeError("p14[0] != 2 or p14[1] != 4 or p14[2] != 6 or p14[3] != 8 or p14[4] != 10")
	
		"""
			double = 0.57721
		    float = 3.359f
		
		    int8 = -11
		    int16 = -21
		    int32 = -31
		    int64 = -41
		
		    uint8 = 51
		    uint16 = 61
		    uint32 = 71
		    uint64 = 81
		
		    bool = 1
		
		    string = "This is an output"
		    string[] = ["return one", "return two"]
		
		    bytes = [20, 40, 60, 80, 100]
		"""

		return new f1result();
	}
}
`

//--------------------------------------------------------------------
func TestJavaExtractorGuest(t *testing.T) {
	
	t.Skip("Java IDL plugin not ready yet")
	
	const pathIDLPlugin = "../../idl-plugin-java/"
	
	idlJavaExtractor, err := os.ReadFile(pathIDLPlugin + "JavaExtractor.json")
	if err != nil {
		t.Fatal(err)
	}
	
	def, err := IDL.NewIDLDefinitionFromJSON(string(idlJavaExtractor))
	if err != nil {
		t.Fatal(err)
	}
	
	err = os.Mkdir("temp_guest2", 0700)
	if err != nil {
		t.Fatal(err)
		return
	}
	
	defer func() {
		err = os.RemoveAll("temp_guest2")
		if err != nil {
			t.Fatal(err)
			return
		}
	}()
	
	cmp := NewGuestCompiler()
	err = cmp.Compile(def, "temp_guest2", "", "", "")
	if err != nil {
		t.Fatal(err)
		return
	}
	
}

//--------------------------------------------------------------------
