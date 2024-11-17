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
    char buffer[MAX_BUFFER] = {0}; //acá se guardan las respuestas para el cliente
    char *token; 
    FILE *file;

    //Necesito copiar a otro array no const para poder trabajar con strtok
    char *command_copy = strdup(command);
    //strtok guarda en token el primer string delimitado por espacios " ", y lo termina con \0
    token = strtok(command_copy, " ");

    //una vez que guardo el primer string en token, empiezo el procesamiento
    if (token != NULL) {
        if (strcmp(token, "SET") == 0) { 
            // Comando SET
            // strtok utiliza el argumento utilizado anteriormente si le paso NULL, pero continuando donde estaba antes
            char *key = strtok(NULL, " ");
            char *value = strtok(NULL, " ");

            //Tengo guardado en *key y *value los punteros a los strings a procesar

            if (key != NULL && value != NULL) {
                file = fopen(key, "w"); //Creo un nuevo file con nombre key en modo write
                if (file) {
                    fprintf(file, "%s", value);
                    fclose(file);
                    snprintf(buffer, sizeof(buffer), "OK\n"); //Guardo en buffer la respuesta a devolverle al cliente
                } else {
                    perror("Error opening file\n");
                }
            } else {
                //Si se utiliza mal el comando llego acá
                snprintf(buffer, sizeof(buffer), "Invalid SET command format\n");
            }

        } else if (strcmp(token, "GET") == 0) {
            // Comando GET, misma logica que anterior
            char *key = strtok(NULL, " ");
            //Agrego este buffer porque me fallaba el sprintf de 4 argumentos (ni idea porque)
            char temp_buffer[MAX_BUFFER] = {0}; 

            if (key != NULL) {
                file = fopen(key, "r"); //Intento abrir un archivo anteriormente creado llamado key
                if (file) {
                    fgets(temp_buffer, sizeof(temp_buffer), file); //Leo archivo anteriormente creado y guardo en buffer para responder
                    fclose(file);
                    snprintf(buffer, sizeof(buffer)+5, "OK\n%s\n", temp_buffer); //Si no ponia +5 no funcionaba (no me compilaba aunque no hubiera manera de superar el tamaño del buffer pero bueno)
                } else {
                    if (errno == ENOENT) {
                        snprintf(buffer, sizeof(buffer), "NOTFOUND\n"); //Respuesta en caso de que no se encuentre la key
                    } else {
                        perror("Error reading file\n");
                    }
                }
            } else {
                snprintf(buffer, sizeof(buffer), "Invalid GET command format\n");
            }

        } else if (strcmp(token, "DEL") == 0) {
            // Comando DEL, misma lógica
            char *key = strtok(NULL, " ");

            if (key != NULL) {
                if (remove(key) == 0) {
                    snprintf(buffer, sizeof(buffer), "OK\n"); //En caso de exito escribimos respuesta
                } else {
                    if (errno == ENOENT) {
                        snprintf(buffer, sizeof(buffer), "Inexistente\n");
                    } else {
                        perror("Error removing file\n");
                    }
                }
            } else {
                snprintf(buffer, sizeof(buffer), "Invalid DEL command format\n");
            }

        } else {
            snprintf(buffer, sizeof(buffer), "Unrecognized command\n");
        }
    } else {
        snprintf(buffer, sizeof(buffer), "Invalid command format\n");
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
  // Cierro conexion con cliente
  close(client_fd);
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

    return EXIT_SUCCESS;
}