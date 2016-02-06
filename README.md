Licence : All samples are "Public Domain" code 
http://en.wikipedia.org/wiki/Public_domain_software

===========================================================================
			
**The project description and technical information :** A multithreaded limit order matching engine written in C++11 using FIX protocol 4.2 . 

- It targets both Linux ( tested on CentOS and Ubuntu ) and Windows systems ( tested on Windows 8.1).

- It uses STL,Boost and also platform specific APIs ( POSIX, a few NP POSIX calls and WindowsAPIs) in some places, QuickFix for FIX protocol.

- It supports GCC4.8 and MSVC120 (VS2013). See "source/compiler_portability" 

- Additionally used GoogleTest for unit testing, Bash for Linux test scripts and Powershell for Windows test scripts.

Detailed articles about the project and its implementation details will be available in my blog at www.nativecoding.wordpress.com.
						
**Limit orders and order matching engines :** For limit orders please see : https://en.wikipedia.org/wiki/Order_%28exchange%29#Limit_order

Basically a limit order is an order which you specify a maximum price for the security you want to buy. 
As for the terminology a limit order to buy is called a "bid" and a limit order to sell is called an "ask".

An order matching engine matches the ask orders and the bid orders. This implementation places the highest bid order on
top of bids table and places the lowest ask order on top of asks table. Then it chekcs the bids and asks and tries to 
match the orders. And then it sends status reports back to the owners of the orders. A status of an order can be :

- "accepted" : the server accepted the order and it will be processed
- "filled" : the order matched
- "partially filled" , meaning that some trading happened , but still more to googlecode
- "rejected" , if order type is not supported.
- "canceled" , if a client wants to cancel an order ,and if the order is canceld , the server informs the client

For general information about the trading systems and the order types , please see :
http://www.investopedia.com/university/intro-to-order-types/

**FIX ( Financial information exchange ) protocol :** It is a session based TCP protocol that carries financial security transcation data.

For more information , please see https://en.wikipedia.org/wiki/Financial_Information_eXchange .

For the time being, this projectis using opensource QuickFix engine and FIX specification 4.2.
	
**How multithreading is implemented for order matching:** If you look at the source , the concurrency layer ( "source/concurrent" , using concurrent word since MS using concurrency for their own libraries ) , 
the engine currently is using 1 lock free SPSC ring buffer and other fine grained lock based ring buffer and queues. Also the engine currently makes use of a set of CPU cache aligned allocators for memory allocations. ( "source/memory" ) See end of this readme for future plans.

The engine mainly consists of 2 parts : a FIX server/engine and the order matching layer.

The core of order matching layer is called a central order book, which keeps order books per security symbol.
Each order book has a table for bids and another table for asks. Briefly the multithreaded system works as below :

1. The FIX engine will listen for session requests from the clients, and if a session is established
then it listens for incoming ask/bid orders. If the order type is not supported. It sends "rejected" message. Otherwise the message will be submitted to an incoming message dispatcher which is a fine grained MPSC unbounded queue.
	
2. Central Order book has a thread pool

		- The thread pool will have N SPSC queues for N threads ( N = num of symbol ).
		
		- Central Order book also has 1 MPMC queue for outgoing messages.
		
		- When a new message arrives ( new order, or cancel ) from the FIX engine, it will be submitted to corresponding thread`s queue in the thread pool of the central order book.
		
3. Each thread in the thread pool will get message from their SPSC queue in the thread pool , and add them to corresponding order queue which is used by only itself
and eventually trigger the order matching process for that queue. At the end of the order matching , worker threads will submit messages ( FILLED or PARTIALLY FILLED ) to the outgoing messages queue 

4. Outgoing message processor which is a MPMC queue will process the outgoing messages.

**Build dependencies :** For Linux , the project is built and tested with GCC4.8. As for Windows it is using MSVC1200(VS2013). In the libraries side :

- Boost 1.59 : using a compacted version by using the command below :

				bcp --boost=c:\boost_1_59_0 shared_ptr scoped_ptr any optional tokenizer format c:\boost
						
- QuickFix & its requirements : libxml2-devel & http://www.quickfixengine.org/quickfix/doc/html/install.html

**Runtime dependencies :** For Windows, you have to install MSVC120 runtime : https://www.microsoft.com/en-gb/download/details.aspx?id=40784
For Linux, you need GNU Lib C runtime and libxml2.
			
**How to build the project on Linux :**
	
	cd build/linux
	make clean
	make debug  OR make release

**How to build the project on Linux using Netbeans 8.0.2 C++ IDE:**

	Open Netbeans.
	Open the project from the project directory. ( Choose "nbproject" directory )
	Build the project inside Netbeans IDE.

Why Netbeans : In Netbeans, it is too straightforward to setup remote debugging, therefore it is quite convenient to build and debug on Linux from Windows via SSH and Samba. You can see an article about this setup here in my blog. It is for Debian but it should be trivial to apply it to any other distribution : https://nativecoding.wordpress.com/2014/10/24/configuring-a-debian-virtual-machine-for-linux-c-development-via-windows-step-by-step/
	
**How to build the project on Windows  :**
	
	You can build with Visual Studio 2013
	Go to "build/windows" directory
	Use SLN file to launch VS with the project

**Server parameters and running the matching engine :** The engine executable looks for "ome.ini" file. Here is the list of things you can set :

		- FILE_LOGGING_ENABLED							enables/disables logging
		- CONSOLE_OUTPUT_ENABLED						enables/disables output to stdout
		- CENTRAL_ORDER_BOOK_PIN_THREADS_TO_CORES		whether to pin threads of the threadpool to different CPU cores
		- HYPER_THREADING								if hyperthreading is off and pinning is on ,then it will pin threads to only cores with an even index
		- CENTRAL_ORDER_BOOK_QUEUE_SIZE_PER_THREAD		Queue size per worker thread in the central order book`s thread pool
		- LOG_BUFFER_SIZE								Maximum buffer size for the logging system as it is built on a ring buffer.
		
You will also need to have "quickfix_FIX42.xml" and "quickfix_server.cfg" files to be in the same directory with OME executable. You can find them in "bin" directory.

Once you start the ome executable , initially you will see a screen like this :

		06-02-2016 16:22:00 : INFO , Main thread , starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(0) MSFT starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(1) AAPL starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(2) INTC starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(3) GOOGL starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(4) QCOM starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(5) QQQ starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(6) BBRY starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(7) SIRI starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(8) ZNGA starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(9) ARCP starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(10) XIV starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(11) FOXA starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(12) TVIX starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(13) YHOO starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(14) HBAN starting
		06-02-2016 16:22:00 : INFO , Thread pool , Thread(15) BARC starting
		06-02-2016 16:22:00 : INFO , Incoming message dispatcher , Thread starting
		06-02-2016 16:22:00 : INFO , Outgoing message processor , Thread starting
		06-02-2016 16:22:00 : INFO , FIX Engine , Acceptor started

		Available commands :

				display : Shows all order books in the central order book
				quit : Shutdowns the server
				
**Example log message from the engine :** The engine produces log messages below when it receives 1 buy order with quantity 1 and 1 sell order with quantity 1 for the same symbol :

	06-02-2016 20:16:09 : INFO , FIX Engine , New logon , session ID : FIX.4.2:OME->TEST_CLIENT1
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15435=834=543=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9556=011=414=017=1820=037=438=139=054=155=MSFT150=0151=110=000
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15535=834=643=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9556=011=514=017=1920=037=538=239=054=255=GOOGL150=0151=210=070
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15535=834=743=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=614=017=2020=037=638=139=054=155=GOOGL150=0151=110=060
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16435=834=843=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=414=117=2120=031=132=137=438=139=254=155=MSFT150=2151=010=168
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16435=834=943=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=314=117=2220=031=132=137=338=139=254=255=MSFT150=2151=010=169
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15635=834=1043=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=714=017=2320=037=738=139=054=155=GOOGL150=0151=110=108
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=15435=834=1143=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=011=814=017=2420=037=838=139=854=155=xxx150=8151=110=110
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1243=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=614=117=2520=031=132=137=638=139=254=155=GOOGL150=2151=010=027
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1343=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=514=117=2620=031=132=137=538=239=154=255=GOOGL150=1151=110=028
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1443=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=714=117=2720=031=132=137=738=139=254=155=GOOGL150=2151=010=033
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=16635=834=1543=Y49=OME52=20160206-20:16:09.29556=TEST_CLIENT1122=20160206-20:15:03.9716=111=514=217=2820=031=132=137=538=239=254=255=GOOGL150=2151=010=034
	06-02-2016 20:16:09 : INFO , FIX Engine , Receiving fix message : 8=FIX.4.29=12335=D34=1549=TEST_CLIENT152=20160206-20:16:09.34256=OME11=121=138=140=244=154=255=MSFT59=060=20160206-20:16:0910=124
	06-02-2016 20:16:09 : INFO , FIX Engine , New order message received :8=FIX.4.29=12335=D34=1549=TEST_CLIENT152=20160206-20:16:09.34256=OME11=121=138=140=244=154=255=MSFT59=060=20160206-20:16:0910=124
	06-02-2016 20:16:09 : INFO , Central Order Book , New order accepted, client TEST_CLIENT1, client order ID 1 
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing ACCEPTED for order : Client TEST_CLIENT1 Client ID 1 Symbol MSFT Side SELL 
	06-02-2016 20:16:09 : INFO , Thread pool , MSFT thread got a new task to execute
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=12535=834=2349=OME52=20160206-20:16:09.34256=TEST_CLIENT16=011=114=017=11320=037=138=139=054=255=MSFT150=0151=110=079
	06-02-2016 20:16:09 : INFO , Central Order Book , Order processing for symbol MSFT took 0000000 milliseconds , num of processed orders : 0
	06-02-2016 20:16:09 : INFO , FIX Engine , Receiving fix message : 8=FIX.4.29=12335=D34=1649=TEST_CLIENT152=20160206-20:16:09.34256=OME11=221=138=140=244=154=155=MSFT59=060=20160206-20:16:0910=125
	06-02-2016 20:16:09 : INFO , FIX Engine , New order message received :8=FIX.4.29=12335=D34=1649=TEST_CLIENT152=20160206-20:16:09.34256=OME11=221=138=140=244=154=155=MSFT59=060=20160206-20:16:0910=125
	06-02-2016 20:16:09 : INFO , Central Order Book , New order accepted, client TEST_CLIENT1, client order ID 2 
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing ACCEPTED for order : Client TEST_CLIENT1 Client ID 2 Symbol MSFT Side BUY 
	06-02-2016 20:16:09 : INFO , Thread pool , MSFT thread got a new task to execute
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=12535=834=2449=OME52=20160206-20:16:09.34256=TEST_CLIENT16=011=214=017=11420=037=238=139=054=155=MSFT150=0151=110=082
	06-02-2016 20:16:09 : INFO , Central Order Book , Order processing for symbol MSFT took 0000000 milliseconds , num of processed orders : 2
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing FILLED for order : Client TEST_CLIENT1 Client ID 2 Symbol MSFT Side BUY 
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=13535=834=2549=OME52=20160206-20:16:09.34256=TEST_CLIENT16=111=214=117=11520=031=132=137=238=139=254=155=MSFT150=2151=010=001
	06-02-2016 20:16:09 : INFO , Outgoing message processor , Processing FILLED for order : Client TEST_CLIENT1 Client ID 1 Symbol MSFT Side SELL 
	06-02-2016 20:16:09 : INFO , FIX Engine , Sending fix message : 8=FIX.4.29=13535=834=2649=OME52=20160206-20:16:09.34256=TEST_CLIENT16=111=114=117=11620=031=132=137=138=139=254=255=MSFT150=2151=010=002
	06-02-2016 20:16:11 : INFO , FIX Engine , Logout , session ID : FIX.4.2:OME->TEST_CLIENT1

Note 1: To parse the fix message which can be seen above , you can use one of the online FIX parsers :

		http://fixdecoder.com/fix_decoder.html
		http://fixparser.targetcompid.com/
		http://fix.aprics.net/

Note 2: The executables and scripts help you to send orders to the engine , however if you want to send custom FIX messages, you can use :

		MiniFIX , http://elato.se/minifix/download.html , Windows only with a GUI
		QuickFixMessanger , https://github.com/jramoyo/quickfix-messenger

**Functional testing :** There is a prebuilt executable for both Linux and Windows which can send specified ask/bid orders to the order matching engine.
   
   Under "test_functional" directory :
   
		1. Modify test_data.txt which has the orders to send to the engine as you wish.
		
		2. Modify  the arrays declared on top of client_automated_test.sh/client_automated_test.ps1 script files in order to configure the number of clients. You should provide a name for each client.
		
		3. For firing Linux test client(s), you can use client_automated_test.sh Bash script.
		
		4. For firing Windows test client(s), you can use client_automated_test.bat which drives client_automated_test.ps1 Powershell script.
		
		5. After firing the script, it will be executing all orders in test_data.txt file per client that is declared in the script file.
		
**Unit testing :** The project uses GoogleTest 1.7. You can find a makefile and vcproj under "test_unit" directory.
	
**Building and running unit test on Linux :** You have to build and install Google Test 1.7 first , the instructions for CentOS and Ubuntu :
			
			$ wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
			$ unzip gtest-1.7.0.zip
			$ cd gtest-1.7.0
			$ ./configure
			$ make
			$ sudo cp -a include/gtest /usr/include
			
			On CentOS
			$ sudo cp -a lib/.libs/* /usr/lib64
			
			On Ubuntu
			$ sudo cp -a lib/.libs/* /usr/lib/
			
			Then you can either use Makefile or Netbeans project files under "test_unit" directory.

**Building and running unit test on Windows :** You can use VisualStudio solution in "test_unit" directory.

**Source code and file/directory naming conventions :**
	
	directory names					lower_case_word
	file names 						lower_case_word
	include guards 					_HEADER_H_
	macros							UPPER_CASE_WORD
	enums							UPPER_CASE_WORD
	namespace names 				lower_case_word
	class names 					CamelCase
	method names 					pascalCase
	variable names					pascalCase
	member variables starts with	m_

**Source code indentations and new line usage :**
	
	Space based no tabs ( This needs to be setup in VS )
	By default Netbeans editor uses spaces for tabs
	Needs to be set in VS2013 : https://msdn.microsoft.com/en-gb/library/ms165330(v=vs.90).aspx
	New lines : Unix CR only ( \n ) , VS can handle it even though Windows is \r\n

**Warning level used for GCC :** -Wall

**Warning level used for MSVC :** /W3
		
**Precompiled header file usage :** On Windows , the project is using /FI ( Force include parameter, therefore no need to include the pch header everywhere ) and specified the pch header to be precompiled_header.h. Note that this breaks edit-and-continue in Visual Studio.
For Linux , there is pch rule to enable it in the makefile ( build/linux/Makefile) , but currently that rule is not being used since it doesn`t appear as it is doing much improvement as on Windows.

For GCC see https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html

For MSVC 120 see https://msdn.microsoft.com/en-us/library/8c5ztk84(v=vs.120).aspx

**TODO for near future :**

Benchmarking & Microbenchmarking : Will add probes for SystemTap for Linux, might add performance test cases using existing GoogleTest project

Concurrency : Lockfree containers , currently only SPSC bounded queue is lock free.

Memory : 3rd party memory allocators support : jemalloc, intelTBB, tcMalloc, Lockless. Currently the engine is using a set of CPU cache aligned allocators in "source/memory".

**Considerations for future :**

Order matching : Adding other order types ( market orders, stop loss order) , order update and market data request support, TIF support

Exchange connectivity : Support ITCH ( London Stock Exchange & NASDAQ ) and/or FIX engines , interested in using Libtrade : https://github.com/libtrading/libtrading

Compiler support : Use of CMake, upgrading supported compiler versions and enabling use of C++14/C++17, experimental Clang and IntelC++Compiler

Concurrency : Experiment order book processing on GPU with CUDA

OS Support : Solaris11

New feature : Event broadcasting and sample feed handlers

New feature : Visualisation of transactions with OpenGL/Vulkan. See links below : 

						http://obt.hottolink.com/

						http://parasec.net/transmission/order-book-visualisation/

New feature : Save events in a database