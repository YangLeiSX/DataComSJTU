# README

实验名称:

Exercises 2: EchoClientandServer 

实验内容:

Task1:Writeastreambasedechoclientsendingmessagestothe echo server, receiving each message returned by the server. Terminate the connection when “quit” is entered.

即编写一个客 户端，接受用户数据，向服务器发送数据并显示服务器返回 值。输入 quit 退出程序。

Task2:Writeastreambasedechoserverprintingoutthemessage received from the client, echoing it back, until the client closes the connection.

即编写一个服务器，接受用户的数据并将其发送回 客户端。等待客户端关闭通信。

Task3:ModifyyoursolutiontoExercise2towriteastreambased echo server, which can simultaneously handle multiple clients connecting to it. No modification of the client code is necessary, but multiple instances of the client should be started. Hint: use Windows threads functions. 

即修改上述的服务器使其能同时 连接多个客户端，并使用多线程处理客户端数据。

