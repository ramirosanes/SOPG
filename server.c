#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define PORT 5000
#define MAX_BUFFER 128

// Función para procesar los comandos
void process_command(int client_fd, const char *command) 
{
    char buffer[MAX_BUFFER*2] = {0}; //No me funcionaba si no ponia este x2 y no entiendo porq
    char value[MAX_BUFFER] = {0};
    char key[MAX_BUFFER] = {0};  
    FILE *file;

    //Recibo comando
    if (sscanf(command, "SET %s %s", key, value) == 2) 
    {
        file = fopen(key, "w");
        if (file) 
        {
            fprintf(file, "%s", value);
            fclose(file);
            snprintf(buffer, sizeof(buffer), "OK\n");
        } else {
            perror("Error al abrir archivo\n");
        }
    } else if (sscanf(command, "GET %s", key) == 1) 
    {
        file = fopen(key, "r");
        if (file) 
        {
            fgets(value, sizeof(value), file);
            fclose(file);
            snprintf(buffer, sizeof(buffer), "OK\n%s\n", value);
        } else {
            if (errno == ENOENT) {
                snprintf(buffer, sizeof(buffer), "NOTFOUND\n");
            } else {
                perror("Error al leer archivo\n");
            }
        }
    } else if (sscanf(command, "DEL %s", key) == 1) 
    {
        if (remove(key) == 0) {
            snprintf(buffer, sizeof(buffer), "OK\n");
        } else {
            if (errno == ENOENT) {
                snprintf(buffer, sizeof(buffer), "OK\n");
            } else {
                perror("Error al remover archivo\n");
            }
        }
    } else {
        snprintf(buffer, sizeof(buffer), "Comando no reconocido\n");
    }
    //Escribo en el file del cliente la respuesta
    write(client_fd, buffer, strlen(buffer));
}

// Función para manejar la comunicación con el cliente
void handle_client(int client_fd) 
{
  char buffer[MAX_BUFFER] = {0};
  int valread;

  // Leer el comando del cliente
  valread = read(client_fd, buffer, MAX_BUFFER - 1);
  if (valread < 0) 
  {
    perror("Error al leer del cliente\n");
    close(client_fd);
    return;
  }
  fprintf(stdout, "Mensaje recibido: %s\n", buffer);
  
  // Procesar el comando
  process_command(client_fd, buffer);
}

int main(void) {
    //Se inicializan las variables necesarias
    int server_fd, client_fd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char ipClient[32];

    // Creamos socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Se carga (cofigura) la estructura serveraddr
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr)) <= 0) 
    {
        fprintf(stderr, "ERROR invalid server IP\n");
        exit(EXIT_FAILURE);
    }

    // Abrimos puerto con bind()
    if (bind(server_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
    {
        perror("No se pudo bindear socket\n");
        exit(EXIT_FAILURE);
    }

    // Seteamos socket en modo Listening
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        // Ejecutamos accept() para recibir conexiones entrantes
        printf("server: esperando una conexion...\n");
        if ((client_fd = accept(server_fd, (struct sockaddr *)&serveraddr, &addrlen)) < 0) 
        {
            perror("Error aceptando");
            exit(EXIT_FAILURE);
        }

        //Se toma y se guarda la dirección del cliente para printearla
        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
        printf("server: conexion desde:  %s\n", ipClient);

        handle_client(client_fd);
    }
    
    // Cerramos conexion con cliente
    close(client_fd);
    return EXIT_SUCCESS;
}