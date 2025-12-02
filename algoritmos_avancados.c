#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Para a função rand()

// --- Constantes e Definições ---
#define MAX_PISTAS 100
#define TAMANHO_HASH 10 // Tamanho da Tabela Hash (simplificado)

// --- 1. Estrutura de Dados da Mansão (Árvore Binária de Cômodos) ---
typedef struct Comodo {
    char nome[50];
    char pista[50]; // Pista estática associada ao cômodo
    struct Comodo *esquerda;
    struct Comodo *direita;
} Comodo;

// --- 2. Estrutura de Dados das Pistas Coletadas (Árvore BST) ---
typedef struct PistaNode {
    char pista[50];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// --- 3. Estrutura de Dados da Tabela Hash (Associação Pista -> Suspeito) ---
typedef struct HashEntry {
    char pista[50];
    char suspeito[50];
    struct HashEntry *proximo; // Tratamento de colisão por encadeamento
} HashEntry;

typedef struct {
    HashEntry *tabela[TAMANHO_HASH];
} TabelaHash;

// --- Variáveis Globais de Jogo ---
PistaNode *pistasColetadas = NULL; // Raiz da Árvore BST de Pistas
TabelaHash hashSuspeitos;
// Suspeitos fixos para associação
char suspeitosDefinidos[4][50] = {"Dr. Black", "Sra. Peacock", "Coronel Mustard", "Dona White"}; 

// --- Protótipos das Funções ---
Comodo *criarSala(const char *nome, const char *pista);
void explorarSalas(Comodo *raiz);
PistaNode *inserirPista(PistaNode *raiz, const char *pista);
void listarPistas(PistaNode *raiz);
int calcularHash(const char *chave);
void inserirNaHash(TabelaHash *hash, const char *pista, const char *suspeito);
const char *encontrarSuspeito(TabelaHash *hash, const char *pista);
void verificarSuspeitoFinal();
int contarPistasPorSuspeito(PistaNode *raizPistas, TabelaHash *hash, const char *suspeito);
void inicializarHash(TabelaHash *hash);
void liberarMemoriaComodos(Comodo *raiz);
void liberarMemoriaPistas(PistaNode *raiz);
void liberarMemoriaHash(TabelaHash *hash);

// --- --------------------------------------------------------------------- ---
// ---------------------------- IMPLEMENTAÇÃO --------------------------------
// --- --------------------------------------------------------------------- ---

// -----------------------------------------------------------------------------
//                                FUNÇÕES DA MANSÃO
// -----------------------------------------------------------------------------

/**
 * @brief Cria dinamicamente um novo cômodo para a Árvore Binária.
 * @param nome Nome do cômodo (ex: "Biblioteca").
 * @param pista Pista estática associada a este cômodo.
 * @return Ponteiro para a nova estrutura Comodo.
 */
Comodo *criarSala(const char *nome, const char *pista) {
    Comodo *novoComodo = (Comodo *)malloc(sizeof(Comodo));
    if (novoComodo == NULL) {
        perror("Erro ao alocar memória para o cômodo");
        exit(EXIT_FAILURE);
    }
    strncpy(novoComodo->nome, nome, sizeof(novoComodo->nome) - 1);
    novoComodo->nome[sizeof(novoComodo->nome) - 1] = '\0';
    
    strncpy(novoComodo->pista, pista, sizeof(novoComodo->pista) - 1);
    novoComodo->pista[sizeof(novoComodo->pista) - 1] = '\0';

    novoComodo->esquerda = NULL;
    novoComodo->direita = NULL;
    return novoComodo;
}

/**
 * @brief Permite ao jogador navegar interativamente pela mansão (Árvore Binária).
 * Ativa o sistema de coleta de pistas a cada sala visitada.
 * @param atual Raiz/nó atual da mansão.
 */
void explorarSalas(Comodo *atual) {
    char escolha;
    if (atual == NULL) return;

    do {
        printf("\n🚪 Você está na sala: **%s**\n", atual->nome);

        if (strlen(atual->pista) > 0) {
            printf("  🔎 Pista encontrada: **%s**\n", atual->pista);
            
            // Verifica se a pista já foi associada antes de coletar
            if (encontrarSuspeito(&hashSuspeitos, atual->pista) == NULL) {
                // A lógica de associação Suspeito/Pista é feita aqui de forma codificada
                srand(time(NULL));
                int idxSuspeito = (rand() % 4); // Escolhe um suspeito aleatório
                
                // 1. Insere na Hash
                inserirNaHash(&hashSuspeitos, atual->pista, suspeitosDefinidos[idxSuspeito]);
                
                // 2. Insere na BST
                pistasColetadas = inserirPista(pistasColetadas, atual->pista);
                
                printf("  ✅ Pista adicionada ao caderno de notas e associada a **%s**.\n", encontrarSuspeito(&hashSuspeitos, atual->pista));
            } else {
                 printf("  (Pista já registrada.)\n");
            }
        } else {
            printf("  Nada de novo nesta sala.\n");
        }

        printf("\nEscolha sua próxima ação (e = Esquerda, d = Direita, s = Sair e Acusar): ");
        if (scanf(" %c", &escolha) != 1) {
             // Limpeza e nova tentativa em caso de falha na leitura
             while (getchar() != '\n'); 
             escolha = ' '; 
             continue;
        }
        while (getchar() != '\n'); 

        switch (escolha) {
            case 'e':
            case 'E':
                if (atual->esquerda) {
                    explorarSalas(atual->esquerda);
                    return; 
                } else {
                    printf("❌ Não há cômodo à esquerda. Fique em **%s**.\n", atual->nome);
                }
                break;
            case 'd':
            case 'D':
                if (atual->direita) {
                    explorarSalas(atual->direita);
                    return; 
                } else {
                    printf("❌ Não há cômodo à direita. Fique em **%s**.\n", atual->nome);
                }
                break;
            case 's':
            case 'S':
                printf("\n--- Fim da Exploração. Hora do Julgamento! ---\n");
                return;
            default:
                printf("❌ Opção inválida. Tente novamente.\n");
        }
    } while (1); 
}

// -----------------------------------------------------------------------------
//                              FUNÇÕES DA ÁRVORE BST
// -----------------------------------------------------------------------------

/**
 * @brief Insere uma nova pista na Árvore Binária de Busca (BST) de forma ordenada.
 * @param raiz Raiz da BST.
 * @param pista String contendo o nome da pista.
 * @return Ponteiro para a nova raiz (ou a raiz inalterada se a pista já existia).
 */
PistaNode *inserirPista(PistaNode *raiz, const char *pista) {
    if (raiz == NULL) {
        PistaNode *novoNo = (PistaNode *)malloc(sizeof(PistaNode));
        if (novoNo == NULL) {
            perror("Erro ao alocar memória para a pista");
            exit(EXIT_FAILURE);
        }
        strncpy(novoNo->pista, pista, sizeof(novoNo->pista) - 1);
        novoNo->pista[sizeof(novoNo->pista) - 1] = '\0';
        novoNo->esquerda = novoNo->direita = NULL;
        return novoNo;
    }

    int comparacao = strcmp(pista, raiz->pista);

    if (comparacao < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, pista);
    } else if (comparacao > 0) {
        raiz->direita = inserirPista(raiz->direita, pista);
    } 
    // Se comparacao == 0, a pista já existe (não faz nada)

    return raiz;
}

/**
 * @brief Lista todas as pistas coletadas em ordem alfabética (percurso In-Order).
 * @param raiz Raiz da BST de pistas.
 */
void listarPistas(PistaNode *raiz) {
    if (raiz != NULL) {
        listarPistas(raiz->esquerda);
        printf("- %s\n", raiz->pista);
        listarPistas(raiz->direita);
    }
}

// -----------------------------------------------------------------------------
//                             FUNÇÕES DA TABELA HASH
// -----------------------------------------------------------------------------

/**
 * @brief Inicializa a Tabela Hash, zerando todos os ponteiros.
 */
void inicializarHash(TabelaHash *hash) {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        hash->tabela[i] = NULL;
    }
}

/**
 * @brief Função hash simples para determinar o índice da tabela.
 * @param chave String (nome da pista) para calcular o hash.
 * @return Índice na tabela (0 a TAMANHO_HASH - 1).
 */
int calcularHash(const char *chave) {
    unsigned long hash = 5381;
    int c;
    // O(N) onde N é o tamanho da string, mas O(1) em relação ao tamanho da hash
    while ((c = *chave++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % TAMANHO_HASH;
}

/**
 * @brief Insere a associação Pista -> Suspeito na Tabela Hash.
 * @param hash Ponteiro para a TabelaHash.
 * @param pista Chave da entrada (nome da pista).
 * @param suspeito Valor da entrada (nome do suspeito associado).
 */
void inserirNaHash(TabelaHash *hash, const char *pista, const char *suspeito) {
    int indice = calcularHash(pista);

    // Cria nova entrada
    HashEntry *novaEntrada = (HashEntry *)malloc(sizeof(HashEntry));
    if (novaEntrada == NULL) {
        perror("Erro ao alocar memória para a entrada hash");
        exit(EXIT_FAILURE);
    }
    strncpy(novaEntrada->pista, pista, sizeof(novaEntrada->pista) - 1);
    novaEntrada->pista[sizeof(novaEntrada->pista) - 1] = '\0';
    
    strncpy(novaEntrada->suspeito, suspeito, sizeof(novaEntrada->suspeito) - 1);
    novaEntrada->suspeito[sizeof(novaEntrada->suspeito) - 1] = '\0';
    
    // Insere no início da lista encadeada (tratamento de colisão)
    novaEntrada->proximo = hash->tabela[indice];
    hash->tabela[indice] = novaEntrada;
}

/**
 * @brief Consulta a tabela hash para encontrar o suspeito associado a uma pista.
 * @param hash Ponteiro para a TabelaHash.
 * @param pista Chave a ser buscada.
 * @return String constante do nome do suspeito, ou NULL se não encontrado.
 */
const char *encontrarSuspeito(TabelaHash *hash, const char *pista) {
    int indice = calcularHash(pista);
    HashEntry *atual = hash->tabela[indice];

    while (atual != NULL) {
        if (strcmp(atual->pista, pista) == 0) {
            return atual->suspeito; // Suspeito encontrado (O(1) médio)
        }
        atual = atual->proximo;
    }
    return NULL; // Pista não encontrada na hash
}

// -----------------------------------------------------------------------------
//                             FUNÇÕES DE JULGAMENTO
// -----------------------------------------------------------------------------

/**
 * @brief Função auxiliar recursiva para contar quantas pistas na BST apontam para
 * o suspeito acusado, consultando a tabela hash para cada pista.
 * @param raizPistas Raiz da BST de pistas.
 * @param hash Tabela Hash de associações Pista->Suspeito.
 * @param suspeito Nome do suspeito a ser contado.
 * @return O número total de pistas que apontam para o suspeito.
 */
int contarPistasPorSuspeito(PistaNode *raizPistas, TabelaHash *hash, const char *suspeito) {
    if (raizPistas == NULL) {
        return 0;
    }

    int contagem = 0;
    
    // 1. Visita o nó atual (pista)
    const char *suspeitoDaPista = encontrarSuspeito(hash, raizPistas->pista);
    
    if (suspeitoDaPista != NULL && strcmp(suspeitoDaPista, suspeito) == 0) {
        contagem = 1; // Pista encontrada para o suspeito
    }

    // 2. Visita o lado esquerdo
    contagem += contarPistasPorSuspeito(raizPistas->esquerda, hash, suspeito);
    
    // 3. Visita o lado direito
    contagem += contarPistasPorSuspeito(raizPistas->direita, hash, suspeito);
    
    return contagem;
}

/**
 * @brief Conduz a fase de julgamento final: lista pistas, recebe acusação
 * e avalia a evidência (mínimo de 2 pistas).
 */
void verificarSuspeitoFinal() {
    char acusado[50];
    int numPistas;

    printf("\n\n--- CADERNO DE PISTAS COLETADAS ---\n");
    if (pistasColetadas == NULL) {
        printf("Você não coletou nenhuma pista.\n");
    } else {
        printf("Pistas em Ordem Alfabética (BST):\n");
        listarPistas(pistasColetadas);
    }
    printf("-----------------------------------\n");

    printf("\nQuem você acusa como o culpado?\n");
    printf("Escolha entre: Dr. Black, Sra. Peacock, Coronel Mustard, Dona White\n");
    printf("Nome do Suspeito: ");
    
    if (fgets(acusado, sizeof(acusado), stdin) == NULL) {
        printf("❌ Erro na leitura. Julgamento cancelado.\n");
        return;
    }
    // Remover a nova linha ('\n') se presente
    size_t len = strlen(acusado);
    if (len > 0 && acusado[len - 1] == '\n') {
        acusado[len - 1] = '\0';
    }

    // Contar as evidências
    numPistas = contarPistasPorSuspeito(pistasColetadas, &hashSuspeitos, acusado);

    printf("\n--- VEREDICTO ---\n");
    printf("Acusado: **%s**\n", acusado);
    printf("Evidências coletadas para esta acusação: **%d**\n", numPistas);

    if (numPistas >= 2) {
        printf("🎉 **ACUSAÇÃO BEM-SUCEDIDA!** 🎉\n");
        printf("A evidência é forte o suficiente (%d pistas) para sustentar a prisão de %s.\n", numPistas, acusado);
    } else {
        printf("🚨 **ACUSAÇÃO FRACASSADA!** 🚨\n");
        printf("Você precisa de pelo menos 2 pistas para acusar com sucesso. O culpado escapou!\n");
    }
    printf("-----------------\n");
}

// -----------------------------------------------------------------------------
//                             FUNÇÕES DE LIBERAÇÃO DE MEMÓRIA
// -----------------------------------------------------------------------------

// Funções para liberar a memória alocada dinamicamente
void liberarMemoriaComodos(Comodo *raiz) {
    if (raiz == NULL) return;
    liberarMemoriaComodos(raiz->esquerda);
    liberarMemoriaComodos(raiz->direita);
    free(raiz);
}

void liberarMemoriaPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    liberarMemoriaPistas(raiz->esquerda);
    liberarMemoriaPistas(raiz->direita);
    free(raiz);
}

void liberarMemoriaHash(TabelaHash *hash) {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        HashEntry *atual = hash->tabela[i];
        HashEntry *proximo;
        while (atual != NULL) {
            proximo = atual->proximo;
            free(atual);
            atual = proximo;
        }
        hash->tabela[i] = NULL;
    }
}

// -----------------------------------------------------------------------------
//                                  FUNÇÃO MAIN
// -----------------------------------------------------------------------------

int main() {
    // Inicializa o gerador de números aleatórios (para a associação Pista->Suspeito)
    srand(time(NULL)); 
    
    // Inicializa a Tabela Hash
    inicializarHash(&hashSuspeitos);

    printf("🕵️‍♂️ DETECTIVE QUEST: O MISTÉRIO DA MANSÃO ENIGMA 🕵️‍♀️\n");
    printf("====================================================\n");

    // Montagem do Mapa da Mansão (Árvore Binária Fixa)
    Comodo *hall = criarSala("Hall de Entrada", "Um bilhete amassado com a letra 'M'");
    Comodo *biblioteca = criarSala("Biblioteca", "Um copo de conhaque quebrado");
    Comodo *salaJantar = criarSala("Sala de Jantar", "Uma ponta de cigarro exótico");
    Comodo *cozinha = criarSala("Cozinha", "Nenhuma pista aparente");
    Comodo *quartoPrincipal = criarSala("Quarto Principal", "Um cachecol de seda vermelha");
    Comodo *escritorio = criarSala("Escritório", "O retrato de uma mulher desconhecida");
    Comodo *jardimInverno = criarSala("Jardim de Inverno", "Pegadas de lama frescas");

    // Conexões (Estrutura da Árvore Binária)
    hall->esquerda = biblioteca;
    hall->direita = salaJantar;
    
    biblioteca->esquerda = cozinha;
    biblioteca->direita = quartoPrincipal;
    
    salaJantar->direita = escritorio;
    
    quartoPrincipal->esquerda = jardimInverno;
    
    // Início da Exploração
    explorarSalas(hall);

    // Fase de Julgamento
    verificarSuspeitoFinal();

    // Limpeza da Memória
    liberarMemoriaComodos(hall);
    liberarMemoriaPistas(pistasColetadas);
    liberarMemoriaHash(&hashSuspeitos);
    
    printf("\nMemória liberada. Fim do Jogo.\n");
    
    return 0;
}