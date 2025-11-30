// aeronave.c
#include "common.h"

void *thread_aeronave(void *arg) {
    AeronaveArgs *args = (AeronaveArgs *)arg;
    int id = args->id;
    mqd_t mqd_torre = args->mqd_torre;
    
    // Simula a chegada e espera inicial
    printf("[Aeronave %d] Entrou na área de espera (Holding Pattern).\n", id);
    sleep(1 + (rand() % 3));

    // [1] Enviar Solicitação de POUSO (Modelo Produtor-Consumidor)
    Requisicao req_pouso = { .id_aeronave = id, .acao_solicitada = POUSANDO };
    if (mq_send(mqd_torre, (char *)&req_pouso, MQ_MSG_SIZE, 0) == -1) {
        perror("mq_send pouso");
        pthread_exit(NULL);
    }
    printf("[Aeronave %d] Solicitou POUSO à Torre.\n", id);
    
    // Tempo para pousar e estar em solo
    sleep(5 + (rand() % 5)); 

    // [2] Enviar Solicitação de DECOLAGEM
    Requisicao req_decolagem = { .id_aeronave = id, .acao_solicitada = DECOLANDO };
    if (mq_send(mqd_torre, (char *)&req_decolagem, MQ_MSG_SIZE, 0) == -1) {
        perror("mq_send decolagem");
        pthread_exit(NULL);
    }
    printf("[Aeronave %d] Solicitou DECOLAGEM à Torre.\n", id);
    
    // Tempo para decolar
    sleep(5 + (rand() % 5));

    printf("[Aeronave %d] Em voo. Thread encerrada.\n", id);
    pthread_exit(NULL);
}


int main() {
    // [1] Abrir a Fila de Mensagens (para enviar)
    mqd_t mqd_torre = mq_open(MQ_NAME, O_WRONLY);
    if (mqd_torre == (mqd_t)-1) { perror("mq_open aeronave W"); return 1; }

    // [2] Abrir Memória Compartilhada (apenas para mapear o ponteiro na thread_args)
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open aeronave R"); return 1; }
    Shared_Data *shm_ptr = mmap(NULL, sizeof(Shared_Data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { perror("mmap aeronave R"); return 1; }
    close(shm_fd); 

    // [3] Abrir Semáforo (para fechar no final)
    sem_t *sem_pistas_local = sem_open(SEM_PISTAS_NAME, 0);
    if (sem_pistas_local == SEM_FAILED) { perror("sem_open aeronave R"); return 1; }
    
    pthread_t threads[MAX_AERONAVES];
    AeronaveArgs args[MAX_AERONAVES];
    
    srand(time(NULL));

    // [4] Criar threads de aeronaves (Programação Concorrente)
    for (int i = 0; i < MAX_AERONAVES; i++) {
        args[i].id = i;
        args[i].sem_pistas = sem_pistas_local;
        args[i].mqd_torre = mqd_torre;
        args[i].shm_ptr = shm_ptr;

        if (pthread_create(&threads[i], NULL, thread_aeronave, &args[i]) != 0) {
            perror("pthread_create");
            break;
        }
        usleep(500000); // Espaço entre a criação das aeronaves
    }

    // [5] Esperar a conclusão de todas as threads (pthread_join)
    for (int i = 0; i < MAX_AERONAVES; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n[Processo Gerador] Todas as aeronaves concluíram. Encerrando.\n");
    mq_close(mqd_torre);
    munmap(shm_ptr, sizeof(Shared_Data));
    sem_close(sem_pistas_local);

    return 0;
}
