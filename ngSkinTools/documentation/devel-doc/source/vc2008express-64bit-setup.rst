Visual Studio 2008 and 64bit compilation
----------------------------------------


this is a brief compilation of various guids on the internet to setup VC2008 for 64bit compilation

* install vc2008express
* install Windows SDK for Windows 7 and .NET Framework 3.5 SP1
	* ! not the 4.0 .NET version !
	
* follow http://jenshuebel.wordpress.com/2009/02/12/visual-c-2008-express-edition-and-64-bit-targets/: 

	* Copy keys from
	 [HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\9.0\VC\VC_OBJECTS_PLATFORM_INFO]
	 to 
	 [HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VCExpress\9.0\VC\VC_OBJECTS_PLATFORM_INFO]
	 
	* copy keys from 
	[HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\9.0\CLSID\]
	to
	[HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VCExpress\9.0\CLSID\]
	
	(recommended only those:)
		{656d8763-2429-11d7-8bf6-00b0d03daa06}
		{656d8760-2429-11d7-8bf6-00b0d03daa06}
		{656d8763-2429-11d7-8bf6-00b0d03daa06}
		{656d875f-2429-11d7-8bf6-00b0d03daa06}
		
		
	* rename config files for express:
		c:\program files\Microsoft Visual Studio 9.0\AMD64.VCPlatform.config
		to 	
		c:\program files\Microsoft Visual Studio 9.0\AMD64.VCPlatform.Express.config
	 	