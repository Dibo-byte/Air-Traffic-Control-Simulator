#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

// --- Definições Globais ---
#define MAX_AERONAVES 10
#define NUM_PISTAS 3 // O recurso limitado controlado pelo Semáforo
#define SHM_NAME "/aero_shm" // Nome da Memória Compartilhada
#define MQ_NAME "/aero_mq_req" // Nome da Fila de Mensagens
#define SEM_PISTAS_NAME "/aero_sem_pistas" // Nome do Semáforo
#define MQ_MAX_MSGS 10
#define MQ_MSG_SIZE sizeof(Requisicao)

// -----------------------------------------------------
// 1. Estruturas de Status e Comunicação
// -----------------------------------------------------

// Status que cada aeronave pode ter
typedef enum {
    ESPERANDO = 0,
    POUSANDO,
    DECOLANDO,
    EM_VOO,
    CONCLUIDO
} StatusAeronave;

// Estrutura enviada da Aeronave (Thread) para a Torre (Processo) via Fila de Mensagens
typedef struct {
    int id_aeronave;
    StatusAeronave acao_solicitada; // POUSANDO ou DECOLAGEM
} Requisicao;

// Estrutura de dados globais (Memória Compartilhada)
typedef struct {
    int pistas_ocupadas;
    StatusAeronave status_aeronaves[MAX_AERONAVES];
    // Mutex para proteger o acesso à memória compartilhada (sincronização entre Processos)
    pthread_mutex_t shm_mutex; 
} Shared_Data;

// Estrutura de argumentos para a thread da aeronave
typedef struct {
    int id;
    sem_t *sem_pistas; 
    mqd_t mqd_torre; 
    Shared_Data *shm_ptr; 
} AeronaveArgs;

// --- Funções e Variáveis Compartilhadas ---
void cleanup(void);
void setup_shared_memory(void);

// DECLARAÇÃO GLOBAL DE VARIÁVEL DE IPC (CORRIGIDO)
extern sem_t *sem_pistas; 

#endif // COMMON_H
