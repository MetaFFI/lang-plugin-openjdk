package sanity;

class SomeClass
{
	public void print()
	{
		System.out.println("Hello from inner class");
	}
}

public class TestRuntime
{
	private TestRuntime(){}

	public static void helloWorld()
	{
		System.out.println("Hello World - From Java");
	}

	public static void returnsAnError() throws Exception
	{
		System.out.println("Going to throw exception");
		throw new Exception("Returning an error");
	}

	public static float divIntegers(int x, int y) throws ArithmeticException
	{
		if(y == 0)
			throw new ArithmeticException("Divisor is 0");

		return (float)x / (float)y;
	}

	public static String joinStrings(String[] arr)
	{
		System.out.println("joining strings");
		return String.join(",", arr);
	}

	public static final int fiveSeconds = 5;
	public static void waitABit(int seconds) throws InterruptedException
	{
		System.out.printf("Sleeping for %d seconds\n", seconds);
		Thread.sleep(seconds * 1000);
	}

	public static SomeClass[] getSomeClasses()
	{
		return new SomeClass[]{new SomeClass(), new SomeClass(), new SomeClass()};
	}

	public static void expectThreeSomeClasses(SomeClass[] arr)
	{
		if(arr.length != 3)
			throw new IllegalArgumentException("Array length is not 3");
	}

	public static void expectThreeBuffers(byte[][] buffers)
	{
		if(buffers.length != 3)
			throw new IllegalArgumentException("Buffers length is not 3");
	}

	public static byte[][] getThreeBuffers()
	{
		byte[][] buffers = new byte[3][];
		for(int i = 0; i < 3; i++)
		{
			buffers[i] = new byte[]{1, 2, 3};
		}
		return buffers;
	}
}