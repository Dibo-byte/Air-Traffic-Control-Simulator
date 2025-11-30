// common.c
#include "common.h"

// DEFINIÇÃO DA VARIÁVEL GLOBAL (CORRIGIDO)
sem_t *sem_pistas = SEM_FAILED; // Inicializada como SEM_FAILED

// Implementação da função de limpeza de recursos de IPC (chamada pelo Manager ou Torre)
void cleanup(void) {
    printf("\n[Cleanup] Iniciando remoção de recursos de IPC...\n");
    
    // 1. Unlink (exclui) a Fila de Mensagens
    if (mq_unlink(MQ_NAME) == 0) {
        printf("[Cleanup] Fila de Mensagens removida: %s\n", MQ_NAME);
    }
    
    // 2. Unlink (exclui) o Semáforo
    if (sem_unlink(SEM_PISTAS_NAME) == 0) {
        printf("[Cleanup] Semáforo removido: %s\n", SEM_PISTAS_NAME);
    }
    
    // 3. Unlink (exclui) a Memória Compartilhada
    if (shm_unlink(SHM_NAME) == 0) {
        printf("[Cleanup] Memória Compartilhada removida: %s\n", SHM_NAME);
    }
}

// Função que cria e inicializa a Memória Compartilhada e o Mutex de Processos Compartilhados
void setup_shared_memory(void) {
    // 1. Cria ou abre a SHM
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open setup"); exit(1); }
    
    // 2. Define o tamanho da SHM
    if (ftruncate(shm_fd, sizeof(Shared_Data)) == -1) { perror("ftruncate"); exit(1); }

    // 3. Mapeia a SHM
    Shared_Data *shm_ptr = mmap(NULL, sizeof(Shared_Data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { perror("mmap setup"); exit(1); }
    close(shm_fd);

    // 4. Configura o Mutex para ser compartilhado entre Processos (CRUCIAL)
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) { perror("mutexattr_init"); exit(1); }
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) { perror("setpshared"); exit(1); }
    
    // 5. Inicializa o Mutex e os dados
    if (pthread_mutex_init(&shm_ptr->shm_mutex, &attr) != 0) { perror("mutex_init"); exit(1); }

    shm_ptr->pistas_ocupadas = 0;
    for(int i = 0; i < MAX_AERONAVES; i++) shm_ptr->status_aeronaves[i] = ESPERANDO;

    // Desmapeia. O Mutex e os dados persistem.
    if (munmap(shm_ptr, sizeof(Shared_Data)) == -1) { perror("munmap setup"); exit(1); }
    printf("[Manager] Memória Compartilhada e Mutex de Processos inicializados.\n");
}
