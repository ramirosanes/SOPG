# SOPG

Trabajo práctico para la materia "Sistemas Operativos en Tiempo Real"

## Objetivo

Escribir un servidor TCP que permite almacenar información ASCII en forma de clave-valor.

## Funcionamiento del Servidor

El servidor debe:

1. Esperar a que un cliente se conecte mediante el protocolo TCP, puerto 5000.
2. Esperar a que el cliente envíe un comando a ejecutar. El comando se especifica como una secuencia de caracteres ASCII hasta el `\n`.
3. Realizar la operación correspondiente.
4. Cortar la conexión con el cliente y volver al paso 1.

## Comandos

Los comandos que acepta el servidor son:

- **`SET <clave> <valor>\n`**: 
    - Se crea en el servidor un archivo llamado `<clave>` con el contenido indicado en `<valor>` (sin incluir el `\n`).
    - Se responde al cliente `OK\n`.

- **`GET <clave>\n`**: 
    - Si existe el archivo correspondiente, se responde al cliente con: `OK\n<valor>\n` (es decir, una línea de texto que dice `OK` y otra que contiene el contenido del archivo).
    - Si no existe, se responde con `NOTFOUND\n`.

- **`DEL <clave>\n`**:
    - Si existe el archivo correspondiente, se elimina.
    - Tanto si existe como si no, se responde `OK\n`.

## Manejo de Errores

Ante cualquier caso excepcional, el servidor debe informar la causa y finalizar el proceso con código de error.

## Cliente

Dado que el protocolo de comunicación es ASCII, no es necesario programar un cliente. Se pueden utilizar herramientas como `nc` (netcat) o `telnet`.

En Ubuntu, se pueden instalar con: 

```bash
apt install netcat 
apt install telnet
```
## Ejemplo

- En la consola #1 (server): `./server`
- En la consola #2 (client): `nc localhost 5000`. Si la conexión es exitosa, el procesa se queda esperando a recibir entrada de `stdin`.

```bash
$ nc localhost 5000
SET manzana apple
OK
$ nc localhost 5000
SET perro dog
OK
$ nc localhost 5000
SET hola hello
OK
$ nc localhost 5000
GET perro
OK
dog
$ nc localhost 5000
GET casa
NOTFOUND
$ nc localhost 5000
DEL perro
OK
$ nc localhost 5000
GET perro
NOTFOUND
$
```
