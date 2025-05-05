# Trabalho_Pratico_1_SO

Como compilar e executar:

gcc banqueiro.c -o banqueiro -lpthread
./banqueiro 10 5 7

Funcionalidade:

5 clientes são criados como threads independentes.

Eles solicitam e liberam recursos de forma aleatória.

O algoritmo garante que o sistema só aceite requisições que mantenham o estado seguro.
