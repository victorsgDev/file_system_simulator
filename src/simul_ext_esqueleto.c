#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *superblock);

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombre);

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreantiguo, char *nombrenuevo);

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
             EXT_DATOS *memdatos, char *nombre);

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre, FILE *fich);

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich);

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    int i, j;
    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;

    // Lectura del fichero completo de una sola vez
    fent = fopen("./resources/particion.bin", "r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);


    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *) &datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *) &datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *) &datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *) &datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *) &datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Buce de tratamiento de comandos
    for (;;) {
        do {
            printf(">> ");
            fflush(stdin);
            fgets(comando, LONGITUD_COMANDO, stdin);
        } while (ComprobarComando(comando, orden, argumento1, argumento2) != 0);
        if (strcmp(orden, "dir") == 0) {
            Directorio(*&directorio, &ext_blq_inodos);
            continue;
        }
        else if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
        }
        else if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
        }
        else if (strcmp(orden, "salir") == 0) {
            GrabarDatos(*&memdatos, fent);
            fclose(fent);
            return 0;
        }
        else if (strcmp(orden, "rename") == 0) {
            Renombrar(*&directorio, &ext_blq_inodos, argumento1, argumento2);
        }
        else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(*&directorio, &ext_blq_inodos, *&memdatos, argumento1);
        }
        else {
            // Si el comando no es reconocido, mostrar mensaje de error
            printf("Comando desconocido: %s\n", orden);
        }
        // Escritura de metadatos en comandos rename, remove, copy
        Grabarinodosydirectorio(*&directorio, &ext_blq_inodos, fent);
        GrabarByteMaps(&ext_bytemaps, fent);
        GrabarSuperBloque(&ext_superblock, fent);
        if (grabardatos)
            GrabarDatos(*&memdatos, fent);
        grabardatos = 0;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden, "salir") == 0) {
            GrabarDatos(*&memdatos, fent);
            fclose(fent);
            return 0;
        }
    }
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    char *token;
    int num_tokens = 0;

    // Limpiar cadenas de salida
    orden[0] = '\0';
    argumento1[0] = '\0';
    argumento2[0] = '\0';

    // Eliminar salto de línea final si existe
    strcomando[strcspn(strcomando, "\n")] = '\0';

    // Dividir la cadena en tokens
    token = strtok(strcomando, " ");
    while (token != NULL) {
        num_tokens++;
        if (num_tokens == 1) {
            strcpy(orden, token);  // Primer token es el comando
        } else if (num_tokens == 2) {
            strcpy(argumento1, token);  // Segundo token es el primer argumento
        } else if (num_tokens == 3) {
            strcpy(argumento2, token);  // Tercer token es el segundo argumento
        } else {
            return -1;  // Demasiados argumentos
        }
        token = strtok(NULL, " ");
    }

    // Si no hay un comando (orden está vacío), es entrada inválida
    if (strlen(orden) == 0) {
        return -1;
    }

    return 0;  // Entrada válida
}


void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *superblock) {
    printf("Bloques: %d\n", superblock->s_blocks_count);
    printf("inodos: %d\n", superblock->s_inodes_count);
    printf("Primer bloque de datos: %d\n", superblock->s_first_data_block);
    printf("Bloques libres: %d\n", superblock->s_free_blocks_count);
    printf("Inodos libres: %d\n", superblock->s_free_inodes_count);
    printf("Tamanio del bloque: %d\n", superblock->s_block_size);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);
    fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    int i, j;
    // Ajustamos el ancho de las columnas para que se alineen correctamente
    printf("%-20s\t%-10s\t%-10s\t%-40s\n", "Nombre", "Tamanio", "Inodo", "Bloques");
    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            // Verificar que el inodo no sea el del directorio raíz
            if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, ".") != 0) {
                int inodo_idx = directorio[i].dir_inodo;
                int size_fichero = inodos->blq_inodos[inodo_idx].size_fichero;

                // Imprimir nombre, tamaño y número de inodo
                printf("%-20s\t%-10d\t%-10d\t", directorio[i].dir_nfich, size_fichero, inodo_idx);

                // Imprimir solo los bloques ocupados (no imprimir 65535)
                int first_block = 1;  // Flag para controlar la separación de bloques
                for (j = 0; j < MAX_NUMS_BLOQUE_INODO && inodos->blq_inodos[inodo_idx].i_nbloque[j] != 0; j++) {
                    int block = inodos->blq_inodos[inodo_idx].i_nbloque[j];
                    if (block != 65535) {  // Ignorar bloques 65535
                        if (!first_block) {
                            printf(", ");  // Separador entre bloques
                        }
                        printf("%d", block);
                        first_block = 0;  // Cambiar flag para la primera iteración
                    }
                }

                printf("\n");
            }
        }
    }
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    int i;
    printf("Inodos");
    for (i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            printf(" 0");
        } else {
            printf(" %d", ext_bytemaps->bmap_inodos[i]);
        }
    }
    printf("\nBloques [0-25]");
    for (i = 0; i < 25; i++) {
        if (ext_bytemaps->bmap_bloques[i] == 0) {
            printf(" 0");
        } else {
            printf(" %d", ext_bytemaps->bmap_bloques[i]);
        }
    }
    printf("\n");
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    int inodo_idx = BuscaFich(directorio, inodos, nombreantiguo);  // Buscar el fichero original

    // Comprobar si el fichero origen existe
    if (inodo_idx == -1) {
        printf("Error: El fichero '%s' no encontrado.\n", nombreantiguo);
        return -1;  // Fichero no encontrado
    }

    // Comprobar si el nuevo nombre ya está en uso
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombrenuevo) == 0) {
            printf("Error: El fichero '%s' ya existe.\n", nombrenuevo);
            return -2;  // El nuevo nombre ya está en uso
        }
    }

    // Si no está en uso, renombramos el fichero
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == inodo_idx) {
            // Cambiar el nombre
            strcpy(directorio[i].dir_nfich, nombrenuevo);
            printf("Fichero '%s' renombrado a '%s'.\n", nombreantiguo, nombrenuevo);
            return 0;  // Éxito
        }
    }

    return -3;  // No se encontró la entrada en el directorio (esto no debería ocurrir)
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return directorio[i].dir_inodo;  // Devolver el índice del inodo
        }
    }
    return -1;  // No se encontró el fichero
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(inodos, SIZE_BLOQUE, 1, fich);
    fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    fseek(fich, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {

    if (nombre == NULL || strlen(nombre) == 0) {
        printf("Error: No se ha especificado un nombre de fichero.\n");
        return -1;  // No se especificó un nombre de fichero
    }

    // Buscar el inodo correspondiente al fichero
    int inodo_idx = BuscaFich(directorio, inodos, nombre);

    // Comprobar si el fichero existe
    if (inodo_idx == -1) {
        printf("Error: El fichero '%s' no existe.\n", nombre);
        return -1;  // Fichero no encontrado
    }

    // Obtener el inodo correspondiente al fichero
    EXT_SIMPLE_INODE inodo = inodos->blq_inodos[inodo_idx];

    // Imprimir el contenido de los bloques
    printf("Contenido de '%s':\n", nombre);

    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo.i_nbloque[i] != 0; i++) {
        int bloque = inodo.i_nbloque[i];

        if (bloque != NULL_BLOQUE) {  // Ignorar bloques no válidos
            // Imprimir los datos del bloque
            printf("%s", memdatos[bloque].dato);
        }
    }

    printf("\n");
    return 0;  // Éxito
}