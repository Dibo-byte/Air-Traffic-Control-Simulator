# 🛫 Sistema de Controle de Pousos e Decolagens (Concorrência e IPC)

Este projeto simula o funcionamento de um sistema de controle de tráfego
aéreo utilizando **programação concorrente**, **sincronização entre
threads** e **IPC (Interprocess Communication)**.\
Ele gerencia múltiplas aeronaves concorrendo por pistas, garantindo
segurança, exclusão mútua e coordenação entre processos.

------------------------------------------------------------------------

## ✨ Funcionalidades Principais

-   Gerenciamento simultâneo de aeronaves em:
    -   **Fila de espera**
    -   **Pouso**
    -   **Decolagem**
    -   **Voo**
-   Controle de acesso às pistas usando:
    -   **Mutex**
    -   **Semáforos**
    -   **Variáveis de condição**
-   Estrutura compartilhada com atualização de estado em tempo real.
-   Simulação do ciclo completo de uma aeronave.
-   Log formatado com informações como:
    -   Estado
    -   Requisições pendentes
    -   Pistas livres/ocupadas

------------------------------------------------------------------------

## 🧩 Estrutura do Código

    📁 Projeto
    ├── aeronave.c
		├── common.c
    ├── common.h
    ├── main_manager.c
		├── torre.c
    └── README.md

------------------------------------------------------------------------

## 🔧 Como Compilar

``` bash
# Compila o processo da Torre e liga as bibliotecas IPC e Threads
gcc -o torre torre.c common.c -lrt -pthread

# Compila o processo Gerador de Aeronaves e liga as bibliotecas
gcc -o aeronave aeronave.c common.c -lrt -pthread

# Compila o Processo Manager (ponto de entrada)
gcc -o main_manager main_manager.c common.c -lrt -pthread
```

------------------------------------------------------------------------

## ▶️ Como Executar

``` bash
./main_manager
```

------------------------------------------------------------------------

## 📌 Estados das Aeronaves

  Estado           Descrição
  ---------------- --------------------------------------------------
  **ESPERANDO**  -  Aguardando pista para pouso/decolagem
	
  **POUSANDO**    - Processo de pouso
	
  **DECOLAGEM**    - Processo de decolagem
	
  **EM VOO**       - Aeronave já decolou ou ainda não solicitou pouso

------------------------------------------------------------------------

## 🧵 Concorrência Utilizada

### 🔒 *Mutex*

Garante exclusão mútua no acesso à estrutura compartilhada.

### 🚦 *Semáforos*

Controlam quantas aeronaves podem utilizar as pistas.

### 📡 *Variáveis de condição*

Sinalizam aeronaves quando uma pista está disponível.

------------------------------------------------------------------------

## 📊 Exemplo de Saída do Sistema

    struct Data
    pistas_ocupadas: 3 / 10

    Aeronave 0 — ESPERANDO
    Aeronave 1 — POUSANDO
    Aeronave 2 — POUSANDO
    Aeronave 3 — EM VOO
    Aeronave 4 — EM VOO
    Aeronave 5 — EM VOO

------------------------------------------------------------------------

## 📚 Referências

-   DECEA --- Torre de Controle e Segurança. Disponível em:\
    https://blogsobrevoo.decea.mil.br/torre-de-controle-e-seguranca-torres-de-controle/

------------------------------------------------------------------------

## 👨‍💻 Autor

**Afonso Aguiar de Carvalho**\
**Francisco Brito Veras Filho**\
**Gabriel Augusto Tavares Dibo**\

Projeto desenvolvido para avaliação da disciplina de **Sistemas Eletrônicos de Tempo Real** -- UEA.
