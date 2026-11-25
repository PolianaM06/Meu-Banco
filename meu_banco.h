#ifndef MEU_BANCO_H
#define MEU_BANCO_H

#include <stdio.h>
#include <string.h>

#define MAX_CLIENTES 100
#define MAX_CONTAS 3
#define MAX_HISTORICO 5

#define CORRENTE 1
#define SALARIO 2
#define POUPANCA 3

typedef struct {
    int tipo;
    int numero;
    float saldo;
} Conta;

typedef struct {
    char operacao[30];
    float valor;
} Transacao;

typedef struct {
    char nome[100];

    Conta contas[MAX_CONTAS];
    int qtdContas;

    Transacao historico[MAX_HISTORICO];
    int qtdHistorico;
    int proximaTransacao;

    int possuiTitulo;
    int possuiEmprestimo;
} Cliente;

typedef struct {
    Cliente clientes[MAX_CLIENTES];

    int qtdClientes;
    int aberto;
    int proximaConta;

    char ultimoGanhador[100];
    float ultimoPremio;
    int houveSorteio;
} Banco;

/* Funcoes de consulta */
Conta *buscarConta(Cliente *cliente, int tipo);
float calcularSaldoTotal(Cliente *cliente);
void exibirNumeroConta(Conta *conta);
void verificarAlerta(Cliente *cliente);

/* Funcoes de cadastro e operacoes */
void cadastrarCliente(Banco *banco);
void depositar(Banco *banco);
void sacar(Banco *banco);
void realizarEmprestimo(Banco *banco);
void comprarTitulo(Banco *banco);

/* Funcoes de historico */
void registrarHistorico(Banco *banco, Cliente *cliente,
                        Conta *conta, char operacao[], float valor);

void consultarHistorico(Banco *banco);

/* Funcoes do sorteio */
int fibonacci(int valor);
int somafibonacci(int valor);
int ehprimo(int valor);
void efetuarSorteio(Banco *banco);

/* Menu de operacoes */
void menuOperacoes(Banco *banco);

#endif

