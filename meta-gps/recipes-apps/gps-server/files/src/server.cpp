#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <atomic>
#include <thread>
#include "globals.h"
#include "backend.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

void DieWithError(const char* message) {
    perror(message);
    std::exit(EXIT_FAILURE);
}

class Server {
    private:

        int cli_socketid;
        struct sockaddr_in ser_addrport;
        struct sockaddr_in cli_addrport;
        char msg_buffer[256];
        int recv_msg_size;

    public:

        int ser_socketid;
        int status;
        Server() {
            if ((ser_socketid = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                DieWithError("socket() failed");
            }
            ser_addrport.sin_family = AF_INET;
            ser_addrport.sin_port = htons(5100);
            ser_addrport.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(ser_socketid, (struct sockaddr *) &ser_addrport, sizeof(ser_addrport)) < 0) {
                DieWithError("bind() failed");
            }
            if (listen(ser_socketid, 1) < 0) {
                DieWithError("listen() failed");
            }
        }

        ~Server() {
            status = close(ser_socketid);
        }

        void active_server() {
            while (status != -1) {
                socklen_t cli_addr_len = sizeof(cli_addrport);
                if ((cli_socketid = accept(ser_socketid, (struct sockaddr *) &cli_addrport, &cli_addr_len)) < 0) {
                    DieWithError("accept() failed");
                }

                if ((recv_msg_size = recv(cli_socketid, msg_buffer, sizeof(msg_buffer), 0)) < 0){
                    DieWithError("recv() failed");
                }

                while (recv_msg_size > 0) {
                    std::string raw_data(msg_buffer, recv_msg_size);

                    size_t comma_pos = raw_data.find(',');

                    if (comma_pos != std::string::npos) {
                        try {
                            double parsed_lat = std::stod(raw_data.substr(0, comma_pos));
                            double parsed_lon = std::stod(raw_data.substr(comma_pos + 1));

                            path_buffer[head % BUFFER_SIZE] = {parsed_lat, parsed_lon};
                            head++;
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Invalid data: " << raw_data << '\n';
                        }
                    }

                    if ((recv_msg_size = recv(cli_socketid, msg_buffer, sizeof(msg_buffer), 0)) < 0){
                        DieWithError("recv() failed");
                    }
                }

                close(cli_socketid);
                

            }
        }
};

int main(int argc, char *argv[]) {

    QGuiApplication app(argc, argv);

    Server communication_server;
    std::thread server_thread(&Server::active_server, &communication_server);
    server_thread.detach();

    Backend backend;

    QQmlApplicationEngine engine;
    
    engine.rootContext()->setContextProperty("Backend", &backend);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);

    return app.exec();
}