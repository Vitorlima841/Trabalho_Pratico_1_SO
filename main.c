#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_CLIENTES 5
#define NUM_RECURSOS 3

int disponivel[NUM_RECURSOS];
int maximo[NUM_CLIENTES][NUM_RECURSOS];
int alocado[NUM_CLIENTES][NUM_RECURSOS];
int necessario[NUM_CLIENTES][NUM_RECURSOS];

pthread_mutex_t mutex;

// Verifica se o sistema está em estado seguro
int esta_seguro() {
    int trabalho[NUM_RECURSOS];
    int finalizado[NUM_CLIENTES] = {0};

    for (int i = 0; i < NUM_RECURSOS; i++)
        trabalho[i] = disponivel[i];

    for (int c = 0; c < NUM_CLIENTES; c++) {
        int encontrou = 0;
        for (int i = 0; i < NUM_CLIENTES; i++) {
            if (!finalizado[i]) {
                int j;
                for (j = 0; j < NUM_RECURSOS; j++) {
                    if (necessario[i][j] > trabalho[j])
                        break;
                }
                if (j == NUM_RECURSOS) {
                    for (int k = 0; k < NUM_RECURSOS; k++)
                        trabalho[k] += alocado[i][k];
                    finalizado[i] = 1;
                    encontrou = 1;
                }
            }
        }
        if (!encontrou)
            return 0;
    }
    return 1;
}

// Solicitação de recursos
int solicitar_recursos(int cliente, int solicitacao[]) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < NUM_RECURSOS; i++) {
        if (solicitacao[i] > necessario[cliente][i] || solicitacao[i] > disponivel[i]) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    // Tentativa de alocação
    for (int i = 0; i < NUM_RECURSOS; i++) {
        disponivel[i] -= solicitacao[i];
        alocado[cliente][i] += solicitacao[i];
        necessario[cliente][i] -= solicitacao[i];
    }

    // Verifica se o estado é seguro
    if (!esta_seguro()) {
        // Desfaz alocação
        for (int i = 0; i < NUM_RECURSOS; i++) {
            disponivel[i] += solicitacao[i];
            alocado[cliente][i] -= solicitacao[i];
            necessario[cliente][i] += solicitacao[i];
        }
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

// Liberação de recursos
int liberar_recursos(int cliente, int liberacao[]) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < NUM_RECURSOS; i++) {
        if (liberacao[i] > alocado[cliente][i]) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        disponivel[i] += liberacao[i];
        alocado[cliente][i] -= liberacao[i];
        necessario[cliente][i] += liberacao[i];
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

// Thread do cliente
void* cliente_thread(void* arg) {
    int cliente = *(int*)arg;
    int requisicao[NUM_RECURSOS];
    int liberacao[NUM_RECURSOS];

    while (1) {
        sleep(rand() % 3 + 1);

        // Gera solicitação aleatória
        for (int i = 0; i < NUM_RECURSOS; i++)
            requisicao[i] = rand() % (necessario[cliente][i] + 1);

        if (solicitar_recursos(cliente, requisicao) == 0) {
            printf("Cliente %d solicitou: ", cliente);
            for (int i = 0; i < NUM_RECURSOS; i++)
                printf("%d ", requisicao[i]);
            printf("\n");
        }

        sleep(rand() % 3 + 1);

        // Gera liberação aleatória
        for (int i = 0; i < NUM_RECURSOS; i++)
            liberacao[i] = rand() % (alocado[cliente][i] + 1);

        if (liberar_recursos(cliente, liberacao) == 0) {
            printf("Cliente %d liberou: ", cliente);
            for (int i = 0; i < NUM_RECURSOS; i++)
                printf("%d ", liberacao[i]);
            printf("\n");
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != NUM_RECURSOS + 1) {
        printf("Uso correto: %s <recurso1> <recurso2> <recurso3>\n", argv[0]);
        return -1;
    }

    srand(time(NULL));
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_RECURSOS; i++)
        disponivel[i] = atoi(argv[i + 1]);

    // Inicializa arrays
    for (int i = 0; i < NUM_CLIENTES; i++) {
        for (int j = 0; j < NUM_RECURSOS; j++) {
            maximo[i][j] = rand() % (disponivel[j] + 1);
            alocado[i][j] = 0;
            necessario[i][j] = maximo[i][j];
        }
    }

    pthread_t threads[NUM_CLIENTES];
    int ids[NUM_CLIENTES];

    for (int i = 0; i < NUM_CLIENTES; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, cliente_thread, &ids[i]);
    }

    for (int i = 0; i < NUM_CLIENTES; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}
