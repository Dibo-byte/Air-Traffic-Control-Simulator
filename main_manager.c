// main_manager.c
#include "common.h"

// Função de monitoramento do Log (com Visualização ANSI de Terminal)
void monitor_log(Shared_Data *shm_ptr) {
    // Limpa tela e vai para home (código ANSI)
    printf("\033[2J\033[H"); 

    printf("\033[1m+-----------------------------------------------+\033[0m\n");
    printf("\033[1m| \033[36mSimulador ATC: LOG CENTRAL DE TRÁFEGO\033[0m         |\n");
    printf("\033[1m+-----------------------------------------------+\033[0m\n");

    // Lógica para proteger a leitura com Mutex
    if (pthread_mutex_lock(&shm_ptr->shm_mutex) != 0) {
        perror("Log: lock error");
        return;
    }

    // --- Pistas (Recurso Limitado / Semáforo) ---
    printf("\033[1m| PISTAS OCUPADAS (%d/%d): [\033[0m", shm_ptr->pistas_ocupadas, NUM_PISTAS);
    
    int ocupadas = shm_ptr->pistas_ocupadas;
    for (int i = 0; i < NUM_PISTAS; i++) {
        if (i < ocupadas) {
            printf("\033[32m#\033[0m "); // Ocupada (Verde)
        } else {
            printf("\033[37m.\033[0m "); // Livre (Branco)
        }
    }
    printf("\033[1m]\033[0m\n");
    printf("+-----------------------------------------------+\033[0m\n");
    
    // --- Status das Aeronaves (Threads) ---
    printf("\033[1m| STATUS DAS AERONAVES:\033[0m\n");
    for (int i = 0; i < MAX_AERONAVES; i++) {
        const char *status_str;
        const char *color_code;
        
        switch (shm_ptr->status_aeronaves[i]) {
            case ESPERANDO: status_str = "ESPERANDO PISTA"; color_code = "\033[33m"; break; // Amarelo
            case POUSANDO: status_str = "POUSANDO (SEÇÃO CRÍTICA)"; color_code = "\033[32m"; break; // Verde
            case DECOLANDO: status_str = "DECOLANDO (SEÇÃO CRÍTICA)"; color_code = "\033[32m"; break; // Verde
            case EM_VOO: status_str = "EM VOO"; color_code = "\033[34m"; break; // Azul
            case CONCLUIDO: status_str = "VOO CONCLUÍDO"; color_code = "\033[37m"; break; // Branco
            default: status_str = "INDETERMINADO"; color_code = "\033[31m";
        }
        printf("| Aeronave %02d: %s%s\033[0m\n", i, color_code, status_str);
    }
    
    pthread_mutex_unlock(&shm_ptr->shm_mutex);
    printf("+-----------------------------------------------+\033[0m\n");
    printf("Manager PID: %d. Pressione CTRL+C para encerrar.\n", getpid());
}


int main(int argc, char *argv[]) {
    // 1. Inicializa a SHM e o Mutex de Processos
    setup_shared_memory();

    // -----------------------------------------------------------
    // [1] Criar Processo da TORRE (fork + exec)
    // -----------------------------------------------------------
    pid_t pid_torre = fork();
    if (pid_torre == 0) {
        printf("[Manager] Iniciando Processo da Torre...\n");
        execl("./torre", "torre", NULL);
        perror("execl torre"); 
        exit(1);
    }

    sleep(2);

    // -----------------------------------------------------------
    // [2] Criar Processo do LOG (fork)
    // -----------------------------------------------------------
    pid_t pid_log = fork();
    if (pid_log == 0) {
        printf("[Manager] Iniciando Processo de Log/Monitoramento...\n");
        // Conexão de LEITURA com a Memória Compartilhada.
        int log_shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
        if (log_shm_fd == -1) { perror("Log shm_open"); exit(1); }
        Shared_Data *shm_ptr = mmap(NULL, sizeof(Shared_Data), PROT_READ, MAP_SHARED, log_shm_fd, 0);
        close(log_shm_fd);

        while(1) {
            monitor_log(shm_ptr);
            usleep(500000); // Atualiza a cada 500ms
        }
        munmap(shm_ptr, sizeof(Shared_Data));
        exit(0);
    }

    // -----------------------------------------------------------
    // [3] Criar Processo das AERONAVES (fork + exec)
    // -----------------------------------------------------------
    pid_t pid_aeronaves = fork();
    if (pid_aeronaves == 0) {
        printf("[Manager] Iniciando Processo Gerador de Aeronaves...\n");
        execl("./aeronave", "aeronave", NULL);
        perror("execl aeronave"); 
        exit(1);
    }

    // -----------------------------------------------------------
    // [4] Manager espera e encerra o sistema
    // -----------------------------------------------------------
    int status;
    waitpid(pid_aeronaves, &status, 0); 
    printf("\n[Manager] Processo das Aeronaves terminou. Encerrando Log e Torre.\n");

    // Encerra os processos Log e Torre (IPC Sinais)
    kill(pid_log, SIGTERM); 
    kill(pid_torre, SIGTERM); 

    wait(NULL); 
    
    // O Manager faz a limpeza final dos recursos IPC
    cleanup(); 

    printf("[Manager] Sistema Encerrado.\n");
    return 0;
}
