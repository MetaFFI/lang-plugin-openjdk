import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.Caller;
import metaffi.MetaFFIHandle;
import metaffi.MetaFFITypeInfo;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;

import java.lang.management.ManagementFactory;
import java.lang.management.RuntimeMXBean;
import java.util.List;

class Tag
{
	private MetaFFIHandle instance = null;
	private metaffi.Caller pget = null;

	public Tag(MetaFFIHandle h)
	{
		this.instance = h;

		MetaFFIModule tag = BeautifulSoupTest.runtime.loadModule("bs4.element.Tag");

		this.pget = tag.load("callable=Tag.get",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});
	}

	public String get(String attribute)
	{
		return (String)this.pget.call(this.instance, attribute)[0];
	}
}

class PyListClass
{
	private MetaFFIModule mod = null;
	private metaffi.MetaFFIHandle instance = null;
	private metaffi.Caller pget = null;
	private metaffi.Caller pappend = null;
	private metaffi.Caller plen = null;

	public PyListClass()
	{
		mod = BeautifulSoupTest.runtime.loadModule("builtins");
		metaffi.Caller pconstructor = mod.load("callable=list", null, new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});
		this.instance = (metaffi.MetaFFIHandle)pconstructor.call()[0];

		this.pappend = mod.load("callable=list.append,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=list.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=list.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public PyListClass(metaffi.MetaFFIHandle h)
	{
		mod = BeautifulSoupTest.runtime.loadModule("builtins");
		this.instance = h;

		this.pappend = mod.load("callable=list.append,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=list.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=list.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public MetaFFIHandle getHandle()
	{
		return this.instance;
	}

	Object get(int index)
	{
		return this.pget.call(this.instance, index)[0];
	}

	void append(Object o)
	{
		this.pappend.call(this.instance, o);
	}

	long len()
	{
		return (long)this.plen.call(this.instance)[0];
	}
}

class BeautifulSoup
{
	private metaffi.Caller find_all = null;
	private MetaFFIHandle instance = null;

	public BeautifulSoup(String source, String parser)
	{
		MetaFFIModule bs4 = BeautifulSoupTest.runtime.loadModule("bs4");

		var constructor = bs4.load("callable=BeautifulSoup",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.find_all = bs4.load("callable=BeautifulSoup.find_all",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		// call constructor
		this.instance = (MetaFFIHandle)constructor.call(source, parser)[0];
	}

	public Tag[] FindAll(String tag)
	{
		var handles = this.find_all.call(this.instance, tag)[0];

		PyListClass list = new PyListClass((MetaFFIHandle)handles);

		Tag[] tags = new Tag[(int)list.len()];
		for(int i=0 ; i<(int)list.len() ; i++)
		{
			tags[i] = new Tag((MetaFFIHandle)list.get(i));
		}

		return tags;
	}
}

class Response
{
	private MetaFFIHandle instance = null;
	private Caller getText = null;

	public Response(MetaFFIHandle h)
	{
		MetaFFIModule response = BeautifulSoupTest.runtime.loadModule("requests.Response");
		this.getText = response.load("callable=Response.text.fget,instance_required",
					new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
					new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		this.instance = h;
	}

	public String text()
	{
		return (String)this.getText.call(this.instance)[0];
	}
}

class Requests
{
	private metaffi.Caller requestsGet = null;

	public Requests()
	{
		MetaFFIModule requests = BeautifulSoupTest.runtime.loadModule("requests");

		requestsGet = requests.load("callable=get",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});
	}

	public Response get(String url)
	{
		MetaFFIHandle resHandle = (MetaFFIHandle)requestsGet.call(url)[0];
		return new Response(resHandle);
	}
}

public class BeautifulSoupTest
{
	public static api.MetaFFIRuntime runtime = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python3");
		runtime.loadRuntimePlugin();
	}

	@AfterClass
	public static void fini()
	{
		if(runtime != null)
			runtime.releaseRuntimePlugin();
	}

	@Test
	public void testBeautifulSoup()
	{
		/*
			url = 'https://www.microsoft.com'
		    response = requests.get(url)
		    soup = BeautifulSoup(response.text, 'html.parser')

		    for link in soup.find_all('a'):
		        print(link.get('href'))
	    */

		var req = new Requests();
		var res = req.get("https://microsoft.com/");
		var bs = new BeautifulSoup(res.text(), "html.parser");

		var links = bs.FindAll("a");

		Assert.assertTrue(links.length > 0);

		for(Tag t : links)
		{
			System.out.println(t.get("href"));
		}
	}

}
