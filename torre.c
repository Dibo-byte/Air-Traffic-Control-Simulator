// torre.c
#include "common.h"

// Manipulador de sinal para encerrar de forma limpa (Chamado pelo Manager com SIGTERM)
void handle_sigterm(int signum) {
    printf("\n[Torre] Sinal de encerramento (%d) recebido.\n", signum);
    
    // Tenta fechar o descritor do semáforo.
    if (sem_pistas != SEM_FAILED) {
        sem_close(sem_pistas);
    }
    // O Manager fará o unlink de todos os recursos.
    exit(0);
}

int main() {
    // Registra o manipulador de sinal
    signal(SIGTERM, handle_sigterm); 

    // [1] Abrir a Fila de Mensagens (IPC: Fila de Mensagens)
    struct mq_attr attr = { .mq_maxmsg = MQ_MAX_MSGS, .mq_msgsize = MQ_MSG_SIZE };
    mqd_t mqd_torre = mq_open(MQ_NAME, O_RDONLY | O_CREAT, 0666, &attr);
    if (mqd_torre == (mqd_t)-1) { perror("mq_open Torre"); exit(1); }

    // [2] Abrir/Mapear Memória Compartilhada (IPC: Memória Compartilhada)
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open Torre"); exit(1); }
    Shared_Data *shm_ptr = mmap(NULL, sizeof(Shared_Data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { perror("mmap Torre"); exit(1); }
    close(shm_fd); 

    // [3] Inicializar Semáforo (Sincronização: Semáforos POSIX)
    // O valor inicial é NUM_PISTAS (3 pistas disponíveis)
    sem_pistas = sem_open(SEM_PISTAS_NAME, O_CREAT, 0666, NUM_PISTAS);
    if (sem_pistas == SEM_FAILED) { perror("sem_open Torre"); exit(1); }

    printf("[Torre] Inicializada com %d pistas. Aguardando requisições...\n", NUM_PISTAS);

    // Loop principal de processamento de requisições
    while (1) {
        Requisicao req;
        // Recebe a mensagem da Fila de Mensagens (Bloqueante)
        if (mq_receive(mqd_torre, (char *)&req, MQ_MSG_SIZE, NULL) > 0) {
            printf("[Torre] Recebeu solicitação da Aeronave %d para %s.\n", 
                   req.id_aeronave, (req.acao_solicitada == POUSANDO) ? "POUSO" : "DECOLAGEM");

            // --- SEÇÃO CRÍTICA DO SEMÁFORO ---
            printf("[Torre] Aeronave %d: Tentando obter pista...\n", req.id_aeronave);
            sem_wait(sem_pistas); // Aguarda e decrementa o semáforo
            
            // CONSEGUIU A PISTA
            printf("[Torre] Aeronave %d: Pista concedida. Iniciando %s.\n", 
                   req.id_aeronave, (req.acao_solicitada == POUSANDO) ? "POUSO" : "DECOLAGEM");

            // --- SEÇÃO CRÍTICA DA MEMÓRIA COMPARTILHADA (Mutex) ---
            pthread_mutex_lock(&shm_ptr->shm_mutex);
            shm_ptr->pistas_ocupadas++;
            shm_ptr->status_aeronaves[req.id_aeronave] = req.acao_solicitada;
            pthread_mutex_unlock(&shm_ptr->shm_mutex);
            
            // Simula o tempo de uso da pista
            sleep(2); 

            // Libera a Pista
            sem_post(sem_pistas); // Incrementa o semáforo
            
            printf("[Torre] Aeronave %d: %s CONCLUÍDO. Pista liberada.\n", 
                   req.id_aeronave, (req.acao_solicitada == POUSANDO) ? "POUSO" : "DECOLAGEM");
            
            // Atualiza status final na Memória Compartilhada
            pthread_mutex_lock(&shm_ptr->shm_mutex);
            shm_ptr->pistas_ocupadas--;
            shm_ptr->status_aeronaves[req.id_aeronave] = CONCLUIDO;
            pthread_mutex_unlock(&shm_ptr->shm_mutex);
        }
    }

    mq_close(mqd_torre);
    munmap(shm_ptr, sizeof(Shared_Data));
    sem_close(sem_pistas);
    return 0;
}
