#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8085
#define BUFFER_SIZE 1024

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int signo) {
    wasSigHup = 1;
}

int main() {
    // Регистрация обработчика сигнала SIGHUP
    struct sigaction sa;
    sa.sa_handler = sigHupHandler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    // Блокировка сигнала SIGHUP
    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, NULL);

    // Создание сокета
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка серверного адреса
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Привязка сокета к адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Перевод сокета в режим прослушивания
    if (listen(serverSocket, 5) == -1) {  // Максимальная длина очереди - 5
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Основной цикл
    while (1) {
        // Принятие нового соединения
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            perror("Accept failed");
            continue;
        }

        // Вывод сообщения о новом соединении
        printf("New connection accepted. Client socket: %d\n", clientSocket);

        // Чтение данных от клиента
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == -1) {
            perror("Error reading from client");
        } else if (bytesRead == 0) {
            printf("Client disconnected.\n");
        } else {
            buffer[bytesRead] = '\0';
            printf("Received message from client: %s\n", buffer);

            // Отправка ответа клиенту
            const char* responseMessage = "Hello, Client! I received your message.";
            ssize_t bytesSent = send(clientSocket, responseMessage, strlen(responseMessage), 0);
            if (bytesSent == -1) {
                perror("Error sending response to client");
            } else {
                printf("Response sent to client.\n");
            }
        }

        // Закрытие клиентского сокета
        close(clientSocket);

        // Проверка сигнала SIGHUP
        if (wasSigHup) {
            printf("Received SIGHUP signal.\n");
            // Дополнительные действия при получении сигнала
            wasSigHup = 0; // Сброс флага
        }
    }

    // Закрытие серверного сокета (эта часть кода не будет выполнена, так как цикл бесконечен)
    close(serverSocket);

    return 0;
}
