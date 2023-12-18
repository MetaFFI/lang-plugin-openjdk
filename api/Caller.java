package api;

import java.util.function.Function;

public class Caller
{
	private final Function<Object, Object[]> f;

	public Caller(Function<Object,Object[]> f)
	{
		this.f = f;
	}

	public Object[] call(Object... parameters)
	{
		return this.f.apply(parameters);
	}
}
