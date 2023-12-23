#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8085
#define BUFFER_SIZE 1024

int main() {
    // Создание сокета
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка серверного адреса
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(PORT);

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Дополнительные действия при подключении к серверу
    printf("Connected to the server.\n");

    // Отправка сообщения на сервер
    char message[] = "Hello, Server!";
    ssize_t bytesSent = send(clientSocket, message, sizeof(message), 0);
    if (bytesSent == -1) {
        perror("Error sending message");
    } else {
        printf("Message sent to server.\n");

        // Прием ответа от сервера
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            printf("Received response from server: %s\n", buffer);
        } else if (bytesRead == 0) {
            printf("Server closed the connection.\n");
        } else {
            perror("recv error");
        }
    }

    // Закрытие клиентского сокета
    close(clientSocket);

    return 0;
}
