Licence : All samples are "Public Domain" code 
http://en.wikipedia.org/wiki/Public_domain_software
			
			WHAT THIS PROJECT IS
	
- A multithreaded limit order matching engine written in C++11. 
- It targets both Linux and Windows systems.
- It uses STL,Boost and also platform specific APIs ( POSIX and WindowsAPIs) in some places, QuickFix for FIX protocol 
  Additionally used GoogleTest for unit testing, Bash for Linux test scripts and Powershell for Windows test scripts
- Uses FIX protocol (4.2)

A more detailed explanation will be available in my blog soon at nativecoding.wordpress.com
						
	WHAT LIMIT ORDERS ARE & ORDER MATCHING LOGIC
	
For limit orders see : https://en.wikipedia.org/wiki/Order_%28exchange%29#Limit_order

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

For general information about trading systems and order types , please see :
http://www.investopedia.com/university/intro-to-order-types/

	EXCHANGE CONNECTIVITY : FIX PROTOCOL AND QUICKFIX
	
FIX ( Financial information exchange ) protocol is a session based TCP protocol that carries security transcation data.
For more information , please see https://en.wikipedia.org/wiki/Financial_Information_eXchange
This project for the time being is using opensource QuickFix engine and FIX specification 4.2
	
	HOW IT WORKS
	
The engine mainly consists of 2 parts : a FIX server/engine and the order matching layer.
The core of order matching layer is called a central order book, which keeps order books per security symbol.
Each order book has a table for bids and another table for asks. Briefly the multithreaded system works as below :

1. The FIX engine will listen for session requests from the clients, and if a session is established
then it listens for incoming ask/bid orders. If the order type is not supported. It sends "rejected" message.
	
2. Central Order book has a thread pool
		- The thread pool will have N SPSC queues for N threads ( N = num of symbol ).
		- Central Order book also has 1 MPMC queue for outgoing messages.
		- When a new message arrives ( new order, or cancel ) from the FIX engine , it will be submitted to corresponding thread`s queue in the thread pool of the central order book.
		
3. Each thread in the thread pool will get message from their SPSC queue in the thread pool , and add them to corresponding order queue which is used by only itself
and eventually trigger the order matching process for that queue. At the end of the order matching , worker threads will submit messages ( FILLED or PARTIALLY FILLED ) to the outgoing messages queue 

4. Outgoing message processor which is a MPMC queue will process the outgoing messages.

	DEPENDENCIES BUILD
	
For Linux , the project is built and tested with GCC4.8. As for Windows it is using MSVC1200(VS2013).

Boost 1.59 : using a compacted version by using the command below :
				bcp --boost=c:\boost_1_59_0 shared_ptr scoped_ptr any optional tokenizer format c:\boost
						
QuickFix & its	requirements for quickfix : libxml2-devel , http://www.quickfixengine.org/quickfix/doc/html/install.html

	DEPENDENCIES RUNTIME

For Windows, you have to install MSVC120 runtime :
https://www.microsoft.com/en-gb/download/details.aspx?id=40784

For Linux, you need GNU Lib C runtime and libxml2.
			
	HOW TO BUILD THE PROJECT

1. Windows with Visual Studio 2013
	Go to build/windows directory
	Use SLN file to launch VS with the project

2. Linux from terminal
	
	cd build/linux
	make debug  OR make release

3. For Linux , also provided Netbeans 8.0.2 project files :
	Open Netbeans
	Open project from project directory
	Build the project inside Netbeans IDE.
	
	SERVER PARAMETERS AND RUNNING THE PROJECT
	
The engine executable looks for "ome.ini" file. Here is the list of things you can set :

		FILE_LOGGING_ENABLED							enables/disables logging
		CONSOLE_OUTPUT_ENABLED							enables/disables output to stdout
		
		CENTRAL_ORDER_BOOK_PIN_THREADS_TO_CORES			whether to pin threads of the threadpool to different CPU cores
		
		HYPER_THREADING									if hyperthreading is off and pinning is on ,then it will pin threads 
														to only cores with an even index
		
		CENTRAL_ORDER_BOOK_QUEUE_SIZE_PER_THREAD		Queue size per worker thread in the central order book`s thread pool

		LOG_BUFFER_SIZE									Maximum buffer size for the logging system as it is built on a ring buffer.

	TEST TOOLS
	
1. For unit testing , the project uses GoogleTest. You can find a makefile and vcproj under "test_unit" directory.
2. For functional testing , there is a prebuilt executable for both Linux and Windows which can send ask/bid orders to the order matching engine.
   Under "test_functional" directory :
		- Modify test_data.txt which has the orders to send to the engine
		- Modify  the arrays declared on top of client_automated_test.sh/client_automated_test.ps1 script files in order to configure the number of clients.
		You should provide a name for each client.
		- For firing Linux test client(s), you can use client_automated_test.sh.
		- For firing Windows test client(s), you can use client_automated_test.bat which drives client_automated_test.ps1 Powershell script.
		- After firing the script, it will be executing all orders in test_data.txt file per client that is declared in the script file.
	
	BUILDING AND RUNNING UNIT TEST ( USES GOOGLE TEST )
		
1. Windows : Use VS solution in test_unit directory
2. Linux : You have to build and install Google Test 1.7 first

			$ wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
			$ unzip gtest-1.7.0.zip
			$ cd gtest-1.7.0
			$ ./configure
			$ make
			$ sudo cp -a include/gtest /usr/include
			$ sudo cp -a lib/.libs/* /usr/lib/
			For centos it is cp -a lib/.libs/* /usr/lib64

3. Then you can either use Makefile or Netbeans project files under test_unit directory
			
	SOURCE CODE : FILE, DIRECTORY AND SOURCE CODE NAMING CONVENTIONS

directory names					lower_case_word
file names 						lower_case_word
header guard 					__HEADER_H__
macros							UPPER_CASE_WORD
enums							UPPER_CASE_WORD
namespace names 				lower_case_word
class names 					CamelCase
method names 					pascalCase
variable names					pascalCase
member variables starts with	m_

	SOURCE CODE : INDENTATIONS AND NEW LINE

space based no tabs ( This needs to be setup in VS )
	By default Netbeans editor uses spaces for tabs
	Needs to be set in VS2013 : https://msdn.microsoft.com/en-gb/library/ms165330(v=vs.90).aspx
	
New lines : Unix CR only ( \n ) , VS can handle it even though Windows is \r\n

	COMPILER WARNING LEVELS
		
MSVC 	: /W3
GCC		: -Wall

	PRECOMPILED HEADER FILE USAGE
		
For Windows , using /FI ( Force include parameter, therefore no need to include it to everywhere )
and set precompiled header file settings ( precompiled header file ) Note that this breaks edit-and-continue.

For Linux , there is pch rule to enable it , but currently that rule is not being used since it doesn`t appear as it is doing much improvement
as in Windows.