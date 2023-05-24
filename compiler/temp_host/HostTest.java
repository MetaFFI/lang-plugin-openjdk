
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