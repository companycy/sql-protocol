http://support.microsoft.com/kb/190463
http://stackoverflow.com/questions/103167/what-is-the-difference-between-ole-db-and-odbc-data-sources
http://stackoverflow.com/questions/271504/oledb-v-s-odbc/271517#271517
https://jamesmccaffrey.wordpress.com/2006/05/02/odbc-vs-ole-db/
http://books.google.ca/books?id=JTKX7OUqCEMC&lpg=PP1&dq=contact%20jason%20roff%20activex%20data%20objects&pg=PA12#v=onepage&q=contact%20jason%20roff%20activex%20data%20objects&f=false
http://books.google.ca/books?id=JTKX7OUqCEMC&lpg=PP1&dq=contact%20jason%20roff%20activex%20data%20objects&pg=PA12#v=onepage&q=contact%20jason%20roff%20activex%20data%20objects&f=false

http://www.cnblogs.com/liuzhendong/archive/2012/01/29/2331189.html

<!-- oledb -->
http://technet.microsoft.com/en-us/library/aa198252(v=sql.80).aspx
http://support.microsoft.com/kb/190463

<!-- download links -->
http://msdn.microsoft.com/en-us/data/aa937695.aspx
<!-- download links for mdac -->
http://msdn.microsoft.com/en-us/data/aa937730
<!-- comparison between oledb and odbc in c programming -->
http://msdn.microsoft.com/en-us/library/ms810892.aspx

<!-- create test.udl to get connection string for oledb -->
http://blogs.msdn.com/b/chaitanya_medikonduri/archive/2008/04/09/how-to-run-32-bit-udl-file-on-a-64-bit-operating-system.aspx

db2oledb.dll             -> The OLE DB provider for DB2.
Sqloledb.dll             -> Dynamic-link library that implements the SQLOLEDB provider.  Program files\Common files\System\Ole db



ADOID.LIB                -> GUID library for client C++ classes
Data Links library       -> MSDASC.LIB
The OLE DB library       -> OLEDB.LIB
The OLE DB debug library -> OLEDBD.LIB

<!-- Program and DLL Files for Data Integration -->
http://msdn.microsoft.com/en-US/library/ee252113(v=bts.10).aspx


This requires references to a few COM objects:

%PROGRAMFILES%\Microsoft.NET\Primary Interop Assemblies\adodb.dll <!-- it's .net from vs ide -->
%PROGRAMFILES%\Common Files\System\Ole DB\OLEDB32.DLL



When you add a COM component to a .NET project, an assembly with an Interop prefix is added to the project.
For example, adding the service component adds a dynamically generated wrapper for the COM component named Interop.MSDASC.dll.


Dynamic-link library that implements the SQLOLEDB provider. Sqloledb.dll Program files\Common files\System\Ole db
OLE DB SDK header file for OLE DB providers and consumers.  Oledb.h      Program Files\Microsoft SQL Server\80\Tools\DevTools\Include
Header file used for developing SQLOLEDB consumers.         Sqloledb.h   Program Files\Microsoft SQL Server\80\Tools\DevTools\Include
Library file used for developing SQLOLEDB consumers.        Oledb.lib    Program Files\Microsoft SQL Server\80\Tools\Dev Tools\Lib



Typically you would use CoCreateInstance() to instantiate an object from a COM DLL.
When you do this, there's no need to load the DLL first and get proc addresses like you would need to do with a normal DLL.
This is because Windows "knows" about the types that a COM DLL implements, what DLL they are
implemented in, and how to instantiate them. (Assuming of course that the COM DLL is registered, which it typically is).
