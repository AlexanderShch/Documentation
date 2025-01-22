readme.txt

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998  Microsoft Corporation.  All Rights Reserved.


About:

This directory contains 3 programs, an I/O Completion Port (IOCP) Winsock 
server using WSAAccept (iocpserver), an IOCP Winsock server using AcceptEx
(iocpserverex) and a simple multithreaded Winsock client (iocpclient)
used to test either server.  Unless otherwise noted, comments about iocpserver
apply to iocpserverex as well.

Iocpserver is a simple echo program that uses IOCP to receive data from and 
echo data back to a sending client. The server program supports multiple 
clients connecting via TCP/IP and sending arbitrary sized data buffers which 
the server then echoes back to the client.  For convenience a very simple 
client program, iocpclient, was developed to connect and continually send 
data to the server to stress it using multiple threads (NOTE: please see 
the comments in iocpclient.cpp about scaleability of multithreaded 
applications).

Direct IOCP support was added to Winsock 2 and is fully implemented on the 
NT platform.  IOCP support is not available on Win9x platforms.  IOCPs in 
Winsock2 are modeled on the Win32 IOCP model and compatible with them. IOCPs 
provide a model for developing very high performance and very scablable 
server programs.  As a consequence, iocpserver can only run on NT.  Use
of IOCP-capable functions in Winsock 2 (such as WSASend and WSARecv) is
preferred over using IOCPs in WriteFile and ReadFile when using sockets.  
Additionally, the client is designed to run only on NT as well. 

The basic idea is that this server continuously accepts connection requests 
from a client program.  When this happens, the accepted socket descriptor 
is added to the existing IOCP and an inital receive (WSARecv) is posted on 
that socket.  When the client then sends data on that socket, an I/O packet 
will be delivered and handled by one of the server's worker threads.  The 
worker thread echoes the data back to the sender by posting a send (WSASend) 
containing all the data just received.  When sending the data back to the 
client completes, another I/O packet will be delivered and again handled by 
one of the server's worker threads.  Assuming all the data that needed to be 
sent was actually sent, another receive (WSARecv) is once again posted and 
the scenario repeats itself until the client stops sending data.
		
The client and server can be shut down by pressing CTRL-C. The server can
restart without exiting the process by pressing CTL-BRK.

The basic difference between iocpserver and iocpserverex is that AcceptEx (used
in iocpserverex) allows data to be returned from an accepted connection and AcceptEx 
can be executed in an overlapped manner and thus be used in conjunction with 
an IOCP.



Build:

    Run nmake to use the supplied makefile or create a VC project for 
    iocpserver, iocpserverex, and iocpclient.   When creating a project in VC 
    remember to link with the Winsock 2 library ws2_32.lib.  Also, use the 
    headers and libs from the April98 Platform SDK or later.


Usage:

    Start the server and wait for connections on port 6001
	iocpserver -e:6001

    Start the client with 32 threads in verbose mode and connect to port 6001
    on the server machine, server_machine.
        iocpclient -n:server_machine -t:32 -v -e:6001

readme.txt

ДАННЫЙ КОД И ИНФОРМАЦИЯ ПРЕДОСТАВЛЯЮТСЯ «КАК ЕСТЬ» БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ
КАКОГО-ЛИБО РОДА, ЯВНЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ,
ПОДРАЗУМЕВАЕМЫМИ ГАРАНТИЯМИ ТОВАРНОГО СОСТОЯНИЯ И/ИЛИ ПРИГОДНОСТИ ДЛЯ
ОПРЕДЕЛЕННОЙ ЦЕЛИ.

Авторские права (C) 1998 Microsoft Corporation. Все права защищены.


О:

Этот каталог содержит 3 программы: сервер I/O Completion Port (IOCP) Winsock с использованием WSAAccept (iocpserver), сервер IOCP Winsock с использованием AcceptEx (iocpserverex) и простой многопоточный клиент Winsock (iocpclient), используемый для тестирования любого из серверов. Если не указано иное, комментарии к iocpserver относятся и к iocpserverex.

Iocpserver — это простая программа-эхо, которая использует IOCP для получения данных и их передачи обратно отправляющему клиенту. Серверная программа поддерживает подключение нескольких клиентов по протоколу TCP/IP и отправку буферов данных произвольного размера, которые сервер затем передаёт обратно клиенту. Для удобства была разработана очень простая клиентская программа iocpclient, которая подключается к серверу и постоянно отправляет на него данные, чтобы создать нагрузку с помощью нескольких потоков (ПРИМЕЧАНИЕ: пожалуйста, ознакомьтесь с комментариями в iocpclient.cpp о масштабируемости многопоточных приложений).

Прямая поддержка IOCP была добавлена в Winsock 2 и полностью реализована на платформе NT. Поддержка IOCP недоступна на платформах Win9x. IOCP в Winsock2 созданы по модели Win32 IOCP и совместимы с ней. IOCP обеспечивают модель для разработки очень производительных и масштабируемых серверных программ. Как следствие, iocpserver может работать только на NT. При использовании сокетов предпочтительнее использовать функции Winsock 2, поддерживающие IOCP (например, WSASend и WSARecv), а не IOCP в WriteFile и ReadFile. 
 Кроме того, клиент предназначен для работы только в среде NT. 

 Основная идея заключается в том, что этот сервер непрерывно принимает запросы на подключение от клиентской программы. Когда это происходит, дескриптор принятого сокета добавляется в существующий IOCP, и на этот сокет отправляется начальное получение (WSARecv). Когда клиент отправляет данные по этому сокету, пакет ввода-вывода доставляется и обрабатывается одним из рабочих потоков сервера. Рабочий поток отправляет данные обратно отправителю, отправляя (WSASend) все только что полученные данные. Когда отправка данных обратно клиенту завершится, будет доставлен ещё один пакет ввода-вывода, который снова будет обработан 
один из рабочих потоков сервера. Если предположить, что все данные, которые нужно было отправить, были отправлены, то снова отправляется запрос на получение (WSARecv), и сценарий повторяется до тех пор, пока клиент не перестанет отправлять данные.
 
 Клиент и сервер можно закрыть, нажав CTRL-C. Сервер можно перезапустить, не завершая процесс, нажав CTL-BRK.

Основное различие между iocpserver и iocpserverex заключается в том, что AcceptEx (используемый в iocpserverex) позволяет возвращать данные из принятого соединения, а AcceptEx может выполняться в фоновом режиме и, таким образом, использоваться в сочетании с IOCP.



Сборка:

 Запустите nmake, чтобы использовать прилагаемый файл Makefile, или создайте проект VC для iocpserver, iocpserverex и iocpclient. При создании проекта в VC не забудьте связать его с библиотекой Winsock 2 ws2_32.lib. Также используйте заголовки и библиотеки из April98 Platform SDK или более поздней версии.


Использование:

 Запустите сервер и дождитесь подключений на порту 6001
 iocpserver -e:6001

 Запустите клиент с 32 потоками в подробном режиме и подключитесь к порту 6001
 на сервере, server_machine.
 iocpclient -n:server_machine -t:32 -v -e:6001

