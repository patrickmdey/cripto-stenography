# Trabajo Práctico - Criptografía

## Integrantes

| Legajo | Nombre          |
| ------ | --------------- |
| 59290  | Patrick Dey     |
| 60052  | Santos Rosati   |
| 60527  | Matias Lombardi |
## Dependencias

Asegurarse de instalar las dependencias para que el programa funcione correctamente.
Puede hacerlo utilizando el siguiente comando.

```
sudo apt-get install libssl-dev openssl
```

## Compilación

Ejecutar el siguiente comando en el repositorio.
```
make all
```
## Ejecución

Ejecutar el siguiente comando en el directorio
```
./stegobmp [args]
```

## Manual

    Argumentos:
        -h                  Imprime todos los argumentos del programa
        -embed              Embeber información en un archivo
        -extract            Extraer información de un archivo
        -in [FILE]          Archivo a embeber
        -p [BITMAP FILE]    Archivo portador. Debe ser .bmp
        -out [BITMAP FILE]  Archivo de salida. Tanto para embeber como para extraer
        -steg [ALGORITHM]   Selecciona algoritmo estenográfico
                OPTIONS: LSB1, LSB4, LSBI
        -a [ALGORITHM]      Block Cipher Algorithm
            OPTIONS: aes128, aes192, aes256, des
        -m [MODE]           Block Cipher Algorithm mode
                OPTIONS: ecb, cfb, ofb, cbc
        -pass [PASSWORD]    Contraseña de encriptación
        -v                  Imprime la versión del programa


Es importante tener en cuenta que para utilizar -extract, es obligatorio usar los argumentos
- -p [BITMAP FILE] 
- -out [BITMAP FILE]
- -s [ALGORITHM]

Por otro lado, para utilizar -embed, es obligatorio usar los argumentos 
- -in [FILE] 
- -p [BITMAP FILE] 
- -out [BITMAP FILE]
- -s [ALGORITHM]


## Ejemplos

1. Embeber un .txt utilizando LSB1 sin encriptar

```
    ./stegobmp -embed -s LSB1 -in embed_text.txt -p lado.bmp -out lado_lsb1.bmp
```

2. Embeber una imagen .png utilizando LSBI encriptando con aes192 modo ecb utilizando la password "sorpresa"

```
    ./stegobmp -embed -s LSBI -in embed_image.png -p lado.bmp -out lado_lsbi.bmp -a aes192 -m ecb -pass sorpresa
```

3. Extraer un archivo de un bmp utilizando algoritmo y modo de encripción por default con la password "sorpresa"

```
    ./stegobmp -embed -s LSB1 -in in_text.txt -p lado.bmp -out salida.bmp -pass sorpresa
```

4. Extraer un archivo de un bmp sin cifrado con algoritmo LSB4

```
    ./stegobmp -extract -s LSB4 -p lado.bmp -out salida
```

5. Extraer un archivo de un bmp con algoritmo LSB1 desencriptando con des modo ofb utilizando la password "sorpresa"

```
    ./stegobmp -extract -s LSB1 -p lado.bmp -out salida -a des -m ofb -pass sorpresa
```
