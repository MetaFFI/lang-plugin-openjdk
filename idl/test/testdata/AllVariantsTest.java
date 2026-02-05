package testdata;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AllVariantsTest implements Runnable, Serializable
{
	public static final long serialVersionUID = 1L;

	public int intField = 1;
	public static String staticField = "static";
	private double privateField = 1.5;
	protected int[] intArray = new int[] {1, 2, 3};
	public String[][] stringMatrix = new String[][] {{"a", "b"}, {"c"}};
	public List<String> listField = new ArrayList<>();
	public Map<String, Integer> mapField = new HashMap<>();

	public AllVariantsTest()
	{
	}

	public AllVariantsTest(int x)
	{
		this.intField = x;
	}

	public int add(int a, int b)
	{
		return a + b;
	}

	public long add(long a, long b)
	{
		return a + b;
	}

	public void doNothing()
	{
	}

	public String join(String... parts)
	{
		return String.join(",", parts);
	}

	public int[] returnsArray()
	{
		return new int[] {1, 2, 3};
	}

	public String[][] returnsMatrix()
	{
		return new String[][] {{"x", "y"}};
	}

	public <T> T echo(T value)
	{
		return value;
	}

	public void throwsException() throws Exception
	{
		throw new Exception("boom");
	}

	@Override
	public void run()
	{
		doNothing();
	}

	public enum Color
	{
		RED,
		GREEN,
		BLUE
	}

	public interface Callback
	{
		int call(int x);
	}

	public static class StaticNested
	{
		public int value;

		public StaticNested(int v)
		{
			this.value = v;
		}
	}

	public class Inner
	{
		public int mult(int x)
		{
			return x * intField;
		}
	}

	@Deprecated
	public static class Annotated
	{
		public String name;
	}

	public static class Builder
	{
		private int v;

		public Builder v(int v)
		{
			this.v = v;
			return this;
		}

		public AllVariantsTest build()
		{
			return new AllVariantsTest(v);
		}
	}
}
