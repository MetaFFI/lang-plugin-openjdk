package metaffi;

public class MetaFFIHandle
{
	public long handle;
	public long runtime_id;

	public MetaFFIHandle(long val, long runtime_id)
	{
		this.handle = val;
		this.runtime_id = runtime_id;
	}

	public long Handle()
	{
		return this.handle;
	}

	public long RuntimeID()
	{
		return this.runtime_id;
	}
}
