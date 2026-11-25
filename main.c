#include "meu_banco.h"

int main() {
    Banco banco = {0};

    int opcao;

    /* Inicializa a numeracao das contas */
    banco.proximaConta = 1000;

    do {
        printf("\n========== BANCO DIGITAL ==========\n");
        printf("1 - Abrir banco\n");
        printf("2 - Consultar historico\n");
        printf("3 - Efetuar sorteio\n");
        printf("0 - Sair\n");

        printf("\nEscolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:

                banco.aberto = 1;

                printf("\nBanco aberto com sucesso!\n");

                /* Verifica os saldos dos clientes */
                for (int i = 0; i < banco.qtdClientes; i++) {
                    verificarAlerta(&banco.clientes[i]);
                }

                menuOperacoes(&banco);

                break;

            case 2:

                if (banco.aberto == 0) {
                    printf("\nO banco precisa estar aberto!\n");
                } else {
                    consultarHistorico(&banco);
                }

                break;

            case 3:

                if (banco.aberto == 0) {
                    printf("\nO banco precisa estar aberto!\n");
                } else {
                    efetuarSorteio(&banco);
                }

                break;

            case 0:

                printf("\nEncerrando o sistema...\n");

                break;

            default:

                printf("\nOpcao invalida!\n");
        }

    } while (opcao != 0);

    return 0;
}

