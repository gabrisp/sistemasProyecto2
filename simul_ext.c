#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
   EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodos: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\nBloques [0-25]: ");
    for (int i = 0; i < MAX_BLOQUES_PARTICION; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
      
   strcomando[strcspn(strcomando, "\n")] = '\0';

    int num_args = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);

    if (num_args == 1) {
        return 0;
    } else if (num_args == 2) {
        return 0;
    } else if (num_args == 3) {
        return 0;
    } else {
        printf("Comando desconocido\n");
        return -1;
    }
}


void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
   printf("Bloque %d Bytes\n", psup->s_block_size);
   printf("Inodos particion = %d\n", psup->s_inodes_count);
   printf("Inodos libres = %d\n", psup->s_free_inodes_count);
   printf("Bloques particion = %d\n", psup->s_blocks_count);
   printf("Bloques libres = %d\n", psup->s_free_blocks_count);
   printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {

    printf("Nombre\t\tTamano\tInodo\tBloques\n");

    for (int i = 1; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            int numero_inodo = directorio[i].dir_inodo;

            printf("%s\t", directorio[i].dir_nfich);

            if (inodos->blq_inodos[numero_inodo].size_fichero > 0) {
                printf("%u\t", inodos->blq_inodos[numero_inodo].size_fichero);
            } else {
                printf("0\t");
            }

            printf("%d\t", numero_inodo);

            // Imprimir bloques ocupados por el fichero
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodos->blq_inodos[numero_inodo].i_nbloque[j] != NULL_BLOQUE) {
                    printf("%d ", inodos->blq_inodos[numero_inodo].i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
   (void) inodos;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return directorio[i].dir_inodo; // Inodo Correcto
        }
    }
    return -1;
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {

   (void)inodos;
   int encontrado = -1;
   int nombreNoEsValido = 0;
   for (int i = 0; i < MAX_FICHEROS; i++) {
      if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0) { // Fichero Encontrado.
         encontrado = i;
      }
      if (strcmp(directorio[i].dir_nfich, nombrenuevo) == 0) { // Fichero Encontrado.
         nombreNoEsValido = 1;
      }
   }
   if (encontrado == -1) { 
      printf("El fichero no existe.\n");
      return -1;
   }
   else if (nombreNoEsValido == 1) {
      printf("El nombre del fichero ya existe.\n");
      return -1;
   }
   else {
      strcpy(directorio[encontrado].dir_nfich, nombrenuevo);
   }
   return 0;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
char *nombre,  FILE *fich) {

//Esta función se nos ha complicado en el último paso, pues conseguimos eliminar los datos y los bloques pero no hemos conseguido
//que elimine el nombre de los archivos
	
   (void) fich;
   (void) ext_superblock;

   // Comprobar si el fichero existe
   int ndir = BuscaFich(directorio, inodos, nombre);
   if (ndir == -1) {
      printf("El fichero no existe.\n");
      return -1;
   }

   // Liberar el bytemap
   ext_bytemaps->bmap_inodos[ndir] = 0;

   // marcar los 7 punteros a bloque con NULL_BLOQUE y bytemap a 0
   for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
      if (inodos->blq_inodos[ndir].i_nbloque[i] != NULL_BLOQUE) {
         unsigned short nbloque = inodos->blq_inodos[ndir].i_nbloque[i];
         ext_bytemaps->bmap_bloques[nbloque] = 0;
         inodos->blq_inodos[ndir].i_nbloque[i] = NULL_BLOQUE;

         // Actualizar superbloque
         ext_superblock->s_free_blocks_count++;
         
      }
   }

   // Liberar inodo
   inodos->blq_inodos[ndir].size_fichero = 0;
   // Actualizar el directorio y tamano inodo
   directorio[ndir].dir_inodo = NULL_INODO;
   strcpy(directorio[ndir].dir_nfich, "");

   return 1;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {


//En esta funcion hemos tenido bastantes problemas, finalmente no hemos conseguido que copie los datos del archivo origen pero si que cree un nuevo archivo, 
//que aunque de tamaño 0 y sin bloques tenga su inodo asociado
	
    int inodoOrigen, inodoDestino, i, j;

    // Buscar el inodo del fichero de origen en el directorio
    inodoOrigen = BuscaFich(directorio, inodos, nombreorigen);
    if (inodoOrigen == -1) {
        printf("Error: El fichero de origen no existe.\n");
        return 0;
    }

    // Comprobar que el fichero de destino no exista
    if (BuscaFich(directorio, inodos, nombredestino) != -1) {
        printf("Error: El fichero de destino ya existe.\n");
        return 0;
    }

    // Buscar un inodo libre para el nuevo fichero
    inodoDestino = -1;
    for (i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            inodoDestino = i;
            break;
        }
    }

    if (inodoDestino == -1) {
        printf("Error: No hay inodos libres para el nuevo fichero.\n");
        return 0;
    }

    // Asignar el inodo al nuevo fichero
    ext_bytemaps->bmap_inodos[inodoDestino] = 1;

    // Copiar información del inodo de origen al inodo de destino
	int tamano_fichero1 = inodos->blq_inodos[inodoDestino].size_fichero;
	int tamano_fichero2 = inodos->blq_inodos[inodoOrigen].size_fichero;
	
    tamano_fichero1 = tamano_fichero2;
	

	
   

    // Copiar bloques de datos
    for (i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
		
		int numero_bloque1 = inodos->blq_inodos[inodoDestino].i_nbloque[i] ;
		int numero_bloque2 = inodos->blq_inodos[inodoOrigen].i_nbloque[i] ;
		
        numero_bloque1 = numero_bloque2;
		
        if (numero_bloque2 != NULL_BLOQUE) {
            // Buscar un bloque libre en el bytemap de bloques
            int bloqueDestino = -1;
            for (j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloqueDestino = j;
                    break;
                }
            }

            if (bloqueDestino == -1) {
                printf("Error: No hay bloques libres para el nuevo fichero.\n");
                return 0;
            }

            // Asignar el bloque al nuevo fichero
            ext_bytemaps->bmap_bloques[bloqueDestino] = 1;

            // Copiar datos del bloque de origen al bloque de destino
            memcpy(memdatos[bloqueDestino - PRIM_BLOQUE_DATOS].dato, memdatos[inodos->blq_inodos[inodoOrigen].i_nbloque[i]].dato, SIZE_BLOQUE);

            // Actualizar el número de bloque en el inodo de destino
            numero_bloque1 = bloqueDestino;
        }
    }

    // Actualizar directorio con la nueva entrada
    strcpy(directorio[inodoDestino].dir_nfich, nombredestino);
    directorio[inodoDestino].dir_inodo = inodoDestino;

    printf("Fichero copiado.\n");

    return 1;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
   int ndir = BuscaFich(directorio, inodos, nombre);
   if (ndir == -1) {
      printf("El fichero no existe.\n");
      return -1;
   }

   // Imprimir archivo
   int tamano_fichero = inodos->blq_inodos[ndir].size_fichero;

   for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
      int numero_bloque = inodos->blq_inodos[ndir].i_nbloque[i] - 4;

      if (numero_bloque != NULL_BLOQUE) {
            fwrite(memdatos[numero_bloque].dato, SIZE_BLOQUE, 1, stdout);
            printf("\n");

            // Actualizar tamano restante del fichero
            tamano_fichero -= SIZE_BLOQUE;
      }

      // Si el tamano restante es menor o igual a 0, terminar la lectura
      if (tamano_fichero <= 0) {
            break;
      }
   }
   return 0;  // Ok

    return 1;
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
   fseek(fich, 0, SEEK_SET);
	fwrite(memdatos, sizeof(EXT_DATOS), 1, fich);
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	fseek(fich, SIZE_BLOQUE*2, SEEK_SET); // Nos situamos en el bloque 2
	fwrite(inodos, sizeof(EXT_BLQ_INODOS), 1, fich);
	fseek(fich, SIZE_BLOQUE*3, SEEK_SET); // Nos situamos en el bloque 3
	fwrite(directorio, sizeof(EXT_ENTRADA_DIR), 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
	fseek(fich, SIZE_BLOQUE*1, SEEK_SET); // Nos situamos en el bloque 1
	fwrite(ext_bytemaps, sizeof(EXT_BYTE_MAPS), 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	fseek(fich, 0, SEEK_SET ); // Nos situamos en el bloque 0
	fwrite(ext_superblock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1,fich);
}


int main()
{
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

   // init vars
   i = 0;
   j = 0;
   m = 0;
   entradadir = 0;
   grabardatos = 0;

   // use tmp
   (void)i;
   (void)j;
   (void)m;
   (void)entradadir;
   (void)grabardatos;

   // Lectura del fichero completo de una sola vez
   fent = fopen("particion.bin", "r+b");
   fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

   // Copia del superbloque
   memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], sizeof(EXT_SIMPLE_SUPERBLOCK));
   // Copia del mapa de bits de bloques
   memcpy(&ext_bytemaps, (EXT_BLQ_INODOS *)&datosfich[1], sizeof(EXT_BYTE_MAPS));
   // Copia del mapa de bits de inodos
   memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], sizeof(EXT_BLQ_INODOS));
   // Copia del directorio
   memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS);
   // Copia de los datos
   memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

   // Bucle de tratamiento de comandos
   for (;;)
   {
      do
      {
         printf(">> ");
         fflush(stdin);
         fgets((char *)comando, LONGITUD_COMANDO, stdin);

      }
      while (ComprobarComando((char *)comando, (char *)orden, (char *)argumento1, (char *)argumento2) != 0);

      if(strcmp((const char*)orden, "info")==0){
         LeeSuperBloque(&ext_superblock);
      }
      else if(strcmp((const char*)orden, "bytemaps")==0){
         Printbytemaps(&ext_bytemaps);
      }
      else if(strcmp((const char*)orden, "dir")==0){
			Directorio(directorio, &ext_blq_inodos);
      }
      else if(strcmp((const char*)orden, "rename")==0){
			Renombrar(directorio,&ext_blq_inodos,argumento1,argumento2);
      }
      else if(strcmp((const char*)orden, "imprimir")==0){
         Imprimir(directorio,&ext_blq_inodos,memdatos,argumento1);
      }
      else if(strcmp((const char*)orden, "remove")==0){
         grabardatos = Borrar(directorio,&ext_blq_inodos,&ext_bytemaps,&ext_superblock,argumento1,fent);
      }
      else if(strcmp((const char*)orden, "copy")==0){
         grabardatos = Copiar(directorio,&ext_blq_inodos,&ext_bytemaps,&ext_superblock, memdatos,argumento1, argumento2,fent);
      }
      else if(strcmp((const char*)orden, "salir")==0){
         GrabarDatos((EXT_DATOS *)&memdatos, fent);
         GrabarSuperBloque(&ext_superblock, fent);
			Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
			GrabarByteMaps(&ext_bytemaps, fent);
         fclose(fent);
         break ;
      }
      else {
         
         printf("ERROR: %s: comando desconocido\n", comando);
      }

      if (grabardatos) {
         GrabarDatos((EXT_DATOS *)&memdatos, fent);
			GrabarSuperBloque(&ext_superblock, fent);
			Grabarinodosydirectorio(directorio, &ext_blq_inodos, fent);
			GrabarByteMaps(&ext_bytemaps, fent);
       }
      grabardatos = 0;
   }
   return 0;
}
