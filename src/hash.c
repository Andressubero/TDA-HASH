#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

typedef struct nodo {
	void *elemento;
	char *clave;
	struct nodo *siguiente;
} nodo_t;

struct hash {
	struct nodo **array;
	size_t tope;
	size_t capacidad;
};

size_t funcion_hash (char *clave, size_t capacidad) {
	unsigned long hash = 5381; 
	int c = *clave;
	while (c){
		hash = ((hash << 5) + hash) + (unsigned long)c; /* hash * 33 + c */
		c = *clave++;
	}

	return (size_t)hash % capacidad;
}

hash_t *hash_crear(size_t capacidad)
{	
	size_t nueva_capacidad = 0;
	if (capacidad <= 3)
		nueva_capacidad = 3;
	else
		nueva_capacidad = capacidad;

	hash_t *nuevo_hash = malloc(sizeof(hash_t));
	if(!nuevo_hash)
		return NULL;
	nuevo_hash->tope = 0;
	nuevo_hash->capacidad = nueva_capacidad;
	struct nodo **nuevo_array = calloc(nueva_capacidad, sizeof(struct nodo*));
	if(!nuevo_array){
		free(nuevo_hash);
		return NULL;
	}
	nuevo_hash->array = nuevo_array;
	return nuevo_hash;
}

struct nodo *insertar_nodo (struct nodo *nodo, const char *clave, void *elemento,void **anterior, size_t *tope)
{
	if (!nodo){
		struct nodo *nuevo_nodo = malloc(sizeof(struct nodo));
		if (!nuevo_nodo)
			return NULL;
		size_t largo = strlen(clave);
		char  *nueva_clave = malloc((largo + 1) * sizeof(char));
		if (!nueva_clave){
			free(nuevo_nodo);
			return NULL;
		}
		strcpy(nueva_clave, clave);
		nuevo_nodo->clave = nueva_clave;
		nuevo_nodo->elemento = elemento;
		nuevo_nodo->siguiente = NULL;
		if (anterior)
			*anterior = NULL;
		(*tope)++;
		return nuevo_nodo;
	}

	if (strcmp(clave, nodo->clave) == 0){
		if (anterior)
			*anterior = nodo->elemento;
		nodo->elemento = elemento;
		return nodo;
	} 

	nodo->siguiente = insertar_nodo (nodo->siguiente, clave, elemento, anterior, tope);
	return nodo;
}


hash_t *rehash (hash_t *hash)
{	
	size_t nueva_capacidad = hash->capacidad * 2;
	struct nodo **nuevo_array = calloc(nueva_capacidad, sizeof(struct nodo*));
	size_t tope_aux = 0;
	if (!nuevo_array)
		return NULL;
	for (int i = 0; i < hash->capacidad; i++){
		struct nodo *nodo_actual = hash->array[i];
		while (nodo_actual){
			size_t posicion = funcion_hash(nodo_actual->clave, nueva_capacidad);
			struct nodo *aux = nodo_actual;
			nuevo_array[posicion]  = insertar_nodo(nuevo_array[posicion] , nodo_actual->clave, nodo_actual->elemento, NULL, &tope_aux);
			nodo_actual = nodo_actual->siguiente;
			free(aux->clave);
			free(aux);
		}
	}
	hash->capacidad = nueva_capacidad;
	free(hash->array);
	hash->array = nuevo_array;

	return hash;
}

hash_t *hash_insertar(hash_t *hash, const char *clave, void *elemento,
		      void **anterior)
{	
	if (!hash || !clave)
		return NULL;
	size_t porcentaje = (hash->tope + 1)* 100 / hash->capacidad;
	if (porcentaje >= 75){
		hash_t *nuevo_hash = rehash(hash);
		if (!nuevo_hash)
			return NULL;
		hash = nuevo_hash;
	}
	size_t posicion = funcion_hash((char *)clave, hash->capacidad);
	hash->array[posicion] = insertar_nodo(hash->array[posicion], clave, elemento, anterior, &(hash->tope));
	return hash;
}

struct nodo *eliminar_nodo(struct nodo* nodo, const char *clave ,void **elemento_encontrado, bool *eliminado)
{
	if (!nodo)
		return NULL;
	struct nodo *nodo_aux = NULL;
	if (strcmp(nodo->clave, clave) == 0){
		*elemento_encontrado = nodo->elemento;
		nodo_aux = nodo->siguiente;
		free(nodo->clave);
		free(nodo);
		*eliminado = true;
		return nodo_aux;
	}

	nodo->siguiente = eliminar_nodo(nodo->siguiente, clave, elemento_encontrado, eliminado);
	return nodo;
}

void *hash_quitar(hash_t *hash, const char *clave)
{	
	if (!hash || !clave || hash->tope == 0)
		return NULL;
	size_t posicion = funcion_hash((char *)clave, hash->capacidad);
	void *elemento = NULL;
	bool eliminado = false;
	hash->array[posicion] = eliminar_nodo(hash->array[posicion], clave ,&elemento, &eliminado);
	if (eliminado)
		hash->tope--;
	return elemento;
}

void buscar_nodo(struct nodo* nodo, const char *clave ,void **elemento_encontrado, bool *encontrado)
{
	if (!nodo)
		return;
	if (strcmp(nodo->clave, clave) == 0){
		*elemento_encontrado = nodo->elemento;
		*encontrado = true;
		return;
	}

	buscar_nodo(nodo->siguiente, clave, elemento_encontrado, encontrado);
}

void *hash_obtener(hash_t *hash, const char *clave)
{
	if (!hash || !clave || hash->tope == 0)
		return NULL;
	size_t posicion = funcion_hash((char *)clave, hash->capacidad);
	void *elemento = NULL;
	bool existe = false;
	buscar_nodo(hash->array[posicion], clave ,&elemento, &existe);
	return elemento;
}

bool hash_contiene(hash_t *hash, const char *clave)
{
	if (!hash || !clave || hash->tope == 0)
		return false;
	size_t posicion = funcion_hash((char *)clave, hash->capacidad);
	void *elemento = NULL;
	bool encontrado = false;
	buscar_nodo(hash->array[posicion], clave ,&elemento, &encontrado);
	return encontrado;
}

size_t hash_cantidad(hash_t *hash)
{	
	if (!hash)
		return 0;
	return hash->tope;
}

void hash_destruir(hash_t *hash)
{
	if (!hash)
		return;
	hash_destruir_todo(hash, NULL);
}

void hash_destruir_todo(hash_t *hash, void (*destructor)(void *))
{
	if (!hash)
		return;
	struct nodo *nodo_actual = NULL;
	struct nodo *aux = NULL;
	for (int i = 0; i < hash->capacidad; i++){
		nodo_actual = hash->array[i];
		while(nodo_actual){
			if (nodo_actual->elemento && destructor)
				destructor(nodo_actual->elemento);
			aux = nodo_actual;
			nodo_actual = nodo_actual->siguiente;
			free(aux->clave);
			free(aux);
		}
	}
	free(hash->array);
	free(hash);
}

size_t hash_con_cada_clave(hash_t *hash,
			   bool (*f)(const char *clave, void *valor, void *aux),
			   void *aux)
{	
	if (!hash || !f )
		return 0;
	struct nodo *nodo_actual = NULL;
	size_t veces = 0;
	bool resultado = true;
	for (int i = 0; i < hash->capacidad; i++){
		nodo_actual = hash->array[i];
		while(nodo_actual){
			resultado = f(nodo_actual->clave, nodo_actual->elemento, aux);
			veces++;
			if (!resultado)
				return veces;
			nodo_actual = nodo_actual->siguiente;
		}
	}

	return veces;
}
