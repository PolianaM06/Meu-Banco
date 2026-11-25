#include "meu_banco.h"
#include <stdlib.h>
#include <time.h>

/* =========================================
   FUNCOES AUXILIARES
   ========================================= */

static void limparBuffer() {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static Cliente *selecionarCliente(Banco *banco) {
    int indice;

    if (banco->qtdClientes == 0) {
        printf("\nNenhum cliente cadastrado!\n");
        return NULL;
    }

    printf("\nClientes cadastrados:\n");

    for (int i = 0; i < banco->qtdClientes; i++) {
        printf("%d - %s\n", i + 1, banco->clientes[i].nome);
    }

    printf("Escolha o numero do cliente: ");
    scanf("%d", &indice);

    if (indice < 1 || indice > banco->qtdClientes) {
        printf("Cliente invalido!\n");
        return NULL;
    }

    Cliente *cliente = &banco->clientes[indice - 1];

    verificarAlerta(cliente);

    return cliente;
}

/* =========================================
   BUSCA E CONSULTA DE CONTAS
   ========================================= */

Conta *buscarConta(Cliente *cliente, int tipo) {
    for (int i = 0; i < cliente->qtdContas; i++) {
        if (cliente->contas[i].tipo == tipo) {
            return &cliente->contas[i];
        }
    }

    return NULL;
}

float calcularSaldoTotal(Cliente *cliente) {
    float total = 0;

    for (int i = 0; i < cliente->qtdContas; i++) {
        total += cliente->contas[i].saldo;
    }

    return total;
}

void exibirNumeroConta(Conta *conta) {
    printf("%04d-%02d", conta->numero, conta->tipo);
}

void verificarAlerta(Cliente *cliente) {
    Conta *corrente = buscarConta(cliente, CORRENTE);

    if (corrente != NULL && corrente->saldo < 0) {
        printf("\nATENCAO: sua conta corrente esta negativa!\n");
        printf("Saldo atual: R$ %.2f\n", corrente->saldo);
    }
}

/* =========================================
   REGISTRO PERSISTENTE DO HISTORICO
   ========================================= */

void registrarHistorico(Banco *banco, Cliente *cliente,
                        Conta *conta, char operacao[], float valor) {

    FILE *arquivo;
    time_t agora;
    struct tm *dataHora;
    char data[30];

    agora = time(NULL);
    dataHora = localtime(&agora);

    strftime(data, sizeof(data), "%d/%m/%Y %H:%M:%S", dataHora);

    /* Guarda as ultimas cinco operacoes na memoria */

    int posicao = cliente->proximaTransacao;

    strcpy(cliente->historico[posicao].operacao, operacao);
    cliente->historico[posicao].valor = valor;

    cliente->proximaTransacao =
        (cliente->proximaTransacao + 1) % MAX_HISTORICO;

    if (cliente->qtdHistorico < MAX_HISTORICO) {
        cliente->qtdHistorico++;
    }

    /* Acrescenta o registro ao arquivo */

    arquivo = fopen("historico.txt", "a");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de historico!\n");
        return;
    }

    fprintf(arquivo,
            "[%s] Cliente: %s | Conta: ",
            data, cliente->nome);

    if (conta != NULL) {
        fprintf(arquivo, "%04d-%02d", conta->numero, conta->tipo);
    } else {
        fprintf(arquivo, "Nao informada");
    }

    fprintf(arquivo,
            " | Operacao: %s | Valor: R$ %.2f",
            operacao, valor);

    if (conta != NULL) {
        fprintf(arquivo, " | Saldo: R$ %.2f", conta->saldo);
    }

    fprintf(arquivo, "\n");

    fclose(arquivo);
}

/* =========================================
   CADASTRO DE CLIENTE
   ========================================= */

void cadastrarCliente(Banco *banco) {
    Cliente *cliente;
    int quantidade;
    int tipo;

    if (banco->qtdClientes >= MAX_CLIENTES) {
        printf("\nLimite de clientes atingido!\n");
        return;
    }

    cliente = &banco->clientes[banco->qtdClientes];

    printf("\nNome do cliente: ");
    limparBuffer();
    fgets(cliente->nome, sizeof(cliente->nome), stdin);

    cliente->nome[strcspn(cliente->nome, "\n")] = '\0';

    cliente->qtdContas = 0;
    cliente->qtdHistorico = 0;
    cliente->proximaTransacao = 0;
    cliente->possuiTitulo = 0;
    cliente->possuiEmprestimo = 0;

    printf("Quantas contas deseja abrir (1 a 3)? ");
    scanf("%d", &quantidade);

    if (quantidade < 1 || quantidade > MAX_CONTAS) {
        printf("Quantidade de contas invalida!\n");
        return;
    }

    for (int i = 0; i < quantidade; i++) {
        printf("\nTipos de conta:\n");
        printf("1 - Corrente\n");
        printf("2 - Salario\n");
        printf("3 - Poupanca\n");

        printf("Escolha o tipo da conta: ");
        scanf("%d", &tipo);

        if (tipo < CORRENTE || tipo > POUPANCA) {
            printf("Tipo de conta invalido!\n");
            i--;
            continue;
        }

        if (buscarConta(cliente, tipo) != NULL) {
            printf("O cliente ja possui esse tipo de conta!\n");
            i--;
            continue;
        }

        if ((tipo == CORRENTE &&
             buscarConta(cliente, SALARIO) != NULL) ||
            (tipo == SALARIO &&
             buscarConta(cliente, CORRENTE) != NULL)) {

            printf("O cliente nao pode possuir conta corrente e salario ao mesmo tempo!\n");
            i--;
            continue;
        }

        Conta *conta = &cliente->contas[cliente->qtdContas];

        conta->tipo = tipo;
        conta->numero = banco->proximaConta;
        conta->saldo = 0;

        banco->proximaConta++;
        cliente->qtdContas++;

        printf("Conta criada: ");
        exibirNumeroConta(conta);
        printf("\n");

        registrarHistorico(banco, cliente, conta,
                           "Abertura de conta", 0);
    }

    banco->qtdClientes++;

    printf("\nCliente cadastrado com sucesso!\n");
}

/* =========================================
   DEPOSITO
   ========================================= */

void depositar(Banco *banco) {
    Cliente *cliente = selecionarCliente(banco);

    if (cliente == NULL) {
        return;
    }

    int tipo;
    float valor;

    printf("\nTipo da conta para deposito (1, 2 ou 3): ");
    scanf("%d", &tipo);

    Conta *conta = buscarConta(cliente, tipo);

    if (conta == NULL) {
        printf("Conta nao encontrada!\n");
        return;
    }

    printf("Valor do deposito: R$ ");
    scanf("%f", &valor);

    if (valor <= 0) {
        printf("O valor deve ser positivo!\n");
        return;
    }

    conta->saldo += valor;

    registrarHistorico(banco, cliente, conta, "Deposito", valor);

    printf("Deposito realizado com sucesso!\n");
    printf("Novo saldo: R$ %.2f\n", conta->saldo);
}

/* =========================================
   SAQUE
   ========================================= */

void sacar(Banco *banco) {
    Cliente *cliente = selecionarCliente(banco);

    if (cliente == NULL) {
        return;
    }

    int tipo;
    float valor;

    printf("\nTipo da conta para saque (1, 2 ou 3): ");
    scanf("%d", &tipo);

    Conta *conta = buscarConta(cliente, tipo);

    if (conta == NULL) {
        printf("Conta nao encontrada!\n");
        return;
    }

    printf("Valor do saque: R$ ");
    scanf("%f", &valor);

    if (valor <= 0) {
        printf("O valor deve ser positivo!\n");
        return;
    }

    conta->saldo -= valor;

    registrarHistorico(banco, cliente, conta, "Saque", valor);

    printf("Saque realizado com sucesso!\n");
    printf("Novo saldo: R$ %.2f\n", conta->saldo);

    verificarAlerta(cliente);
}

/* =========================================
   EMPRESTIMO
   ========================================= */

void realizarEmprestimo(Banco *banco) {
    Cliente *cliente = selecionarCliente(banco);

    if (cliente == NULL) {
        return;
    }

    Conta *corrente = buscarConta(cliente, CORRENTE);
    Conta *poupanca = buscarConta(cliente, POUPANCA);

    if (corrente == NULL || poupanca == NULL) {
        printf("O cliente precisa possuir conta corrente e poupanca!\n");
        return;
    }

    if (poupanca->saldo <= 0) {
        printf("A conta poupanca precisa possuir saldo positivo!\n");
        return;
    }

    if (cliente->possuiEmprestimo == 1) {
        printf("O cliente ja realizou um emprestimo!\n");
        return;
    }

    float saldoTotal = calcularSaldoTotal(cliente);
    float limite = 0.25 * (10 * saldoTotal);

    printf("\nLimite de emprestimo: R$ %.2f\n", limite);

    if (limite <= 0) {
        printf("O cliente nao possui limite para emprestimo!\n");
        return;
    }

    corrente->saldo += limite;
    cliente->possuiEmprestimo = 1;

    registrarHistorico(banco, cliente, corrente,
                       "Emprestimo", limite);

    printf("Emprestimo realizado com sucesso!\n");
    printf("Valor creditado: R$ %.2f\n", limite);
}

/* =========================================
   COMPRA DE TITULO DE CAPITALIZACAO
   ========================================= */

void comprarTitulo(Banco *banco) {
    Cliente *cliente = selecionarCliente(banco);

    if (cliente == NULL) {
        return;
    }

    Conta *salario = buscarConta(cliente, SALARIO);

    if (salario == NULL) {
        printf("O cliente nao possui conta salario!\n");
        return;
    }

    if (salario->saldo < 1000 || salario->saldo > 1500) {
        printf("O saldo da conta salario deve estar entre R$ 1.000,00 e R$ 1.500,00!\n");
        return;
    }

    if (cliente->possuiTitulo == 1) {
        printf("O cliente ja possui titulo de capitalizacao!\n");
        return;
    }

    cliente->possuiTitulo = 1;

    registrarHistorico(banco, cliente, salario,
                       "Compra de titulo", 0);

    printf("Titulo de capitalizacao adquirido com sucesso!\n");
}

/* =========================================
   FIBONACCI E VALIDACAO DE NUMEROS
   ========================================= */

int fibonacci(int valor) {
    if (valor == 0) {
        return 0;
    }

    if (valor == 1) {
        return 1;
    }

    return fibonacci(valor - 1) + fibonacci(valor - 2);
}

int somafibonacci(int valor) {
    int soma = 0;

    for (int i = 0; i < valor; i++) {
        soma += fibonacci(i);
    }

    return soma;
}

int ehprimo(int valor) {
    if (valor < 2) {
        return 0;
    }

    for (int i = 2; i < valor; i++) {
        if (valor % i == 0) {
            return 0;
        }
    }

    return 1;
}

/* =========================================
   SORTEIO
   ========================================= */

void efetuarSorteio(Banco *banco) {
    int indices[MAX_CLIENTES];
    int apostas[5];
    int somas[5];

    if (banco->qtdClientes < 5) {
        printf("O banco precisa ter pelo menos cinco clientes!\n");
        return;
    }

    /* Organiza os indices dos clientes por saldo total */

    for (int i = 0; i < banco->qtdClientes; i++) {
        indices[i] = i;
    }

    for (int i = 0; i < banco->qtdClientes - 1; i++) {
        for (int j = 0; j < banco->qtdClientes - 1 - i; j++) {

            float saldo1 =
                calcularSaldoTotal(&banco->clientes[indices[j]]);

            float saldo2 =
                calcularSaldoTotal(&banco->clientes[indices[j + 1]]);

            if (saldo1 < saldo2) {
                int aux = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = aux;
            }
        }
    }

    /* Verifica se os cinco primeiros possuem titulo */

    for (int i = 0; i < 5; i++) {
        Cliente *cliente = &banco->clientes[indices[i]];

        if (cliente->possuiTitulo == 0) {
            printf("Sorteio cancelado: os cinco clientes com maiores saldos precisam possuir titulo.\n");
            return;
        }
    }

    printf("\nParticipantes do sorteio:\n");

    /* Le as apostas e calcula as somas de Fibonacci */

    for (int i = 0; i < 5; i++) {
        Cliente *cliente = &banco->clientes[indices[i]];

        printf("\nParticipante: %s\n", cliente->nome);

        do {
    		printf("Informe um valor de aposta: ");
    		scanf("%d", &apostas[i]);

    		if (apostas[i] < 0) {
        		printf("O valor nao pode ser negativo!\n");
   			}

		} while (apostas[i] < 0);

		somas[i] = somafibonacci(apostas[i]);

        printf("Soma de Fibonacci: %d\n", somas[i]);
    }

    /* Procura a maior soma que seja um numero primo */

    int primeiro = -1;

    for (int i = 0; i < 5; i++) {
        if (ehprimo(somas[i])) {
            if (primeiro == -1 || somas[i] > somas[primeiro]) {
                primeiro = i;
            }
        }
    }

    if (primeiro == -1) {
        printf("\nNenhum participante obteve soma prima. Nao ha ganhador!\n");
        return;
    }

    Cliente *ganhador =
        &banco->clientes[indices[primeiro]];

    printf("\nGanhador: %s\n", ganhador->nome);
    printf("Soma de Fibonacci: %d\n", somas[primeiro]);

    /* O ganhador escolhe a conta que recebera o premio */

    int tipo;

    printf("\nContas do ganhador:\n");

    for (int i = 0; i < ganhador->qtdContas; i++) {
        printf("%d - ", ganhador->contas[i].tipo);
        exibirNumeroConta(&ganhador->contas[i]);
        printf(" | Saldo: R$ %.2f\n",
               ganhador->contas[i].saldo);
    }

    printf("Informe o tipo da conta que recebera o premio: ");
    scanf("%d", &tipo);

    Conta *contaPremiada = buscarConta(ganhador, tipo);

    if (contaPremiada == NULL) {
        printf("Conta invalida. O premio nao foi depositado.\n");
        return;
    }

    float premio = contaPremiada->saldo * 10;

    contaPremiada->saldo += premio;

    strcpy(banco->ultimoGanhador, ganhador->nome);
    banco->ultimoPremio = premio;
    banco->houveSorteio = 1;

    registrarHistorico(banco, ganhador, contaPremiada,
                       "Premio do sorteio", premio);

    printf("\nPremio depositado com sucesso!\n");
    printf("Valor do premio: R$ %.2f\n", premio);
    printf("Novo saldo da conta: R$ %.2f\n",
           contaPremiada->saldo);
}

/* =========================================
   CONSULTA AO HISTORICO
   ========================================= */

void consultarHistorico(Banco *banco) {
    float saldoTotalBanco = 0;
    int clientesEmprestimo = 0;
    int clientesNegativos = 0;
    int clientesTitulo = 0;

    printf("\n========== RELATORIO DO BANCO ==========\n");

    for (int i = 0; i < banco->qtdClientes; i++) {
        Cliente *cliente = &banco->clientes[i];

        saldoTotalBanco += calcularSaldoTotal(cliente);

        Conta *corrente = buscarConta(cliente, CORRENTE);
        Conta *poupanca = buscarConta(cliente, POUPANCA);
        Conta *salario = buscarConta(cliente, SALARIO);

        if (corrente != NULL && poupanca != NULL &&
            poupanca->saldo > 0 && cliente->possuiEmprestimo == 0) {
            clientesEmprestimo++;
        }

        if (corrente != NULL && corrente->saldo < 0) {
            clientesNegativos++;
        }

        if (salario != NULL &&
            salario->saldo >= 1000 &&
            salario->saldo <= 1500 &&
            cliente->possuiTitulo == 0) {
            clientesTitulo++;
        }
    }

    printf("Quantidade de clientes: %d\n", banco->qtdClientes);
    printf("Saldo total do banco: R$ %.2f\n", saldoTotalBanco);

    printf("\nClientes elegiveis para emprestimo: %d\n",
           clientesEmprestimo);

    printf("Clientes com conta corrente negativa: %d\n",
           clientesNegativos);

    printf("Clientes elegiveis para comprar titulo: %d\n",
           clientesTitulo);

    if (banco->houveSorteio == 1) {
        printf("\nUltimo ganhador: %s\n", banco->ultimoGanhador);
        printf("Premio: R$ %.2f\n", banco->ultimoPremio);
    } else {
        printf("\nNenhum ganhador registrado nesta execucao.\n");
    }

    /* Le os registros persistentes */

    printf("\n========== HISTORICO DE OPERACOES ==========\n");

    FILE *arquivo = fopen("historico.txt", "r");

    if (arquivo == NULL) {
        printf("Ainda nao existe historico de operacoes.\n");
        return;
    }

    char linha[300];

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        printf("%s", linha);
    }

    fclose(arquivo);
}

/* =========================================
   MENU DE OPERACOES
   ========================================= */

void menuOperacoes(Banco *banco) {
    int opcao;

    do {
        printf("\n========== OPERACOES BANCARIAS ==========\n");
        printf("1 - Cadastrar cliente\n");
        printf("2 - Depositar\n");
        printf("3 - Sacar\n");
        printf("4 - Solicitar emprestimo\n");
        printf("5 - Comprar titulo de capitalizacao\n");
        printf("0 - Voltar ao menu principal\n");

        printf("\nEscolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                cadastrarCliente(banco);
                break;

            case 2:
                depositar(banco);
                break;

            case 3:
                sacar(banco);
                break;

            case 4:
                realizarEmprestimo(banco);
                break;

            case 5:
                comprarTitulo(banco);
                break;

            case 0:
                printf("Voltando ao menu principal...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 0);
}

